"""Transfer a new ELF/manifest into the running guest and publish the manifest last."""
import functools
import http.server
import shutil
import threading
from pathlib import Path


class Server:
    def __init__(self, directory):
        class Handler(http.server.SimpleHTTPRequestHandler):
            def log_message(self, *args):
                pass
        handler = functools.partial(Handler, directory=str(directory))
        self.server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), handler)
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()
        self.url = f'http://10.0.2.2:{self.server.server_port}'

    def close(self):
        self.server.shutdown(); self.server.server_close(); self.thread.join()


def install(guest, server, kind):
    guest.command(f'curl -fsS -m 30 -o /usr/lib/fsplugins/{kind}.tmp {server.url}/{kind}.elf', timeout=45)
    guest.command(f'chmod 0755 /usr/lib/fsplugins/{kind}.tmp')
    guest.command(f'mv /usr/lib/fsplugins/{kind}.tmp /usr/lib/fsplugins/{kind}.elf')
    guest.command(f'curl -fsS -m 30 -o /etc/fsplugins/{kind}.new {server.url}/{kind}.conf', timeout=45)
    guest.command(f'mv /etc/fsplugins/{kind}.new /etc/fsplugins/{kind}.conf')
