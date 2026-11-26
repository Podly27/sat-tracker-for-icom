#!/usr/bin/env python3
"""Install SAT 0.66 on an IC-9700 SD card; never installs or flashes firmware."""
import argparse
import datetime
import hashlib
import os
from pathlib import Path
import shutil
import struct
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'sd-card' / 'IC-9700' / 'Apps'
MODULES = ('SATUPD.BIN', 'SATCORE.BIN', 'SAT.APP')  # Publish entry point last.
CONFIGS = ('SAT.CFG', 'SATPOS.CFG', 'SATLIVE.CFG')
RUNTIME = {
    'ICAPRT.BIN': '72a016936680b57974d56dd20f48f772252988bf1a3812205fa90f0daaff6ac1',
    'ICAPCOLD.BIN': '33611026880a7662d21857a8f2e5b6ce497d028320152eac3411a299da45d404',
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def atomic_write(path, data):
    temp = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, prefix='SAT-', suffix='.tmp', delete=False) as f:
            temp = Path(f.name)
            f.write(data)
            f.flush()
            os.fsync(f.fileno())
        os.replace(temp, path)
        if path.read_bytes() != data:
            raise OSError('Read-back verification failed: ' + str(path))
    finally:
        if temp is not None and temp.exists():
            temp.unlink()


def valid_seed(data):
    return (len(data) == 56 and data[:4] == b'SNE1'
            and struct.unpack_from('<I', data, 4)[0] > 0
            and hashlib.sha256(data[:40]).digest()[:16] == data[40:])


def install(card):
    card = Path(card).resolve()
    if not card.is_dir():
        raise ValueError('SD card directory does not exist: ' + str(card))
    dest = card / 'IC-9700' / 'Apps'
    # This release is bound to the matching runtime. Check before any writes.
    for name, expected in RUNTIME.items():
        p = dest / name
        if not p.is_file() or digest(p.read_bytes()) != expected:
            raise ValueError('Install the matching SQ6EMM 9700E150_20261119v14 runtime first: ' + name)
    sums = {}
    for row in (ROOT / 'sd-card' / 'SHA256SUMS.txt').read_text().splitlines():
        sha, name = row.split(None, 1)
        sums[name] = sha
    payloads = {name: (SOURCE / name).read_bytes() for name in MODULES + CONFIGS}
    for name, data in payloads.items():
        if digest(data) != sums['IC-9700/Apps/' + name]:
            raise ValueError('Distribution checksum mismatch: ' + name)
    seeds = [p for p in (dest / 'SATNET.S0', dest / 'SATNET.S1') if p.exists()]
    if seeds and not any(valid_seed(p.read_bytes()) for p in seeds):
        raise ValueError('Existing TLS state is invalid. Keep a backup; do not reset it blindly.')
    backup = None
    for name in MODULES:
        p = dest / name
        if p.exists() and p.read_bytes() != payloads[name]:
            if backup is None:
                stamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S-%f')
                backup = card / 'IC-9700' / ('SAT-backup-' + stamp)
                backup.mkdir()
            shutil.copyfile(p, backup / name)
            if (backup / name).read_bytes() != p.read_bytes():
                raise OSError('Backup verification failed: ' + name)
    if not seeds:
        seed = b'SNE1' + struct.pack('<I', 1) + os.urandom(32)
        atomic_write(dest / 'SATNET.S0', seed + hashlib.sha256(seed).digest()[:16])
    for name in CONFIGS:
        if not (dest / name).exists():
            atomic_write(dest / name, payloads[name])
    atomic_write(dest / 'BSSL.TXT', (ROOT / 'licenses' / 'BearSSL-MIT.txt').read_bytes())
    for name in MODULES:
        atomic_write(dest / name, payloads[name])
    print('Installed SAT TRACKER 0.66. Firmware/runtime files were not changed.')
    if backup:
        print('Previous app files:', backup)
    print('Set your own position in', dest / 'SATPOS.CFG')
    print('Keep SATNET.S0/S1 private. Safely eject the card before removing it.')
    return dest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('card', help='SD card root directory, not its Apps directory')
    args = parser.parse_args()
    try:
        install(args.card)
    except (OSError, ValueError, KeyError) as error:
        parser.exit(1, 'Installation failed: ' + str(error) + '\n')


if __name__ == '__main__':
    main()
