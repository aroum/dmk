"""Execute production compact storage against byte-addressed EEPROM and power cuts."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]

class CompactStorageTest(unittest.TestCase):
    def test_ansi_and_jis_transactions(self):
        for jis in (False, True):
            with self.subTest(jis=jis), tempfile.TemporaryDirectory() as temp:
                exe = str(Path(temp) / 'storage')
                command = ['clang', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                           '-Wno-unused-function', '-Wno-unused-variable',
                           '-fsanitize=address,undefined', '-fno-omit-frame-pointer']
                if jis:
                    command += ['-DHHKB_JIS']
                for include in ['tests/hhkb_classic/mocks', 'keyboards/hhkb',
                                'platforms/stm32l072', 'keyboards/hhkb/modules/ec',
                                'keyboards/hhkb/modules/via_settings', 'dmk_core/features', 'dmk_core/include']:
                    command += ['-I', str(ROOT / include)]
                subprocess.run(command + [str(ROOT/'tests/hhkb_classic/test_compact_storage.c'), '-o', exe], check=True)
                subprocess.run([exe], check=True)
                # Private same-board backup stays outside Git; validate it locally
                # when available without making CI depend on proprietary data.
                backup = ROOT / 'keyboards/hhkb/binaries/backups/hhkb-eeprom.bin'
                if backup.exists():
                    subprocess.run([exe, str(backup)], check=True)
