"""Small ANSI screen model for byte-oriented UART UI assertions."""
class Screen:
    def __init__(self, columns=80, rows=24):
        self.columns, self.rows = columns, rows
        self.lines = [[' '] * columns for _ in range(rows)]
        self.row = self.column = 0
        self.wrap = False
        self.escape = ''
        self.primary = None

    def feed(self, data):
        for byte in data:
            value = chr(byte) if isinstance(byte, int) else byte
            if self.escape:
                self.escape += value
                if self.escape == '\x1b[':
                    continue
                if len(self.escape) == 2 and value != '[':
                    self.escape = ''
                elif len(self.escape) > 2 and '@' <= value <= '~':
                    self.control(self.escape[2:-1], value)
                    self.escape = ''
                continue
            if value == '\x1b':
                self.escape = value
            elif value == '\r':
                self.column = 0; self.wrap = False
            elif value == '\n':
                self.newline(); self.column = 0; self.wrap = False
            elif value == '\b':
                self.column = max(0, self.column - 1); self.wrap = False
            elif value >= ' ':
                if self.wrap:
                    self.newline(); self.column = 0; self.wrap = False
                self.lines[self.row][self.column] = value
                if self.column == self.columns - 1:
                    self.wrap = True
                else:
                    self.column += 1

    def newline(self):
        self.row += 1
        if self.row == self.rows:
            self.lines.pop(0); self.lines.append([' '] * self.columns)
            self.row -= 1

    def control(self, parameters, final):
        if parameters.startswith('?'):
            if parameters == '?1049' and final == 'h':
                self.primary = (self.lines, self.row, self.column, self.wrap)
                self.lines = [[' '] * self.columns for _ in range(self.rows)]
                self.row = self.column = 0; self.wrap = False
            elif parameters == '?1049' and final == 'l' and self.primary:
                self.lines, self.row, self.column, self.wrap = self.primary
                self.primary = None
            return
        args = [int(v) if v else 0 for v in parameters.split(';')]
        n = args[0] or 1
        if final in 'Hf':
            self.row = min(self.rows - 1, n - 1)
            self.column = min(self.columns - 1, (args[1] if len(args) > 1 and args[1] else 1) - 1)
        elif final == 'A': self.row = max(0, self.row - n)
        elif final == 'B': self.row = min(self.rows - 1, self.row + n)
        elif final == 'C': self.column = min(self.columns - 1, self.column + n)
        elif final == 'D': self.column = max(0, self.column - n)
        elif final == 'K':
            start = 0 if args[0] in (1, 2) else self.column
            end = self.column + 1 if args[0] == 1 else self.columns
            self.lines[self.row][start:end] = [' '] * (end - start)
        elif final == 'J':
            first = 0 if args[0] == 2 else self.row
            for row in range(first, self.rows):
                column = self.column if row == first and args[0] == 0 else 0
                self.lines[row][column:] = [' '] * (self.columns - column)
        if final in 'HfABCD': self.wrap = False

    @property
    def text(self):
        return '\n'.join(''.join(row).rstrip() for row in self.lines)
