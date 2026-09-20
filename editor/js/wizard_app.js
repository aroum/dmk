        let currentPaletteCategory = 'ALL';

        // State Model
        const configState = {
            step: 1,
            kbName: "Corne Custom",
            mcu: "rp2040",
            isSplit: true,
            masterSide: "LEFT",
            matrixType: "COL2ROW",
            matrixPolarity: "NORMAL",
            debounceMs: 10,
            rowPins: ["GPIO4", "GPIO5", "GPIO6", "GPIO7"],
            colPins: ["GPIO29", "GPIO28", "GPIO27", "GPIO26", "GPIO22", "GPIO20"],
            directPins: ["GPIO9", "GPIO8", "GPIO6", "GPIO5", "GPIO10", "GPIO7", "GPIO4", "GPIO2", "GPIO1", "GPIO3"],
            activeKeys: [], // Matrix boolean [r][c]
            enableMouse: true,
            enableGamepad: true,
            enableMidi: true,
            enableLed: true,
            ledPins: ["GPIO25"],
            ledDebug: 0,
            ledCaps: "none",
            ledNum: "none",
            ledScroll: "none",
            ledActivityPin: "",
            enableRgb: true,
            rgbCount: 42,
            rgbPin: "GPIO0",
            enableEncoders: false,
            encPinsA: ["GPIO2", "GPIO4"],
            encPinsB: ["GPIO3", "GPIO5"],
            enableVial: true,
            vialProtocol: "vial", // "vial" (Protocol v9) or "via_v3" (Protocol v12)
            // Layers & Layout State
            layers: ["DEF", "LOWER", "RAISE"],
            currentLayer: "DEF",
            keymaps: {
                "DEF": [],
                "LOWER": [],
                "RAISE": []
            },
            keyGeometry: [], // Array of { w: 1, h: 1, x: 0, y: 0 } per layout key
            selectedKeyIndex: null,
            // Custom User Macros and Chords
            customMacros: [],
            customChords: []
        };

        // Dirty state tracking to prevent accidental unload / page reload
        let isPageDirty = false;

        function markDirty() {
            isPageDirty = true;
        }

        function clearDirty() {
            isPageDirty = false;
        }

        window.addEventListener('beforeunload', (e) => {
            if (isPageDirty) {
                e.preventDefault();
                e.returnValue = '';
                return '';
            }
        });

        document.addEventListener('input', (e) => {
            if (e.target && e.target.id === 'paletteSearchInput') return;
            markDirty();
        });

        document.addEventListener('change', (e) => {
            if (e.target && (e.target.id === 'btnThemeToggle' || e.target.id === 'btnLangToggle')) return;
            markDirty();
        });

        // History Stacks for Undo/Redo
        const undoStack = [];
        const redoStack = [];
        const MAX_HISTORY_STATES = 30;

        const getKeymapSnapshot = () => ({
            keymaps: structuredClone(configState.keymaps),
            keymapByCoord: structuredClone(configState.keymapByCoord || {}),
            encoderKeymaps: structuredClone(configState.encoderKeymaps || {})
        });

        const applyKeymapSnapshot = (s) => {
            if (!s) return;
            configState.keymaps = s.keymaps;
            configState.keymapByCoord = s.keymapByCoord || {};
            configState.encoderKeymaps = s.encoderKeymaps || {};
            renderVisualKeymap();
            renderLayerEncodersUI();
        };

        function saveUndoState() {
            markDirty();
            try {
                undoStack.push(getKeymapSnapshot());
                if (undoStack.length > MAX_HISTORY_STATES) undoStack.shift();
                redoStack.length = 0;
            } catch (e) {
                console.error("Failed to save undo state", e);
            }
        }

        function undoKeymap() {
            if (undoStack.length === 0) return;
            markDirty();
            try {
                redoStack.push(getKeymapSnapshot());
                applyKeymapSnapshot(undoStack.pop());
            } catch (e) {
                console.error("Failed to undo keymap", e);
            }
        }

        function redoKeymap() {
            if (redoStack.length === 0) return;
            markDirty();
            try {
                undoStack.push(getKeymapSnapshot());
                applyKeymapSnapshot(redoStack.pop());
            } catch (e) {
                console.error("Failed to redo keymap", e);
            }
        }

        function normalizePinName(pin, mcu) {
            if (!pin || typeof pin !== 'string') return '';
            let p = pin.trim().toUpperCase();
            if (!p || p === 'NONE' || p === 'NO_PIN') return p;

            const targetMcu = (mcu === 'all') ? (currentUniversalMcu || 'rp2040') : (mcu || 'rp2040');

            // For RP2040 / RP2350: allow bare numbers e.g. "19" -> "GPIO19", or "GP19" -> "GPIO19"
            if (targetMcu === 'rp2040' || targetMcu === 'rp2350') {
                if (/^([0-9]|[1-3][0-9]|4[0-7])$/.test(p)) {
                    return `GPIO${p}`;
                }
                if (/^GP([0-9]|[1-3][0-9]|4[0-7])$/.test(p)) {
                    return `GPIO${p.slice(2)}`;
                }
            }
            // For Baikal: allow bare numbers e.g. "19" -> "GPIO19" or "GP19" -> "GPIO19"
            if (targetMcu === 'baikal') {
                if (/^([0-9]|[12][0-9]|3[01])$/.test(p)) {
                    return `GPIO${p}`;
                }
                if (/^GP([0-9]|[12][0-9]|3[01])$/.test(p)) {
                    return `GPIO${p.slice(2)}`;
                }
            }
            return p;
        }

        function validatePinFormat(pin, mcu) {
            if (!pin) return true;
            const p = normalizePinName(pin, mcu);
            if (!p || p === 'NONE' || p === 'NO_PIN') return true;

            const isRp = (pinName) => /^GPIO([0-9]|[1-3][0-9]|4[0-7])$/i.test(pinName);
            const isMilandr = (pinName) => /^P[A-F]([0-9]|1[0-5])$/i.test(pinName);
            const isNrf = (pinName) => /^P(0|1)_(0[0-9]|[1-9]|[12][0-9]|3[01])$/i.test(pinName);
            const isBaikal = (pinName) => /^(GPIO([0-9]|[12][0-9]|3[01])|P[A-C]([0-9]|1[0-5]))$/i.test(pinName);

            switch (mcu) {
                case 'rp2040':
                    return isRp(p);
                case 'milandr':
                    return isMilandr(p);
                case 'nrf52840':
                    return isNrf(p);
                case 'baikal':
                    return isBaikal(p);
                case 'all':
                default:
                    return isRp(p) || isMilandr(p) || isNrf(p) || isBaikal(p);
            }
        }

        function checkPinConflicts() {
            const noticeEl = document.getElementById('pinConflictNotice');
            const detailsEl = document.getElementById('pinConflictDetails');
            const noticeEl3 = document.getElementById('pinConflictNoticeStep3');
            const detailsEl3 = document.getElementById('pinConflictDetailsStep3');

            const pinUsage = {}; // pinName -> list of usages
            const invalidPins = []; // list of { pin, label }
            const activeMcu = (configState.mcu === 'all') ? (currentUniversalMcu || 'rp2040') : (configState.mcu || 'rp2040');

            const addUsage = (pin, label) => {
                if (!pin) return;
                const clean = normalizePinName(pin, activeMcu);
                if (!clean || clean === 'NONE' || clean === 'NO_PIN') return;

                if (!validatePinFormat(clean, activeMcu)) {
                    invalidPins.push({ pin: clean, label });
                }

                if (!pinUsage[clean]) pinUsage[clean] = [];
                pinUsage[clean].push(label);
            };

            const t = I18N[currentLanguage] || I18N.ru;

            if (configState.matrixType === 'DIRECT') {
                const dPins = parsePins(document.getElementById('directPins')?.value);
                dPins.forEach((p, idx) => addUsage(p, `${t.pinUsageDirect || 'Прямая клавиша'} #${idx + 1}`));
            } else {
                const rPins = parsePins(document.getElementById('rowPins')?.value);
                const cPins = parsePins(document.getElementById('colPins')?.value);
                rPins.forEach((p, idx) => addUsage(p, `${t.pinUsageRow || 'Строка'} #${idx + 1}`));
                cPins.forEach((p, idx) => addUsage(p, `${t.pinUsageCol || 'Колонка'} #${idx + 1}`));
            }

            if (configState.isSplit) {
                const uartMode = document.getElementById('splitUartMode')?.value || 'hardware';
                if (configState.mcu === 'all') {
                    const txPin = document.getElementById('mcuSplitTxPin')?.value;
                    const rxPin = document.getElementById('mcuSplitRxPin')?.value;
                    addUsage(txPin, t.pinUsageSplitTx || 'Split TX');
                    if (rxPin && rxPin !== txPin) addUsage(rxPin, t.pinUsageSplitRx || 'Split RX');
                } else if ((activeMcu === 'milandr' || activeMcu === 'baikal') && uartMode === 'hardware') {
                    const txPin = document.getElementById('splitTxPin')?.value;
                    const rxPin = document.getElementById('splitRxPin')?.value;
                    addUsage(txPin, t.pinUsageSplitTx || 'Split TX');
                    addUsage(rxPin, t.pinUsageSplitRx || 'Split RX');
                } else {
                    const sPin = document.getElementById('serialPinRP')?.value;
                    addUsage(sPin, t.pinUsageSplitSerial || 'Split Serial');
                }
            }

            const hardwarePinErrors = [];
            if (configState.isSplit && (document.getElementById('splitUartMode')?.value === 'hardware')) {
                if (activeMcu === 'milandr') {
                    const tx = normalizePinName(document.getElementById('splitTxPin')?.value || '', 'milandr');
                    const rx = normalizePinName(document.getElementById('splitRxPin')?.value || '', 'milandr');
                    const isValidMdr = (tx === 'PF1' && rx === 'PF0') || (tx === 'PA7' && rx === 'PA6');
                    if (!isValidMdr) {
                        hardwarePinErrors.push(t.errMilandrHwUart || 'Миландр: для Hardware UART поддерживаются только пары PF1 (TX) + PF0 (RX) или PA7 (TX) + PA6 (RX).');
                    }
                } else if (activeMcu === 'baikal') {
                    const tx = normalizePinName(document.getElementById('splitTxPin')?.value || '', 'baikal');
                    const rx = normalizePinName(document.getElementById('splitRxPin')?.value || '', 'baikal');
                    const validBaikalPairs = [
                        ['PC6', 'PC7'], ['PC8', 'PC9'],
                        ['PA6', 'PA7'], ['PA2', 'PA3'], ['PA4', 'PA5'], ['PA10', 'PA11'], ['PA12', 'PA13'],
                        ['PB6', 'PB7'], ['PB2', 'PB3'], ['PB4', 'PB5'], ['PB10', 'PB11'], ['PB12', 'PB13']
                    ];
                    const isValidBaikal = validBaikalPairs.some(([vTx, vRx]) => vTx === tx && vRx === rx);
                    if (!isValidBaikal) {
                        hardwarePinErrors.push(t.errBaikalHwUart || 'Байкал BE-U1000: для Hardware UART поддерживаются пары PC6 (TX) + PC7 (RX), PC8 (TX) + PC9 (RX), PA6 (TX) + PA7 (RX)...');
                    }
                }
            }

            const enableRgb = document.getElementById('enableRgb')?.checked;
            if (enableRgb) {
                const rgbPin = document.getElementById('rgbPin')?.value;
                addUsage(rgbPin, t.pinUsageRgb || 'Подсветка RGB');
            }

            const enableEnc = document.getElementById('enableEncoders')?.checked;
            if (enableEnc) {
                const encCount = parseInt(document.getElementById('encCount')?.value, 10) || 0;
                for (let i = 0; i < encCount; i++) {
                    const pA = document.getElementById(`enc_${i}_pinA`)?.value;
                    const pB = document.getElementById(`enc_${i}_pinB`)?.value;
                    addUsage(pA, `${t.pinUsageEncoder || 'Энкодер'} #${i + 1} Pin A`);
                    addUsage(pB, `${t.pinUsageEncoder || 'Энкодер'} #${i + 1} Pin B`);
                }
            }

            const enableLed = document.getElementById('s5EnableLed')?.checked ?? configState.enableLed;
            if (enableLed) {
                if (Array.isArray(configState.ledRows) && configState.ledRows.length > 0) {
                    configState.ledRows.forEach((row, idx) => {
                        const actionName = (row.action === 'caps') ? 'Caps Lock'
                            : (row.action === 'num') ? 'Num Lock'
                            : (row.action === 'scroll') ? 'Scroll Lock'
                            : (row.action === 'debug') ? (t.ledActionDebug || 'Отладка / Heartbeat')
                            : (row.action === 'activity') ? (t.ledActionActivity || 'Активность клавиш')
                            : (t.ledActionNone || 'Индикатор');
                        if (row.pin) addUsage(row.pin, `${t.pinUsageLed || 'Индикатор LED'} #${idx + 1} (${actionName})`);
                    });
                } else {
                    const lPins = parsePins(document.getElementById('ledPins')?.value);
                    lPins.forEach((p, idx) => addUsage(p, `${t.pinUsageLed || 'Индикатор LED'} #${idx + 1}`));
                    const actPin = document.getElementById('ledActivityPin')?.value;
                    if (actPin) addUsage(actPin, t.pinUsageLedActivity || 'LED Activity');
                }
            }

            const conflicts = Object.keys(pinUsage).filter(p => pinUsage[p].length > 1);
            const hasIssues = (conflicts.length > 0) || (invalidPins.length > 0) || (hardwarePinErrors.length > 0);

            const mcuHints = {
                rp2040: 'GPIO0–GPIO29 / GPIO47',
                milandr: 'PA0–PA15 ... PF0–PF15',
                nrf52840: 'P0_00–P0_31, P1_00–P1_15',
                baikal: 'PA0–PC15 / GPIO0–GPIO31',
                all: 'GPIO... / PA... / P0_...'
            };
            const currentMcuHint = mcuHints[activeMcu] || mcuHints.rp2040;

            const errorHtmlParts = [];
            const showSubheaders = (conflicts.length > 0 && invalidPins.length > 0);

            if (conflicts.length > 0) {
                if (showSubheaders) {
                    errorHtmlParts.push(`<strong>${t.pinConflictSubheader || 'Конфликты пинов (назначены одновременно):'}</strong>`);
                }
                conflicts.forEach(p => {
                    errorHtmlParts.push(`• <strong>${p}</strong>: ${pinUsage[p].join(', ')}`);
                });
            }
            if (invalidPins.length > 0) {
                if (showSubheaders && errorHtmlParts.length > 0) errorHtmlParts.push('');
                errorHtmlParts.push(`<strong>${t.pinInvalidSubheader || 'Некорректный формат пинов для'} ${activeMcu.toUpperCase()} (${t.pinExpected || 'ожидается:'} ${currentMcuHint}):</strong>`);
                invalidPins.forEach(item => {
                    errorHtmlParts.push(`• <strong>${item.pin}</strong> (${item.label})`);
                });
            }
            if (hardwarePinErrors.length > 0) {
                if (errorHtmlParts.length > 0) errorHtmlParts.push('');
                hardwarePinErrors.forEach(err => {
                    errorHtmlParts.push(`• <strong>${err}</strong>`);
                });
            }

            const finalErrorHtml = errorHtmlParts.join('<br>');

            [[noticeEl, detailsEl], [noticeEl3, detailsEl3]].forEach(([notice, details]) => {
                if (notice && details) {
                    notice.style.display = hasIssues ? 'flex' : 'none';
                    details.innerHTML = hasIssues ? finalErrorHtml : '';
                }
            });

            return hasIssues;
        }

        const parsePins = (str) => (str || '').split(',').map(s => s.trim()).filter(Boolean);

        const MCU_DEFAULTS = {
            rp2040: {
                serial: "GPIO1",
                splitTx: "GPIO0",
                splitRx: "GPIO1",
                rgb: "GPIO0",
                ledPins: ["GPIO25"],
                ledActivity: "",
                rows: ["GPIO4", "GPIO5", "GPIO6", "GPIO7"],
                cols: ["GPIO29", "GPIO28", "GPIO27", "GPIO26", "GPIO22", "GPIO20"],
                direct: ["GPIO9", "GPIO8", "GPIO6", "GPIO5", "GPIO10", "GPIO7", "GPIO4", "GPIO2", "GPIO1", "GPIO3"],
                encA: ["GPIO2", "GPIO4"],
                encB: ["GPIO3", "GPIO5"],
                hint: "Платформа Raspberry Pi RP2040 / RP2350 (пины GPIO0–GPIO29 / GPIO47)"
            },
            milandr: {
                serial: "PA0",
                splitTx: "PF1",
                splitRx: "PF0",
                rgb: "PC0",
                ledPins: ["PA4"],
                ledActivity: "",
                rows: ["PB0", "PB1", "PB2", "PB3"],
                cols: ["PA0", "PA1", "PA2", "PA3", "PA4", "PA5"],
                direct: ["PE0", "PE1", "PE2", "PE3", "PE4", "PD0", "PD1", "PD2", "PA3", "PD4"],
                encA: ["PB2", "PB4"],
                encB: ["PB3", "PB5"],
                hint: "Миландр K1986BE92FI (порты PA, PB, PC, PD, PE, PF)"
            },
            nrf52840: {
                serial: "P0_00",
                splitTx: "P0_00",
                splitRx: "P0_01",
                rgb: "P0_13",
                ledPins: ["P0_15"],
                ledActivity: "",
                rows: ["P0_00", "P0_01", "P0_02", "P0_03"],
                cols: ["P0_04", "P0_05", "P0_06", "P0_07", "P0_08", "P0_09"],
                direct: ["P0_06", "P0_08", "P1_00", "P0_24", "P0_09", "P0_11", "P0_22", "P0_17", "P0_10", "P0_20"],
                encA: ["P0_12", "P0_14"],
                encB: ["P0_13", "P0_15"],
                hint: "Nordic nRF52840 (порты P0_00–P0_31, P1_00–P1_15)"
            },
            baikal: {
                serial: "GPIO0",
                splitTx: "PC6",
                splitRx: "PC7",
                rgb: "GPIO25",
                ledPins: ["GPIO24"],
                ledActivity: "",
                rows: ["GPIO0", "GPIO1", "GPIO2", "GPIO3"],
                cols: ["GPIO4", "GPIO5", "GPIO6", "GPIO7", "GPIO8", "GPIO9"],
                direct: ["GPIO0", "GPIO1", "GPIO2", "GPIO3", "GPIO4", "GPIO5", "GPIO6", "GPIO7", "GPIO8", "GPIO9"],
                encA: ["GPIO10", "GPIO12"],
                encB: ["GPIO11", "GPIO13"],
                hint: "Байкал BE-U1000 RISC-V (порты PA, PB, PC или GPIO)"
            },
            all: {
                serial: "GPIO1",
                splitTx: "GPIO0",
                splitRx: "GPIO1",
                rgb: "GPIO0",
                ledPins: ["GPIO25"],
                ledActivity: "",
                rows: ["GPIO4", "GPIO5", "GPIO6", "GPIO7"],
                cols: ["GPIO29", "GPIO28", "GPIO27", "GPIO26", "GPIO22", "GPIO20"],
                direct: ["GPIO9", "GPIO8", "GPIO6", "GPIO5", "GPIO10", "GPIO7", "GPIO4", "GPIO2", "GPIO1", "GPIO3"],
                encA: ["GPIO2", "GPIO4"],
                encB: ["GPIO3", "GPIO5"],
                hint: "Мультиплатформенный шаблон с директивами #if defined(MCU_...)"
            }
        };

        // Multi-MCU per-platform pin configurations for Universal mode derived from MCU_DEFAULTS
        configState.multiMcuPins = structuredClone(MCU_DEFAULTS);
        configState.rpDefaultMcu = 'rp2040';
        let currentUniversalMcu = 'rp2040';

        const MCU_AVAILABLE_PINS = {
            rp2040: [
                { pin: "GPIO0", label: "UART0 TX, I2C0 SDA, PWM0 A, PIO" },
                { pin: "GPIO1", label: "UART0 RX, I2C0 SCL, PWM0 B, PIO" },
                { pin: "GPIO2", label: "SPI0 SCK, I2C1 SDA, PWM1 A, PIO" },
                { pin: "GPIO3", label: "SPI0 TX, I2C1 SCL, PWM1 B, PIO" },
                { pin: "GPIO4", label: "UART1 TX, I2C0 SDA, PWM2 A, PIO" },
                { pin: "GPIO5", label: "UART1 RX, I2C0 SCL, PWM2 B, PIO" },
                { pin: "GPIO6", label: "SPI0 SCK, I2C1 SDA, PWM3 A, PIO" },
                { pin: "GPIO7", label: "SPI0 TX, I2C1 SCL, PWM3 B, PIO" },
                { pin: "GPIO8", label: "SPI1 RX, UART1 TX, PWM4 A, PIO" },
                { pin: "GPIO9", label: "SPI1 CSn, UART1 RX, PWM4 B, PIO" },
                { pin: "GPIO10", label: "SPI1 SCK, I2C1 SDA, PWM5 A, PIO" },
                { pin: "GPIO11", label: "SPI1 TX, I2C1 SCL, PWM5 B, PIO" },
                { pin: "GPIO12", label: "SPI1 RX, UART0 TX, PWM6 A, PIO" },
                { pin: "GPIO13", label: "SPI1 CSn, UART0 RX, PWM6 B, PIO" },
                { pin: "GPIO14", label: "SPI1 SCK, I2C1 SDA, PWM7 A, PIO" },
                { pin: "GPIO15", label: "SPI1 TX, I2C1 SCL, PWM7 B, PIO" },
                { pin: "GPIO16", label: "SPI0 RX, UART0 TX, PWM0 A, PIO" },
                { pin: "GPIO17", label: "SPI0 CSn, UART0 RX, PWM0 B, PIO" },
                { pin: "GPIO18", label: "SPI0 SCK, I2C1 SDA, PWM1 A, PIO" },
                { pin: "GPIO19", label: "SPI0 TX, I2C1 SCL, PWM1 B, PIO" },
                { pin: "GPIO20", label: "UART1 TX, I2C0 SDA, PWM2 A, PIO" },
                { pin: "GPIO21", label: "UART1 RX, I2C0 SCL, PWM2 B, PIO" },
                { pin: "GPIO22", label: "SPI0 SCK, I2C1 SDA, PWM3 A, PIO" },
                { pin: "GPIO23", label: "Power Save, PWM3 B, PIO" },
                { pin: "GPIO24", label: "VBUS Sense, PWM4 A, PIO" },
                { pin: "GPIO25", label: "Onboard LED, PWM4 B, PIO" },
                { pin: "GPIO26", label: "ADC0, I2C1 SDA, PWM5 A" },
                { pin: "GPIO27", label: "ADC1, I2C1 SCL, PWM5 B" },
                { pin: "GPIO28", label: "ADC2, UART0 TX, PWM6 A" },
                { pin: "GPIO29", label: "ADC3, UART0 RX, PWM6 B" }
            ],
            rp2350_extra: Array.from({ length: 18 }, (_, i) => {
                const idx = 30 + i;
                const isAdc = idx >= 40 && idx <= 47;
                return {
                    pin: `GPIO${idx}`,
                    label: isAdc ? `ADC${idx - 40} / QFN-80 Extended GPIO` : 'QFN-80 Extended GPIO'
                };
            }),
            milandr: (() => {
                const list = [];
                ['PA', 'PB', 'PC', 'PD', 'PE', 'PF'].forEach(port => {
                    for (let i = 0; i <= 15; i++) {
                        let desc = `Порт ${port}`;
                        if (port === 'PF' && i === 0) desc = 'UART2 RX (Связь половин)';
                        else if (port === 'PF' && i === 1) desc = 'UART2 TX (Связь половин)';
                        else if (port === 'PB') desc = 'Порт B (Строки матрицы)';
                        else if (port === 'PA') desc = 'Порт A (Колонки матрицы)';
                        else if (port === 'PC' && i === 0) desc = 'Порт C (RGB подсветка)';
                        list.push({ pin: `${port}${i}`, label: desc });
                    }
                });
                return list;
            })(),
            nrf52840: (() => {
                const list = [];
                for (let i = 0; i <= 31; i++) {
                    const pad = String(i).padStart(2, '0');
                    let desc = 'Port 0 GPIO';
                    if (i === 0 || i === 1) desc = 'XL1 / XL2 Crystal / GPIO';
                    else if (i >= 2 && i <= 5) desc = `AIN${i - 2} / Analog Input`;
                    else if (i === 8) desc = 'UART TX / GPIO';
                    else if (i === 9 || i === 10) desc = `NFC${i - 8} / GPIO`;
                    else if (i === 13) desc = 'RGB Data / GPIO';
                    else if (i === 18) desc = 'RESET / GPIO';
                    else if (i >= 28) desc = `AIN${i - 24} / Analog Input`;
                    list.push({ pin: `P0_${pad}`, label: desc });
                }
                for (let i = 0; i <= 15; i++) {
                    const pad = String(i).padStart(2, '0');
                    list.push({ pin: `P1_${pad}`, label: 'Port 1 GPIO' });
                }
                return list;
            })(),
            baikal: (() => {
                const list = [];
                for (let i = 0; i <= 31; i++) {
                    list.push({ pin: `GPIO${i}`, label: 'Baikal RISC-V GPIO' });
                }
                ['PA', 'PB', 'PC'].forEach(port => {
                    for (let i = 0; i <= 15; i++) {
                        list.push({ pin: `${port}${i}`, label: `Порт ${port}` });
                    }
                });
                return list;
            })()
        };

        function setRpDefaultMcu(target) {
            markDirty();
            configState.rpDefaultMcu = target;
            const r2040 = document.getElementById('radioRp2040');
            const r2350 = document.getElementById('radioRp2350');
            if (r2040) r2040.checked = (target === 'rp2040');
            if (r2350) r2350.checked = (target === 'rp2350');
            updateAvailablePinsDatalist();
            checkPinConflicts();
        }

        function ensureAvailablePinsDatalist() {
            let dl = document.getElementById('availablePinsList');
            if (!dl) {
                dl = document.createElement('datalist');
                dl.id = 'availablePinsList';
                document.body.appendChild(dl);
            }
            updateAvailablePinsDatalist();
        }

        function updateAvailablePinsDatalist() {
            let dl = document.getElementById('availablePinsList');
            if (!dl) return;
            dl.innerHTML = '';
            if (dl.children) dl.children.length = 0;

            let targetMcu = configState.mcu;
            if (targetMcu === 'all') {
                targetMcu = currentUniversalMcu || 'rp2040';
            }

            let pins = [];
            if (targetMcu === 'rp2040') {
                pins = (configState.rpDefaultMcu === 'rp2350')
                    ? [...MCU_AVAILABLE_PINS.rp2040, ...MCU_AVAILABLE_PINS.rp2350_extra]
                    : MCU_AVAILABLE_PINS.rp2040;
            } else if (targetMcu === 'rp2350') {
                pins = [...MCU_AVAILABLE_PINS.rp2040, ...MCU_AVAILABLE_PINS.rp2350_extra];
            } else if (MCU_AVAILABLE_PINS[targetMcu]) {
                pins = MCU_AVAILABLE_PINS[targetMcu];
            } else {
                pins = MCU_AVAILABLE_PINS.rp2040;
            }

            pins.forEach(item => {
                const opt = document.createElement('option');
                opt.value = item.pin;
                if (item.label) opt.label = `${item.pin} — ${item.label}`;
                dl.appendChild(opt);
            });
        }

        function isKnownDatalistPin(pin) {
            if (!pin || typeof pin !== 'string') return false;
            const clean = pin.trim().toUpperCase();
            if (!clean) return false;
            const dl = document.getElementById('availablePinsList');
            if (dl && dl.children) {
                for (let i = 0; i < dl.children.length; i++) {
                    const optVal = (dl.children[i].value || '').toUpperCase();
                    if (optVal === clean) return true;
                }
            }
            if (/^GPIO\d+$/i.test(clean) || /^P[A-F]\d+$/i.test(clean) || /^P[01]_\d+$/i.test(clean)) {
                return true;
            }
            return false;
        }

        function getCurrentMcuPins() {
            let targetMcu = configState.mcu;
            if (targetMcu === 'all') {
                targetMcu = currentUniversalMcu || 'rp2040';
            }

            if (targetMcu === 'rp2040') {
                return (configState.rpDefaultMcu === 'rp2350')
                    ? [...MCU_AVAILABLE_PINS.rp2040, ...MCU_AVAILABLE_PINS.rp2350_extra]
                    : MCU_AVAILABLE_PINS.rp2040;
            } else if (targetMcu === 'rp2350') {
                return [...MCU_AVAILABLE_PINS.rp2040, ...MCU_AVAILABLE_PINS.rp2350_extra];
            } else if (MCU_AVAILABLE_PINS[targetMcu]) {
                return MCU_AVAILABLE_PINS[targetMcu];
            }
            return MCU_AVAILABLE_PINS.rp2040;
        }

        function filterPinsForCurrentMcu(query) {
            const pins = getCurrentMcuPins();
            const q = (query || '').trim().toLowerCase();

            if (!q) {
                return pins;
            }

            const numOnly = q.replace(/\D/g, '');

            const scored = [];
            pins.forEach(item => {
                const pinName = item.pin.toLowerCase();
                const pinNum = item.pin.replace(/\D/g, '');
                const label = (item.label || '').toLowerCase();
                let score = 0;

                if (pinName === q) {
                    score = 100;
                } else if (numOnly && pinNum === numOnly) {
                    score = 90;
                } else if (pinName.startsWith(q)) {
                    score = 80;
                } else if (numOnly && pinNum.startsWith(numOnly)) {
                    score = 70;
                } else if (pinName.includes(q)) {
                    score = 60;
                } else if (label.includes(q)) {
                    score = 50;
                }

                if (score > 0) {
                    scored.push({ item, score });
                }
            });

            scored.sort((a, b) => b.score - a.score);
            return scored.map(s => s.item);
        }

        let activeSmartInput = null;
        let activeSmartIsMulti = false;
        let activeSmartIndex = -1;
        let smartDropdownItems = [];

        function ensureSmartPinDropdown() {
            let dd = document.getElementById('smartPinDropdown');
            if (!dd) {
                dd = document.createElement('div');
                dd.id = 'smartPinDropdown';
                document.body.appendChild(dd);

                dd.addEventListener('mousedown', (e) => {
                    e.preventDefault();
                });
            }
            return dd;
        }

        function hideSmartPinDropdown() {
            const dd = document.getElementById('smartPinDropdown');
            if (dd) {
                dd.style.display = 'none';
                dd.innerHTML = '';
            }
            activeSmartInput = null;
            activeSmartIndex = -1;
            smartDropdownItems = [];
        }

        function updateSmartDropdownActiveItem() {
            const dd = document.getElementById('smartPinDropdown');
            if (!dd) return;
            const items = dd.querySelectorAll('.pin-suggest-item');
            items.forEach((item, idx) => {
                const isActive = (idx === activeSmartIndex);
                item.classList.toggle('active', isActive);
                if (isActive) {
                    item.scrollIntoView({ block: 'nearest' });
                }
            });
        }

        function showSmartPinDropdown(inputEl, isMulti) {
            if (!inputEl) return;
            activeSmartInput = inputEl;
            activeSmartIsMulti = isMulti;

            const dd = ensureSmartPinDropdown();

            const val = inputEl.value || '';
            let token = val.trim();
            if (isMulti) {
                const selStart = (inputEl.selectionStart !== null && inputEl.selectionStart !== undefined) ? inputEl.selectionStart : val.length;
                const beforeCursor = val.slice(0, selStart);
                const lastComma = beforeCursor.lastIndexOf(',');
                const tokenStart = (lastComma === -1) ? 0 : lastComma + 1;
                const afterCursor = val.slice(selStart);
                const nextComma = afterCursor.indexOf(',');
                const tokenEnd = (nextComma === -1) ? val.length : selStart + nextComma;
                token = val.slice(tokenStart, tokenEnd).trim();
            }

            const matches = filterPinsForCurrentMcu(token);
            if (!matches || matches.length === 0) {
                hideSmartPinDropdown();
                return;
            }

            smartDropdownItems = matches.slice(0, 15);
            activeSmartIndex = 0;

            const currentUsages = {};
            const activeMcu = (configState.mcu === 'all') ? (currentUniversalMcu || 'rp2040') : (configState.mcu || 'rp2040');
            const collectUsage = (pin, label) => {
                if (!pin) return;
                const p = normalizePinName(pin, activeMcu);
                if (p) currentUsages[p] = label;
            };
            parsePins(document.getElementById('rowPins')?.value).forEach((p, i) => collectUsage(p, `Row #${i+1}`));
            parsePins(document.getElementById('colPins')?.value).forEach((p, i) => collectUsage(p, `Col #${i+1}`));
            parsePins(document.getElementById('directPins')?.value).forEach((p, i) => collectUsage(p, `Direct #${i+1}`));
            collectUsage(document.getElementById('rgbPin')?.value, 'RGB');
            collectUsage(document.getElementById('serialPinRP')?.value, 'Split Serial');

            dd.innerHTML = '';
            smartDropdownItems.forEach((item, idx) => {
                const div = document.createElement('div');
                div.className = 'pin-suggest-item' + (idx === 0 ? ' active' : '');
                
                const nameSpan = document.createElement('span');
                nameSpan.className = 'pin-suggest-name';
                nameSpan.textContent = item.pin;

                const rightBox = document.createElement('div');
                rightBox.style.display = 'flex';
                rightBox.style.alignItems = 'center';
                rightBox.style.gap = '8px';

                if (currentUsages[item.pin]) {
                    const usedSpan = document.createElement('span');
                    usedSpan.className = 'pin-suggest-used';
                    usedSpan.textContent = currentUsages[item.pin];
                    rightBox.appendChild(usedSpan);
                }

                if (item.label) {
                    const descSpan = document.createElement('span');
                    descSpan.className = 'pin-suggest-desc';
                    descSpan.textContent = item.label;
                    descSpan.title = `${item.pin} — ${item.label}`;
                    rightBox.appendChild(descSpan);
                }

                div.appendChild(nameSpan);
                div.appendChild(rightBox);

                div.addEventListener('click', () => {
                    selectSmartPinItem(item.pin);
                });

                dd.appendChild(div);
            });

            const rect = (typeof inputEl.getBoundingClientRect === 'function')
                ? inputEl.getBoundingClientRect()
                : { left: 0, bottom: 0, width: 240 };
            const scrollX = window.scrollX || window.pageXOffset || 0;
            const scrollY = window.scrollY || window.pageYOffset || 0;
            
            dd.style.left = `${rect.left + scrollX}px`;
            dd.style.top = `${rect.bottom + scrollY + 4}px`;
            dd.style.minWidth = `${Math.max(rect.width || 240, 240)}px`;
            dd.style.display = 'block';
        }

        let suppressSmartFocus = false;

        function selectSmartPinItem(pin) {
            if (!activeSmartInput) return;
            const inputEl = activeSmartInput;
            const isMulti = activeSmartIsMulti;

            if (!isMulti) {
                inputEl.value = pin;
            } else {
                const val = inputEl.value || '';
                const selStart = (inputEl.selectionStart !== null && inputEl.selectionStart !== undefined) ? inputEl.selectionStart : val.length;
                const beforeCursor = val.slice(0, selStart);
                const commasBefore = beforeCursor.split(',').length - 1;

                const parts = val.split(',').map(s => s.trim());
                if (commasBefore < parts.length) {
                    parts[commasBefore] = pin;
                } else {
                    parts.push(pin);
                }

                const cleaned = parts.filter((p, i) => p.length > 0 || i === commasBefore);
                inputEl.value = cleaned.join(', ');

                const prefixLength = cleaned.slice(0, commasBefore).join(', ').length + (commasBefore > 0 ? 2 : 0) + pin.length;
                if (typeof inputEl.setSelectionRange === 'function') {
                    try {
                        inputEl.setSelectionRange(prefixLength, prefixLength);
                    } catch (_) {}
                }
            }

            hideSmartPinDropdown();
            suppressSmartFocus = true;
            try {
                if (typeof Event !== 'undefined') {
                    inputEl.dispatchEvent(new Event('input', { bubbles: true }));
                    inputEl.dispatchEvent(new Event('change', { bubbles: true }));
                } else if (typeof inputEl.dispatchEvent === 'function') {
                    inputEl.dispatchEvent({ type: 'input' });
                    inputEl.dispatchEvent({ type: 'change' });
                }
                if (typeof saveCurrentMcuPinState === 'function') {
                    saveCurrentMcuPinState();
                }
                if (typeof inputEl.focus === 'function') {
                    inputEl.focus();
                }
            } finally {
                suppressSmartFocus = false;
            }
        }

        function attachSmartPinAutocomplete(inputEl, isMulti = false) {
            if (!inputEl) return;
            if (typeof inputEl.removeAttribute === 'function') {
                inputEl.removeAttribute('list');
            }
            if (typeof inputEl.setAttribute === 'function') {
                inputEl.setAttribute('autocomplete', 'off');
            }

            let lastVal = inputEl.value;
            inputEl.addEventListener('focus', () => {
                if (suppressSmartFocus) return;
                lastVal = inputEl.value;
                showSmartPinDropdown(inputEl, isMulti);
            });

            inputEl.addEventListener('input', (e) => {
                if (suppressSmartFocus) return;
                const cur = inputEl.value;

                // Support for headless / browser datalist replacement events
                if (isMulti && lastVal.includes(',') && !cur.includes(',')) {
                    const isReplacement = (e && e.inputType === 'insertReplacementText');
                    const isKnownPin = isKnownDatalistPin(cur.trim());
                    const isDelete = (e && e.inputType && (e.inputType.startsWith('delete') || e.inputType === 'historyUndo' || e.inputType === 'historyRedo' || e.inputType === 'deleteByCut'));

                    if (!isDelete && (isReplacement || (isKnownPin && !lastVal.includes(cur.trim())))) {
                        const trimmedLast = lastVal.trim();
                        const hadTrailingComma = trimmedLast.endsWith(',');
                        const parts = lastVal.split(',').map(s => s.trim()).filter(Boolean);
                        if (hadTrailingComma) {
                            parts.push(cur.trim());
                        } else if (parts.length > 0) {
                            parts[parts.length - 1] = cur.trim();
                        } else {
                            parts.push(cur.trim());
                        }
                        inputEl.value = parts.join(', ');
                        if (typeof saveCurrentMcuPinState === 'function') {
                            saveCurrentMcuPinState();
                        }
                    }
                }
                lastVal = inputEl.value;

                showSmartPinDropdown(inputEl, isMulti);
            });

            inputEl.addEventListener('keydown', (e) => {
                const dd = document.getElementById('smartPinDropdown');
                if (!dd || dd.style.display === 'none' || smartDropdownItems.length === 0) return;

                if (e.key === 'ArrowDown') {
                    e.preventDefault();
                    activeSmartIndex = (activeSmartIndex + 1) % smartDropdownItems.length;
                    updateSmartDropdownActiveItem();
                } else if (e.key === 'ArrowUp') {
                    e.preventDefault();
                    activeSmartIndex = (activeSmartIndex - 1 + smartDropdownItems.length) % smartDropdownItems.length;
                    updateSmartDropdownActiveItem();
                } else if (e.key === 'Enter' || e.key === 'Tab') {
                    if (activeSmartIndex >= 0 && activeSmartIndex < smartDropdownItems.length) {
                        e.preventDefault();
                        selectSmartPinItem(smartDropdownItems[activeSmartIndex].pin);
                    }
                } else if (e.key === 'Escape') {
                    hideSmartPinDropdown();
                }
            });

            inputEl.addEventListener('blur', () => {
                setTimeout(() => {
                    hideSmartPinDropdown();
                    const activeMcu = (configState.mcu === 'all') ? (currentUniversalMcu || 'rp2040') : (configState.mcu || 'rp2040');
                    if (isMulti) {
                        const parts = parsePins(inputEl.value);
                        if (parts.length > 0) {
                            const normalized = parts.map(p => normalizePinName(p, activeMcu));
                            if (normalized.join(', ') !== parts.join(', ')) {
                                inputEl.value = normalized.join(', ');
                                saveCurrentMcuPinState();
                            }
                        }
                    } else if (inputEl.value && inputEl.value.trim()) {
                        const norm = normalizePinName(inputEl.value.trim(), activeMcu);
                        if (norm !== inputEl.value.trim()) {
                            inputEl.value = norm;
                            saveCurrentMcuPinState();
                        }
                    }
                }, 200);
            });
        }

        function attachCommaPinAutocomplete(inputEl) {
            attachSmartPinAutocomplete(inputEl, true);
        }

        function saveCurrentMcuPinState() {
            if (configState.mcu === 'all' && configState.multiMcuPins && currentUniversalMcu) {
                if (!configState.multiMcuPins[currentUniversalMcu]) {
                    configState.multiMcuPins[currentUniversalMcu] = structuredClone(MCU_DEFAULTS[currentUniversalMcu] || MCU_DEFAULTS.rp2040);
                }
                const cur = configState.multiMcuPins[currentUniversalMcu];
                const rowEl = document.getElementById('rowPins');
                if (rowEl) cur.rows = parsePins(rowEl.value);
                const colEl = document.getElementById('colPins');
                if (colEl) cur.cols = parsePins(colEl.value);
                const dirEl = document.getElementById('directPins');
                if (dirEl) cur.direct = parsePins(dirEl.value);
                const splitTxEl = document.getElementById('mcuSplitTxPin');
                if (splitTxEl && splitTxEl.value.trim()) cur.splitTx = splitTxEl.value.trim();
                const splitRxEl = document.getElementById('mcuSplitRxPin');
                if (splitRxEl && splitRxEl.value.trim()) cur.splitRx = splitRxEl.value.trim();
                const rgbEl = document.getElementById('rgbPin');
                if (rgbEl && rgbEl.value.trim()) cur.rgb = rgbEl.value.trim();

                const ledPinsEl = document.getElementById('ledPins');
                if (ledPinsEl) cur.ledPins = parsePins(ledPinsEl.value);
                const ledActEl = document.getElementById('ledActivityPin');
                if (ledActEl) cur.ledActivity = ledActEl.value.trim();

                const encCount = parseInt(document.getElementById('encCount')?.value, 10) || 0;
                if (!cur.encA) cur.encA = [];
                if (!cur.encB) cur.encB = [];
                for (let i = 0; i < encCount; i++) {
                    const pA = document.getElementById(`enc_${i}_pinA`)?.value;
                    const pB = document.getElementById(`enc_${i}_pinB`)?.value;
                    if (pA) cur.encA[i] = pA.trim();
                    if (pB) cur.encB[i] = pB.trim();
                }
            }
            checkPinConflicts();
        }

        function setMcu(mcu, resetPins = false) {
            markDirty();
            const mcuChanged = (configState.mcu !== mcu);
            configState.mcu = mcu;
            ['rp2040', 'milandr', 'nrf52840', 'baikal', 'all'].forEach(k => {
                const el = document.getElementById(`opt-mcu-${k}`);
                if (el) el.classList.toggle('selected', k === mcu);
            });

            const rpContainer = document.getElementById('rpDefaultMcuContainer');
            if (rpContainer) rpContainer.style.display = (mcu === 'rp2040') ? 'flex' : 'none';

            const isAll = (mcu === 'all');
            const headerTabs = document.getElementById('headerUniversalMcuContainer');
            if (headerTabs) headerTabs.style.display = isAll ? 'flex' : 'none';

            const badgeEl = document.getElementById('activeUniversalMcuBadge');
            if (badgeEl) badgeEl.style.display = isAll ? 'flex' : 'none';

            const extrasEl = document.getElementById('universalMcuExtrasContainer');
            if (extrasEl) extrasEl.style.display = (isAll && configState.isSplit) ? 'flex' : 'none';

            if (isAll) {
                switchUniversalMcuTab(currentUniversalMcu);
            } else if (resetPins && mcuChanged) {
                const defs = MCU_DEFAULTS[mcu] || MCU_DEFAULTS.rp2040;
                document.getElementById('serialPinRP').value = defs.serial;
                const txEl = document.getElementById('splitTxPin');
                if (txEl) txEl.value = defs.splitTx || 'PF1';
                const rxEl = document.getElementById('splitRxPin');
                if (rxEl) rxEl.value = defs.splitRx || 'PF0';
                document.getElementById('rgbPin').value = defs.rgb;
                const ledPinsEl = document.getElementById('ledPins');
                if (ledPinsEl) ledPinsEl.value = (defs.ledPins || ['GPIO25']).join(', ');
                const ledActEl = document.getElementById('ledActivityPin');
                if (ledActEl) ledActEl.value = defs.ledActivity || '';
                updateLedRoleSelects();
                if (configState.matrixType === 'DIRECT') {
                    document.getElementById('directPins').value = defs.direct.join(', ');
                } else {
                    document.getElementById('rowPins').value = defs.rows.join(', ');
                    document.getElementById('colPins').value = defs.cols.join(', ');
                }
                if (document.getElementById('enableEncoders')?.checked) {
                    updateEncoderInputs();
                }
                initMatrixGrid();
            }

            updateAvailablePinsDatalist();
            updateSplitPinsUI();
            checkPinConflicts();
        }

        function switchUniversalMcuTab(mcu) {
            // 1. Save inputs of current platform before switching
            if (configState.mcu === 'all') {
                saveCurrentMcuPinState();
            }

            currentUniversalMcu = mcu;

            // 2. Synchronize active state for all platform buttons in header and across page
            document.querySelectorAll('.universal-mcu-btn').forEach(btn => {
                const btnMcu = btn.getAttribute('data-mcu');
                btn.classList.toggle('active', btnMcu === mcu);
            });

            // Update badge text on Step 2
            const badgeName = document.getElementById('activeUniversalMcuName');
            if (badgeName) {
                const names = {
                    rp2040: 'RP2040 / RP2350',
                    milandr: 'Миландр',
                    nrf52840: 'nRF52840',
                    baikal: 'Байкал'
                };
                badgeName.textContent = names[mcu] || mcu;
            }

            // 3. Load pins for target platform
            if (!configState.multiMcuPins) {
                configState.multiMcuPins = structuredClone(MCU_DEFAULTS);
            }
            if (!configState.multiMcuPins[mcu]) {
                configState.multiMcuPins[mcu] = structuredClone(MCU_DEFAULTS[mcu] || MCU_DEFAULTS.rp2040);
            }
            const d = configState.multiMcuPins[mcu];
            if (d) {
                const rowEl = document.getElementById('rowPins');
                if (rowEl) rowEl.value = (d.rows || []).join(', ');
                const colEl = document.getElementById('colPins');
                if (colEl) colEl.value = (d.cols || []).join(', ');
                const dirEl = document.getElementById('directPins');
                if (dirEl) dirEl.value = (d.direct || []).join(', ');
                const splitTxEl = document.getElementById('mcuSplitTxPin');
                if (splitTxEl) splitTxEl.value = d.splitTx || 'GPIO0';
                const splitRxEl = document.getElementById('mcuSplitRxPin');
                if (splitRxEl) splitRxEl.value = d.splitRx || 'GPIO1';
                const rgbEl = document.getElementById('rgbPin');
                if (rgbEl) rgbEl.value = d.rgb || 'GPIO0';
                const ledPinsEl = document.getElementById('ledPins');
                if (ledPinsEl) ledPinsEl.value = (d.ledPins || ['GPIO25']).join(', ');
                const ledActEl = document.getElementById('ledActivityPin');
                if (ledActEl) ledActEl.value = d.ledActivity || '';
                updateLedRoleSelects();
            }

            if (document.getElementById('enableEncoders')?.checked) {
                updateEncoderInputs();
            }

            updateAvailablePinsDatalist();
            updateSplitPinsUI();
            checkPinConflicts();
        }

        function updateSplitPinsUI() {
            const isSplit = Boolean(configState.isSplit);
            const splitPanel = document.getElementById('splitSettings');
            if (splitPanel) splitPanel.style.display = isSplit ? 'flex' : 'none';
            if (!isSplit) return;

            const activeMcu = (configState.mcu === 'all') ? currentUniversalMcu : configState.mcu;
            const mode = document.getElementById('splitUartMode')?.value || 'hardware';
            const isMilandr = (activeMcu === 'milandr');
            const isEn = (currentLanguage === 'en');

            // 1. Update option labels for splitUartMode to match exact MCU capabilities
            const optBitbang = document.querySelector('#splitUartMode option[value="bitbang"]');
            const optHardware = document.querySelector('#splitUartMode option[value="hardware"]');

            if (optBitbang && optHardware) {
                if (configState.mcu === 'all') {
                    optBitbang.textContent = isEn ? 'Bit-Bang UART (software)' : 'Bit-Bang UART (программный)';
                    optHardware.textContent = isEn ? 'Hardware / PIO UART (hardware)' : 'Hardware / PIO UART (аппаратный)';
                } else if (isMilandr) {
                    optBitbang.textContent = isEn ? 'Bit-Bang UART (software)' : 'Bit-Bang UART (программный)';
                    optHardware.textContent = isEn ? 'Hardware UART (hardware TX + RX)' : 'Hardware UART (аппаратный TX + RX)';
                } else if (activeMcu === 'rp2040') {
                    optBitbang.textContent = isEn ? 'Bit-Bang UART (software)' : 'Bit-Bang UART (программный)';
                    optHardware.textContent = isEn ? 'Hardware PIO UART (hardware)' : 'Hardware PIO UART (аппаратный)';
                } else if (activeMcu === 'nrf52840') {
                    optBitbang.textContent = isEn ? 'Bit-Bang UART (software)' : 'Bit-Bang UART (программный)';
                    optHardware.textContent = isEn ? 'Hardware UARTE (hardware)' : 'Hardware UARTE (аппаратный)';
                } else if (activeMcu === 'baikal') {
                    optBitbang.textContent = isEn ? 'Bit-Bang UART (software)' : 'Bit-Bang UART (программный)';
                    optHardware.textContent = isEn ? 'Hardware UART (hardware)' : 'Hardware UART (аппаратный)';
                }
            }

            // 2. Step 1 inputs: single pin vs dual TX+RX pins
            const singleGroup = document.getElementById('splitSinglePinGroup');
            const dualGroup = document.getElementById('splitDualPinsGroup');
            const isDualHw = (isMilandr || activeMcu === 'baikal') && (mode === 'hardware');
            if (singleGroup && dualGroup) {
                if (isDualHw) {
                    singleGroup.style.display = 'none';
                    dualGroup.style.display = 'grid';
                    const txEl = document.getElementById('splitTxPin');
                    const rxEl = document.getElementById('splitRxPin');
                    if (isMilandr) {
                        if (txEl && (!txEl.value || txEl.value.startsWith('GPIO') || txEl.value.startsWith('PC'))) txEl.value = 'PF1';
                        if (rxEl && (!rxEl.value || rxEl.value.startsWith('GPIO') || rxEl.value.startsWith('PC'))) rxEl.value = 'PF0';
                    } else if (activeMcu === 'baikal') {
                        if (txEl && (!txEl.value || txEl.value.startsWith('GPIO') || txEl.value.startsWith('PF'))) txEl.value = 'PC6';
                        if (rxEl && (!rxEl.value || rxEl.value.startsWith('GPIO') || rxEl.value.startsWith('PF'))) rxEl.value = 'PC7';
                    }
                } else {
                    singleGroup.style.display = 'block';
                    dualGroup.style.display = 'none';
                }
            }

            // Labels for single pin on Step 1
            const lblSerial = document.getElementById('lblSerialPin');
            const serialInput = document.getElementById('serialPinRP');
            if (lblSerial && serialInput) {
                lblSerial.textContent = isEn ? 'Communication Pin:' : 'Пин связи:';
                if (isMilandr) {
                    if (!serialInput.value || serialInput.value.startsWith('GPIO') || serialInput.value.startsWith('PC')) serialInput.value = 'PF0';
                } else if (activeMcu === 'rp2040') {
                    if (!serialInput.value || serialInput.value.startsWith('P')) serialInput.value = 'GPIO1';
                } else if (activeMcu === 'nrf52840') {
                    if (!serialInput.value || serialInput.value.startsWith('GPIO') || serialInput.value.startsWith('PF') || serialInput.value.startsWith('PC')) serialInput.value = 'P0_00';
                } else if (activeMcu === 'baikal') {
                    if (!serialInput.value || serialInput.value.startsWith('GPIO') || serialInput.value.startsWith('PF')) serialInput.value = 'PC6';
                }
            }

            // 3. Step 2 Universal Multi-MCU Split section
            const multiExtras = document.getElementById('universalMcuExtrasContainer');
            if (multiExtras && configState.mcu === 'all') {
                multiExtras.style.display = 'flex';
                const lblMultiTx = document.getElementById('lblMultiSplitTx');
                const lblMultiRx = document.getElementById('lblMultiSplitRx');
                const hintMulti = document.getElementById('hintMultiSplit');
                const rxGroup = document.getElementById('multiSplitRxGroup');
                if (lblMultiTx) lblMultiTx.textContent = isEn ? 'TX Pin:' : 'Пин TX:';
                if (lblMultiRx) lblMultiRx.textContent = isEn ? 'RX Pin:' : 'Пин RX:';
                const mcuHints = {
                    milandr: { rx: 'block', hint: isEn ? 'Milandr hardware UART requires 2 pins: PF1 (TX) and PF0 (RX).' : 'На Миландере аппаратный UART требует 2 раздельных пина: PF1 (TX) и PF0 (RX).' },
                    rp2040: { rx: 'none', hint: isEn ? 'RP2040 uses a single GPIO pin for PIO half-duplex.' : 'RP2040 использует 1 провод через аппаратный полудуплексный PIO.' },
                    nrf52840: { rx: 'block', hint: isEn ? 'nRF52840 UARTE pin configuration.' : 'Настройки пинов UARTE для nRF52840.' },
                    baikal: { rx: 'block', hint: isEn ? 'Baikal UART pin configuration.' : 'Настройки пинов UART для Байкал.' }
                };
                const info = mcuHints[currentUniversalMcu] || mcuHints.rp2040;
                if (hintMulti) hintMulti.textContent = info.hint;
                if (rxGroup) rxGroup.style.display = info.rx;
            } else if (multiExtras) {
                multiExtras.style.display = 'none';
            }
        }

        function sanitizeKeyboardName(name) {
            if (!name) return '';
            // Replace spaces, %, and other invalid path/C-identifier characters with underscores
            return name
                .replace(/[\s%\\/:*?"<>|#&{}'!`+=;@^~,.]+/g, '_')
                .replace(/_+/g, '_')
                .replace(/^_/, '');
        }

        function init() {
            // Explicitly sync checkboxes with initial state
            [
                ['enableEncoders', configState.enableEncoders],
                ['enableRgb', configState.enableRgb],
                ['enableChords', false],
                ['enableCustomMacros', false],
                ['enableLayerHotkeys', false]
            ].forEach(([id, val]) => {
                const el = document.getElementById(id);
                if (el) el.checked = Boolean(val);
            });

            applyLanguage();
            ['mouse', 'gamepad', 'midi', 'led'].forEach(f => {
                const key = 'enable' + f.charAt(0).toUpperCase() + f.slice(1);
                toggleFeatureSubsystem(f, configState[key] !== false);
            });
            initMatrixGrid();
            toggleEncoderOptions();
            toggleRgbOptions();
            toggleLedOptions();
            toggleChordsOptions();
            toggleCustomMacrosOptions();
            toggleLayerHotkeyOptions();
            ensureAvailablePinsDatalist();
            ['rowPins', 'colPins', 'directPins'].forEach(id => {
                attachSmartPinAutocomplete(document.getElementById(id), true);
            });
            ['serialPinRP', 'splitTxPin', 'splitRxPin', 'mcuSplitTxPin', 'mcuSplitRxPin', 'rgbPin', 'ledPins', 'ledActivityPin'].forEach(id => {
                attachSmartPinAutocomplete(document.getElementById(id), false);
            });
            updateSplitPinsUI();

            const kbNameInput = document.getElementById('kbName');
            const vialNameInput = document.getElementById('vialName');
            if (kbNameInput && vialNameInput) {
                let vialManuallyEdited = false;
                vialNameInput.addEventListener('input', () => {
                    vialManuallyEdited = (vialNameInput.value !== kbNameInput.value);
                });
                kbNameInput.addEventListener('input', () => {
                    const sanitized = sanitizeKeyboardName(kbNameInput.value);
                    if (kbNameInput.value !== sanitized) {
                        const start = kbNameInput.selectionStart;
                        const end = kbNameInput.selectionEnd;
                        kbNameInput.value = sanitized;
                        if (typeof kbNameInput.setSelectionRange === 'function' && start !== null && end !== null) {
                            kbNameInput.setSelectionRange(start, end);
                        }
                    }
                    if (!vialManuallyEdited) {
                        vialNameInput.value = kbNameInput.value;
                    }
                });
                if (!vialNameInput.value || vialNameInput.value === 'DMK Custom') {
                    vialNameInput.value = kbNameInput.value;
                }
            }

            gotoStep(1);
            isPageDirty = false;
        }

        function setSplitMode(isSplit) {
            markDirty();
            configState.isSplit = isSplit;
            document.getElementById('opt-single').classList.toggle('selected', !isSplit);
            document.getElementById('opt-split').classList.toggle('selected', isSplit);
            updateSplitPinsUI();
            initMatrixGrid();
        }

        function setMatrixType(type) {
            markDirty();
            configState.matrixType = type;
            document.getElementById('opt-col2row').classList.toggle('selected', type === 'COL2ROW');
            document.getElementById('opt-row2col').classList.toggle('selected', type === 'ROW2COL');
            document.getElementById('opt-direct').classList.toggle('selected', type === 'DIRECT');

            const isDirect = (type === 'DIRECT');
            document.getElementById('gridMatrixPins').style.display = isDirect ? 'none' : 'flex';
            document.getElementById('directMatrixPins').style.display = isDirect ? 'flex' : 'none';
            initMatrixGrid();
        }

        function setProtocolMode(mode) {
            markDirty();
            const proto = (mode === 'via_v3') ? 'via_v3' : 'vial';
            configState.vialProtocol = proto;
            configState.protocolMode = proto;

            const isVia = (proto === 'via_v3');
            const btnVial = document.getElementById('btnProtoVial');
            const btnVia = document.getElementById('btnProtoVia');
            if (btnVial) btnVial.classList.toggle('active', !isVia);
            if (btnVia) btnVia.classList.toggle('active', isVia);

            const selectEl = document.getElementById('vialProtocolMode');
            if (selectEl) selectEl.value = proto;

            updateJsonExportLabels();
        }

        function updateJsonExportLabels() {
            const isVia = (configState.vialProtocol === 'via_v3' || configState.protocolMode === 'via_v3');
            const jsonFileName = isVia ? 'via.json' : 'vial.json';

            const titleEl = document.getElementById('jsonCodeTitle');
            if (titleEl) titleEl.textContent = jsonFileName;

            const btnCopy = document.getElementById('btnCopyJson');
            if (btnCopy) {
                btnCopy.textContent = (currentLanguage === 'en' ? 'Copy ' : 'Копировать ') + jsonFileName;
            }

            const btnDownload = document.getElementById('btnDownloadJson');
            if (btnDownload) {
                btnDownload.textContent = (currentLanguage === 'en' ? 'Download ' : 'Скачать ') + jsonFileName;
            }

            const btnGen = document.getElementById('btnGenerateFiles');
            if (btnGen) {
                btnGen.innerHTML = (currentLanguage === 'en' ? `Generate config.h & ${jsonFileName} &rarr;` : `Сгенерировать config.h & ${jsonFileName} &rarr;`);
            }
        }

        function applyPreset(key) {
            markDirty();
            const p = PRESETS[key];
            if (!p) return;

            document.getElementById('kbName').value = p.name;
            const vialNameEl = document.getElementById('vialName');
            if (vialNameEl) vialNameEl.value = p.name;
            setSplitMode(p.isSplit);
            setMatrixType(p.matrixType);

            if (p.rowPins) document.getElementById('rowPins').value = p.rowPins.join(', ');
            if (p.colPins) document.getElementById('colPins').value = p.colPins.join(', ');
            if (p.directPins) document.getElementById('directPins').value = p.directPins.join(', ');

            configState.enableRgb = p.enableRgb;
            document.getElementById('enableRgb').checked = p.enableRgb;
            document.getElementById('rgbCount').value = p.rgbCount;
            if (p.rgbPin) document.getElementById('rgbPin').value = p.rgbPin;
            toggleRgbOptions();

            if (p.enableEncoders) {
                document.getElementById('enableEncoders').checked = true;
                configState.enableEncoders = true;
                document.getElementById('encPinsA').value = p.encPinsA.join(', ');
                document.getElementById('encPinsB').value = p.encPinsB.join(', ');
            } else {
                document.getElementById('enableEncoders').checked = false;
                configState.enableEncoders = false;
            }
            toggleEncoderOptions();

            // Load grid pattern
            initMatrixGrid(p.pattern);
        }

        const DEFAULT_ENC_ACTS = [
            { cw: "K_VOLU", ccw: "K_VOLD" },
            { cw: "K_MNXT", ccw: "K_MPRV" },
            { cw: "K_PGUP", ccw: "K_PGDN" },
            { cw: "K_RIGHT", ccw: "K_LEFT" }
        ];

        function toggleFeatureSubsystem(feat, enabled) {
            const cap = feat.toUpperCase();
            const camelKey = `enable${feat[0].toUpperCase()}${feat.slice(1)}`;
            configState[camelKey] = enabled;
            const el = document.getElementById(`s5Enable${feat[0].toUpperCase()}${feat.slice(1)}`);
            if (el) el.checked = enabled;
            const tab = document.getElementById(`tab-pal-${cap}`);
            if (tab) tab.style.display = enabled ? '' : 'none';
            if (feat === 'led') {
                toggleLedOptions();
            } else if ((!enabled && currentPaletteCategory === cap) || currentPaletteCategory === 'ALL') {
                renderPalette('ALL');
            }
        }

        function toggleLedOptions() {
            const en = document.getElementById('s5EnableLed')?.checked ?? configState.enableLed;
            const block = document.getElementById('ledOptionsBlock');
            if (block) block.style.display = en ? 'flex' : 'none';
            if (en) {
                if (!configState.ledRows || !Array.isArray(configState.ledRows) || configState.ledRows.length === 0) {
                    configState.ledRows = [{ pin: '', action: 'none' }];
                }
                renderLedRows();
            }
            checkPinConflicts();
        }

        function ensureLedRowsState() {
            if (!configState.ledRows || !Array.isArray(configState.ledRows)) {
                configState.ledRows = [];
                const legacyPins = configState.ledPins;
                if (legacyPins && legacyPins.length > 0) {
                    legacyPins.forEach((p, idx) => {
                        let act = 'none';
                        if (configState.ledDebug === idx || (configState.ledDebug === undefined && idx === 0)) act = 'debug';
                        else if (configState.ledCaps === idx) act = 'caps';
                        else if (configState.ledNum === idx) act = 'num';
                        else if (configState.ledScroll === idx) act = 'scroll';
                        configState.ledRows.push({ pin: p, action: act });
                    });
                    if (configState.ledActivityPin) {
                        configState.ledRows.push({ pin: configState.ledActivityPin, action: 'activity' });
                    }
                }
            }
            if (configState.ledRows.length === 0) {
                configState.ledRows.push({ pin: '', action: 'none' });
            }
        }

        function renderLedRows() {
            ensureLedRowsState();
            const list = document.getElementById('ledRowsList');
            if (!list) return;
            const t = I18N[currentLanguage] || I18N.ru;

            const actions = [
                { value: 'debug', label: t.ledActionDebug || 'Отладка / Heartbeat' },
                { value: 'caps', label: t.ledActionCaps || 'Caps Lock' },
                { value: 'num', label: t.ledActionNum || 'Num Lock' },
                { value: 'scroll', label: t.ledActionScroll || 'Scroll Lock' },
                { value: 'activity', label: t.ledActionActivity || 'Активность нажатий' },
                { value: 'none', label: t.ledActionNone || 'Без действия' }
            ];

            let html = `
                <div style="display: grid; grid-template-columns: 1fr 1fr 36px; gap: 8px; font-size: 0.8rem; font-weight: 600; color: var(--text-muted); padding-bottom: 2px;">
                    <div>${t.ledTablePinCol || 'Пин'}</div>
                    <div>${t.ledTableActionCol || 'Действие'}</div>
                    <div></div>
                </div>
            `;

            configState.ledRows.forEach((row, idx) => {
                const optHtml = actions.map(a => `<option value="${a.value}" ${row.action === a.value ? 'selected' : ''}>${a.label}</option>`).join('');
                html += `
                    <div style="display: grid; grid-template-columns: 1fr 1fr 36px; gap: 8px; align-items: center;" data-led-idx="${idx}">
                        <input type="text" class="led-row-pin" autocomplete="off" value="${row.pin || ''}" placeholder="GPIO25" oninput="updateLedRow(${idx}, 'pin', this.value)" style="margin: 0;">
                        <select class="led-row-action" onchange="updateLedRow(${idx}, 'action', this.value)" style="margin: 0;">
                            ${optHtml}
                        </select>
                        <button type="button" class="btn btn-sm" onclick="removeLedRow(${idx})" title="Удалить индикатор" style="padding: 6px; display: inline-flex; align-items: center; justify-content: center; color: var(--danger, #ef4444);">✕</button>
                    </div>
                `;
            });

            list.innerHTML = html;
            list.querySelectorAll('.led-row-pin').forEach(input => {
                attachSmartPinAutocomplete(input, false);
            });
            syncLedStateFromRows();
        }

        function addLedRow() {
            markDirty();
            ensureLedRowsState();
            configState.ledRows.push({ pin: '', action: 'none' });
            renderLedRows();
            checkPinConflicts();
        }

        function removeLedRow(idx) {
            markDirty();
            ensureLedRowsState();
            configState.ledRows.splice(idx, 1);
            if (configState.ledRows.length === 0) {
                configState.ledRows.push({ pin: '', action: 'none' });
            }
            renderLedRows();
            checkPinConflicts();
        }

        function updateLedRow(idx, field, val) {
            markDirty();
            ensureLedRowsState();
            if (configState.ledRows[idx]) {
                configState.ledRows[idx][field] = (field === 'pin') ? val.trim() : val;
            }
            syncLedStateFromRows();
            checkPinConflicts();
        }

        function syncLedStateFromRows() {
            ensureLedRowsState();
            const pins = [];
            let debugIdx = 'none';
            let capsIdx = 'none';
            let numIdx = 'none';
            let scrollIdx = 'none';
            let actPin = '';

            configState.ledRows.forEach(row => {
                const p = (row.pin || '').trim();
                if (!p) return;
                if (row.action === 'activity') {
                    actPin = p;
                    return;
                }
                let idx = pins.indexOf(p);
                if (idx === -1) {
                    pins.push(p);
                    idx = pins.length - 1;
                }
                if (row.action === 'debug') debugIdx = idx;
                else if (row.action === 'caps') capsIdx = idx;
                else if (row.action === 'num') numIdx = idx;
                else if (row.action === 'scroll') scrollIdx = idx;
            });

            configState.ledPins = pins;
            configState.ledDebug = debugIdx;
            configState.ledCaps = capsIdx;
            configState.ledNum = numIdx;
            configState.ledScroll = scrollIdx;
            configState.ledActivityPin = actPin;

            // Sync hidden/legacy DOM inputs if present to avoid breaking backward compatibility
            const ledPinsEl = document.getElementById('ledPins');
            if (ledPinsEl) ledPinsEl.value = pins.join(', ');
            const ledActEl = document.getElementById('ledActivityPin');
            if (ledActEl) ledActEl.value = actPin;
            const ledDebugEl = document.getElementById('ledDebugPin');
            if (ledDebugEl) ledDebugEl.value = String(debugIdx);
            const ledCapsEl = document.getElementById('ledCapsPin');
            if (ledCapsEl) ledCapsEl.value = String(capsIdx);
            const ledNumEl = document.getElementById('ledNumPin');
            if (ledNumEl) ledNumEl.value = String(numIdx);
            const ledScrollEl = document.getElementById('ledScrollPin');
            if (ledScrollEl) ledScrollEl.value = String(scrollIdx);

            if (configState.mcu === 'all') {
                saveCurrentMcuPinState();
            }
        }

        // Backward compatibility helpers for legacy tests and external integrations
        function updateLedRoleSelects() {
            // no-op or sync if legacy inputs exist
        }

        function onLedPinsInput() {
            const el = document.getElementById('ledPins');
            if (el && el.value) {
                const pins = parsePins(el.value);
                configState.ledRows = pins.map((p, i) => ({
                    pin: p,
                    action: (i === 0 ? 'debug' : 'none')
                }));
                syncLedStateFromRows();
                renderLedRows();
            }
        }

        function onLedRoleChange() {
            const debugVal = document.getElementById('ledDebugPin')?.value;
            const capsVal = document.getElementById('ledCapsPin')?.value;
            const numVal = document.getElementById('ledNumPin')?.value;
            const scrollVal = document.getElementById('ledScrollPin')?.value;
            const actVal = document.getElementById('ledActivityPin')?.value;

            if (Array.isArray(configState.ledRows)) {
                configState.ledRows.forEach((row, idx) => {
                    if (debugVal !== undefined && debugVal === String(idx)) row.action = 'debug';
                    else if (capsVal !== undefined && capsVal === String(idx)) row.action = 'caps';
                    else if (numVal !== undefined && numVal === String(idx)) row.action = 'num';
                    else if (scrollVal !== undefined && scrollVal === String(idx)) row.action = 'scroll';
                    else if (row.action !== 'activity') row.action = 'none';
                });
                if (actVal) {
                    const actRow = configState.ledRows.find(r => r.action === 'activity');
                    if (actRow) actRow.pin = actVal;
                    else configState.ledRows.push({ pin: actVal, action: 'activity' });
                }
                syncLedStateFromRows();
                renderLedRows();
            }
        }

        function toggleRgbOptions() {
            const en = document.getElementById('enableRgb').checked;
            document.getElementById('rgbOptionsBlock').style.display = en ? 'flex' : 'none';
        }

        function toggleEncoderOptions() {
            const en = document.getElementById('enableEncoders').checked;
            document.getElementById('encoderOptionsBlock').style.display = en ? 'flex' : 'none';
            if (en) updateEncoderInputs();
        }

        function updateEncoderInputs() {
            const count = parseInt(document.getElementById('encCount').value, 10) || 1;
            const container = document.getElementById('encoderPinList');
            const t = I18N[currentLanguage];

            // Save existing pin values to preserve across language switches if not in multi-MCU mode
            const existingVals = [];
            for (let i = 0; i < 4; i++) {
                const a = document.getElementById(`enc_${i}_pinA`)?.value;
                const b = document.getElementById(`enc_${i}_pinB`)?.value;
                if (a || b) existingVals[i] = { a, b };
            }

            container.innerHTML = '';
            const isAll = (configState.mcu === 'all');
            const multiPins = (isAll && configState.multiMcuPins && configState.multiMcuPins[currentUniversalMcu])
                ? configState.multiMcuPins[currentUniversalMcu]
                : null;
            const defs = isAll
                ? (multiPins || MCU_DEFAULTS[currentUniversalMcu] || MCU_DEFAULTS.rp2040)
                : (MCU_DEFAULTS[configState.mcu] || MCU_DEFAULTS.rp2040);

            for (let i = 0; i < count; i++) {
                const row = document.createElement('div');
                row.style.background = 'var(--panel)';
                row.style.border = '1px solid var(--border)';
                row.style.borderRadius = '6px';
                row.style.padding = '12px';
                row.style.display = 'grid';
                row.style.gridTemplateColumns = '1fr 1fr';
                row.style.gap = '12px';

                let pinAVal = isAll ? (multiPins?.encA?.[i] || defs.encA?.[i]) : (existingVals[i]?.a || defs.encA?.[i]);
                if (!pinAVal) {
                    pinAVal = `GPIO${2 + i*2}`;
                }
                let pinBVal = isAll ? (multiPins?.encB?.[i] || defs.encB?.[i]) : (existingVals[i]?.b || defs.encB?.[i]);
                if (!pinBVal) {
                    pinBVal = `GPIO${3 + i*2}`;
                }

                row.innerHTML = `
                    <div class="form-group" style="margin: 0;">
                        <label>${t.encPinA || 'Encoder'} #${i+1} — ${t.encPinALabel || 'Pin A'}:</label>
                        <input type="text" id="enc_${i}_pinA" autocomplete="off" value="${pinAVal}" oninput="saveCurrentMcuPinState()">
                    </div>
                    <div class="form-group" style="margin: 0;">
                        <label>${t.encPinB || 'Encoder'} #${i+1} — ${t.encPinBLabel || 'Pin B'}:</label>
                        <input type="text" id="enc_${i}_pinB" autocomplete="off" value="${pinBVal}" oninput="saveCurrentMcuPinState()">
                    </div>
                `;
                container.appendChild(row);
                attachSmartPinAutocomplete(document.getElementById(`enc_${i}_pinA`), false);
                attachSmartPinAutocomplete(document.getElementById(`enc_${i}_pinB`), false);
            }
            checkPinConflicts();
            renderLayerEncodersUI();
        }

        function renderLayerEncodersUI() {
            const container = document.getElementById('layerEncodersContainer');
            const listEl = document.getElementById('layerEncodersList');
            if (!container || !listEl) return;

            const isEnabled = document.getElementById('enableEncoders')?.checked;
            if (!isEnabled) {
                container.style.display = 'none';
                return;
            }

            container.style.display = 'flex';
            listEl.innerHTML = '';
            const t = I18N[currentLanguage];
            const count = parseInt(document.getElementById('encCount')?.value, 10) || 1;
            const curLayer = configState.currentLayer || 'DEF';

            if (!configState.encoderKeymaps) configState.encoderKeymaps = {};
            if (!configState.encoderKeymaps[curLayer]) {
                configState.encoderKeymaps[curLayer] = [];
                for (let i = 0; i < 4; i++) {
                    configState.encoderKeymaps[curLayer].push(curLayer === 'DEF' ? (DEFAULT_ENC_ACTS[i] || DEFAULT_ENC_ACTS[0]) : { cw: "K_TRNS", ccw: "K_TRNS" });
                }
            }

            for (let i = 0; i < count; i++) {
                const row = document.createElement('div');
                row.style.background = 'var(--panel)';
                row.style.border = '1px solid var(--border)';
                row.style.borderRadius = '6px';
                row.style.padding = '10px 14px';
                row.style.display = 'grid';
                row.style.gridTemplateColumns = '140px 1fr 1fr';
                row.style.gap = '12px';
                row.style.alignItems = 'center';

                const curAct = configState.encoderKeymaps[curLayer][i] || { cw: "K_VOLU", ccw: "K_VOLD" };

                row.innerHTML = `
                    <div style="font-weight: 600; font-size: 0.85rem;">${t.encPinA || 'Энкодер'} #${i+1}:</div>
                    <div class="form-group" style="margin: 0;">
                        <label style="font-size: 0.72rem; color: var(--text-muted);">${t.encCcw || 'Вращение влево (CCW):'}</label>
                        <input type="text" id="layer_enc_${i}_ccw" value="${curAct.ccw}" oninput="updateLayerEncoderAction(${i}, 'ccw', this.value)">
                    </div>
                    <div class="form-group" style="margin: 0;">
                        <label style="font-size: 0.72rem; color: var(--text-muted);">${t.encCw || 'Вращение вправо (CW):'}</label>
                        <input type="text" id="layer_enc_${i}_cw" value="${curAct.cw}" oninput="updateLayerEncoderAction(${i}, 'cw', this.value)">
                    </div>
                `;
                listEl.appendChild(row);
            }
        }

        function updateLayerEncoderAction(encIdx, dir, val) {
            const curLayer = configState.currentLayer || 'DEF';
            if (!configState.encoderKeymaps) configState.encoderKeymaps = {};
            if (!configState.encoderKeymaps[curLayer]) configState.encoderKeymaps[curLayer] = [];
            if (!configState.encoderKeymaps[curLayer][encIdx]) configState.encoderKeymaps[curLayer][encIdx] = { cw: "K_TRNS", ccw: "K_TRNS" };
            configState.encoderKeymaps[curLayer][encIdx][dir] = val.trim() || "K_TRNS";
        }

        function toggleVialOptions() {
            const en = document.getElementById('enableVial').checked;
            document.getElementById('vialOptionsBlock').style.display = en ? 'flex' : 'none';
        }

        function getParsedPins() {
            if (configState.matrixType === 'DIRECT') {
                const pins = parsePins(document.getElementById('directPins').value);
                return { rows: [""], cols: pins, totalCols: pins.length, totalRows: 1 };
            }

            const rows = parsePins(document.getElementById('rowPins').value);
            const cols = parsePins(document.getElementById('colPins').value);
            const totalCols = configState.isSplit ? cols.length * 2 : cols.length;

            return { rows, cols, totalCols, totalRows: rows.length };
        }

        function ensureActiveKeys(customPattern) {
            const { totalRows, totalCols } = getParsedPins();
            if (!Array.isArray(configState.activeKeys)) {
                configState.activeKeys = [];
            }
            while (configState.activeKeys.length < totalRows) {
                configState.activeKeys.push([]);
            }
            if (configState.activeKeys.length > totalRows) {
                configState.activeKeys.length = totalRows;
            }
            const isPatternArray = Array.isArray(customPattern) && customPattern !== configState.activeKeys;
            for (let r = 0; r < totalRows; r++) {
                if (!Array.isArray(configState.activeKeys[r])) configState.activeKeys[r] = [];
                while (configState.activeKeys[r].length < totalCols) {
                    const c = configState.activeKeys[r].length;
                    let isActive = true;
                    if (isPatternArray && customPattern[r] && typeof customPattern[r][c] !== 'undefined') {
                        isActive = Boolean(customPattern[r][c] === 1 || customPattern[r][c] === true);
                    }
                    configState.activeKeys[r].push(isActive);
                }
                if (configState.activeKeys[r].length > totalCols) {
                    configState.activeKeys[r].length = totalCols;
                }
                if (isPatternArray) {
                    for (let c = 0; c < totalCols; c++) {
                        if (customPattern[r] && typeof customPattern[r][c] !== 'undefined') {
                            configState.activeKeys[r][c] = Boolean(customPattern[r][c] === 1 || customPattern[r][c] === true);
                        }
                    }
                }
            }
            return configState.activeKeys;
        }

        function initMatrixGrid(customPattern) {
            const { totalRows, totalCols } = getParsedPins();
            const container = document.getElementById('matrixContainer');
            container.innerHTML = '';
            container.style.gridTemplateColumns = `repeat(${totalCols}, 52px)`;

            ensureActiveKeys(customPattern);
            let activeCount = 0;

            for (let r = 0; r < totalRows; r++) {
                for (let c = 0; c < totalCols; c++) {
                    const isActive = configState.activeKeys[r][c];
                    if (isActive) activeCount++;

                    const keyEl = document.createElement('div');
                    keyEl.className = 'grid-key' + (isActive ? ' active' : ' disabled');
                    keyEl.id = `key-${r}-${c}`;
                    keyEl.innerHTML = `
                        <span class="coord">${r},${c}</span>
                        <span class="idx">${isActive ? activeCount : '—'}</span>
                    `;
                    keyEl.onclick = (evt) => handleMatrixKeyClick(evt, r, c);
                    container.appendChild(keyEl);
                }
            }

            updateKeyIndices();
        }

        function addCustomLayer() {
            const t = I18N[currentLanguage];
            const isRu = (currentLanguage !== 'en');
            const defaultName = "LAYER_" + configState.layers.length;

            const contentHtml = `
                <div style="display: flex; flex-direction: column; gap: 8px;">
                    <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">${isRu ? 'Имя нового слоя:' : 'New Layer Name:'}</label>
                    <input type="text" id="modalLayerNameInput" value="${defaultName}" placeholder="NUMPAD, NAV, MEDIA, RU..." style="padding: 8px 10px; font-size: 0.9rem; font-family: var(--font-mono);">
                </div>
            `;

            openAppModal(t.btnAddLayer || "Добавить слой", contentHtml, () => {
                const name = document.getElementById('modalLayerNameInput').value;
                if (!name) return;
                const cleanName = name.trim().toUpperCase().replace(/\s+/g, '_');
                if (configState.layers.includes(cleanName)) {
                    alert(t.alertLayerExists || "Слой с таким именем уже существует!");
                    return;
                }
                markDirty();
                configState.layers.push(cleanName);
                configState.keymaps[cleanName] = Array(getActiveLayoutPairs().length).fill("K_TRNS");
                selectLayer(cleanName);
                renderPalette(currentPaletteCategory);
            });
        }

        function duplicateCurrentLayer() {
            const t = I18N[currentLanguage];
            const isRu = (currentLanguage !== 'en');
            const srcName = configState.currentLayer || 'DEF';
            const defaultName = srcName + "_COPY";

            const contentHtml = `
                <div style="display: flex; flex-direction: column; gap: 8px;">
                    <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">${isRu ? `Имя копии слоя (${srcName}):` : `Layer Copy Name (${srcName}):`}</label>
                    <input type="text" id="modalDupLayerNameInput" value="${defaultName}" placeholder="NUMPAD, NAV, MEDIA..." style="padding: 8px 10px; font-size: 0.9rem; font-family: var(--font-mono);">
                </div>
            `;

            openAppModal(t.btnDupLayer || "Дублировать слой", contentHtml, () => {
                const name = document.getElementById('modalDupLayerNameInput').value;
                if (!name) return;
                const cleanName = name.trim().toUpperCase().replace(/\s+/g, '_');
                if (configState.layers.includes(cleanName)) {
                    alert(t.alertLayerExists || "Слой с таким именем уже существует!");
                    return;
                }
                markDirty();
                configState.layers.push(cleanName);
                configState.keymaps[cleanName] = [...(configState.keymaps[srcName] || [])];
                if (configState.encoderKeymaps && configState.encoderKeymaps[srcName]) {
                    if (!configState.encoderKeymaps) configState.encoderKeymaps = {};
                    configState.encoderKeymaps[cleanName] = configState.encoderKeymaps[srcName].map(e => ({ ...e }));
                }
                selectLayer(cleanName);
                renderPalette(currentPaletteCategory);
            });
        }

        function clearCurrentLayer() {
            const t = I18N[currentLanguage];
            const curLayer = configState.currentLayer || 'DEF';
            const msg = currentLanguage === 'en'
                ? `Clear all keys on layer "${curLayer}" to transparent (K_TRNS)?`
                : `Очистить все клавиши на слое "${curLayer}" в прозрачные (K_TRNS)?`;
            if (!confirm(msg)) return;

            saveUndoState();
            configState.keymaps[curLayer] = Array(getActiveLayoutPairs().length).fill("K_TRNS");
            if (configState.encoderKeymaps && configState.encoderKeymaps[curLayer]) {
                configState.encoderKeymaps[curLayer] = configState.encoderKeymaps[curLayer].map(() => ({ cw: "K_TRNS", ccw: "K_TRNS" }));
            }
            renderVisualKeymap();
        }

        function renameCurrentLayer() {
            const t = I18N[currentLanguage];
            const isRu = (currentLanguage !== 'en');
            const oldName = configState.currentLayer;
            const curIdx = configState.layers.indexOf(oldName);

            const contentHtml = `
                <div style="display: flex; flex-direction: column; gap: 8px;">
                    <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">${isRu ? `Новое имя для слоя ${curIdx} (${oldName}):` : `New Name for Layer ${curIdx} (${oldName}):`}</label>
                    <input type="text" id="modalLayerRenameInput" value="${oldName}" placeholder="BASE, DEF, NUMPAD, RU..." style="padding: 8px 10px; font-size: 0.9rem; font-family: var(--font-mono);">
                </div>
            `;

            openAppModal(t.btnRenameLayer || "Переименовать слой", contentHtml, () => {
                const newName = document.getElementById('modalLayerRenameInput').value;
                if (!newName) return;
                const cleanName = newName.trim().toUpperCase().replace(/\s+/g, '_');
                if (cleanName === oldName) return;
                if (configState.layers.includes(cleanName)) {
                    alert(t.alertLayerExists || "Слой с таким именем уже существует!");
                    return;
                }

                markDirty();
                const idx = configState.layers.indexOf(oldName);
                if (idx !== -1) {
                    configState.layers[idx] = cleanName;
                }
                configState.keymaps[cleanName] = configState.keymaps[oldName] || [];
                delete configState.keymaps[oldName];

                if (configState.encoderKeymaps) {
                    if (configState.encoderKeymaps[oldName]) {
                        configState.encoderKeymaps[cleanName] = configState.encoderKeymaps[oldName];
                        delete configState.encoderKeymaps[oldName];
                    }
                }

                // Update any references across all keymaps (e.g. MO(OLD) -> MO(NEW))
                configState.layers.forEach(l => {
                    const km = configState.keymaps[l];
                    if (Array.isArray(km)) {
                        for (let i = 0; i < km.length; i++) {
                            if (km[i] === `MO(${oldName})`) km[i] = `MO(${cleanName})`;
                            if (km[i] === `TG(${oldName})`) km[i] = `TG(${cleanName})`;
                            if (km[i] === `OS(${oldName})`) km[i] = `OS(${cleanName})`;
                            if (km[i] === `OSL(${oldName})`) km[i] = `OS(${cleanName})`;
                            if (km[i] === `TO(${oldName})`) km[i] = `TO(${cleanName})`;
                            if (km[i] && km[i].startsWith(`HT(${oldName},`)) {
                                km[i] = km[i].replace(`HT(${oldName},`, `HT(${cleanName},`);
                            }
                        }
                    }
                });

                configState.currentLayer = cleanName;
                renderLayersUI();
                renderVisualKeymap();
                renderPalette(currentPaletteCategory);
            });
        }

        function removeCurrentLayer() {
            const t = I18N[currentLanguage];
            if (configState.layers.length <= 1) {
                alert(t.alertCannotDeleteBase || "Нельзя удалить единственный оставшийся слой!");
                return;
            }
            const idx = configState.layers.indexOf(configState.currentLayer);
            if (idx === -1) return;
            const toRemove = configState.currentLayer;
            
            markDirty();
            // Remove layer from array and clean up keymaps
            configState.layers.splice(idx, 1);
            delete configState.keymaps[toRemove];
            if (configState.encoderKeymaps && configState.encoderKeymaps[toRemove]) {
                delete configState.encoderKeymaps[toRemove];
            }

            // Layer index shifting: activate the layer that now occupies this position (or previous if deleted last)
            const newActiveIndex = Math.min(idx, configState.layers.length - 1);
            configState.currentLayer = configState.layers[newActiveIndex];

            renderLayersUI();
            renderVisualKeymap();
            renderPalette(currentPaletteCategory);
        }

        function selectLayer(layerName) {
            configState.currentLayer = layerName;
            renderLayersUI();
            renderVisualKeymap();
        }

        function renderLayersUI() {
            const badgeContainer = document.getElementById('layerBadgeList');
            if (!badgeContainer) return;
            badgeContainer.innerHTML = '';
            configState.layers.forEach((l, idx) => {
                const b = document.createElement('span');
                b.className = 'preset-badge' + (l === configState.currentLayer ? ' active' : '');
                b.textContent = `${l} (${idx})`;
                b.onclick = () => selectLayer(l);
                badgeContainer.appendChild(b);
            });
            const titleEl = document.getElementById('currentLayerTitle');
            if (titleEl) {
                titleEl.textContent = `${configState.currentLayer} (${configState.layers.indexOf(configState.currentLayer)})`;
            }
            renderLayerEncodersUI();
            if (document.getElementById('enableLayerHotkeys')?.checked) {
                renderLayerHotkeysUI();
            }
        }

        // In-App Modal Dialog Management
        let appModalCallback = null;

        function openAppModal(title, contentHtml, onConfirm) {
            const isRu = (currentLanguage !== 'en');
            const cancelBtn = document.getElementById('btnAppModalCancel');
            const confirmBtn = document.getElementById('btnAppModalConfirm');
            if (cancelBtn) cancelBtn.textContent = isRu ? 'Отмена' : 'Cancel';
            if (confirmBtn) confirmBtn.textContent = isRu ? 'Применить' : 'Apply';

            document.getElementById('appModalTitle').textContent = title;
            document.getElementById('appModalContent').innerHTML = contentHtml;
            appModalCallback = onConfirm;
            const overlay = document.getElementById('appModalOverlay');
            if (overlay.showModal) {
                overlay.showModal();
            } else {
                overlay.style.display = 'flex';
            }
        }

        function closeAppModal() {
            const overlay = document.getElementById('appModalOverlay');
            if (overlay.close) {
                overlay.close();
            } else {
                overlay.style.display = 'none';
            }
            appModalCallback = null;
        }

        function confirmAppModal() {
            if (typeof appModalCallback === 'function') {
                appModalCallback();
            }
            closeAppModal();
        }

        function ensureLayerExists(target) {
            if (!configState.layers.includes(target)) {
                configState.layers.push(target);
                configState.keymaps[target] = Array(getActiveLayoutPairs().length).fill("K_TRNS");
                renderLayersUI();
            }
        }

        function promptLayerModifierChoice(modifierType, callback) {
            const isRu = (currentLanguage !== 'en');
            const layers = configState.layers;
            const defaultTarget = layers[1] || layers[0];

            let optionsHtml = layers.map((l, idx) => `<option value="${l}">${l} (${isRu ? 'Слой' : 'Layer'} ${idx})</option>`).join('');
            optionsHtml += `<option value="__custom__">${isRu ? '+ Другой / новый слой...' : '+ Other / new layer...'}</option>`;

            const contentHtml = `
                <div style="display: flex; flex-direction: column; gap: 12px;">
                    <div style="display: flex; flex-direction: column; gap: 6px;">
                        <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">${isRu ? `Слой для ${modifierType}:` : `Target Layer for ${modifierType}:`}</label>
                        <select id="modalLayerSelect" onchange="toggleCustomLayerField(this.value)" style="padding: 8px 10px; font-size: 0.9rem;">
                            ${optionsHtml}
                        </select>
                    </div>
                    <div id="modalCustomLayerGroup" style="display: none; flex-direction: column; gap: 4px;">
                        <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">${isRu ? 'Имя слоя:' : 'Layer Name:'}</label>
                        <input type="text" id="modalLayerInput" placeholder="NUMPAD, NAV, MEDIA..." style="padding: 8px 10px; font-size: 0.9rem; font-family: var(--font-mono);">
                    </div>
                </div>
            `;

            openAppModal(isRu ? `Настройка ${modifierType}` : `Configure ${modifierType}`, contentHtml, () => {
                const selectVal = document.getElementById('modalLayerSelect').value;
                let target = selectVal;
                if (selectVal === '__custom__') {
                    const rawVal = (document.getElementById('modalLayerInput').value || '').trim().toUpperCase().replace(/\s+/g, '_');
                    target = rawVal || layers[0];
                }

                ensureLayerExists(target);
                callback(`${modifierType}(${target})`);
            });

            window.toggleCustomLayerField = function(val) {
                const grp = document.getElementById('modalCustomLayerGroup');
                if (grp) grp.style.display = (val === '__custom__') ? 'flex' : 'none';
            };

            const sel = document.getElementById('modalLayerSelect');
            if (sel && defaultTarget) {
                sel.value = defaultTarget;
            }
        }

        function promptOneShotChoice(callback) {
            const isRu = (currentLanguage !== 'en');
            const layers = configState.layers;
            const defaultLayer = layers[1] || layers[0];

            let layerOptionsHtml = layers.map((l, idx) => `<option value="${l}">${l} (${isRu ? 'Слой' : 'Layer'} ${idx})</option>`).join('');

            const contentHtml = `
                <div style="display: flex; flex-direction: column; gap: 14px;">
                    <div style="display: flex; flex-direction: column; gap: 6px;">
                        <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">${isRu ? 'Тип One-Shot (залипание на 1 нажатие):' : 'One-Shot Type (sticky on 1 tap):'}</label>
                        <select id="modalOsTypeSelect" onchange="toggleModalOsFields(this.value)" style="padding: 8px 10px; font-size: 0.9rem;">
                            <optgroup label="${isRu ? 'Левые модификаторы (Left OSM)' : 'Left Modifiers (Left OSM)'}">
                                <option value="MOD_LSHIFT">Left Shift — OS(MOD_LSHIFT)</option>
                                <option value="MOD_LCTRL">Left Ctrl — OS(MOD_LCTRL)</option>
                                <option value="MOD_LALT">Left Alt — OS(MOD_LALT)</option>
                                <option value="MOD_LGUI">Left Win / Cmd — OS(MOD_LGUI)</option>
                            </optgroup>
                            <optgroup label="${isRu ? 'Правые модификаторы (Right OSM)' : 'Right Modifiers (Right OSM)'}">
                                <option value="MOD_RSHIFT">Right Shift — OS(MOD_RSHIFT)</option>
                                <option value="MOD_RCTRL">Right Ctrl — OS(MOD_RCTRL)</option>
                                <option value="MOD_RALT">Right Alt — OS(MOD_RALT)</option>
                                <option value="MOD_RGUI">Right Win / Cmd — OS(MOD_RGUI)</option>
                            </optgroup>
                            <optgroup label="${isRu ? 'Комбинации' : 'Combinations'}">
                                <option value="MOD_HYPER">Hyper (Ctrl+Shift+Alt+Cmd) — OS(MOD_HYPER)</option>
                                <option value="MOD_MEH">Meh (Ctrl+Shift+Alt) — OS(MOD_LCTRL | MOD_LSHIFT | MOD_LALT)</option>
                            </optgroup>
                            <optgroup label="${isRu ? 'Слой (One-Shot Layer)' : 'Layer (One-Shot Layer)'}">
                                <option value="LAYER">${isRu ? 'Выбрать слой...' : 'Select layer...'}</option>
                            </optgroup>
                        </select>
                    </div>

                    <div id="modalOsLayerGroup" style="display: none; flex-direction: column; gap: 6px;">
                        <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">${isRu ? 'Целевой слой для One-Shot:' : 'Target Layer for One-Shot:'}</label>
                        <select id="modalOsLayerSelect" onchange="toggleCustomOsLayerField(this.value)" style="padding: 8px 10px; font-size: 0.9rem;">
                            ${layerOptionsHtml}
                            <option value="__custom__">${isRu ? '+ Другой / новый слой...' : '+ Other / new layer...'}</option>
                        </select>
                    </div>
                    <div id="modalCustomOsLayerGroup" style="display: none; flex-direction: column; gap: 4px;">
                        <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">${isRu ? 'Имя нового слоя:' : 'New Layer Name:'}</label>
                        <input type="text" id="modalOsLayerInput" placeholder="NUMPAD, NAV, MEDIA..." style="padding: 8px 10px; font-size: 0.9rem; font-family: var(--font-mono);">
                    </div>

                    <!-- Live Generated Code Preview -->
                    <div style="background: var(--bg); border: 1px solid var(--border); border-radius: 6px; padding: 10px 12px; display: flex; align-items: center; justify-content: space-between;">
                        <span style="font-size: 0.75rem; color: var(--text-muted);">${isRu ? 'Итоговый кейкод:' : 'Resulting keycode:'}</span>
                        <span id="modalOsPreview" style="font-family: var(--font-mono); font-weight: 700; font-size: 0.85rem; color: var(--accent);">OS(MOD_LSHIFT)</span>
                    </div>
                </div>
            `;

            openAppModal(isRu ? "Настройка One-Shot (OS)" : "Configure One-Shot (OS)", contentHtml, () => {
                const type = document.getElementById('modalOsTypeSelect').value;
                if (type === 'LAYER') {
                    const selVal = document.getElementById('modalOsLayerSelect').value;
                    let target = selVal;
                    if (selVal === '__custom__') {
                        const rawVal = (document.getElementById('modalOsLayerInput').value || '').trim().toUpperCase().replace(/\s+/g, '_');
                        target = rawVal || layers[0];
                    }
                    ensureLayerExists(target);
                    callback(`OS(${target})`);
                } else if (type === 'MOD_MEH') {
                    callback('OS(MOD_LCTRL | MOD_LSHIFT | MOD_LALT)');
                } else {
                    callback(`OS(${type})`);
                }
            });

            window.toggleCustomOsLayerField = function(val) {
                const grp = document.getElementById('modalCustomOsLayerGroup');
                if (grp) grp.style.display = (val === '__custom__') ? 'flex' : 'none';
                const prev = document.getElementById('modalOsPreview');
                if (prev) {
                    if (val === '__custom__') {
                        const customVal = (document.getElementById('modalOsLayerInput')?.value || '').trim().toUpperCase() || 'CUSTOM';
                        prev.textContent = `OS(${customVal})`;
                    } else {
                        prev.textContent = `OS(${val})`;
                    }
                }
            };

            window.toggleModalOsFields = function(val) {
                const grp = document.getElementById('modalOsLayerGroup');
                const customGrp = document.getElementById('modalCustomOsLayerGroup');
                const prev = document.getElementById('modalOsPreview');
                if (val === 'LAYER') {
                    if (grp) grp.style.display = 'flex';
                    const layerSel = document.getElementById('modalOsLayerSelect')?.value || defaultLayer;
                    if (customGrp) customGrp.style.display = (layerSel === '__custom__') ? 'flex' : 'none';
                    if (prev) {
                        const target = (layerSel === '__custom__') ? ((document.getElementById('modalOsLayerInput')?.value || '').trim().toUpperCase() || 'CUSTOM') : layerSel;
                        prev.textContent = `OS(${target})`;
                    }
                } else {
                    if (grp) grp.style.display = 'none';
                    if (customGrp) customGrp.style.display = 'none';
                    if (prev) {
                        if (val === 'MOD_MEH') prev.textContent = 'OS(MOD_LCTRL | MOD_LSHIFT | MOD_LALT)';
                        else prev.textContent = `OS(${val})`;
                    }
                }
            };

            const layerInp = document.getElementById('modalOsLayerInput');
            if (layerInp) {
                layerInp.oninput = () => {
                    const prev = document.getElementById('modalOsPreview');
                    const target = (layerInp.value || '').trim().toUpperCase() || 'DEF';
                    if (prev) prev.textContent = `OS(${target})`;
                };
            }
        }

        function promptMidiCcChoice(initialMode, callback) {
            if (typeof initialMode === 'function') {
                callback = initialMode;
                initialMode = 'STATIC';
            }
            const isRu = (currentLanguage !== 'en');

            let defaultAction = 'STATIC';
            let defaultVal = 127;
            if (initialMode === 'CC_TOG(...)') defaultAction = 'TOGGLE';
            else if (initialMode === 'CC_VAL127(...)') { defaultAction = 'STATIC'; defaultVal = 127; }
            else if (initialMode === 'CC_VAL0(...)') { defaultAction = 'STATIC'; defaultVal = 0; }
            else if (initialMode === 'CC_INC(...)') defaultAction = 'INC';
            else if (initialMode === 'CC_DEC(...)') defaultAction = 'DEC';

            const COMMON_CC_LIST = [
                { num: 1, name: isRu ? "Модуляция (Modulation Wheel)" : "Modulation Wheel" },
                { num: 2, name: isRu ? "Дыхательный контроллер (Breath)" : "Breath Controller" },
                { num: 7, name: isRu ? "Громкость канала (Channel Volume)" : "Channel Volume" },
                { num: 10, name: isRu ? "Панорама (Pan)" : "Pan" },
                { num: 11, name: isRu ? "Экспрессия (Expression)" : "Expression" },
                { num: 64, name: isRu ? "Сустейн педаль (Sustain Pedal)" : "Sustain Pedal" },
                { num: 65, name: isRu ? "Портаменто (Portamento)" : "Portamento" },
                { num: 66, name: isRu ? "Состенуто (Sostenuto)" : "Sostenuto" },
                { num: 67, name: isRu ? "Мягкая педаль (Soft Pedal)" : "Soft Pedal" },
                { num: 71, name: isRu ? "Резонанс фильтра (Filter Resonance / Q)" : "Filter Resonance" },
                { num: 74, name: isRu ? "Частота среза фильтра (Cutoff Frequency)" : "Filter Cutoff" },
                { num: 91, name: isRu ? "Реверберация (Reverb Send)" : "Reverb Send" },
                { num: 93, name: isRu ? "Хорус (Chorus Send)" : "Chorus Send" },
                { num: 120, name: isRu ? "Выключить все звуки (All Sound Off)" : "All Sound Off" },
                { num: 123, name: isRu ? "Все ноты выкл (All Notes Off)" : "All Notes Off" }
            ];

            let ccOptionsHtml = COMMON_CC_LIST.map(item => `<option value="${item.num}">CC ${item.num} — ${item.name}</option>`).join('');

            const contentHtml = `
                <div style="display: flex; flex-direction: column; gap: 14px;">
                    <!-- Controller number selection with presets + custom input -->
                    <div style="display: flex; flex-direction: column; gap: 6px;">
                        <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">
                            ${isRu ? 'Контроллер CC (номер 0–127):' : 'CC Controller (number 0–127):'}
                        </label>
                        <div style="display: grid; grid-template-columns: 1fr auto; gap: 8px;">
                            <input type="number" id="modalCcNumberInput" min="0" max="127" value="1" list="modalCcCommonDatalist" oninput="updateMidiCcPreview()" style="padding: 8px 10px; font-size: 0.9rem; font-family: var(--font-mono);">
                            <select onchange="document.getElementById('modalCcNumberInput').value = this.value; updateMidiCcPreview();" style="padding: 8px 10px; font-size: 0.85rem; max-width: 220px;">
                                <option value="">${isRu ? '— Пресеты CC —' : '— Common CC —'}</option>
                                ${ccOptionsHtml}
                            </select>
                        </div>
                        <datalist id="modalCcCommonDatalist">
                            ${ccOptionsHtml}
                        </datalist>
                    </div>

                    <!-- Action Type & Value (Velocity / Value) -->
                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;">
                        <div style="display: flex; flex-direction: column; gap: 6px;">
                            <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">
                                ${isRu ? 'Режим отправки:' : 'Trigger Mode:'}
                            </label>
                            <select id="modalCcActionSelect" onchange="updateMidiCcModalUI()" style="padding: 8px 10px; font-size: 0.85rem;">
                                <option value="STATIC" ${defaultAction === 'STATIC' ? 'selected' : ''}>${isRu ? 'Фиксированное значение' : 'Fixed Value (MIDI_CC)'}</option>
                                <option value="TOGGLE" ${defaultAction === 'TOGGLE' ? 'selected' : ''}>${isRu ? 'Переключатель (0 ↔ 127)' : 'Toggle (0 ↔ 127)'}</option>
                                <option value="VAL127" ${defaultAction === 'VAL127' ? 'selected' : ''}>${isRu ? 'Статичный On (127)' : 'Static On (127)'}</option>
                                <option value="INC" ${defaultAction === 'INC' ? 'selected' : ''}>${isRu ? 'Инкремент +1 (Inc)' : 'Increment +1 (Inc)'}</option>
                                <option value="DEC" ${defaultAction === 'DEC' ? 'selected' : ''}>${isRu ? 'Декремент -1 (Dec)' : 'Decrement -1 (Dec)'}</option>
                            </select>
                        </div>
                        <div id="modalCcValueGroup" style="display: ${defaultAction === 'STATIC' ? 'flex' : 'none'}; flex-direction: column; gap: 6px;">
                            <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">
                                ${isRu ? 'Значение / Сила нажатия (0–127):' : 'Value / Velocity (0–127):'}
                            </label>
                            <input type="number" id="modalCcValueInput" min="0" max="127" value="${defaultVal}" oninput="updateMidiCcPreview()" style="padding: 8px 10px; font-size: 0.9rem; font-family: var(--font-mono);">
                        </div>
                    </div>

                    <!-- Live Preview -->
                    <div style="background: var(--bg); border: 1px solid var(--border); border-radius: 6px; padding: 10px 12px; display: flex; align-items: center; justify-content: space-between;">
                        <span style="font-size: 0.75rem; color: var(--text-muted);">${isRu ? 'Итоговый кейкод:' : 'Resulting keycode:'}</span>
                        <span id="modalMidiCcPreview" style="font-family: var(--font-mono); font-weight: 700; font-size: 0.85rem; color: var(--accent);">MIDI_CC(1, ${defaultVal})</span>
                    </div>
                </div>
            `;

            openAppModal(isRu ? "Настройка MIDI CC" : "Configure MIDI CC", contentHtml, () => {
                const act = document.getElementById('modalCcActionSelect').value;
                const ccNum = Math.max(0, Math.min(127, parseInt(document.getElementById('modalCcNumberInput').value, 10) || 0));
                const ccVal = Math.max(0, Math.min(127, parseInt(document.getElementById('modalCcValueInput').value, 10) || 0));

                let finalKeycode = `MIDI_CC(${ccNum}, ${ccVal})`;
                if (act === 'VAL127') finalKeycode = `MIDI_CC_VAL_127(${ccNum & 0x0F})`;
                else if (act === 'TOGGLE') finalKeycode = `MIDI_CC_TOGGLE(${ccNum & 0x0F})`;
                else if (act === 'INC') finalKeycode = `MIDI_CC_INC(${ccNum & 0x0F})`;
                else if (act === 'DEC') finalKeycode = `MIDI_CC_DEC(${ccNum & 0x0F})`;
                else finalKeycode = `MIDI_CC(${ccNum}, ${ccVal})`;

                callback(finalKeycode);
            });

            window.updateMidiCcModalUI = function() {
                const act = document.getElementById('modalCcActionSelect').value;
                const valGrp = document.getElementById('modalCcValueGroup');
                if (valGrp) {
                    valGrp.style.display = (act === 'STATIC') ? 'flex' : 'none';
                }
                updateMidiCcPreview();
            };

            window.updateMidiCcPreview = function() {
                const act = document.getElementById('modalCcActionSelect')?.value || 'STATIC';
                const ccNum = Math.max(0, parseInt(document.getElementById('modalCcNumberInput')?.value, 10) || 0);
                const ccVal = Math.max(0, parseInt(document.getElementById('modalCcValueInput')?.value, 10) || 0);
                const prev = document.getElementById('modalMidiCcPreview');
                if (!prev) return;

                if (act === 'VAL127') prev.textContent = `MIDI_CC_VAL_127(${ccNum & 0x0F})`;
                else if (act === 'TOGGLE') prev.textContent = `MIDI_CC_TOGGLE(${ccNum & 0x0F})`;
                else if (act === 'INC') prev.textContent = `MIDI_CC_INC(${ccNum & 0x0F})`;
                else if (act === 'DEC') prev.textContent = `MIDI_CC_DEC(${ccNum & 0x0F})`;
                else prev.textContent = `MIDI_CC(${ccNum & 0x7F}, ${ccVal & 0x7F})`;
            };
            setTimeout(updateMidiCcPreview, 0);
        }

        // Multi-Modifier Builder State
        let activeModalModifiers = [];

        function promptKeyChoice(initialModType, callback) {
            const t = I18N[currentLanguage];

            // Parse initial modifiers (Left and Right)
            activeModalModifiers = [];
            if (initialModType === 'LS(...)') activeModalModifiers = ['LS'];
            else if (initialModType === 'LC(...)') activeModalModifiers = ['LC'];
            else if (initialModType === 'LA(...)') activeModalModifiers = ['LA'];
            else if (initialModType === 'LG(...)') activeModalModifiers = ['LG'];
            else if (initialModType === 'RS(...)') activeModalModifiers = ['RS'];
            else if (initialModType === 'RC(...)') activeModalModifiers = ['RC'];
            else if (initialModType === 'RA(...)') activeModalModifiers = ['RA'];
            else if (initialModType === 'RG(...)') activeModalModifiers = ['RG'];
            else if (initialModType === 'HYPER(...)') activeModalModifiers = ['LG', 'LS', 'LA', 'LC'];
            else if (initialModType === 'MEH(...)') activeModalModifiers = ['LC', 'LS', 'LA'];
            else if (initialModType === 'HT(...)') activeModalModifiers = ['HT_LSHIFT'];
            else activeModalModifiers = ['LS'];

            renderModalKeyChoiceContent(callback);
        }

        function renderModalKeyChoiceContent(callback) {
            const isRu = (currentLanguage !== 'en');
            ensureKeycodesDatalist();

            let title = isRu ? "Настройка модификаторов и клавиши" : "Configure Modifiers & Key";

            const contentHtml = `
                <div style="display: flex; flex-direction: column; gap: 14px;">
                    <!-- Modifiers List with + and delete -->
                    <div style="display: flex; flex-direction: column; gap: 8px;">
                        <div style="display: flex; align-items: center; justify-content: space-between; flex-wrap: wrap; gap: 8px;">
                            <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">${isRu ? 'Активные модификаторы:' : 'Active Modifiers:'}</label>
                            <div style="display: flex; gap: 6px; align-items: center;">
                                <select id="modalAddModSelect" style="padding: 5px 8px; font-size: 0.8rem; min-width: 170px;">
                                    <optgroup label="${isRu ? 'Левые модификаторы' : 'Left Modifiers'}">
                                        <option value="LS">Left Shift (LS)</option>
                                        <option value="LC">Left Ctrl (LC)</option>
                                        <option value="LA">Left Alt (LA)</option>
                                        <option value="LG">Left Win/Cmd (LG)</option>
                                    </optgroup>
                                    <optgroup label="${isRu ? 'Правые модификаторы' : 'Right Modifiers'}">
                                        <option value="RS">Right Shift (RS)</option>
                                        <option value="RC">Right Ctrl (RC)</option>
                                        <option value="RA">Right Alt (RA)</option>
                                        <option value="RG">Right Win/Cmd (RG)</option>
                                    </optgroup>
                                    <optgroup label="${isRu ? 'Комбинации' : 'Combinations'}">
                                        <option value="HYPER">Hyper (Ctrl+Shift+Alt+Cmd)</option>
                                        <option value="MEH">Meh (Ctrl+Shift+Alt)</option>
                                    </optgroup>
                                    <optgroup label="${isRu ? 'Удержание-Тап (Hold-Tap)' : 'Hold-Tap'}">
                                        <option value="HT_LSHIFT">Hold Shift / Tap Key</option>
                                        <option value="HT_RSHIFT">Hold R-Shift / Tap Key</option>
                                        <option value="HT_LCTRL">Hold Ctrl / Tap Key</option>
                                        <option value="HT_LALT">Hold Alt / Tap Key</option>
                                        <option value="HT_LGUI">Hold Win / Tap Key</option>
                                    </optgroup>
                                </select>
                                <button class="btn btn-sm" onclick="addModalModifier()" style="padding: 5px 10px; white-space: nowrap;">${isRu ? '+ Добавить' : '+ Add'}</button>
                            </div>
                        </div>
                        <div id="modalModifiersTags" style="display: flex; flex-wrap: wrap; gap: 6px; min-height: 36px; align-items: center; background: var(--bg); padding: 6px 10px; border-radius: 6px; border: 1px solid var(--border);">
                            <!-- Rendered dynamically -->
                        </div>
                    </div>

                    <!-- Single Input with Suggestions -->
                    <div style="display: flex; flex-direction: column; gap: 4px;">
                        <label style="font-size: 0.8rem; font-weight: 600; color: var(--text);">${isRu ? 'Клавиша (введите имя или выберите из подсказок):' : 'Keycode (type name or select from suggestions):'}</label>
                        <input type="text" id="modalKeySingleInput" list="allKeycodesList" value="K_9" placeholder="K_9, K_A, K_ENT, K_SPC..." style="padding: 8px 10px; font-size: 0.9rem; font-family: var(--font-mono);" oninput="if (typeof SYMBOL_KEY_MAP !== 'undefined' && SYMBOL_KEY_MAP[this.value.trim()]) { this.value = SYMBOL_KEY_MAP[this.value.trim()]; } updateModalKeyPreview()" onchange="const resolved = resolveKeycodeFromSearch(this.value); if (resolved) { this.value = resolved; } updateModalKeyPreview()">
                    </div>

                    <!-- Live Generated Code Preview -->
                    <div style="background: var(--bg); border: 1px solid var(--border); border-radius: 6px; padding: 10px 12px; display: flex; align-items: center; justify-content: space-between;">
                        <span style="font-size: 0.75rem; color: var(--text-muted);">${isRu ? 'Итоговый кейкод:' : 'Resulting keycode:'}</span>
                        <span id="modalKeycodePreview" style="font-family: var(--font-mono); font-weight: 700; font-size: 0.85rem; color: var(--accent);">LS(K_9)</span>
                    </div>
                </div>
            `;

            openAppModal(title, contentHtml, () => {
                const code = buildFinalModalKeycode();
                callback(code);
            });

            renderModalModifiersTags();
            updateModalKeyPreview();
        }

        function renderModalModifiersTags() {
            const isRu = (currentLanguage !== 'en');
            const container = document.getElementById('modalModifiersTags');
            if (!container) return;
            container.innerHTML = '';

            if (activeModalModifiers.length === 0) {
                container.innerHTML = `<span style="font-size: 0.75rem; color: var(--text-muted); padding: 2px;">${isRu ? 'Без модификаторов (прямое нажатие)' : 'No modifiers (direct press)'}</span>`;
                return;
            }

            activeModalModifiers.forEach((mod, idx) => {
                const tag = document.createElement('span');
                tag.style.background = 'var(--panel)';
                tag.style.border = '1px solid var(--accent)';
                tag.style.color = 'var(--accent)';
                tag.style.borderRadius = '4px';
                tag.style.padding = '3px 8px';
                tag.style.fontSize = '0.75rem';
                tag.style.fontWeight = '600';
                tag.style.display = 'inline-flex';
                tag.style.alignItems = 'center';
                tag.style.gap = '6px';

                tag.innerHTML = `
                    <span>${mod}</span>
                    <button onclick="removeModalModifier(${idx})" style="background: none; border: none; color: var(--danger); cursor: pointer; padding: 0 2px; font-weight: 700; font-size: 0.85rem; line-height: 1;">✕</button>
                `;
                container.appendChild(tag);
            });
        }

        function addModalModifier() {
            const val = document.getElementById('modalAddModSelect').value;
            if (val === 'HYPER') {
                ['LG', 'LS', 'LA', 'LC'].forEach(m => {
                    if (!activeModalModifiers.includes(m)) activeModalModifiers.push(m);
                });
            } else if (val === 'MEH') {
                ['LC', 'LS', 'LA'].forEach(m => {
                    if (!activeModalModifiers.includes(m)) activeModalModifiers.push(m);
                });
            } else {
                if (!activeModalModifiers.includes(val)) {
                    activeModalModifiers.push(val);
                }
            }
            renderModalModifiersTags();
            updateModalKeyPreview();
        }

        function removeModalModifier(idx) {
            activeModalModifiers.splice(idx, 1);
            renderModalModifiersTags();
            updateModalKeyPreview();
        }

        function buildFinalModalKeycode() {
            const rawVal = (document.getElementById('modalKeySingleInput')?.value || '').trim();
            if (!rawVal) return 'K_NO';
            let key = resolveKeycodeFromSearch(rawVal);
            if (!key) {
                key = rawVal.startsWith('K_') ? rawVal.toUpperCase() : `K_${rawVal.toUpperCase()}`;
            }

            let result = key;
            activeModalModifiers.slice().reverse().forEach(mod => {
                if (mod === 'HT_LSHIFT' || mod === 'HT_SHIFT') {
                    result = `HT(MOD_LSHIFT, ${result})`;
                } else if (mod === 'HT_RSHIFT') {
                    result = `HT(MOD_RSHIFT, ${result})`;
                } else if (mod === 'HT_LCTRL') {
                    result = `HT(MOD_LCTRL, ${result})`;
                } else if (mod === 'HT_RCTRL') {
                    result = `HT(MOD_RCTRL, ${result})`;
                } else if (mod === 'HT_LALT') {
                    result = `HT(MOD_LALT, ${result})`;
                } else if (mod === 'HT_RALT') {
                    result = `HT(MOD_RALT, ${result})`;
                } else if (mod === 'HT_LGUI') {
                    result = `HT(MOD_LGUI, ${result})`;
                } else if (mod === 'HT_RGUI') {
                    result = `HT(MOD_RGUI, ${result})`;
                } else {
                    result = `${mod}(${result})`;
                }
            });
            return result;
        }

        function updateModalKeyPreview() {
            const previewEl = document.getElementById('modalKeycodePreview');
            if (previewEl) {
                previewEl.textContent = buildFinalModalKeycode();
            }
        }

        // --- Graphical Macro, Chord & Layer Trigger Builders (Column/Table UI) ---

        function toggleCustomMacrosOptions() {
            const en = document.getElementById('enableCustomMacros').checked;
            document.getElementById('customMacrosBlock').style.display = en ? 'flex' : 'none';
            const tab = document.getElementById('tab-pal-MACROS');
            if (tab) tab.style.display = en ? '' : 'none';
            if (!en && currentPaletteCategory === 'MACROS') {
                renderPalette('ALL');
            } else {
                renderPalette(currentPaletteCategory);
            }
            if (en) renderCustomMacrosUI();
        }

        function getMacroActionTypes() {
            const isRu = (currentLanguage !== 'en');
            return [
                { label: isRu ? "Нажать (Tap)" : "Tap Key", value: "TAP" },
                { label: isRu ? "Зажать (Down)" : "Press (Down)", value: "DN" },
                { label: isRu ? "Отпустить (Up)" : "Release (Up)", value: "UP" },
                { label: isRu ? "Задержка (мс)" : "Delay (ms)", value: "DELAY" }
            ];
        }

        // Track collapsed states for macros
        const collapsedMacros = new Set();

        function toggleMacroCollapse(idx) {
            if (collapsedMacros.has(idx)) {
                collapsedMacros.delete(idx);
            } else {
                collapsedMacros.add(idx);
            }
            renderCustomMacrosUI();
        }

        function parseMacroStepsToArray(stepsStr) {
            if (!stepsStr || !stepsStr.trim()) return [];
            const rawParts = parsePins(stepsStr);
            const list = [];
            
            for (let i = 0; i < rawParts.length; i++) {
                const part = rawParts[i];
                if (part.startsWith('M_DN(')) {
                    const key = part.slice(5, -1).trim();
                    // Check if next is M_UP with same key => TAP
                    if (i + 1 < rawParts.length && rawParts[i+1].startsWith('M_UP(') && rawParts[i+1].slice(5, -1).trim() === key) {
                        list.push({ type: 'TAP', key: key });
                        i++; // skip next
                    } else {
                        list.push({ type: 'DN', key: key });
                    }
                } else if (part.startsWith('M_UP(')) {
                    list.push({ type: 'UP', key: part.slice(5, -1).trim() });
                } else if (part.startsWith('M_D(')) {
                    list.push({ type: 'DELAY', key: part.slice(4, -1).trim() });
                } else {
                    list.push({ type: 'TAP', key: part.replace(/^[KM_()]+/, '') || 'K_A' });
                }
            }
            return list;
        }

        // Direct and shifted symbol mappings for smart search & substitution
        const SYMBOL_KEY_MAP = {
            // Direct punctuation
            '-': 'K_MINS', '=': 'K_EQL', '[': 'K_LBRC', ']': 'K_RBRC', '\\': 'K_BSLS',
            ';': 'K_SCLN', '\'': 'K_QUOT', '`': 'K_GRV', ',': 'K_COMM', '.': 'K_DOT', '/': 'K_SLSH',
            // Shifted symbols
            '>': 'LS(K_DOT)', '<': 'LS(K_COMM)', '+': 'LS(K_EQL)', '_': 'LS(K_MINS)',
            '&': 'LS(K_7)', '#': 'LS(K_3)', '@': 'LS(K_2)', '!': 'LS(K_1)', '$': 'LS(K_4)',
            '%': 'LS(K_5)', '^': 'LS(K_6)', '*': 'LS(K_8)', '(': 'LS(K_9)', ')': 'LS(K_0)',
            '{': 'LS(K_LBRC)', '}': 'LS(K_RBRC)', '|': 'LS(K_BSLS)', ':': 'LS(K_SCLN)',
            '"': 'LS(K_QUOT)', '?': 'LS(K_SLSH)', '~': 'LS(K_GRV)'
        };

        function resolveKeycodeFromSearch(query) {
            if (!query) return '';
            const raw = query.trim();
            if (!raw) return '';

            // Check smart symbol map first (e.g. ">", "<", "\", "&", "#", "+", "-", "=")
            if (SYMBOL_KEY_MAP[raw]) return SYMBOL_KEY_MAP[raw];

            const q = raw.toUpperCase();
            
            // Check direct match
            if (q.startsWith('K_')) return q;
            if (q.startsWith('0X') || q.includes('(')) return q;

            // Check aliases dictionary
            for (const [kc, aliases] of Object.entries(KEY_SEARCH_ALIASES)) {
                if (kc === `K_${q}` || aliases.includes(q) || aliases.includes(raw)) {
                    return kc;
                }
            }

            // Fallback prefix
            return `K_${q}`;
        }

        function serializeMacroArrayToSteps(arr) {
            const out = [];
            arr.forEach(item => {
                let k = item.key ? item.key.trim() : '';
                if (item.type === 'DELAY') {
                    const ms = parseInt(k, 10) || 50;
                    out.push(`M_D(${ms})`);
                } else if (k) {
                    const resolved = resolveKeycodeFromSearch(k);
                    if (resolved) {
                        if (item.type === 'DN') {
                            out.push(`M_DN(${resolved})`);
                        } else if (item.type === 'UP') {
                            out.push(`M_UP(${resolved})`);
                        } else if (item.type === 'TAP') {
                            out.push(`M_DN(${resolved})`);
                            out.push(`M_UP(${resolved})`);
                        }
                    }
                }
            });
            return out.join(', ');
        }

        function getMacroStepList(macro) {
            if (!macro) return [];
            if (Array.isArray(macro.stepList)) {
                return macro.stepList;
            }
            macro.stepList = parseMacroStepsToArray(macro.steps || '');
            return macro.stepList;
        }

        function syncMacroStepsString(macro) {
            if (!macro) return '';
            const list = getMacroStepList(macro);
            macro.steps = serializeMacroArrayToSteps(list);
            return macro.steps;
        }

        function renderMacroStepRowHtml(step, sIdx, onActionChange, onKeyChange, onRemove, onChangeKey) {
            const isRu = (currentLanguage !== 'en');
            const actionOptions = getMacroActionTypes().map(a => `<option value="${a.value}" ${step.type === a.value ? 'selected' : ''}>${a.label}</option>`).join('');
            const isDelay = step.type === 'DELAY';
            const changeAttr = onChangeKey ? `onchange="${onChangeKey}"` : '';

            return `
                <div style="display: grid; grid-template-columns: 32px 140px 1fr 32px; gap: 8px; align-items: center; background: var(--bg); padding: 5px 10px; border-radius: 6px; border: 1px solid var(--border);">
                    <span style="font-size: 0.75rem; color: var(--text-muted); font-weight: 700; text-align: center;">#${sIdx + 1}</span>
                    <select onchange="${onActionChange}" style="padding: 4px 8px; font-size: 0.8rem;">
                        ${actionOptions}
                    </select>
                    <input type="${isDelay ? 'number' : 'text'}" ${isDelay ? 'placeholder="50"' : `list="allKeycodesList" placeholder="${isRu ? 'Поиск кнопки (Enter, Dot, [, Shift...)' : 'Search key (Enter, Dot, [, Shift...)'}"`} value="${step.key || ''}" oninput="${onKeyChange}" ${changeAttr} style="padding: 4px 8px; font-size: 0.85rem; font-family: var(--font-mono);">
                    <button onclick="${onRemove}" style="background: none; border: none; color: var(--danger); cursor: pointer; padding: 2px; font-weight: 700; font-size: 0.9rem;" title="${isRu ? 'Удалить шаг' : 'Delete step'}">✕</button>
                </div>
            `;
        }

        function renderCustomMacrosUI() {
            const isRu = (currentLanguage !== 'en');
            const container = document.getElementById('customMacrosList');
            if (!container) return;
            container.innerHTML = '';

            ensureKeycodesDatalist();

            configState.customMacros.forEach((macro, mIdx) => {
                const isCollapsed = collapsedMacros.has(mIdx);
                const card = document.createElement('div');
                card.style.background = 'var(--panel)';
                card.style.border = '1px solid var(--border)';
                card.style.borderRadius = '8px';
                card.style.padding = '12px 14px';
                card.style.display = 'flex';
                card.style.flexDirection = 'column';
                card.style.gap = '10px';

                const stepsArray = getMacroStepList(macro);

                let stepsRowsHtml = '';
                if (!isCollapsed) {
                    stepsArray.forEach((step, sIdx) => {
                        stepsRowsHtml += renderMacroStepRowHtml(
                            step,
                            sIdx,
                            `onMacroStepActionChange(${mIdx}, ${sIdx}, this.value)`,
                            `onMacroStepKeyChange(${mIdx}, ${sIdx}, this.value)`,
                            `removeMacroStep(${mIdx}, ${sIdx})`,
                            `const res = resolveKeycodeFromSearch(this.value); if (res) { this.value = res; onMacroStepKeyChange(${mIdx}, ${sIdx}, res); }`
                        );
                    });
                }

                card.innerHTML = `
                    <div style="display: flex; align-items: center; justify-content: space-between; gap: 8px;">
                        <div style="display: flex; align-items: center; gap: 8px; flex-wrap: wrap; flex: 1;">
                            <button class="btn btn-sm" onclick="toggleMacroCollapse(${mIdx})" style="padding: 2px 6px; font-size: 0.75rem; line-height: 1;">${isCollapsed ? '►' : '▼'}</button>
                            <span style="font-family: var(--font-mono); font-weight: 700; font-size: 0.9rem; color: var(--accent); background: var(--bg); padding: 3px 8px; border-radius: 4px; border: 1px solid var(--border);">M(${mIdx})</span>
                            <input type="text" placeholder="MC_NAME" value="${macro.name || 'MC_MACRO_' + mIdx}" oninput="configState.customMacros[${mIdx}].name = this.value" style="width: 140px; padding: 4px 8px; font-size: 0.85rem; font-weight: 600;">
                            <input type="text" placeholder="${isRu ? 'Описание макроса' : 'Macro description'}" value="${macro.desc || ''}" oninput="configState.customMacros[${mIdx}].desc = this.value" style="flex: 1; min-width: 160px; padding: 4px 8px; font-size: 0.8rem;">
                        </div>
                        <div style="display: flex; align-items: center; gap: 6px;">
                            <button class="btn btn-sm btn-danger" onclick="removeCustomMacro(${mIdx})">${ICONS.trash}</button>
                        </div>
                    </div>

                    <!-- Collapsible Steps Table -->
                    <div style="display: ${isCollapsed ? 'none' : 'flex'}; flex-direction: column; gap: 6px; margin-top: 4px;">
                        <div style="display: grid; grid-template-columns: 32px 140px 1fr 32px; gap: 8px; padding: 0 10px; font-size: 0.75rem; color: var(--text-muted); font-weight: 600;">
                            <span style="text-align: center;">#</span>
                            <span>${isRu ? 'Действие' : 'Action'}</span>
                            <span>${isRu ? 'Кнопка или задержка' : 'Key or delay'}</span>
                            <span></span>
                        </div>
                        <div id="macro_steps_container_${mIdx}" style="display: flex; flex-direction: column; gap: 6px;">
                            ${stepsRowsHtml || `<div style="font-size: 0.8rem; color: var(--text-muted); padding: 8px; text-align: center;">${isRu ? 'Нет шагов. Нажмите «+ Добавить».' : 'No steps. Click "+ Add".'}</div>`}
                        </div>
                        <div style="display: flex; align-items: center; justify-content: space-between; margin-top: 4px;">
                            <button class="btn btn-sm" onclick="addMacroStep(${mIdx})" style="padding: 4px 10px; font-size: 0.8rem;">${isRu ? '+ Добавить' : '+ Add'}</button>
                            <span id="macro_steps_preview_${mIdx}" style="font-size: 0.7rem; color: var(--text-muted); font-family: var(--font-mono); max-width: 380px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;">${macro.steps || ''}</span>
                        </div>
                    </div>
                `;
                container.appendChild(card);
            });
        }

        function onMacroStepActionChange(mIdx, sIdx, actionVal) {
            const macro = configState.customMacros[mIdx];
            if (!macro) return;
            const steps = getMacroStepList(macro);
            if (steps[sIdx]) {
                steps[sIdx].type = actionVal;
                if (actionVal === 'DELAY' && isNaN(parseInt(steps[sIdx].key, 10))) {
                    steps[sIdx].key = '50';
                } else if (actionVal !== 'DELAY' && !isNaN(parseInt(steps[sIdx].key, 10))) {
                    steps[sIdx].key = '';
                }
                syncMacroStepsString(macro);
                renderCustomMacrosUI();
            }
        }

        function onMacroStepKeyChange(mIdx, sIdx, keyVal) {
            const macro = configState.customMacros[mIdx];
            if (!macro) return;
            const steps = getMacroStepList(macro);
            if (steps[sIdx]) {
                steps[sIdx].key = keyVal;
                syncMacroStepsString(macro);
                const previewEl = document.getElementById(`macro_steps_preview_${mIdx}`);
                if (previewEl) {
                    previewEl.textContent = macro.steps || '';
                }
                renderPalette(currentPaletteCategory);
            }
        }

        function addMacroStep(mIdx) {
            const macro = configState.customMacros[mIdx];
            if (!macro) return;
            const steps = getMacroStepList(macro);
            steps.push({ type: 'TAP', key: '' });
            syncMacroStepsString(macro);
            renderCustomMacrosUI();
        }

        function removeMacroStep(mIdx, sIdx) {
            const macro = configState.customMacros[mIdx];
            if (!macro) return;
            const steps = getMacroStepList(macro);
            steps.splice(sIdx, 1);
            syncMacroStepsString(macro);
            renderCustomMacrosUI();
        }

        function addCustomMacro() {
            const idx = configState.customMacros.length;
            configState.customMacros.push({
                name: `MC_MACRO_${idx}`,
                desc: "",
                steps: "",
                stepList: []
            });
            renderCustomMacrosUI();
            renderPalette(currentPaletteCategory);
        }

        function removeCustomMacro(idx) {
            collapsedMacros.delete(idx);
            configState.customMacros.splice(idx, 1);
            renderCustomMacrosUI();
            renderPalette(currentPaletteCategory);
        }

        function toggleChordsOptions() {
            const en = document.getElementById('enableChords').checked;
            document.getElementById('chordsBlock').style.display = en ? 'flex' : 'none';
            if (en) renderChordsUI();
        }

        // Track collapsed states for chords
        const collapsedChords = new Set();

        function toggleChordCollapse(idx) {
            if (collapsedChords.has(idx)) {
                collapsedChords.delete(idx);
            } else {
                collapsedChords.add(idx);
            }
            renderChordsUI();
        }

        const COMMON_CHORD_ACTIONS = [
            { label: "Копировать (Ctrl+C)", name: "do_copy" },
            { label: "Вставить (Ctrl+V)", name: "do_paste" },
            { label: "Escape", name: "do_esc" },
            { label: "Enter", name: "do_enter" },
            { label: "Скриншот", name: "do_screenshot" },
            { label: "Смена языка", name: "do_switch_lang" },
            { label: "Пользовательская функция...", name: "do_custom" }
        ];

        // Flexible smart coordinate parser: supports "1.11", "1, 11", "1 11", "1/11", "1;11" -> { r: 0, c: 0 }
        function parseFlexibleCoordinate(rawStr) {
            if (!rawStr || typeof rawStr !== 'string') return { r: 0, c: 0, formatted: '' };
            const clean = rawStr.replace(/[\[\]\(\)\{\}]/g, '').trim();
            if (!clean) return { r: 0, c: 0, formatted: '' };
            const parts = clean.split(/[\s,./;:\-_|\\]+/).filter(Boolean);
            let r = 0, c = 0;
            if (parts.length >= 2) {
                r = parseInt(parts[0], 10) || 0;
                c = parseInt(parts[1], 10) || 0;
            } else if (parts.length === 1 && parts[0].length >= 2 && !isNaN(parts[0])) {
                r = parseInt(parts[0][0], 10) || 0;
                c = parseInt(parts[0].slice(1), 10) || 0;
            }
            return { r, c, formatted: `${r}, ${c}` };
        }

        function getCoordForKeycode(targetKey) {
            const resolved = resolveKeycodeFromSearch(targetKey);
            const pairs = getActiveLayoutPairs();
            const defKm = configState.keymaps['DEF'] || [];
            const idx = defKm.indexOf(resolved);
            if (idx !== -1 && pairs[idx]) {
                return { r: pairs[idx].r, c: pairs[idx].c };
            }
            return { r: 0, c: 0 };
        }

        function parseChordKeys(chord) {
            if (Array.isArray(chord.keyList)) return chord.keyList;
            const list = [];
            const rawParts = (chord.keys || '').split(';').map(s => s.trim()).filter(Boolean);
            rawParts.forEach(p => {
                const parsed = parseFlexibleCoordinate(p);
                list.push({ r: parsed.r, c: parsed.c, coordStr: parsed.formatted });
            });
            chord.keyList = list;
            return list;
        }

        let pickingChordTarget = null;

        function pickChordKeyFromLayout(cIdx, kIdx) {
            pickingChordTarget = { cIdx, kIdx };
            const el = document.getElementById('visualKeymapContainer');
            if (el) {
                el.scrollIntoView({ behavior: 'smooth', block: 'center' });
                el.style.boxShadow = '0 0 0 3px var(--accent)';
                setTimeout(() => { el.style.boxShadow = 'none'; }, 2000);
            }
        }

        function onMatrixKeySelectedForChord(r, c) {
            if (!pickingChordTarget) return false;
            const { cIdx, kIdx } = pickingChordTarget;
            const chord = configState.customChords[cIdx];
            if (chord) {
                const list = parseChordKeys(chord);
                if (list[kIdx]) {
                    list[kIdx].r = r;
                    list[kIdx].c = c;
                    list[kIdx].coordStr = `${r}, ${c}`;
                }
                chord.keyList = list;
                pickingChordTarget = null;
                renderChordsUI();
                const targetCard = document.getElementById(`chord_card_${cIdx}`);
                if (targetCard) targetCard.scrollIntoView({ behavior: 'smooth', block: 'center' });
            }
            return true;
        }

        function renderChordsUI() {
            const isRu = (currentLanguage !== 'en');
            const container = document.getElementById('chordsList');
            if (!container) return;
            container.innerHTML = '';

            ensureKeycodesDatalist();
            const layers = configState.layers;

            configState.customChords.forEach((chord, cIdx) => {
                const isCollapsed = collapsedChords.has(cIdx);
                const card = document.createElement('div');
                card.id = `chord_card_${cIdx}`;
                card.style.background = 'var(--panel)';
                card.style.border = '1px solid var(--border)';
                card.style.borderRadius = '8px';
                card.style.padding = '12px 14px';
                card.style.display = 'flex';
                card.style.flexDirection = 'column';
                card.style.gap = '10px';

                if (!Array.isArray(chord.targetLayers)) {
                    chord.targetLayers = [0];
                }
                const allLayersSelected = layers.length > 0 && chord.targetLayers.length === layers.length;

                const layerCheckboxesHtml = layers.map((l, lIdx) => {
                    const checked = chord.targetLayers.includes(lIdx);
                    return `
                        <label style="display: inline-flex; align-items: center; gap: 4px; font-size: 0.8rem; cursor: pointer; background: var(--panel); padding: 2px 6px; border-radius: 4px; border: 1px solid var(--border);">
                            <input type="checkbox" ${checked ? 'checked' : ''} onchange="toggleChordLayer(${cIdx}, ${lIdx}, this.checked)">
                            <span>${l} (${lIdx})</span>
                        </label>
                    `;
                }).join('');

                const keyList = parseChordKeys(chord);

                 let keysRowsHtml = '';
                if (!isCollapsed) {
                    keyList.forEach((kItem, kIdx) => {
                        const valStr = kItem.coordStr !== undefined ? kItem.coordStr : `${kItem.r || 0}, ${kItem.c || 0}`;
                        keysRowsHtml += `
                            <div style="display: grid; grid-template-columns: 28px 1fr auto 32px; gap: 8px; align-items: center; background: var(--bg); padding: 5px 10px; border-radius: 6px; border: 1px solid var(--border);">
                                <span style="font-size: 0.75rem; color: var(--text-muted); font-weight: 700; text-align: center;">${kIdx + 1}</span>
                                <input type="text" placeholder="${isRu ? '1, 11 (или 1.11, 1 11...)' : '1, 11 (or 1.11, 1 11...)'}" value="${valStr}" onblur="onChordKeyBlur(${cIdx}, ${kIdx}, this.value)" oninput="onChordKeyInput(${cIdx}, ${kIdx}, this.value)" style="padding: 4px 8px; font-size: 0.85rem; font-family: var(--font-mono);">
                                <button class="btn btn-sm" onclick="pickChordKeyFromLayout(${cIdx}, ${kIdx})" style="padding: 4px 8px; font-size: 0.75rem; white-space: nowrap;">${isRu ? 'Выбрать из раскладки' : 'Pick from layout'}</button>
                                <button onclick="removeChordKey(${cIdx}, ${kIdx})" style="background: none; border: none; color: var(--danger); cursor: pointer; padding: 2px; font-weight: 700; font-size: 0.9rem;" title="${isRu ? 'Удалить кнопку' : 'Delete key'}">✕</button>
                            </div>
                        `;
                    });
                }

                card.innerHTML = `
                    <div style="display: flex; align-items: center; justify-content: space-between; gap: 8px;">
                        <div style="display: flex; align-items: center; gap: 8px; flex-wrap: wrap; flex: 1;">
                            <button class="btn btn-sm" onclick="toggleChordCollapse(${cIdx})" style="padding: 2px 6px; font-size: 0.75rem; line-height: 1;">${isCollapsed ? '►' : '▼'}</button>
                            <span style="font-family: var(--font-mono); font-weight: 700; font-size: 0.9rem; color: var(--accent); background: var(--bg); padding: 3px 8px; border-radius: 4px; border: 1px solid var(--border);">${isRu ? 'Аккорд' : 'Chord'} #${cIdx + 1}</span>
                            <div style="display: flex; align-items: center; gap: 6px; flex: 1; min-width: 280px;">
                                <label style="font-size: 0.75rem; color: var(--text-muted); font-weight: 600; white-space: nowrap;">${isRu ? 'Действие:' : 'Action:'}</label>
                                <input type="text" list="allKeycodesList" placeholder="K_ESC, LC(K_C), MO(1), do_custom..." value="${chord.action || ''}" onchange="const resolved = resolveKeycodeFromSearch(this.value); if (resolved) { this.value = resolved; } configState.customChords[${cIdx}].action = this.value;" oninput="const resolved = resolveKeycodeFromSearch(this.value); if (resolved && (SYMBOL_KEY_MAP[this.value.trim()] || KEY_SEARCH_ALIASES[this.value.trim().toUpperCase()])) { this.value = resolved; } configState.customChords[${cIdx}].action = this.value;" style="flex: 1; padding: 4px 8px; font-size: 0.85rem; font-family: var(--font-mono);">
                            </div>
                        </div>
                        <div style="display: flex; align-items: center; gap: 6px;">
                            <button class="btn btn-sm btn-danger" onclick="removeChord(${cIdx})">${ICONS.trash}</button>
                        </div>
                    </div>

                    <!-- Collapsible Layers & Keys Section -->
                    <div style="display: ${isCollapsed ? 'none' : 'flex'}; flex-direction: column; gap: 10px; margin-top: 4px;">
                        <!-- Multi-Layer Checkbox Selector -->
                        <div style="display: flex; flex-direction: column; gap: 6px; background: var(--bg); padding: 8px 10px; border-radius: 6px; border: 1px solid var(--border);">
                            <div style="display: flex; align-items: center; justify-content: space-between;">
                                <span style="font-size: 0.75rem; color: var(--text-muted); font-weight: 600;">${isRu ? 'Активен на слоях:' : 'Active on layers:'}</span>
                                <label style="display: inline-flex; align-items: center; gap: 4px; font-size: 0.75rem; cursor: pointer; color: var(--accent); font-weight: 600;">
                                    <input type="checkbox" ${allLayersSelected ? 'checked' : ''} onchange="toggleChordAllLayers(${cIdx}, this.checked)">
                                    <span>${isRu ? 'Все слои' : 'All layers'}</span>
                                </label>
                            </div>
                            <div style="display: flex; gap: 6px; flex-wrap: wrap;">
                                ${layerCheckboxesHtml}
                            </div>
                        </div>

                        <!-- Keys In Chord List -->
                        <div style="display: flex; flex-direction: column; gap: 6px;">
                            <div style="display: grid; grid-template-columns: 28px 1fr auto 32px; gap: 8px; padding: 0 10px; font-size: 0.75rem; color: var(--text-muted); font-weight: 600;">
                                <span style="text-align: center;">#</span>
                                <span>${isRu ? 'Координаты строки и колонки (Row, Col)' : 'Row and Col Coordinates (Row, Col)'}</span>
                                <span></span>
                                <span></span>
                            </div>
                            <div style="display: flex; flex-direction: column; gap: 6px;">
                                ${keysRowsHtml || `<div style="font-size: 0.8rem; color: var(--text-muted); padding: 6px; text-align: center;">${isRu ? 'Нет клавиш. Нажмите «+ Добавить».' : 'No keys. Click "+ Add".'}</div>`}
                            </div>
                            <div>
                                <button class="btn btn-sm" onclick="addChordKey(${cIdx})" style="padding: 4px 10px; font-size: 0.8rem;">${isRu ? '+ Добавить' : '+ Add'}</button>
                            </div>
                        </div>
                    </div>
                `;
                container.appendChild(card);
            });
        }

        function toggleChordLayer(cIdx, layerIdx, isChecked) {
            const chord = configState.customChords[cIdx];
            if (!Array.isArray(chord.targetLayers)) chord.targetLayers = [0];
            if (isChecked) {
                if (!chord.targetLayers.includes(layerIdx)) chord.targetLayers.push(layerIdx);
            } else {
                chord.targetLayers = chord.targetLayers.filter(x => x !== layerIdx);
            }
            renderChordsUI();
        }

        function toggleChordAllLayers(cIdx, isChecked) {
            const chord = configState.customChords[cIdx];
            if (isChecked) {
                chord.targetLayers = configState.layers.map((_, i) => i);
            } else {
                chord.targetLayers = [0];
            }
            renderChordsUI();
        }

        function onChordKeyInput(cIdx, kIdx, rawVal) {
            const chord = configState.customChords[cIdx];
            const list = parseChordKeys(chord);
            if (list[kIdx]) {
                list[kIdx].coordStr = rawVal;
                const parsed = parseFlexibleCoordinate(rawVal);
                list[kIdx].r = parsed.r;
                list[kIdx].c = parsed.c;
            }
            chord.keyList = list;
        }

        function onChordKeyBlur(cIdx, kIdx, rawVal) {
            const chord = configState.customChords[cIdx];
            const list = parseChordKeys(chord);
            if (list[kIdx]) {
                const parsed = parseFlexibleCoordinate(rawVal);
                list[kIdx].r = parsed.r;
                list[kIdx].c = parsed.c;
                list[kIdx].coordStr = parsed.formatted;
            }
            chord.keyList = list;
            renderChordsUI();
        }

        function addChordKey(cIdx) {
            const chord = configState.customChords[cIdx];
            const list = parseChordKeys(chord);
            list.push({ r: 0, c: 0, coordStr: '' });
            chord.keyList = list;
            renderChordsUI();
        }

        function removeChordKey(cIdx, kIdx) {
            const chord = configState.customChords[cIdx];
            const list = parseChordKeys(chord);
            list.splice(kIdx, 1);
            chord.keyList = list;
            renderChordsUI();
        }

        function addChord() {
            const idx = configState.customChords.length;
            configState.customChords.push({
                action: "",
                targetLayers: [0],
                keyList: [],
                comment: ""
            });
            renderChordsUI();
        }

        function removeChord(idx) {
            collapsedChords.delete(idx);
            configState.customChords.splice(idx, 1);
            renderChordsUI();
        }

        function ensureKeycodesDatalist() {
            if (!document.getElementById('allKeycodesList')) {
                const dl = document.createElement('datalist');
                dl.id = 'allKeycodesList';
                
                const seen = new Set();
                const addOpt = (val, label) => {
                    if (!val || seen.has(val)) return;
                    seen.add(val);
                    const opt = document.createElement('option');
                    opt.value = val;
                    if (label && label !== val) {
                        opt.label = label;
                    }
                    dl.appendChild(opt);
                };

                // Add basic & common keys
                ALL_PALETTE_CODES.BASIC.forEach(k => {
                    const disp = (window.DISPLAY_MAP && window.DISPLAY_MAP[k]) ? window.DISPLAY_MAP[k] : '';
                    const aliases = KEY_SEARCH_ALIASES[k] ? KEY_SEARCH_ALIASES[k].slice(0, 2).join(', ') : '';
                    const labelDesc = [disp, aliases].filter(Boolean).join(' - ');
                    addOpt(k, labelDesc || k);
                });
                ALL_PALETTE_CODES.MODS.forEach(k => { if (!k.includes('...')) addOpt(k); });
                ALL_PALETTE_CODES.MEDIA.forEach(k => addOpt(k));

                // Add shifted symbol keycodes directly from SYMBOL_KEY_MAP
                Object.entries(SYMBOL_KEY_MAP).forEach(([sym, kc]) => addOpt(kc, `${sym} (${kc})`));

                // Add modifier combinations & One-Shot modifiers
                ["LC(K_C)", "LC(K_V)", "LC(K_Z)", "LC(K_A)", "LC(K_X)", "LS(K_A)", "LA(K_TAB)", "LG(K_SPC)", "LG(LS(K_S))", "HYPER(K_A)", "MEH(K_A)", "OS_SHIFT", "OS_CTRL", "OS_ALT", "OS_GUI", "OS_HYPER"].forEach(k => addOpt(k));

                // Add layer shifters (DMK C macros)
                configState.layers.forEach((l, i) => {
                    addOpt(`MO(${l})`, `Слой ${l} (пока зажата)`);
                    addOpt(`TG(${l})`, `Слой ${l} (переключатель)`);
                    addOpt(`OS(${l})`, `Слой ${l} (One-Shot на 1 клавишу)`);
                    addOpt(`TO(${l})`, `Слой ${l} (активировать)`);
                    addOpt(`HT(${l}, K_SPC)`, `Hold-Tap: зажатие ${l} / тап Space`);
                });

                // Add custom macros
                if (configState.customMacros) {
                    configState.customMacros.forEach((m, i) => {
                        addOpt(`M(${i})`, m.name || `Макрос ${i}`);
                    });
                }

                document.body.appendChild(dl);
            }
        }

        // Track collapsed states for layer trigger macros
        const collapsedLayerMacros = new Set();

        function toggleLayerMacroCollapse(idx) {
            if (collapsedLayerMacros.has(idx)) {
                collapsedLayerMacros.delete(idx);
            } else {
                collapsedLayerMacros.add(idx);
            }
            renderLayerHotkeysUI();
        }

        function toggleLayerHotkeyOptions() {
            const en = document.getElementById('enableLayerHotkeys').checked;
            document.getElementById('layerHotkeysContainer').style.display = en ? 'flex' : 'none';
            if (en) renderLayerHotkeysUI();
        }

        function parseLayerMacroSteps(layerName) {
            if (!configState.layerTriggerMacros) configState.layerTriggerMacros = {};
            return (configState.layerTriggerMacros[layerName] ||= []);
        }

        function renderLayerHotkeysUI() {
            const isRu = (currentLanguage !== 'en');
            const container = document.getElementById('layerHotkeysContainer');
            if (!container) return;
            container.innerHTML = '';

            ensureKeycodesDatalist();

            configState.layers.forEach((l, idx) => {
                const isCollapsed = collapsedLayerMacros.has(idx);
                const card = document.createElement('div');
                card.style.background = 'var(--panel)';
                card.style.border = '1px solid var(--border)';
                card.style.borderRadius = '8px';
                card.style.padding = '12px 14px';
                card.style.display = 'flex';
                card.style.flexDirection = 'column';
                card.style.gap = '10px';

                const stepsArray = parseLayerMacroSteps(l);

                let stepsRowsHtml = '';
                if (!isCollapsed) {
                    stepsArray.forEach((st, sIdx) => {
                        stepsRowsHtml += renderMacroStepRowHtml(
                            st,
                            sIdx,
                            `onLayerMacroStepActionChange('${l}', ${sIdx}, this.value)`,
                            `onLayerMacroStepKeyChange('${l}', ${sIdx}, this.value)`,
                            `removeLayerMacroStep('${l}', ${sIdx})`,
                            `const res = resolveKeycodeFromSearch(this.value); if (res) { this.value = res; onLayerMacroStepKeyChange('${l}', ${sIdx}, res); }`
                        );
                    });
                }

                card.innerHTML = `
                    <div style="display: flex; align-items: center; justify-content: space-between; gap: 8px;">
                        <div style="display: flex; align-items: center; gap: 8px; flex-wrap: wrap; flex: 1;">
                            <button class="btn btn-sm" onclick="toggleLayerMacroCollapse(${idx})" style="padding: 2px 6px; font-size: 0.75rem; line-height: 1;">${isCollapsed ? '►' : '▼'}</button>
                            <span style="font-family: var(--font-mono); font-weight: 700; font-size: 0.9rem; color: var(--accent); background: var(--bg); padding: 3px 8px; border-radius: 4px; border: 1px solid var(--border);">${l} (${idx})</span>
                        </div>
                    </div>

                    <!-- Steps List -->
                    <div style="display: ${isCollapsed ? 'none' : 'flex'}; flex-direction: column; gap: 6px;">
                        <div style="display: grid; grid-template-columns: 28px 130px 1fr 32px; gap: 8px; padding: 0 10px; font-size: 0.75rem; color: var(--text-muted); font-weight: 600;">
                            <span style="text-align: center;">#</span>
                            <span>${isRu ? 'Действие' : 'Action'}</span>
                            <span>${isRu ? 'Кнопка / Значение' : 'Key / Value'}</span>
                            <span></span>
                        </div>
                        <div style="display: flex; flex-direction: column; gap: 6px;">
                            ${stepsRowsHtml || `<div style="font-size: 0.8rem; color: var(--text-muted); padding: 6px; text-align: center;">${isRu ? 'Нет действий. Нажмите «+ Добавить».' : 'No actions. Click "+ Add".'}</div>`}
                        </div>
                        <div>
                            <button class="btn btn-sm" onclick="addLayerMacroStep('${l}')" style="padding: 4px 10px; font-size: 0.8rem;">${isRu ? '+ Добавить' : '+ Add'}</button>
                        </div>
                    </div>
                `;
                container.appendChild(card);
            });
        }

        function onLayerMacroStepActionChange(layerName, sIdx, actionType) {
            const steps = parseLayerMacroSteps(layerName);
            if (steps[sIdx]) {
                steps[sIdx].type = actionType;
                if (actionType === 'DELAY') {
                    steps[sIdx].delay = steps[sIdx].delay || 100;
                } else {
                    steps[sIdx].key = steps[sIdx].key || '';
                }
            }
            renderLayerHotkeysUI();
        }

        function onLayerMacroStepKeyChange(layerName, sIdx, keyVal) {
            const steps = parseLayerMacroSteps(layerName);
            if (steps[sIdx]) {
                const resolved = resolveKeycodeFromSearch(keyVal);
                steps[sIdx].key = resolved || keyVal;
            }
        }

        function onLayerMacroStepDelayChange(layerName, sIdx, delayVal) {
            const steps = parseLayerMacroSteps(layerName);
            if (steps[sIdx]) {
                steps[sIdx].delay = parseInt(delayVal, 10) || 100;
            }
        }

        function addLayerMacroStep(layerName) {
            const steps = parseLayerMacroSteps(layerName);
            steps.push({ type: 'TAP', key: '' });
            renderLayerHotkeysUI();
        }

        function removeLayerMacroStep(layerName, sIdx) {
            const steps = parseLayerMacroSteps(layerName);
            steps.splice(sIdx, 1);
            renderLayerHotkeysUI();
        }

        function getActiveLayoutPairs() {
            const { totalRows, totalCols } = getParsedPins();
            ensureActiveKeys();

            const pairs = [];
            for (let r = 0; r < totalRows; r++) {
                for (let c = 0; c < totalCols; c++) {
                    if (configState.activeKeys[r] && configState.activeKeys[r][c]) {
                        pairs.push({ r, c });
                    }
                }
            }
            return pairs;
        }

        function initKeymapsIfEmpty() {
            const pairs = getActiveLayoutPairs();
            if (!configState.keyGeometry) configState.keyGeometry = [];
            if (!configState.keyGeomByCoord) configState.keyGeomByCoord = {};
            if (!configState.keymapByCoord) configState.keymapByCoord = {};

            // Synchronize geometry by coordinate
            pairs.forEach((p, idx) => {
                const coordKey = `${p.r},${p.c}`;
                if (configState.keyGeomByCoord[coordKey]) {
                    configState.keyGeometry[idx] = { ...configState.keyGeomByCoord[coordKey] };
                } else if (configState.keyGeometry[idx]) {
                    configState.keyGeomByCoord[coordKey] = { ...configState.keyGeometry[idx] };
                } else {
                    const defaultGeom = { w: 1, h: 1, x: 0, y: 0 };
                    configState.keyGeometry[idx] = defaultGeom;
                    configState.keyGeomByCoord[coordKey] = defaultGeom;
                }
            });
            configState.keyGeometry.length = pairs.length;

            configState.layers.forEach(l => {
                if (!configState.keymaps[l]) configState.keymaps[l] = [];
                if (!configState.keymapByCoord[l]) configState.keymapByCoord[l] = {};

                const newLayerKeys = [];
                pairs.forEach((p, idx) => {
                    const coordKey = `${p.r},${p.c}`;
                    if (configState.keymapByCoord[l][coordKey]) {
                        newLayerKeys.push(configState.keymapByCoord[l][coordKey]);
                    } else if (configState.keymaps[l][idx]) {
                        newLayerKeys.push(configState.keymaps[l][idx]);
                        configState.keymapByCoord[l][coordKey] = configState.keymaps[l][idx];
                    } else {
                        const defKey = (l === 'DEF') ? (DEFAULT_ALPHA_PRESET[idx] || "K_TRNS") : "K_TRNS";
                        newLayerKeys.push(defKey);
                        configState.keymapByCoord[l][coordKey] = defKey;
                    }
                });
                configState.keymaps[l] = newLayerKeys;
            });
        }

        const DEFAULT_ALPHA_PRESET = [
            "K_TAB", "K_Q", "K_W", "K_E", "K_R", "K_T", "K_Y", "K_U", "K_I", "K_O", "K_P", "K_BSPC",
            "K_LCTRL", "K_A", "K_S", "K_D", "K_F", "K_G", "K_H", "K_J", "K_K", "K_L", "K_SCLN", "K_QUOT",
            "K_LSHIFT", "K_Z", "K_X", "K_C", "K_V", "K_B", "K_N", "K_M", "K_COMM", "K_DOT", "K_SLSH", "K_ENT",
            "K_LALT", "K_LGUI", "K_SPC", "K_SPC", "K_RGUI", "K_RALT"
        ];

        function renderVisualKeymap() {
            initKeymapsIfEmpty();
            const container = document.getElementById('visualKeymapContainer');
            container.innerHTML = '';

            const { totalRows, totalCols } = getParsedPins();
            const enableEnc = document.getElementById('enableEncoders')?.checked;
            const encCount = parseInt(document.getElementById('encCount')?.value, 10) || 0;
            const curLayer = configState.currentLayer || 'DEF';

            const UNIT_SIZE = 52;
            const GAP = 8;
            const STEP = UNIT_SIZE + GAP; // 60px

            const visualElements = [];

            // 1. Encoders (Row 0 of visual layout if enabled)
            if (enableEnc && encCount > 0) {
                if (!configState.encoderKeymaps) configState.encoderKeymaps = {};
                if (!configState.encoderKeymaps[curLayer]) configState.encoderKeymaps[curLayer] = [];

                for (let e = 0; e < encCount; e++) {
                    const eGeom = (configState.encoderGeometry && configState.encoderGeometry[e]) ? configState.encoderGeometry[e] : { x: 0, y: 0 };
                    const curAct = configState.encoderKeymaps[curLayer][e] || (curLayer === 'DEF' ? { cw: "K_VOLU", ccw: "K_VOLD" } : { cw: "K_TRNS", ccw: "K_TRNS" });

                    const baseLeft = (e * 2) * STEP + (eGeom.x || 0) * STEP;
                    const baseTop = -1 * STEP + (eGeom.y || 0) * STEP; // Placed 1 row above matrix

                    // CCW button (0, e)
                    const isCcwSel = isElementSelected('enc', e, 'ccw');
                    const ccwLabel = (window.DISPLAY_MAP && window.DISPLAY_MAP[curAct.ccw]) ? window.DISPLAY_MAP[curAct.ccw] : (curAct.ccw || 'TRNS').replace(/^K_/, '');
visualElements.push({
                        type: 'enc',
                        id: e,
                        dir: 'ccw',
                        coordText: `E${e+1} ↺`,
                        label: ccwLabel,
                        isSel: isCcwSel,
                        left: baseLeft,
                        top: baseTop,
                        w: UNIT_SIZE,
                        h: UNIT_SIZE,
                        onClick: (evt) => handleItemClick(evt, 'enc', e, 'ccw')
                    });

                    // CW button (1, e)
                    const isCwSel = isElementSelected('enc', e, 'cw');
                    const cwLabel = (window.DISPLAY_MAP && window.DISPLAY_MAP[curAct.cw]) ? window.DISPLAY_MAP[curAct.cw] : (curAct.cw || 'TRNS').replace(/^K_/, '');
                    visualElements.push({
                        type: 'enc',
                        id: e,
                        dir: 'cw',
                        coordText: `E${e+1} ↻`,
                        label: cwLabel,
                        isSel: isCwSel,
                        left: baseLeft + STEP,
                        top: baseTop,
                        w: UNIT_SIZE,
                        h: UNIT_SIZE,
                        onClick: (evt) => handleItemClick(evt, 'enc', e, 'cw')
                    });
                }
            }

            // 2. Matrix Keys
            let keySeqIndex = 0;
            for (let r = 0; r < totalRows; r++) {
                for (let c = 0; c < totalCols; c++) {
                    const isActive = configState.activeKeys[r] && configState.activeKeys[r][c];
                    if (!isActive) continue;

                    const curIdx = keySeqIndex;
                    const keycode = (configState.keymaps[configState.currentLayer] && configState.keymaps[configState.currentLayer][curIdx]) ? configState.keymaps[configState.currentLayer][curIdx] : "K_TRNS";
                    const label = (window.DISPLAY_MAP && window.DISPLAY_MAP[keycode]) ? window.DISPLAY_MAP[keycode] : keycode.replace(/^K_/, '');
                    const geom = configState.keyGeometry[curIdx] || { w: 1, h: 1, x: 0, y: 0 };
                    const isSel = isElementSelected('key', curIdx);

                    const widthPx = Math.round(UNIT_SIZE * geom.w + (geom.w - 1) * GAP);
                    const heightPx = Math.round(UNIT_SIZE * geom.h + (geom.h - 1) * GAP);

                    const leftPx = c * STEP + (geom.x || 0) * STEP;
                    const topPx = r * STEP + (geom.y || 0) * STEP;

                    visualElements.push({
                        type: 'key',
                        id: curIdx,
                        coordText: `${r},${c}${geom.w > 1 ? ` (${geom.w}U)` : ''}`,
                        label: label,
                        isSel: isSel,
                        left: leftPx,
                        top: topPx,
                        w: widthPx,
                        h: heightPx,
                        onClick: (evt) => handleItemClick(evt, 'key', curIdx, { r, c })
                    });
                    keySeqIndex++;
                }
            }

            if (visualElements.length === 0) return;

            // Calculate precise bounding box of all elements
            let minLeft = Infinity, maxRight = -Infinity, minTop = Infinity, maxBottom = -Infinity;
            visualElements.forEach(el => {
                if (el.left < minLeft) minLeft = el.left;
                if (el.left + el.w > maxRight) maxRight = el.left + el.w;
                if (el.top < minTop) minTop = el.top;
                if (el.top + el.h > maxBottom) maxBottom = el.top + el.h;
            });

            const PADDING = 20;
            const canvasWidth = Math.round(maxRight - minLeft + PADDING * 2);
            const canvasHeight = Math.round(maxBottom - minTop + PADDING * 2);

            const canvas = document.createElement('div');
            canvas.style.position = 'relative';
            canvas.style.width = `${canvasWidth}px`;
            canvas.style.height = `${canvasHeight}px`;
            canvas.style.flexShrink = '0';

            const frag = document.createDocumentFragment();
            visualElements.forEach(el => {
                const keyEl = document.createElement('div');
                keyEl.className = `visual-key${el.isSel ? ' selected' : ''}`;
                keyEl.style.left = `${el.left - minLeft + PADDING}px`;
                keyEl.style.top = `${el.top - minTop + PADDING}px`;
                keyEl.style.width = `${el.w}px`;
                keyEl.style.height = `${el.h}px`;

                const fontSize = el.label.length > 8 ? '0.65rem' : '0.78rem';
                keyEl.innerHTML = `
                    <span class="coord">${el.coordText}</span>
                    <span class="idx" style="font-size: ${fontSize};">${el.label}</span>
                `;
                keyEl.onclick = el.onClick;
                frag.appendChild(keyEl);
            });
            canvas.appendChild(frag);
            container.appendChild(canvas);
        }

        function toggleTheme() {
            document.body.classList.toggle('light-theme');
            const isLight = document.body.classList.contains('light-theme');
            localStorage.setItem('dmk_theme', isLight ? 'light' : 'dark');
            applyLanguage();
            if (configState.step === 4) initMatrixGrid();
            if (configState.step === 5) renderVisualKeymap();
        }

        function toggleLanguage() {
            currentLanguage = (currentLanguage === 'ru') ? 'en' : 'ru';
            localStorage.setItem('dmk_lang', currentLanguage);
            applyLanguage();
        }

        function applyLanguage() {
            const t = I18N[currentLanguage];
            const isLight = document.body.classList.contains('light-theme');
            
            // Header buttons with SVG icons
            const themeBtn = document.getElementById('btnThemeToggle');
            if (themeBtn) themeBtn.innerHTML = `${ICONS.theme} ${isLight ? t.themeLight : t.themeDark}`;
            const langBtn = document.getElementById('btnLangToggle');
            if (langBtn) langBtn.innerHTML = `${ICONS.globe} ${t.langToggle}`;
            const docsLink = document.getElementById('btnDocsLink');
            if (docsLink) {
                docsLink.href = (currentLanguage === 'ru')
                    ? 'https://github.com/aroum/dmk/tree/main/docs/ru'
                    : 'https://github.com/aroum/dmk/tree/main/docs/en';
                docsLink.title = (currentLanguage === 'ru')
                    ? 'Документация по прошивке DMK на GitHub'
                    : 'DMK Firmware Documentation on GitHub';
            }

            // Search input placeholder
            if (document.getElementById('paletteSearchInput')) {
                document.getElementById('paletteSearchInput').placeholder = t.paletteSearch;
            }

            // Universal data-i18n text replacement for all elements
            document.querySelectorAll('[data-i18n]').forEach(el => {
                const key = el.getAttribute('data-i18n');
                if (t[key]) {
                    el.textContent = t[key];
                }
            });

            // Universal data-i18n-ph placeholder replacement
            document.querySelectorAll('[data-i18n-ph]').forEach(el => {
                const key = el.getAttribute('data-i18n-ph');
                if (t[key]) {
                    el.placeholder = t[key];
                }
            });

            // Stepper items
            const stepNames = [t.step1, t.step2, t.step3, t.step4, t.step5, t.step6];
            document.querySelectorAll('.step-item').forEach((el, idx) => {
                const span = el.querySelectorAll('span')[1];
                if (span && stepNames[idx]) span.textContent = stepNames[idx];
            });

            // Navigation and Action Buttons with Icons
            [
                ['#step-1 .nav-actions button.btn-primary', `${t.btnNextPins} ${ICONS.arrowRight}`],
                ['#step-2 .nav-actions button:first-child', `${ICONS.arrowLeft} ${t.btnBack}`],
                ['#step-2 .nav-actions button.btn-primary', `${t.btnNextLighting} ${ICONS.arrowRight}`],
                ['#step-3 .nav-actions button:first-child', `${ICONS.arrowLeft} ${t.btnBack}`],
                ['#step-3 .nav-actions button.btn-primary', `${t.btnNextGrid} ${ICONS.arrowRight}`],
                ['#step-4 .nav-actions button:first-child', `${ICONS.arrowLeft} ${t.btnBack}`],
                ['#step-4 .nav-actions button.btn-primary', `${t.btnNextLayout} ${ICONS.arrowRight}`],
                ['#step-5 .nav-actions button:first-child', `${ICONS.arrowLeft} ${t.btnBack}`],
                ['#step-5 .nav-actions button.btn-primary', `${t.btnGenerateFiles} ${ICONS.arrowRight}`],
                ['#step-6 .nav-actions button:first-child', `${ICONS.arrowLeft} ${t.btnBackSettings}`],
                ['#step-4 button[onclick="selectAllGridKeys(true)"]', `${ICONS.check} ${t.btnSelectAll}`],
                ['#step-4 button[onclick="selectAllGridKeys(false)"]', `${ICONS.trash} ${t.btnClearAll}`],
                ['#btnUndo', `${ICONS.undo} ${t.btnUndo || 'Undo'}`],
                ['#btnRedo', `${ICONS.redo} ${t.btnRedo || 'Redo'}`],
                ['#step-5 button[onclick="addCustomLayer()"]', `${ICONS.plus} ${t.btnAddLayer}`],
                ['#step-5 button[onclick="duplicateCurrentLayer()"]', `${ICONS.copy} ${t.btnDupLayer}`],
                ['#step-5 button[onclick="clearCurrentLayer()"]', `${ICONS.trash} ${t.btnClearLayer}`],
                ['#step-5 button[onclick="renameCurrentLayer()"]', `${ICONS.edit} ${t.btnRenameLayer}`],
                ['#step-5 button[onclick="removeCurrentLayer()"]', `${ICONS.trash} ${t.btnRemoveLayer}`],
                ['button[onclick="copyConfigCode()"]', `${ICONS.copy} ${t.copyConfig}`],
                ['button[onclick="downloadConfigFile()"]', `${ICONS.download} ${t.downloadConfig}`],
                ['button[onclick="copyVialJson()"]', `${ICONS.copy} ${t.copyVial}`],
                ['button[onclick="downloadVialJson()"]', `${ICONS.download} ${t.downloadVial}`]
            ].forEach(([sel, html]) => {
                const el = document.querySelector(sel);
                if (el) el.innerHTML = html;
            });

            // Refresh Inspector Key Coordinate text based on current selection
            const inspectorEl = document.getElementById('inspectorKeyCoord');
            if (inspectorEl) {
                if (configState.selectedKeyIndex === null) {
                    inspectorEl.textContent = t.inspectorKeyPrompt;
                } else {
                    const pairs = getActiveLayoutPairs();
                    if (pairs[configState.selectedKeyIndex]) {
                        const { r, c } = pairs[configState.selectedKeyIndex];
                        inspectorEl.textContent = `${t.keySelectedPrefix || 'Кнопка #'}${configState.selectedKeyIndex + 1} — ${t.keyMatrixCoord || 'Матрица'} {${r}, ${c}}`;
                    }
                }
            }

            // Modal buttons
            const modalCancelBtn = document.getElementById('btnAppModalCancel');
            if (modalCancelBtn) modalCancelBtn.textContent = t.btnCancel || (currentLanguage === 'en' ? 'Cancel' : 'Отмена');
            const modalConfirmBtn = document.getElementById('btnAppModalConfirm');
            if (modalConfirmBtn) modalConfirmBtn.textContent = t.btnApply || (currentLanguage === 'en' ? 'Apply' : 'Применить');

            // Re-render dynamic sub-panels if visible
            if (document.getElementById('enableCustomMacros')?.checked) {
                renderCustomMacrosUI();
            }
            if (document.getElementById('enableChords')?.checked) {
                renderChordsUI();
            }
            if (document.getElementById('enableLayerHotkeys')?.checked) {
                renderLayerHotkeysUI();
            }
            updateJsonExportLabels();
            updateSplitPinsUI();
            if (typeof checkPinConflicts === 'function') {
                checkPinConflicts();
            }
        }

        // Apply saved theme & lang on startup
        if (localStorage.getItem('dmk_theme') === 'light') {
            document.body.classList.add('light-theme');
        }
        if (localStorage.getItem('dmk_lang') === 'en') {
            currentLanguage = 'en';
        }

        // Multi-selection state: array of { type: 'key'|'enc', id: number, dir?: 'ccw'|'cw', r?: number, c?: number }
        configState.selectedItems = [];
        let lastVisualClickedItem = null;

        function isElementSelected(type, id, dir) {
            return configState.selectedItems.some(item => item.type === type && item.id === id && (type !== 'enc' || item.dir === dir));
        }

        function handleItemClick(evt, type, id, extra) {
            // Check if picking key coordinate for chords
            if (type === 'key' && extra && onMatrixKeySelectedForChord(extra.r, extra.c)) {
                return;
            }

            const isShift = evt && (evt.shiftKey);
            const isCtrl = evt && (evt.ctrlKey || evt.metaKey) && !isShift;

            if (isShift && lastVisualClickedItem && lastVisualClickedItem.type === 'key' && type === 'key' && extra) {
                const pairs = getActiveLayoutPairs();
                const anchor = lastVisualClickedItem;

                const anchorGeom = (configState.keyGeometry && configState.keyGeometry[anchor.id]) || { x: 0, y: 0, w: 1, h: 1 };
                const targetGeom = (configState.keyGeometry && configState.keyGeometry[id]) || { x: 0, y: 0, w: 1, h: 1 };

                const anchorVx = anchor.c + (anchorGeom.x || 0);
                const anchorVy = anchor.r + (anchorGeom.y || 0);
                const targetVx = extra.c + (targetGeom.x || 0);
                const targetVy = extra.r + (targetGeom.y || 0);

                const minVx = Math.min(anchorVx, targetVx) - 0.1;
                const maxVx = Math.max(anchorVx, targetVx) + 0.1;
                const minVy = Math.min(anchorVy, targetVy) - 0.1;
                const maxVy = Math.max(anchorVy, targetVy) + 0.1;

                const minR = Math.min(anchor.r, extra.r);
                const maxR = Math.max(anchor.r, extra.r);
                const minC = Math.min(anchor.c, extra.c);
                const maxC = Math.max(anchor.c, extra.c);
                const minId = Math.min(anchor.id, id);
                const maxId = Math.max(anchor.id, id);

                const sameVisualRow = Math.abs(anchorVy - targetVy) < 0.35;
                const sameMatrixRow = (anchor.r === extra.r);

                let selectedKeys = [];

                if (sameVisualRow) {
                    // Select all keys on this visual row between anchor and target
                    pairs.forEach((p, idx) => {
                        const g = (configState.keyGeometry && configState.keyGeometry[idx]) || { x: 0, y: 0 };
                        const vx = p.c + (g.x || 0);
                        const vy = p.r + (g.y || 0);
                        if (Math.abs(vy - anchorVy) < 0.35 && vx >= minVx && vx <= maxVx) {
                            selectedKeys.push({ type: 'key', id: idx, r: p.r, c: p.c });
                        }
                    });
                } else if (sameMatrixRow) {
                    // Select all keys in this matrix row between minC and maxC
                    pairs.forEach((p, idx) => {
                        if (p.r === anchor.r && p.c >= minC && p.c <= maxC) {
                            selectedKeys.push({ type: 'key', id: idx, r: p.r, c: p.c });
                        }
                    });
                } else {
                    // 2D bounding box (visual and matrix)
                    pairs.forEach((p, idx) => {
                        const g = (configState.keyGeometry && configState.keyGeometry[idx]) || { x: 0, y: 0 };
                        const vx = p.c + (g.x || 0);
                        const vy = p.r + (g.y || 0);
                        const inVisualBox = (vx >= minVx && vx <= maxVx && vy >= minVy && vy <= maxVy);
                        const inMatrixBox = (p.r >= minR && p.r <= maxR && p.c >= minC && p.c <= maxC);
                        if (inVisualBox || inMatrixBox) {
                            selectedKeys.push({ type: 'key', id: idx, r: p.r, c: p.c });
                        }
                    });
                }

                // Fallback to linear index if anchor or target is missing
                if (!selectedKeys.some(k => k.id === id) || !selectedKeys.some(k => k.id === anchor.id)) {
                    selectedKeys = [];
                    pairs.forEach((p, idx) => {
                        if (idx >= minId && idx <= maxId) {
                            selectedKeys.push({ type: 'key', id: idx, r: p.r, c: p.c });
                        }
                    });
                }

                const seen = new Set();
                configState.selectedItems = selectedKeys.filter(k => {
                    if (seen.has(k.id)) return false;
                    seen.add(k.id);
                    return true;
                });
            } else if (isCtrl) {
                const existsIdx = configState.selectedItems.findIndex(item => item.type === type && item.id === id && (type !== 'enc' || item.dir === extra));
                if (existsIdx !== -1) {
                    configState.selectedItems.splice(existsIdx, 1);
                } else {
                    configState.selectedItems.push({
                        type,
                        id,
                        dir: type === 'enc' ? extra : undefined,
                        r: type === 'key' ? extra.r : undefined,
                        c: type === 'key' ? extra.c : undefined
                    });
                }
                if (type === 'key' && extra) {
                    lastVisualClickedItem = { type, id, r: extra.r, c: extra.c };
                }
            } else {
                configState.selectedItems = [{
                    type,
                    id,
                    dir: type === 'enc' ? extra : undefined,
                    r: type === 'key' ? extra.r : undefined,
                    c: type === 'key' ? extra.c : undefined
                }];
                if (type === 'key' && extra) {
                    lastVisualClickedItem = { type, id, r: extra.r, c: extra.c };
                } else {
                    lastVisualClickedItem = null;
                }
            }

            // Sync legacy state
            if (configState.selectedItems.length === 1) {
                const first = configState.selectedItems[0];
                if (first.type === 'enc') {
                    configState.selectedEncoder = { idx: first.id, dir: first.dir };
                    configState.selectedKeyIndex = null;
                } else {
                    configState.selectedKeyIndex = first.id;
                    configState.selectedEncoder = null;
                }
            } else {
                configState.selectedKeyIndex = null;
                configState.selectedEncoder = null;
            }

            updateInspectorUI();
            renderVisualKeymap();
        }

        function updateInspectorUI() {
            const t = I18N[currentLanguage];
            const coordEl = document.getElementById('inspectorKeyCoord');
            const wInput = document.getElementById('propKeyW');
            const hInput = document.getElementById('propKeyH');
            const xInput = document.getElementById('propKeyX');
            const yInput = document.getElementById('propKeyY');
            const wGroup = document.getElementById('propKeyWGroup');
            const hGroup = document.getElementById('propKeyHGroup');

            if (configState.selectedItems.length === 0) {
                coordEl.textContent = t.inspectorKeyPrompt || '[Кликните по клавише]';
                if (wInput) wInput.value = 1;
                if (hInput) hInput.value = 1;
                if (xInput) xInput.value = 0;
                if (yInput) yInput.value = 0;
                return;
            }

            if (configState.selectedItems.length > 1) {
                coordEl.textContent = currentLanguage === 'en' ? `Selected ${configState.selectedItems.length} items (Arrow keys to move)` : `Выбрано клавиш: ${configState.selectedItems.length} (Стрелки для сдвига)`;
                if (wGroup) wGroup.style.opacity = '1';
                if (hGroup) hGroup.style.opacity = '1';
                if (wInput) {
                    const firstKey = configState.selectedItems.find(i => i.type === 'key');
                    const geom = (firstKey && configState.keyGeometry && configState.keyGeometry[firstKey.id]) ? configState.keyGeometry[firstKey.id] : { w: 1, h: 1 };
                    wInput.value = geom.w || 1;
                    wInput.disabled = false;
                }
                if (hInput) {
                    const firstKey = configState.selectedItems.find(i => i.type === 'key');
                    const geom = (firstKey && configState.keyGeometry && configState.keyGeometry[firstKey.id]) ? configState.keyGeometry[firstKey.id] : { w: 1, h: 1 };
                    hInput.value = geom.h || 1;
                    hInput.disabled = false;
                }
                if (xInput) { xInput.value = 0; }
                if (yInput) { yInput.value = 0; }
                return;
            }

            const item = configState.selectedItems[0];
            if (item.type === 'enc') {
                const dirText = (item.dir === 'ccw') ? (t.encDirCcw || 'Влево / CCW') : (t.encDirCw || 'Вправо / CW');
                coordEl.textContent = `${t.encInspectorPrefix || 'Энкодер'} #${item.id + 1} (${dirText})`;
                if (wInput) { wInput.value = 1; wInput.disabled = true; }
                if (hInput) { hInput.value = 1; hInput.disabled = true; }
                if (wGroup) wGroup.style.opacity = '0.4';
                if (hGroup) hGroup.style.opacity = '0.4';

                if (!configState.encoderGeometry) configState.encoderGeometry = [];
                const geom = configState.encoderGeometry[item.id] || { x: 0, y: 0 };
                if (xInput) xInput.value = geom.x || 0;
                if (yInput) yInput.value = geom.y || 0;
            } else {
                coordEl.textContent = `${t.keySelectedPrefix || 'Кнопка #'}${item.id + 1} — ${t.keyMatrixCoord || 'Матрица'} {${item.r}, ${item.c}}`;
                const geom = configState.keyGeometry[item.id] || { w: 1, h: 1, x: 0, y: 0 };
                if (wInput) { wInput.value = geom.w; wInput.disabled = false; }
                if (hInput) { hInput.value = geom.h; hInput.disabled = false; }
                if (wGroup) wGroup.style.opacity = '1';
                if (hGroup) hGroup.style.opacity = '1';
                if (xInput) xInput.value = geom.x || 0;
                if (yInput) yInput.value = geom.y || 0;
            }
        }

        function selectEncoderForInspector(encIdx, dir) {
            handleItemClick(null, 'enc', encIdx, dir);
        }

        function selectKeyForInspector(idx, r, c) {
            handleItemClick(null, 'key', idx, { r, c });
        }

        function moveSelectedItemsBy(dx, dy) {
            if (!configState.selectedItems || configState.selectedItems.length === 0) return;
            markDirty();

            configState.selectedItems.forEach(item => {
                if (item.type === 'enc') {
                    if (!configState.encoderGeometry) configState.encoderGeometry = [];
                    if (!configState.encoderGeometry[item.id]) configState.encoderGeometry[item.id] = { x: 0, y: 0 };
                    configState.encoderGeometry[item.id].x = Math.round(((configState.encoderGeometry[item.id].x || 0) + dx) * 100) / 100;
                    configState.encoderGeometry[item.id].y = Math.round(((configState.encoderGeometry[item.id].y || 0) + dy) * 100) / 100;
                } else if (item.type === 'key') {
                    if (!configState.keyGeometry) configState.keyGeometry = [];
                    if (!configState.keyGeometry[item.id]) configState.keyGeometry[item.id] = { w: 1, h: 1, x: 0, y: 0 };
                    configState.keyGeometry[item.id].x = Math.round(((configState.keyGeometry[item.id].x || 0) + dx) * 100) / 100;
                    configState.keyGeometry[item.id].y = Math.round(((configState.keyGeometry[item.id].y || 0) + dy) * 100) / 100;
                    if (!configState.keyGeomByCoord) configState.keyGeomByCoord = {};
                    if (item.r !== undefined && item.c !== undefined) {
                        configState.keyGeomByCoord[`${item.r},${item.c}`] = { ...configState.keyGeometry[item.id] };
                    }
                }
            });

            updateInspectorUI();
            renderVisualKeymap();
        }

        function updateSelectedKeyProperty(prop, value) {
            const num = parseFloat(value);
            const val = isNaN(num) ? 0 : num;

            if (!configState.selectedItems || configState.selectedItems.length === 0) return;

            configState.selectedItems.forEach(item => {
                if (item.type === 'enc') {
                    if (prop !== 'x' && prop !== 'y') return;
                    if (!configState.encoderGeometry) configState.encoderGeometry = [];
                    if (!configState.encoderGeometry[item.id]) configState.encoderGeometry[item.id] = { x: 0, y: 0 };
                    configState.encoderGeometry[item.id][prop] = val;
                } else if (item.type === 'key') {
                    if (!configState.keyGeometry) configState.keyGeometry = [];
                    if (!configState.keyGeometry[item.id]) configState.keyGeometry[item.id] = { w: 1, h: 1, x: 0, y: 0 };
                    configState.keyGeometry[item.id][prop] = val;
                    if (!configState.keyGeomByCoord) configState.keyGeomByCoord = {};
                    if (item.r !== undefined && item.c !== undefined) {
                        configState.keyGeomByCoord[`${item.r},${item.c}`] = { ...configState.keyGeometry[item.id] };
                    }
                }
            });

            renderVisualKeymap();
        }

        const KEY_SEARCH_ALIASES = {
            "K_BSPC": ["BACKSPACE", "BSPC", "БЕКСПЕЙС", "БЭКСПЕЙС", "СТЕРЕТЬ", "УДАЛИТЬ", "BACK"],
            "K_ENT":  ["ENTER", "RETURN", "ENT", "ЕНТЕР", "ВВОД"],
            "K_SPC":  ["SPACE", "SPACEBAR", "SPC", "ПРОБЕЛ"],
            "K_ESC":  ["ESCAPE", "ESC", "ЭСКЕЙП", "ОТМЕНА"],
            "K_TAB":  ["TAB", "ТАБУЛЯЦИЯ", "ТАБ"],
            "K_CAPS": ["CAPS", "CAPSLOCK", "КАПС"],
            "K_DEL":  ["DELETE", "DEL", "УДАЛИТЬ"],
            "K_INS":  ["INSERT", "INS", "ВСТАВИТЬ"],
            "K_HOME": ["HOME", "В НАЧАЛО", "ХОУМ"],
            "K_END":  ["END", "В КОНЕЦ", "ЭНД"],
            "K_PGUP": ["PAGE UP", "PGUP", "СТРАНИЦА ВВЕРХ"],
            "K_PGDN": ["PAGE DOWN", "PGDN", "СТРАНИЦА ВНИЗ"],
            "K_UP":   ["UP", "ARROW UP", "СТРЕЛКА ВВЕРХ", "ВВЕРХ"],
            "K_DOWN": ["DOWN", "ARROW DOWN", "СТРЕЛКА ВНИЗ", "ВНИЗ"],
            "K_LEFT": ["LEFT", "ARROW LEFT", "СТРЕЛКА ВЛЕВО", "ВЛЕВО"],
            "K_RIGHT":["RIGHT", "ARROW RIGHT", "СТРЕЛКА ВПРАВО", "ВПРАВО"],
            "K_LBRC": ["LBRC", "[", "{", "Х", "СКОБКА"],
            "K_RBRC": ["RBRC", "]", "}", "Ъ", "СКОБКА"],
            "K_BSLS": ["BSLS", "BACKSLASH", "\\", "|", "СЛЭШ"],
            "K_SLSH": ["SLSH", "SLASH", "/", "?", "ДЕЛЕНИЕ"],
            "K_MINS": ["MINS", "MINUS", "-", "_", "МИНУС", "ТИРЕ"],
            "K_EQL":  ["EQL", "EQUAL", "=", "+", "РАВНО", "ПЛЮС"],
            "K_SCLN": ["SCLN", "SEMICOLON", ";", ":", "Ж"],
            "K_QUOT": ["QUOT", "QUOTE", "'", "\"", "Э", "КАВЫЧКА"],
            "K_GRV":  ["GRV", "GRAVE", "`", "~", "Ё", "ТИЛЬДА"],
            "K_COMM": ["COMM", "COMMA", ",", "<", "Б", "ЗАПЯТАЯ"],
            "K_DOT":  ["DOT", "PERIOD", ".", ">", "Ю", "ТОЧКА"],
            "K_LCTRL": ["LCTRL", "CTRL", "КОНТРОЛ", "ЛЕВЫЙ КОНТРОЛ"],
            "K_RCTRL": ["RCTRL", "ПРАВЫЙ КОНТРОЛ"],
            "K_LSHIFT":["LSHIFT", "SHIFT", "ШИФТ", "ЛЕВЫЙ ШИФТ"],
            "K_RSHIFT":["RSHIFT", "ПРАВЫЙ ШИФТ"],
            "K_LALT":  ["LALT", "ALT", "OPTION", "АЛЬТ", "ЛЕВЫЙ АЛЬТ"],
            "K_RALT":  ["RALT", "RIGHT ALT", "ALTGR", "ПРАВЫЙ АЛЬТ"],
            "K_LGUI":  ["LGUI", "GUI", "WIN", "WINDOWS", "CMD", "COMMAND", "ВИН", "КОМАНД"],
            "K_RGUI":  ["RGUI", "RIGHT GUI", "RIGHT WIN", "RIGHT CMD"],
            "K_VOLU":  ["VOLU", "VOLUME UP", "ГРОМКОСТЬ ВВЕРХ", "ГРОМЧЕ", "ЗВУК +"],
            "K_VOLD":  ["VOLD", "VOLUME DOWN", "ГРОМКОСТЬ ВНИЗ", "ТИШЕ", "ЗВУК -"],
            "K_MUTE":  ["MUTE", "БЕЗ ЗВУКА", "МУТ", "ТИШИНА"],
            "K_MPLY":  ["MPLY", "PLAY", "PAUSE", "ПЛЕЙ", "ПАУЗА"],
            "K_MNXT":  ["MNXT", "NEXT TRACK", "СЛЕДУЮЩИЙ ТРЕК"],
            "K_MPRV":  ["MPRV", "PREV TRACK", "ПРЕДЫДУЩИЙ ТРЕК"]
        };

        const ALL_PALETTE_CODES = {
            BASIC: [
                "K_A","K_B","K_C","K_D","K_E","K_F","K_G","K_H","K_I","K_J","K_K","K_L","K_M",
                "K_N","K_O","K_P","K_Q","K_R","K_S","K_T","K_U","K_V","K_W","K_X","K_Y","K_Z",
                "K_1","K_2","K_3","K_4","K_5","K_6","K_7","K_8","K_9","K_0",
                "K_ENT","K_ESC","K_BSPC","K_TAB","K_SPC","K_MINS","K_EQL","K_LBRC","K_RBRC",
                "K_BSLS","K_SCLN","K_QUOT","K_GRV","K_COMM","K_DOT","K_SLSH","K_CAPS",
                "K_NONUS_HASH","K_NONUS_BSLASH",
                ...Array.from({length: 24}, (_, i) => `K_F${i + 1}`)
            ],
            NUMPAD: [
                "K_NLCK","K_KP_SLASH","K_KP_ASTERISK","K_KP_MINUS","K_KP_PLUS","K_KP_ENTER","K_KP_DOT","K_KP_EQUAL",
                "K_KP_COMMA","K_KP_EQUAL_AS400",
                ...Array.from({length: 10}, (_, i) => `K_KP${i}`)
            ],
            MODS: [
                // Modified Keys & Wrappers
                "LS(...)", "LC(...)", "LA(...)", "LG(...)",
                "RS(...)", "RC(...)", "RA(...)", "RG(...)",
                "HYPER(...)", "MEH(...)",
                // Unified One-Shot (OSM & Layers)
                "OS(...)",
                // Universal Hold-Tap (Mod-Tap / Layer-Tap)
                "HT(...)",
                // Standard modifier keys
                "K_LCTRL","K_LSHIFT","K_LALT","K_LGUI","K_RCTRL","K_RSHIFT","K_RALT","K_RGUI",
                "K_LOCKING_CAPS","K_LOCKING_NUM","K_LOCKING_SCROLL",
                "K_UP","K_DOWN","K_LEFT","K_RIGHT","K_HOME","K_END","K_PGUP","K_PGDN","K_DEL","K_INS","K_PSCR","K_SCRL","K_PAUS","K_MENU"
            ],
            MEDIA: [
                "K_VOLU","K_VOLD","K_MUTE","K_MPLY","K_MNXT","K_MPRV","K_MSTP",
                "K_MFFD","K_MRWD","K_EJCT","K_MSEL","K_BRIU","K_BRID","K_CALC","K_MYCM",
                "K_MAIL","K_WSCH","K_WHOM","K_WBAK","K_WFWD","K_WSTP","K_WREF","K_WFAV",
                "K_EXECUTE","K_HELP","K_MENU_KB","K_SELECT","K_STOP_KB","K_AGAIN","K_UNDO","K_CUT","K_COPY","K_PASTE","K_FIND",
                // RGB Backlight
                "RGB_TOGG","RGB_NEXT","RGB_PREV","RGB_HUI","RGB_HUD","RGB_SAI","RGB_SAD","RGB_VAI","RGB_VAD","RGB_SPI","RGB_SPD"
            ],
            MOUSE: [
                "K_MS_UP","K_MS_DOWN","K_MS_LEFT","K_MS_RIGHT",
                "K_MS_BTN1","K_MS_BTN2","K_MS_BTN3","K_MS_BTN4","K_MS_BTN5",
                "K_MS_WH_UP","K_MS_WH_DOWN","K_MS_WH_LEFT","K_MS_WH_RIGHT",
                "K_MS_ACCEL0","K_MS_ACCEL1","K_MS_ACCEL2"
            ],
            GAMEPAD: [
                "GP_A","GP_B","GP_X","GP_Y",
                "GP_LB","GP_RB","GP_LT","GP_RT",
                "GP_SELECT","GP_START","GP_L3","GP_R3",
                "GP_DPAD_UP","GP_DPAD_DOWN","GP_DPAD_LEFT","GP_DPAD_RIGHT",
                "GP_LX_LEFT","GP_LX_RIGHT","GP_LY_UP","GP_LY_DOWN",
                "GP_RX_LEFT","GP_RX_RIGHT","GP_RY_UP","GP_RY_DOWN",
                ...Array.from({length: 32}, (_, i) => `GP_BTN${i + 1}`)
            ],
            MIDI: [
                // Interactive CC modal configurators
                "CC X",
                "CC_TOG(...)",
                "CC_VAL127(...)",
                "CC_VAL0(...)",
                "CC_INC(...)",
                "CC_DEC(...)",
                // Transport, Octave, Transpose & Velocity
                "MIDI_OCTAVE_DOWN","MIDI_OCTAVE_UP",
                "MIDI_TRANSPOSE_DOWN","MIDI_TRANSPOSE_UP",
                "MIDI_VELOCITY_DOWN","MIDI_VELOCITY_UP",
                "MIDI_CHANNEL_DOWN","MIDI_CHANNEL_UP",
                "MIDI_ALL_NOTES_OFF",
                "MIDI_SUSTAIN","MIDI_PORTAMENTO","MIDI_SOSTENUTO",
                "MIDI_SOFT","MIDI_LEGATO","MIDI_MODULATION",
                "MIDI_PITCH_BEND_DOWN","MIDI_PITCH_BEND_UP",
                // Chromatic Notes C1-B5 generated dynamically
                ...[1, 2, 3, 4, 5].flatMap(oct => 
                    ["C", "C_SHARP", "D", "D_SHARP", "E", "F", "F_SHARP", "G", "G_SHARP", "A", "A_SHARP", "B"]
                        .map(n => `MIDI_NOTE_${n}_${oct}`)
                )
            ],
            LAYERS: [
                "MO(...)", "TG(...)", "OS(...)", "TO(...)",
                "K_LYRUP","K_LYRDWN",
                ...Array.from({length: 9}, (_, i) => `K_INT${i + 1}`),
                ...Array.from({length: 9}, (_, i) => `K_LANG${i + 1}`),
                "K_TRNS","K_NO","K_BOOTLOADER"
            ]
        };

        const KLE_104_DATA = [
            ["Esc", {x: 1}, "F1", "F2", "F3", "F4", {x: 0.5}, "F5", "F6", "F7", "F8", {x: 0.5}, "F9", "F10", "F11", "F12", {x: 0.25}, "PrtSc", "ScrLk", "Pause", {x: 0.25}, "Calc", "Mute", "Vol-", "Vol+"],
            [{x: 2}, "F13", "F14", "F15", "F16", {x: 0.5}, "F17", "F18", "F19", "F20", {x: 0.5}, "F21", "F22", "F23", "F24"],
            [{y: 0.5}, "~\n`", "!\n1", "@\n2", "#\n3", "$\n4", "%\n5", "^\n6", "&\n7", "*\n8", "(\n9", ")\n0", "_\n-", "+\n=", {w: 2}, "Backspace", {x: 0.25}, "Ins", "Home", "PgUp", {x: 0.25}, "Num\nLock", "/", "*", "-"],
            [{w: 1.5}, "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{\n[", "}\n]", {w: 1.5}, "|\n\\", {x: 0.25}, "Del", "End", "PgDn", {x: 0.25}, "7", "8", "9", {h: 2}, "+"],
            [{w: 1.75}, "Caps Lock", "A", "S", "D", "F", "G", "H", "J", "K", "L", ":\n;", "\"\n'", {w: 2.25}, "Enter", {x: 3.5}, "4", "5", "6"],
            [{w: 2.25}, "Shift", "Z", "X", "C", "V", "B", "N", "M", "<\n,", ">\n.", "?\n/", {w: 2.75, code: "K_RSHIFT"}, "Shift", {x: 1.25}, "↑", {x: 1.25}, "1", "2", "3", {h: 2, code: "K_KP_ENTER"}, "↵"],
            [{w: 1.25}, "Ctrl", {w: 1.25}, "Win", {w: 1.25}, "Alt", {a: 7, w: 6.25, code: "K_SPC"}, "Space", {a: 4, w: 1.25, code: "K_RALT"}, "Alt", {w: 1.25, code: "K_RGUI"}, "Win", {w: 1.25}, "Menu", {w: 1.25, code: "K_RCTRL"}, "Ctrl", {x: 0.25}, "←", "↓", "→", {x: 0.25, w: 2, code: "K_KP0"}, "0", {code: "K_KP_DOT"}, "."]
        ];

        const KLE_SPECIAL_MAP = {
            "Esc": "K_ESC", "Tab": "K_TAB", "Enter": "K_ENT", "Backspace": "K_BSPC", "Space": "K_SPC", "": "K_SPC",
            "Ctrl": "K_LCTRL", "Shift": "K_LSHIFT", "Alt": "K_LALT", "Win": "K_LGUI", "Menu": "K_MENU",
            "Caps Lock": "K_CAPS", "PrtSc": "K_PSCR", "ScrLk": "K_SCRL", "Pause": "K_PAUS", "Pause\nBreak": "K_PAUS",
            "Ins": "K_INS", "Del": "K_DEL", "Home": "K_HOME", "End": "K_END", "PgUp": "K_PGUP", "PgDn": "K_PGDN",
            "←": "K_LEFT", "→": "K_RIGHT", "↑": "K_UP", "↓": "K_DOWN",
            "Calc": "K_CALC", "Mute": "K_MUTE", "Vol-": "K_VOLD", "Vol+": "K_VOLU",
            "Num\nLock": "K_NLCK", "Num Lock": "K_NLCK", "/": "K_KP_SLASH", "*": "K_KP_ASTERISK", "-": "K_KP_MINUS", "+": "K_KP_PLUS", "↵": "K_KP_ENTER", ".": "K_KP_DOT",
            "0": "K_KP0", "1": "K_KP1", "2": "K_KP2", "3": "K_KP3", "4": "K_KP4", "5": "K_KP5", "6": "K_KP6", "7": "K_KP7", "8": "K_KP8", "9": "K_KP9",
            "{\n[": "K_LBRC", "}\n]": "K_RBRC", "|\n\\": "K_BSLS", ":\n;": "K_SCLN", "\"\n'": "K_QUOT", "<\n,": "K_COMM", ">\n.": "K_DOT", "?\n/": "K_SLSH", "_\n-": "K_MINS", "+\n=": "K_EQL", "~\n`": "K_GRV"
        };

        function renderPhysicalBasicKeyboard(container) {
            const PITCH = 38;
            const GAP = 4;

            let current_x = 0;
            let current_y = 0;
            let current_w = 1;
            let current_h = 1;
            let current_code = null;

            const parsedKeys = [];

            for (let r = 0; r < KLE_104_DATA.length; r++) {
                const row = KLE_104_DATA[r];
                current_x = 0;
                for (let i = 0; i < row.length; i++) {
                    const item = row[i];
                    if (typeof item === 'object' && item !== null) {
                        if (item.x !== undefined) current_x += item.x;
                        if (item.y !== undefined) current_y += item.y;
                        if (item.w !== undefined) current_w = item.w;
                        if (item.h !== undefined) current_h = item.h;
                        if (item.code !== undefined) current_code = item.code;
                    } else if (typeof item === 'string') {
                        let finalCode = current_code;
                        if (!finalCode) {
                            if (KLE_SPECIAL_MAP[item]) finalCode = KLE_SPECIAL_MAP[item];
                            else if (item.includes('\n')) finalCode = `K_${item.split('\n')[1]}`;
                            else if (/^[A-Z0-9]$/i.test(item)) finalCode = `K_${item.toUpperCase()}`;
                            else if (item.startsWith('F') && parseInt(item.slice(1), 10)) finalCode = `K_${item}`;
                            else finalCode = `K_${item.toUpperCase().replace(/\s+/g, '_')}`;
                        }

                        parsedKeys.push({
                            label: item,
                            x: current_x,
                            y: current_y,
                            w: current_w,
                            h: current_h,
                            code: finalCode
                        });

                        current_x += current_w;
                        current_w = 1;
                        current_h = 1;
                        current_code = null;
                    }
                }
                current_y += 1;
            }

            let maxX = 0;
            let maxY = 0;
            parsedKeys.forEach(k => {
                if (k.x + k.w > maxX) maxX = k.x + k.w;
                if (k.y + k.h > maxY) maxY = k.y + k.h;
            });

            const canvas = document.createElement('div');
            canvas.style.position = 'relative';
            canvas.style.width = `${Math.round(maxX * PITCH)}px`;
            canvas.style.height = `${Math.round(maxY * PITCH)}px`;
            canvas.style.userSelect = 'none';

            const frag = document.createDocumentFragment();

            parsedKeys.forEach(k => {
                const btn = document.createElement('button');
                const isSpecial = (k.w > 1 || k.h > 1 || ['K_ESC','K_TAB','K_CAPS','K_ENT','K_LSHIFT','K_RSHIFT','K_LCTRL','K_RCTRL','K_LALT','K_RALT','K_LGUI','K_RGUI','K_SPC','K_NLCK','K_KP_ENTER'].includes(k.code));
                btn.className = 'palette-key' + (isSpecial ? ' special' : '');
                btn.style.position = 'absolute';
                btn.style.left = `${Math.round(k.x * PITCH)}px`;
                btn.style.top = `${Math.round(k.y * PITCH)}px`;
                btn.style.width = `${Math.round(k.w * PITCH - GAP)}px`;
                btn.style.height = `${Math.round(k.h * PITCH - GAP)}px`;
                btn.title = k.code;

                if (k.label.includes('\n')) {
                    const parts = k.label.split('\n');
                    btn.innerHTML = `<span class="sub-lbl" style="font-size: 0.60rem; line-height: 1; color: var(--text-muted);">${parts[0]}</span><span style="font-size: 0.72rem; font-weight: 600; line-height: 1;">${parts[1]}</span>`;
                } else {
                    btn.innerHTML = `<span style="font-size: 0.72rem; font-weight: 600; line-height: 1;">${k.label}</span>`;
                }

                btn.onclick = () => assignKeycode(k.code);
                frag.appendChild(btn);
            });

            canvas.appendChild(frag);
            container.appendChild(canvas);
        }

        function filterPalette(searchQuery) {
            renderPalette(currentPaletteCategory, searchQuery);
        }

        function renderPalette(category, filterText) {
            currentPaletteCategory = category || 'ALL';
            ['ALL', 'BASIC', 'NUMPAD', 'MODS', 'MOUSE', 'GAMEPAD', 'MIDI', 'MEDIA', 'LAYERS', 'MACROS'].forEach(cat => {
                const tab = document.getElementById(`tab-pal-${cat}`);
                if (tab) tab.classList.toggle('active', cat === currentPaletteCategory);
            });

            const container = document.getElementById('quickPaletteContainer');
            container.innerHTML = '';
            const t = I18N[currentLanguage];

            const rawQuery = (filterText !== undefined ? filterText : (document.getElementById('paletteSearchInput')?.value || '')).trim();

            // If BASIC tab is active and there is no search query, render full physical 104 keyboard layout
            if (currentPaletteCategory === 'BASIC' && !rawQuery) {
                renderPhysicalBasicKeyboard(container);
                return;
            }

            let codes = [];
            if (currentPaletteCategory === 'ALL') {
                codes = [
                    ...ALL_PALETTE_CODES.BASIC,
                    ...ALL_PALETTE_CODES.NUMPAD,
                    ...ALL_PALETTE_CODES.MODS,
                    ...(configState.enableMouse !== false ? ALL_PALETTE_CODES.MOUSE : []),
                    ...(configState.enableGamepad !== false ? ALL_PALETTE_CODES.GAMEPAD : []),
                    ...(configState.enableMidi !== false ? ALL_PALETTE_CODES.MIDI : []),
                    ...ALL_PALETTE_CODES.MEDIA,
                    ...ALL_PALETTE_CODES.LAYERS
                ];
            } else if (currentPaletteCategory === 'MACROS') {
                codes = [];
                if (configState.customMacros && configState.customMacros.length > 0) {
                    configState.customMacros.forEach((m, idx) => {
                        codes.push(`M(${idx})`);
                    });
                } else {
                    codes.push('M(0)');
                }
            } else if (ALL_PALETTE_CODES[currentPaletteCategory]) {
                codes = [...ALL_PALETTE_CODES[currentPaletteCategory]];
            }

            if (currentPaletteCategory === 'ALL' || currentPaletteCategory === 'LAYERS') {
                // Custom user macro keys if defined
                if (configState.customMacros && configState.customMacros.length > 0) {
                    configState.customMacros.forEach((m, idx) => {
                        codes.push(`M(${idx})`);
                    });
                }
            }

            const query = rawQuery.toUpperCase();
            const symbolResolved = (typeof SYMBOL_KEY_MAP !== 'undefined' && SYMBOL_KEY_MAP[rawQuery]) ? SYMBOL_KEY_MAP[rawQuery] : null;

            const filtered = codes.filter(code => {
                if (!query) return true;

                // 0. Smart symbol map match (e.g. searching ">", "<", "+", "&", "#", "-", "=")
                if (symbolResolved && (code === symbolResolved || symbolResolved.includes(code))) return true;
                
                // 1. Direct code check (e.g. K_BSPC, K_ENT, K_1, LS(...))
                if (code.toUpperCase().includes(query)) return true;

                // 2. Display label check (e.g. Backspace, Enter, Esc, [, ], \)
                const label = (window.DISPLAY_MAP && window.DISPLAY_MAP[code]) ? window.DISPLAY_MAP[code].toUpperCase() : '';
                if (label.includes(query) || (rawQuery && label.includes(rawQuery))) return true;

                // 3. Aliases dictionary check (e.g. "бекспейс", "ентер", "пробел", "скобка")
                const aliases = KEY_SEARCH_ALIASES[code];
                if (aliases) {
                    return aliases.some(alias => alias.includes(query) || alias.includes(rawQuery.toUpperCase()));
                }

                // 4. Custom macro name or description match
                if (code.startsWith('M(')) {
                    const mIdx = parseInt(code.slice(2, -1), 10);
                    const m = configState.customMacros && configState.customMacros[mIdx];
                    if (m) {
                        if (m.name && m.name.toUpperCase().includes(query)) return true;
                        if (m.desc && m.desc.toUpperCase().includes(query)) return true;
                    }
                }

                return false;
            });

            if (filtered.length === 0) {
                container.innerHTML = `<div style="font-size: 0.8rem; color: var(--text-muted); padding: 8px;">${currentLanguage === 'en' ? 'No keycodes found.' : 'Ничего не найдено.'}</div>`;
                return;
            }

            const fragment = document.createDocumentFragment();
            filtered.forEach(code => {
                const btn = document.createElement('button');
                btn.className = 'btn btn-sm';
                let label = (window.DISPLAY_MAP && window.DISPLAY_MAP[code]) ? window.DISPLAY_MAP[code] : code.replace(/^K_/, '');
                if (code.startsWith('M(')) {
                    const mIdx = parseInt(code.slice(2, -1), 10);
                    const m = configState.customMacros && configState.customMacros[mIdx];
                    if (m && m.name && m.name !== `MC_MACRO_${mIdx}`) {
                        label = m.name;
                    }
                    btn.title = (m && m.desc) ? `${code}: ${m.name || code} (${m.desc})` : (m && m.name ? `${code}: ${m.name}` : code);
                    btn.style.borderColor = 'var(--accent)';
                    btn.style.fontWeight = '600';
                } else if (code.includes('(...)') || code === 'CC X' || code.startsWith('CC_')) {
                    btn.style.borderColor = 'var(--accent)';
                    btn.style.color = 'var(--accent)';
                }
                btn.textContent = label;
                if (!btn.title) btn.title = code;
                btn.onclick = () => assignKeycode(code);
                fragment.appendChild(btn);
            });
            container.appendChild(fragment);
        }

        function assignKeycode(code) {
            const t = I18N[currentLanguage];
            const curLayer = configState.currentLayer || 'DEF';

            // 1. Check if this is a One-Shot trigger: OS(...) or OSL(...)
            if (code === 'OS(...)' || code === 'OS' || code === 'OSL(...)' || code === 'OSL') {
                promptOneShotChoice((chosenCode) => {
                    executeAssignKeycode(chosenCode);
                });
                return;
            }

            // 2. Check if this is a layer modifier: MO, TG, TO
            const layerModMatch = code.match(/^(MO|TG|TO)(\((.*)\))?$/);
            if (layerModMatch && (!layerModMatch[3] || layerModMatch[3] === '...')) {
                const fn = layerModMatch[1];
                promptLayerModifierChoice(fn, (chosenCode) => {
                    executeAssignKeycode(chosenCode);
                });
                return;
            }

            // 3. Check if this is an interactive MIDI CC trigger: CC X, CC(...), CC, CC_TOG(...), etc.
            if (code === 'CC X' || code === 'CC(...)' || code === 'CC' || code.startsWith('CC_')) {
                promptMidiCcChoice(code, (chosenCode) => {
                    executeAssignKeycode(chosenCode);
                });
                return;
            }

            // 4. Check if this is an interactive compound modifier: LS(...), LC(...), LA(...), LG(...), HYPER(...), MEH(...), HT(...)
            if (code.includes('(...)')) {
                promptKeyChoice(code, (chosenCode) => {
                    executeAssignKeycode(chosenCode);
                });
                return;
            }

            executeAssignKeycode(code);
        }

        function executeAssignKeycode(code) {
            const t = I18N[currentLanguage];
            const curLayer = configState.currentLayer || 'DEF';

            saveUndoState();

            if (configState.selectedEncoder) {
                const { idx, dir } = configState.selectedEncoder;
                if (!configState.encoderKeymaps) configState.encoderKeymaps = {};
                if (!configState.encoderKeymaps[curLayer]) configState.encoderKeymaps[curLayer] = [];
                if (!configState.encoderKeymaps[curLayer][idx]) configState.encoderKeymaps[curLayer][idx] = { cw: "K_TRNS", ccw: "K_TRNS" };
                configState.encoderKeymaps[curLayer][idx][dir] = code;
                
                const inputEl = document.getElementById(`layer_enc_${idx}_${dir}`);
                if (inputEl) inputEl.value = code;

                renderVisualKeymap();
                return;
            }

            if (configState.selectedItems && configState.selectedItems.length > 0) {
                if (!configState.keymaps[curLayer]) {
                    configState.keymaps[curLayer] = [];
                }
                if (!configState.keymapByCoord) configState.keymapByCoord = {};
                if (!configState.keymapByCoord[curLayer]) configState.keymapByCoord[curLayer] = {};

                const pairs = getActiveLayoutPairs();
                configState.selectedItems.forEach(item => {
                    if (item.type === 'key') {
                        configState.keymaps[curLayer][item.id] = code;
                        const pair = (item.r !== undefined && item.c !== undefined) ? item : pairs[item.id];
                        if (pair) {
                            configState.keymapByCoord[curLayer][`${pair.r},${pair.c}`] = code;
                        }
                    } else if (item.type === 'enc') {
                        if (!configState.encoderKeymaps) configState.encoderKeymaps = {};
                        if (!configState.encoderKeymaps[curLayer]) configState.encoderKeymaps[curLayer] = [];
                        if (!configState.encoderKeymaps[curLayer][item.id]) configState.encoderKeymaps[curLayer][item.id] = { cw: "K_TRNS", ccw: "K_TRNS" };
                        configState.encoderKeymaps[curLayer][item.id][item.dir] = code;
                    }
                });

                // Auto advance if single item selected
                if (configState.selectedItems.length === 1 && configState.selectedItems[0].type === 'key') {
                    const total = pairs.length;
                    const nextId = (configState.selectedItems[0].id + 1) % total;
                    if (pairs[nextId]) {
                        selectKeyForInspector(nextId, pairs[nextId].r, pairs[nextId].c);
                    } else {
                        renderVisualKeymap();
                    }
                } else {
                    renderVisualKeymap();
                }
                return;
            }

            if (configState.selectedKeyIndex === null) {
                alert(t.alertSelectKeyFirst || "Сначала нажмите на кнопку в раскладке выше!");
                return;
            }
            if (!configState.keymaps[curLayer]) {
                configState.keymaps[curLayer] = [];
            }
            if (!configState.keymapByCoord) configState.keymapByCoord = {};
            if (!configState.keymapByCoord[curLayer]) configState.keymapByCoord[curLayer] = {};

            const pairs = getActiveLayoutPairs();
            configState.keymaps[curLayer][configState.selectedKeyIndex] = code;
            if (pairs[configState.selectedKeyIndex]) {
                const pair = pairs[configState.selectedKeyIndex];
                configState.keymapByCoord[curLayer][`${pair.r},${pair.c}`] = code;
            }
            
            const total = pairs.length;
            configState.selectedKeyIndex = (configState.selectedKeyIndex + 1) % total;
            if (pairs[configState.selectedKeyIndex]) {
                selectKeyForInspector(configState.selectedKeyIndex, pairs[configState.selectedKeyIndex].r, pairs[configState.selectedKeyIndex].c);
            } else {
                renderVisualKeymap();
            }
        }

        let lastMatrixClickedKey = null;

        function handleMatrixKeyClick(evt, r, c) {
            const isShift = evt && (evt.shiftKey || evt.ctrlKey || evt.metaKey);

            if (isShift && lastMatrixClickedKey !== null) {
                // Determine target state based on the last clicked key's current state (or default to true)
                const targetState = configState.activeKeys[lastMatrixClickedKey.r][lastMatrixClickedKey.c];
                const minR = Math.min(lastMatrixClickedKey.r, r);
                const maxR = Math.max(lastMatrixClickedKey.r, r);
                const minC = Math.min(lastMatrixClickedKey.c, c);
                const maxC = Math.max(lastMatrixClickedKey.c, c);

                for (let row = minR; row <= maxR; row++) {
                    for (let col = minC; col <= maxC; col++) {
                        configState.activeKeys[row][col] = targetState;
                        const el = document.getElementById(`key-${row}-${col}`);
                        if (el) {
                            el.classList.toggle('active', targetState);
                            el.classList.toggle('disabled', !targetState);
                        }
                    }
                }
                updateKeyIndices();
                lastMatrixClickedKey = { r, c };
                return;
            }

            // Normal single click toggle
            toggleMatrixKey(r, c);
            lastMatrixClickedKey = { r, c };
        }

        function toggleMatrixKey(r, c) {
            markDirty();
            configState.activeKeys[r][c] = !configState.activeKeys[r][c];
            const el = document.getElementById(`key-${r}-${c}`);
            if (el) {
                el.classList.toggle('active', configState.activeKeys[r][c]);
                el.classList.toggle('disabled', !configState.activeKeys[r][c]);
            }
            updateKeyIndices();
        }

        function selectAllGridKeys(status) {
            markDirty();
            const { totalRows, totalCols } = getParsedPins();
            for (let r = 0; r < totalRows; r++) {
                for (let c = 0; c < totalCols; c++) {
                    configState.activeKeys[r][c] = status;
                    const el = document.getElementById(`key-${r}-${c}`);
                    if (el) {
                        el.classList.toggle('active', status);
                        el.classList.toggle('disabled', !status);
                    }
                }
            }
            updateKeyIndices();
        }

        function updateKeyIndices() {
            let count = 0;
            const { totalRows, totalCols } = getParsedPins();
            for (let r = 0; r < totalRows; r++) {
                for (let c = 0; c < totalCols; c++) {
                    const isActive = configState.activeKeys[r] && configState.activeKeys[r][c];
                    const el = document.getElementById(`key-${r}-${c}`);
                    if (isActive) {
                        count++;
                        if (el) el.querySelector('.idx').textContent = count;
                    } else {
                        if (el) el.querySelector('.idx').textContent = '—';
                    }
                }
            }
            document.getElementById('totalActiveKeysCount').textContent = count;
        }

        function gotoStep(stepNum) {
            configState.step = stepNum;
            for (let i = 1; i <= 6; i++) {
                const el = document.getElementById(`step-${i}`);
                if (el) el.style.display = (i === stepNum) ? 'flex' : 'none';
            }

            // Stepper classes
            const items = document.querySelectorAll('.step-item');
            items.forEach((item, idx) => {
                const s = idx + 1;
                item.classList.toggle('active', s === stepNum);
                item.classList.toggle('completed', s < stepNum);
            });

            if (stepNum === 2 || stepNum === 3) {
                if (document.getElementById('enableEncoders')?.checked) {
                    updateEncoderInputs();
                }
                checkPinConflicts();
            } else if (stepNum === 4) {
                checkPinConflicts();
                initMatrixGrid();
            } else if (stepNum === 5) {
                const enableMacros = !!document.getElementById('enableCustomMacros')?.checked;
                const tabMacros = document.getElementById('tab-pal-MACROS');
                if (tabMacros) tabMacros.style.display = enableMacros ? '' : 'none';
                renderLayersUI();
                renderVisualKeymap();
                renderPalette('BASIC');
            } else if (stepNum === 6) {
                generateConfigCode();
                generateVialJson();
            }
        }

        function generateConfigModal() {
            gotoStep(6);
        }

        function copyTextWithFeedback(text, filename) {
            navigator.clipboard.writeText(text).then(() => {
                alert(currentLanguage === 'en' ? `${filename} copied to clipboard!` : `${filename} успешно скопирован в буфер обмена!`);
            });
        }

        function copyConfigCode() {
            copyTextWithFeedback(document.getElementById('configCodeOutput').textContent, 'config.h');
        }

        function downloadBlob(filename, content, type = 'text/plain;charset=utf-8;') {
            const blob = new Blob([content], { type });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = filename;
            a.click();
            URL.revokeObjectURL(url);
        }

        function downloadConfigFile() {
            downloadBlob('config.h', document.getElementById('configCodeOutput').textContent, 'text/x-c;charset=utf-8;');
        }

        function copyVialJson() {
            const isVia = (configState.vialProtocol === 'via_v3' || configState.protocolMode === 'via_v3');
            copyTextWithFeedback(document.getElementById('vialJsonOutput').textContent, isVia ? 'via.json' : 'vial.json');
        }

        function downloadVialJson() {
            const isVia = (configState.vialProtocol === 'via_v3' || configState.protocolMode === 'via_v3');
            downloadBlob(isVia ? 'via.json' : 'vial.json', document.getElementById('vialJsonOutput').textContent, 'application/json;charset=utf-8;');
        }



        // Arrow key listener to move selected keys/encoders and Ctrl+Z / Ctrl+Y for Undo/Redo
        window.addEventListener('keydown', (e) => {
            const activeTag = document.activeElement ? document.activeElement.tagName.toLowerCase() : '';
            if (activeTag === 'input' || activeTag === 'textarea' || activeTag === 'select') return;

            const step5 = document.getElementById('step-5');
            if (!step5 || step5.style.display === 'none') return;

            // Ctrl+Z / Cmd+Z (Undo) and Ctrl+Y / Cmd+Y / Ctrl+Shift+Z (Redo)
            if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'z') {
                e.preventDefault();
                if (e.shiftKey) {
                    redoKeymap();
                } else {
                    undoKeymap();
                }
                return;
            }
            if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'y') {
                e.preventDefault();
                redoKeymap();
                return;
            }

            const step = e.shiftKey ? 1.0 : 0.25;

            if (e.key === 'ArrowLeft') {
                e.preventDefault();
                moveSelectedItemsBy(-step, 0);
            } else if (e.key === 'ArrowRight') {
                e.preventDefault();
                moveSelectedItemsBy(step, 0);
            } else if (e.key === 'ArrowUp') {
                e.preventDefault();
                moveSelectedItemsBy(0, -step);
            } else if (e.key === 'ArrowDown') {
                e.preventDefault();
                moveSelectedItemsBy(0, step);
            }
        });

        // Run setup on load
        window.addEventListener('DOMContentLoaded', init);
