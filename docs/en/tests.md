# Hardware validation checklist for dev

This checklist covers physical behavior affected by the accepted changes. Unchecked items are planned tests, not verified results. Record the firmware commit, model (Classic / Classic Type-S / Hybrid), ANSI/JIS layout, host OS, USB topology, VIA settings and measurement equipment with each result. Repeat model-specific checks on the actual model: a Hybrid ANSI result does not validate Classic or JIS.

CI covers encoding, state-machine logic, bounds, serialization and simulated failures. Those checks are intentionally omitted here. Host logs alone do not establish wire timing, latency or power consumption.

## USB SOF scheduling — PR #10

Use the same hardware and settings for A/B builds with `HHKB_SOF_LOCK=0` and `HHKB_SOF_LOCK=1`. The default build also allows switching phase lock for the current session in **VIA → HHKB Settings → Scan Rate**; use identical firmware/settings for that A/B comparison. Keep `HHKB_SOF_MARGIN_US=150` initially; this is a starting value, not a validated margin. Use an HHKB scan-diagnostic build and enable Polling Rate Monitor in VIA.

- [ ] Capture USB SOF, keyboard IN transactions and scan timing with a USB analyzer and/or synchronized logic-analyzer trace. Establish a reproducible measurement method before interpreting `[LAT]` as end-to-end latency.
- [ ] At 1000 Hz, compare locked/unlocked scan-to-IN delay, jitter, worst case and missed polls under idle, ordinary typing, rapid repeated presses and chords. Report distributions, not just the best value.
- [ ] Measure physical key movement/contact-to-host latency using a repeatable actuator or electrical/optical trigger and a host-side capture. Keep host and trigger timestamps synchronized; distinguish scan-to-IN timing from end-to-end latency.
- [ ] Check convergence after connection, SOF interruption and host resume. Verify that the controller reacquires phase without stuck keys or bursts of scans.
- [ ] Compare direct USB, USB 2/3 hubs and a dock, and at least two host controllers/operating systems. Check that the lead margin remains useful across different host IN phases.
- [ ] Repeat with HID Console off/on, EC stream off/on, VIA traffic and media reports. Confirm that other endpoint completions do not masquerade as keyboard completions or bias `[LAT]`/phase tracking; correlate with the USB trace.
- [ ] Change VIA rates between 50/125/250/500/1000 Hz while connected. Verify actual physical scan periods and report delivery, with no lock-induced drift or starvation of USB/Nordic work.
- [ ] Test USB suspend/resume, unplug/replug, rapid reconnects, and connection with a key already held. Verify input resumes and releases reach the host.
- [ ] On Hybrid, transition USB → battery Bluetooth → USB repeatedly. Confirm SOF synchronization does not change battery scan timing, wake behavior or battery current.

## EC sampling and calibration — issues #8 and #12, PR #7

- [ ] On Classic ANSI, Classic Type-S ANSI/JIS and Hybrid ANSI/JIS, record `[SCAN]`/`[RATE]` at 1000 Hz with RT and the travel curve off/on. Confirm steady `work_max` has margin below 1 ms and deadline misses do not accumulate.
- [ ] Measure row strobe, C57 discharge and ADC acquisition timing with a logic analyzer/oscilloscope. Verify minimum discharge and settling times and check that longer discharge intervals have not changed resting readings or cross-key interference.
- [ ] Compare normal scanning, baseline capture and peak capture on the same untouched board. Check the first key after OpAmp enable (R0 C0) and the noisiest keys for systematic shifts, using EC streaming disabled for the initial comparison.
- [ ] Repeat the resting test with EC streaming enabled. Record whether logging changes scan timing or resting ADC readings. Include at least 60 seconds untouched and several minutes of typing.
- [ ] Calibrate each physical key with normal bottom-out force. Verify that every real key can be captured, particularly JIS-specific keys, without false activity on neighboring keys. Compare resting/pressed readings after Finish and after a cold power cycle.
- [ ] If a unit has a genuine resting baseline below 100 ADC counts, calibrate, save, cold-boot and verify that key retains its behavior and measurements. Synthetic low-baseline tests in CI are not a substitute for this unit.
- [ ] Start and finish calibration repeatedly from VIA while pressing/releasing keys. Confirm the OS receives no calibration typing, the matrix tester remains usable and normal input resumes without a stuck key after Finish.
- [ ] With real USB/Nordic traffic during calibration, check actual deadline misses, first-key baseline bias and host responsiveness. Determine whether a different calibration cadence or OpAmp policy is needed; the fork's half-rate schedule was not imported.

