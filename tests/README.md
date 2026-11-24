# Host regression tests

From the repository root, install Clang, Python through `uv`, and Node.js, then run:

```bash
uv --version
bash tests/run_all_tests.sh
```

For a short core-only run, including both immediate and batched USB reports:

```bash
bash tests/run_runtime_tests.sh
DMK_TEST_SANITIZERS=1 bash tests/run_runtime_tests.sh
```

The runtime runner uses a unique temporary directory and removes its binaries on exit. `CC` selects the compiler (Clang by default). Sanitizers enable AddressSanitizer and UndefinedBehaviorSanitizer; failures stop the run.

Runtime coverage includes shared 6KRO key ownership, slot reuse, HT behavior, independent momentary/persistent layers, 20,000 deterministic model-based layer operations, One-Shot retapping, repeated lazy modifier activation, tick rollover and tracker exhaustion. HHKB tests additionally exercise calibration, ANSI/JIS EEPROM persistence and interrupted writes, scan processing, power/button/radio state machines, VIA controls, and Grave Escape modifier/override combinations and release ordering.

The separate **Core and HHKB Tests** workflow runs the full suite on Linux and the sanitizer tests on Linux and macOS. It runs for relevant pushes and pull requests, and can be started from **Actions → Core and HHKB Tests → Run workflow**, selecting `dev`. Each job uploads its log, including on failure.

These are host tests with simulated peripheral inputs. They do not establish ADC settling, USB/Bluetooth latency, physical wake behavior, or current consumption. Firmware builds and hardware measurements remain separate checks.
