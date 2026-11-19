#!/usr/bin/env python3
"""Measure host-observed Bluetooth HID report intervals on macOS."""

import argparse
import statistics
import sys
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--list", action="store_true", help="list candidate HHKB Bluetooth keyboards")
    parser.add_argument("--device", type=int, help="device index from --list")
    parser.add_argument("--seconds", type=float, default=20, help="capture duration")
    args = parser.parse_args()
    if sys.platform != "darwin":
        parser.error("this capture tool is intended for macOS")
    if not 0 < args.seconds <= 3600:
        parser.error("--seconds must be between 0 and 3600")

    import hid

    devices = [
        device for device in hid.enumerate()
        if device.get("bus_type") in (None, 0, 2)  # unknown or HID_API_BUS_BLUETOOTH
        and (device.get("usage_page"), device.get("usage")) == (1, 6)
        and "hhkb" in (device.get("product_string") or "").lower()
    ]
    if args.list:
        if not devices:
            print("No HHKB found. Pair it over Bluetooth with USB disconnected.", file=sys.stderr)
        for index, device in enumerate(devices):
            transport = "Bluetooth" if device.get("bus_type") == 2 else "transport unknown"
            print(f"{index}: {device.get('product_string') or 'Unnamed keyboard'} "
                  f"({device['vendor_id']:04x}:{device['product_id']:04x}, {transport})")
        return 0 if devices else 1
    if args.device is None or not 0 <= args.device < len(devices):
        parser.error("select a Bluetooth keyboard with --device INDEX from --list")

    device = devices[args.device]
    if device.get("bus_type") != 2:
        print("Transport unreported by HIDAPI. Make sure the HHKB USB cable is disconnected.")
    timestamps = []
    handle = hid.device()
    try:
        handle.open_path(device["path"])
        print(f"Press and release keys on {device.get('product_string')} for {args.seconds:g} s.")
        deadline = time.monotonic() + args.seconds
        while time.monotonic() < deadline:
            if handle.read(1024, 100):
                timestamps.append(time.perf_counter_ns())
    except KeyboardInterrupt:
        pass
    except OSError as exc:
        print(f"HID capture failed: {exc}. Check macOS Input Monitoring permissions.", file=sys.stderr)
        return 1
    finally:
        handle.close()

    print(f"Reports received: {len(timestamps)}")
    if len(timestamps) < 2:
        print("Need at least two reports. Check Input Monitoring permissions and the selected device.")
        return 1
    intervals = [(right - left) / 1e6 for left, right in zip(timestamps, timestamps[1:])]
    p95 = sorted(intervals)[(95 * len(intervals) + 99) // 100 - 1]
    print(f"Intervals (ms): min {min(intervals):.3f}, median {statistics.median(intervals):.3f}, "
          f"p95 {p95:.3f}, max {max(intervals):.3f}")
    if sum(intervals) > 0:
        print(f"Mean observed report rate: {1000 / statistics.mean(intervals):.1f} reports/s")
    print("Includes typing cadence and host scheduling. BLE connection interval requires an HCI trace.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
