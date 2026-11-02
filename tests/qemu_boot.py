"""Shared userspace bootstrap payload for ordinary guest boots."""
from pathlib import Path


def bootstrap_args(kernel, initrd=None):
    payload = Path(initrd) if initrd else Path(kernel).resolve().parent / 'user/image/_fsboot'
    if not payload.is_file():
        raise FileNotFoundError(f'missing userspace bootstrap {payload}; build user_fsboot first')
    return ['-initrd', str(payload)]