## Rapid Trigger and key actions — issue #11 and modifier/ownership fixes

- [ ] Hold a key bottomed out for at least 30 seconds, press harder, then relax finger pressure without lifting. Check RT state and ADC/travel traces with the curve off/on. The accepted cap is at calibrated bottom, not an adjustable 0.5 mm zone; document any remaining false release below peak.
- [ ] Test shallow actuation and repeated partial release/repress on several switches, including a modified dome/silencing-ring unit if available. Check physical repeatability and noise-triggered input.
- [ ] Verify actual OS shortcuts with right Ctrl/Shift/Alt/GUI, HT modifiers, One-Shot and `MK(...)`, over USB and Hybrid Bluetooth. Include AltGr in a host layout that uses it.
- [ ] Hold the same ordinary key or modifier through two physical assignments/HT/macro sources; release them in both orders. Confirm the host keeps the surviving hold and receives the final release.
- [ ] Hold a layer through two assignments, switch a persistent TG layer while held and release in both orders. Verify the effective physical layout and HHKB system indication stay correct.
- [ ] Assign QK_GESC and test both sides of Shift/GUI plus Ctrl/Alt combinations. Confirm actual host output/shortcuts, including each override, and release modifiers before GESC. Repeat over Bluetooth on Hybrid.

## Hybrid power, wake and Nordic transport

- [ ] Measure battery current at 3 V for idle, typing, Fn indication off/on, OFF, and matrix-wake sleep at 1 Hz and 10 Hz. Use a release build without an attached debugger; compare with stock on the same unit.
- [ ] Repeat sleep → OFF → wake and direct OFF → wake several times with PC5. Verify the stock-style hold/release thresholds, cancellation of an excessively long power-on hold, and successful typing after reconnection.
- [ ] With matrix wake disabled, verify the inactivity timeout reaches OFF directly. With matrix wake enabled, verify wake by a held key and the configured sleep-to-OFF timeout. Repeat with USB inserted during each state.
- [ ] Verify that USB-powered operation does not enter battery sleep automatically, and USB/BT profile changes retain a functioning keyboard after repeated transitions.
- [ ] With a controllable supply, verify battery indication thresholds and critical-battery shutdown. Confirm PC5 does not wake the critical-battery STOP path, USB wakes it, and normal operation returns with restored supply voltage.
- [ ] Verify pairing to a new host, switching all saved profiles, USB/BT switching and media-key delivery on actual hosts. Include a lost/out-of-range host and recovery without removing batteries.

## Update, recovery and persistence on physical boards

- [ ] Upgrade an already-configured keyboard through Bank 1 recovery and return to DMK. Verify custom calibration, layout and settings remain usable without reconfiguration.
- [ ] Test SW6 recovery alone and SW5+SW6 DMK wipe on Classic and Hybrid units with complete backups available. Confirm ordinary recovery preserves DMK data and the wipe does not alter factory-owned data or prevent stock operation.
- [ ] On a disposable/test setup with SWD recovery and full backups, interrupt power during settings saves and verify a usable previous/new configuration returns. Compare factory-owned EEPROM regions before/after; do not perform destructive experiments on an unbacked-up daily-use board.
- [ ] Test the local HTML updater with supported physical Classic/Hybrid variants and their correct factory containers. Verify real device reconnect/recovery behavior and successful return to both DMK and stock. Use only locally obtained proprietary firmware.

## Other affected platforms

- [ ] On a real RP2040/RP2350 keyboard, save settings across normal power cycles and an interrupted save. Confirm USB input remains responsive during writes.
- [ ] On a real Milandr RGB board, inspect WS2812 frame/reset timing and exercise USB typing during animated SSP/DMA output. Check for corrupt frames and delayed/stuck input under load.
