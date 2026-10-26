"""Small offline LSP client for documentation generation and verification."""
from pathlib import Path
import json
import queue
import subprocess
import tempfile
import threading
import time


def position(text, offset):
    prefix = text[:offset]
    return {"line": prefix.count("\n"),
            "character": len(prefix.rsplit("\n", 1)[-1].encode("utf-16-le")) // 2}


class Client:
    def __init__(self, executable, root, *, index=None, config=False, background=False, compilation_database=None):
        self.root = Path(root)
        self.messages = queue.Queue()
        self.next_id = 0
        self.errors = tempfile.TemporaryFile(mode="w+b")
        arguments = [str(executable), "--background-index=" + str(background).lower(),
                     "-j=2", "--log=error",
                     "--enable-config=" + str(config).lower(),
                     "--query-driver=" + str(Path(executable).resolve().parent / "clang++")]
        if not config:
            arguments.append("--compile-commands-dir=" + str(compilation_database or self.root / "build/dev"))
        if index:
            arguments.append("--index-file=" + str(index))
        self.process = subprocess.Popen(arguments, cwd=self.root, stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE, stderr=self.errors)
        self.reader = threading.Thread(target=self._read, daemon=True)
        self.reader.start()
        self.request("initialize", {
            "processId": None, "rootUri": self.root.as_uri(),
            "capabilities": {"textDocument": {
                "hover": {"contentFormat": ["markdown", "plaintext"]},
                "documentSymbol": {"hierarchicalDocumentSymbolSupport": True},
                "definition": {"linkSupport": True}}},
        })
        self.notify("initialized", {})

    def _read(self):
        try:
            while True:
                headers = {}
                while True:
                    line = self.process.stdout.readline()
                    if not line:
                        raise EOFError("clangd closed its LSP output")
                    if line in (b"\r\n", b"\n"):
                        break
                    key, value = line.decode().split(":", 1)
                    headers[key.lower()] = value.strip()
                remaining = int(headers["content-length"])
                chunks = []
                while remaining:
                    data = self.process.stdout.read(remaining)
                    if not data:
                        raise EOFError("incomplete LSP message")
                    chunks.append(data)
                    remaining -= len(data)
                self.messages.put(json.loads(b"".join(chunks)))
        except Exception as error:
            self.messages.put(error)

    def _send(self, message):
        data = json.dumps(message, ensure_ascii=False).encode("utf-8")
        self.process.stdin.write(f"Content-Length: {len(data)}\r\n\r\n".encode() + data)
        self.process.stdin.flush()

    def receive(self, predicate, timeout=30):
        deadline = time.monotonic() + timeout
        while True:
            try:
                message = self.messages.get(timeout=max(0.01, deadline - time.monotonic()))
            except queue.Empty as error:
                raise TimeoutError("clangd response timed out") from error
            if isinstance(message, Exception):
                raise message
            if "method" in message and "id" in message:
                self._send({"jsonrpc": "2.0", "id": message["id"], "result": None})
            if predicate(message):
                return message
            if time.monotonic() >= deadline:
                raise TimeoutError("clangd response timed out")

    def notify(self, method, params):
        self._send({"jsonrpc": "2.0", "method": method, "params": params})

    def request(self, method, params):
        self.next_id += 1
        current = self.next_id
        self._send({"jsonrpc": "2.0", "id": current, "method": method, "params": params})
        response = self.receive(lambda message: message.get("id") == current)
        if "error" in response:
            raise RuntimeError(f"{method}: {response['error']}")
        return response.get("result")

    def open(self, relative, *, strict=True):
        path = self.root / relative
        text = path.read_text()
        uri = path.as_uri()
        self.notify("textDocument/didOpen", {
            "textDocument": {"uri": uri, "languageId": "cpp", "version": 1, "text": text}})
        diagnostic = self.receive(lambda message:
            message.get("method") == "textDocument/publishDiagnostics" and
            message.get("params", {}).get("uri") == uri)
        errors = [item for item in diagnostic["params"].get("diagnostics", [])
                  if item.get("severity") == 1]
        self.last_diagnostics = errors
        if errors and strict:
            raise RuntimeError(f"{relative}: {errors}")
        return text

    def at(self, relative, text, offset):
        return {"textDocument": {"uri": (self.root / relative).as_uri()},
                "position": position(text, offset)}

    def close(self):
        try:
            if self.process.poll() is None:
                self.request("shutdown", {})
                self.notify("exit", {})
                self.process.wait(timeout=5)
        finally:
            if self.process.poll() is None:
                self.process.kill()
                self.process.wait()
            self.process.stdin.close()
            self.process.stdout.close()
            self.reader.join(timeout=2)
            self.errors.close()

    def __enter__(self):
        return self

    def __exit__(self, *exception):
        self.close()
