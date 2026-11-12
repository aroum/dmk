"""Offline tests for HHKB firmware validation; no device is written."""
import importlib.util
from pathlib import Path
import sys
import types
import unittest
from unittest.mock import patch
import tempfile

sys.modules.setdefault('hid', types.ModuleType('hid'))
spec = importlib.util.spec_from_file_location('hhkb_flasher', Path(__file__).resolve().parents[2] / 'tools/hhkb/hhkb_flasher.py')
flasher = importlib.util.module_from_spec(spec)
spec.loader.exec_module(flasher)

class FlasherValidationTest(unittest.TestCase):
    def setUp(self):
        self.good = flasher.wrap_binary_to_hfb(b'\x00\x50\x00\x20\x01\x00\x01\x08')

    def test_models(self):
        self.assertEqual(flasher.device_model('PD-KB401W'), 'AHHX01')
        self.assertEqual(flasher.device_model('PD-KB401BSC'), 'AHHS01')
        self.assertIsNone(flasher.device_model('unknown'))
        self.assertEqual(flasher.validate_container(self.good, 'AHHX01'), 'AHHX01')
        with self.assertRaises(ValueError): flasher.validate_container(self.good, 'AHHS01')
        with self.assertRaises(ValueError): flasher.validate_container(self.good, None)
        self.assertEqual(flasher.validate_container(self.good, None, check_model=False), 'AHHX01')

    def test_crc_checks_independent_of_model(self):
        bad = bytearray(self.good)
        bad[10] ^= 1
        with self.assertRaisesRegex(ValueError, 'outer CRC'):
            flasher.validate_container(bad, 'AHHX01', check_model=False)
        outer = flasher.compute_crc16_mcrf4xx(bad[2:])
        bad[:2] = outer.to_bytes(2, 'little')
        with self.assertRaisesRegex(ValueError, 'firmware CRC'):
            flasher.validate_container(bad, 'AHHX01')
        self.assertEqual(flasher.validate_container(bad, 'AHHX01', check_crc=False), 'AHHX01')
        with self.assertRaises(ValueError): flasher.validate_container(bad, 'AHHS01', check_crc=False)

    def test_short_write_is_failure(self):
        dev = flasher.HHKBDevice.__new__(flasher.HHKBDevice)
        dev.handle = types.SimpleNamespace(write=lambda _: 2)
        with self.assertRaises(IOError): dev.send_report(b'hello')

    def test_reconnect_reopens_hid_after_mode_switch(self):
        old_handle = types.SimpleNamespace(close=lambda: None)
        new_handle = types.SimpleNamespace(open_path=lambda path: None,
                                           set_nonblocking=lambda value: None)
        dev = flasher.HHKBDevice.__new__(flasher.HHKBDevice)
        dev.dev_info = {'product_id': 32, 'path': b'old'}
        dev.handle = old_handle
        with patch.object(flasher, 'find_hhkb_device', side_effect=[
            {'product_id': 32, 'path': b'old'}, {'product_id': 32, 'path': b'new'}]), \
             patch.object(flasher.hid, 'device', return_value=new_handle, create=True):
            dev.reconnect()
        self.assertIs(dev.handle, new_handle)
        self.assertEqual(dev.dev_info['path'], b'new')

    def test_firmware_transfer_matches_pfu_packet_format(self):
        image = flasher.wrap_binary_to_hfb(bytes(range(256)) * 256)
        dev = flasher.HHKBDevice.__new__(flasher.HHKBDevice)
        packets = []
        dev.send_report = packets.append
        dev.reconnect = lambda: None
        dev.read_report = lambda timeout_ms: b'\x55\x55' + packets[-1][2:3] + b'\x00\x00\x02'
        with patch.object(flasher.time, 'sleep'):
            self.assertTrue(dev.flash_firmware(image))
        self.assertEqual(packets[0][2], flasher.CMD_FIRMUP_MODE)
        self.assertEqual(packets[1][2], flasher.CMD_FIRMUP_START)
        self.assertEqual(packets[1][5:9], len(image).to_bytes(4, 'little'))
        self.assertEqual(packets[1][9:11], image[:2])
        chunks = packets[2:-1]
        self.assertEqual(b''.join(packet[7:] for packet in chunks), image[2:])
        self.assertTrue(all(len(packet) <= 64 for packet in chunks))
        self.assertTrue(all(packet[4] == len(packet) - 5 for packet in chunks))
        self.assertEqual(packets[-1][2], flasher.CMD_FIRMUP_END)

    def test_info_uses_payload_offsets(self):
        response = bytearray(64)
        response[:6] = b'\x55\x55\x02\x00\x00\x39'
        response[6:15] = b'PD-KB401W'
        response[46:50] = bytes([10, 4, 2, 9])
        response[62] = 1
        dev = flasher.HHKBDevice.__new__(flasher.HHKBDevice)
        dev.send_report = lambda _: None
        dev.read_report = lambda timeout_ms: bytes(response)
        info = dev.get_info()
        self.assertEqual(info['type_number'], 'PD-KB401W')
        self.assertEqual(info['version'], 'A4.29')
        self.assertTrue(info['bank2_broken'])

    def test_invalid_crc_is_rejected_before_update(self):
        class Keyboard:
            started = False
            def __init__(self, _): pass
            def __enter__(self): return self
            def __exit__(self, *_): pass
            def get_info(self):
                return {'type_number': 'PD-KB401W', 'version': 'B4.16',
                        'raw_version': '', 'bank2_broken': False}
            def flash_firmware(self, *_args, **_kwargs):
                Keyboard.started = True
                return True
        bad = bytearray(self.good)
        bad[4] ^= 1
        with tempfile.NamedTemporaryFile() as image:
            image.write(bad)
            image.flush()
            args = ['hhkb_flasher.py', image.name]
            with patch.object(flasher, 'find_hhkb_device', return_value={'product_id': 32}), \
                 patch.object(flasher, 'HHKBDevice', Keyboard), patch.object(sys, 'argv', args):
                with self.assertRaisesRegex(ValueError, 'CRC'): flasher.main()
            self.assertFalse(Keyboard.started)

if __name__ == '__main__': unittest.main()
