// =========================================================================
// Import & Auto-Fill from config.h / vial.json
// =========================================================================

const padPins = (pins, targetCount) => {
    const res = pins.slice(0, targetCount);
    while (res.length < targetCount) res.push(`GPIO${res.length}`);
    return res.join(', ');
};

async function handleConfigFileUpload(e) {
    const file = e.target.files && e.target.files[0];
    if (!file) return;
    try {
        const text = await file.text();
        parseAndApplyConfigH(text, file.name);
    } catch (err) {
        alert("Ошибка чтения config.h: " + err.message);
    } finally {
        e.target.value = '';
    }
}

async function handleVialFileUpload(e) {
    const file = e.target.files && e.target.files[0];
    if (!file) return;
    try {
        const json = JSON.parse(await file.text());
        parseAndApplyVialJson(json, file.name);
    } catch (err) {
        alert("Ошибка чтения JSON файла: " + err.message);
    } finally {
        e.target.value = '';
    }
}

function parseAndApplyConfigH(text, filename) {
    let loadedSomething = false;

    // Capture existing geometry by coordinate (r, c) before any pin or mode changes
    const oldGeomMap = {};
    if (configState.keyGeometry && Array.isArray(configState.keyGeometry)) {
        const oldPairs = getActiveLayoutPairs();
        oldPairs.forEach((p, idx) => {
            if (configState.keyGeometry[idx]) {
                oldGeomMap[`${p.r},${p.c}`] = configState.keyGeometry[idx];
            }
        });
    }

    // 1. Vial definitions
    const vNameMatch = text.match(/#define\s+VIAL_KEYBOARD_NAME\s+"([^"]+)"/);
    if (vNameMatch) {
        document.getElementById('kbName').value = vNameMatch[1];
        document.getElementById('vialName').value = vNameMatch[1];
        loadedSomething = true;
    } else if (filename) {
        const cleanName = filename.replace(/\.(h|c|txt)$/i, '').replace(/_/g, ' ');
        if (cleanName && cleanName.toLowerCase() !== 'config') {
            document.getElementById('kbName').value = cleanName;
            document.getElementById('vialName').value = cleanName;
        }
    }

    const vVidMatch = text.match(/#define\s+VIAL_VENDOR_ID\s+(0x[0-9a-fA-F]+)/);
    if (vVidMatch) document.getElementById('vialVid').value = vVidMatch[1];
    const vPidMatch = text.match(/#define\s+VIAL_PRODUCT_ID\s+(0x[0-9a-fA-F]+)/);
    if (vPidMatch) document.getElementById('vialPid').value = vPidMatch[1];

    const isViaV3 = /#define\s+(?:VIA_V3|USE_VIA_V3)\b/.test(text);
    setProtocolMode(isViaV3 ? 'via_v3' : 'vial');

    // 2. MCU detection (preserve custom pins / don't reset pins with default MCU pins)
    const defMcuMatch = text.match(/#define\s+(?:DEFAULT_MCU|MCU_DEFAULT|MCU)\s+["']?([a-zA-Z0-9_]+)["']?/i);
    const defMcu = defMcuMatch ? defMcuMatch[1].toLowerCase() : null;

    if (text.includes('MCU_milandr') || defMcu === 'milandr') setMcu('milandr', false);
    else if (text.includes('MCU_nrf52840') || defMcu === 'nrf52840') setMcu('nrf52840', false);
    else if (text.includes('MCU_baikal') || defMcu === 'baikal') setMcu('baikal', false);
    else if (text.includes('MCU_rp2040') || text.includes('MCU_rp2350') || defMcu === 'rp2040' || defMcu === 'rp2350') {
        setMcu('rp2040', false);
        setRpDefaultMcu(defMcu === 'rp2350' ? 'rp2350' : 'rp2040');
    }

    // 3. Split vs Single
    const masterMatch = text.match(/#define\s+MASTER_SIDE\s+(LEFT|RIGHT|AUTO)/);
    const serialMatch = text.match(/#define\s+SERIAL_PIN\s+([A-Za-z0-9_]+)/);
    const splitTxMatch = text.match(/#define\s+SPLIT_TX_PIN\s+([A-Za-z0-9_]+)/);
    const splitRxMatch = text.match(/#define\s+SPLIT_RX_PIN\s+([A-Za-z0-9_]+)/);
    const splitConnMatch = text.match(/#define\s+SPLIT_CONNECTION_TYPE\s+([A-Za-z0-9_]+)/);
    if (masterMatch || serialMatch || splitTxMatch || text.includes('NUM_ROWS_SPLIT')) {
        setSplitMode(true);
        if (masterMatch) document.getElementById('masterSide').value = masterMatch[1];
        if (serialMatch) document.getElementById('serialPinRP').value = serialMatch[1];
        if (splitTxMatch) {
            document.getElementById('serialPinRP').value = splitTxMatch[1];
            const txEl = document.getElementById('splitTxPin');
            if (txEl) txEl.value = splitTxMatch[1];
        }
        if (splitRxMatch) {
            const rxEl = document.getElementById('splitRxPin');
            if (rxEl) rxEl.value = splitRxMatch[1];
        }
        if (splitConnMatch) {
            const uartEl = document.getElementById('splitUartMode');
            if (uartEl) uartEl.value = (splitConnMatch[1] === 'SOFT') ? 'bitbang' : 'hardware';
        }
        updateSplitPinsUI();
    } else {
        setSplitMode(false);
    }

    // 4. Matrix Type & Polarity & Debounce
    const mTypeMatch = text.match(/#define\s+MATRIX_TYPE\s+(COL2ROW|ROW2COL|DIRECT)/);
    if (mTypeMatch) {
        setMatrixType(mTypeMatch[1]);
        loadedSomething = true;
    }
    if (text.includes('#define CUSTOM_MATRIX')) {
        loadedSomething = true;
    }

    document.getElementById('matrixPolarity').value = text.includes('#define MATRIX_ACTIVE_LOW') ? 'ACTIVE_LOW' : 'NORMAL';

    const debMatch = text.match(/#define\s+DEBOUNCE\s+(\d+)/);
    if (debMatch) document.getElementById('debounceMs').value = parseInt(debMatch[1], 10);

    // 5. Pins
    const rowPinsMatch = text.match(/#define\s+(?:MATRIX_)?ROW_PINS\s+\{([^}]+)\}/);
    if (rowPinsMatch) {
        document.getElementById('rowPins').value = rowPinsMatch[1].trim();
        loadedSomething = true;
    }
    const colPinsMatch = text.match(/#define\s+(?:MATRIX_)?COL_PINS\s+\{([^}]+)\}/);
    if (colPinsMatch) {
        document.getElementById('colPins').value = colPinsMatch[1].trim();
        loadedSomething = true;
    }
    const dirPinsMatch = text.match(/#define\s+DIRECT_PINS\s+\{([^}]+)\}/);
    if (dirPinsMatch) {
        document.getElementById('directPins').value = dirPinsMatch[1].trim();
        setMatrixType('DIRECT');
        loadedSomething = true;
    }

    // 6. Subsystems / Feature flags
    ['mouse', 'gamepad', 'midi', 'led'].forEach(f => {
        toggleFeatureSubsystem(f, !new RegExp(`#define\\s+NO_${f.toUpperCase()}\\b`).test(text));
    });
    const noLed = /#define\s+NO_LED\b/.test(text);
    const ledPinsMatch = text.match(/#define\s+LED_PINS\s+\{([^}]+)\}/);
    const ledDebugMatch = text.match(/#define\s+LED_DEBUG\s+(\d+)/);
    const ledCapsMatch = text.match(/#define\s+LED_HID_CAPS_LOCK\s+(\d+)/);
    const ledNumMatch = text.match(/#define\s+LED_HID_NUM_LOCK\s+(\d+)/);
    const ledScrollMatch = text.match(/#define\s+LED_HID_SCROLL_LOCK\s+(\d+)/);
    const ledActMatch = text.match(/#define\s+LED_ACTIVITY_PIN\s+([A-Za-z0-9_]+)/);

    if (!noLed && (ledPinsMatch || ledActMatch)) {
        loadedSomething = true;
        const importedPins = ledPinsMatch ? parsePins(ledPinsMatch[1]) : [];
        const dIdx = ledDebugMatch ? parseInt(ledDebugMatch[1], 10) : -1;
        const cIdx = ledCapsMatch ? parseInt(ledCapsMatch[1], 10) : -1;
        const nIdx = ledNumMatch ? parseInt(ledNumMatch[1], 10) : -1;
        const sIdx = ledScrollMatch ? parseInt(ledScrollMatch[1], 10) : -1;

        configState.ledRows = [];
        importedPins.forEach((pin, idx) => {
            let act = 'none';
            if (idx === dIdx) act = 'debug';
            else if (idx === cIdx) act = 'caps';
            else if (idx === nIdx) act = 'num';
            else if (idx === sIdx) act = 'scroll';
            configState.ledRows.push({ pin, action: act });
        });
        if (ledActMatch) {
            configState.ledRows.push({ pin: ledActMatch[1].trim(), action: 'activity' });
        }
        syncLedStateFromRows();
        renderLedRows();
    }
    toggleLedOptions();

    const noRgb = /#define\s+NO_RGB\b/.test(text);

    // 7. RGB
    const rgbNumMatch = text.match(/#define\s+RGB_NUM\s+(\d+)/);
    const rgbPinMatch = text.match(/#define\s+RGB_PIN\s+([A-Za-z0-9_]+)/);
    if (rgbNumMatch && !noRgb) {
        document.getElementById('enableRgb').checked = true;
        document.getElementById('rgbCount').value = parseInt(rgbNumMatch[1], 10);
        if (rgbPinMatch) document.getElementById('rgbPin').value = rgbPinMatch[1];
    } else if (noRgb) {
        document.getElementById('enableRgb').checked = false;
    }
    toggleRgbOptions();

    // 7. Encoders
    const encAMatch = text.match(/#define\s+ENCODER_PINS_A\s+\{([^}]+)\}/);
    const encBMatch = text.match(/#define\s+ENCODER_PINS_B\s+\{([^}]+)\}/);
    const encResMatch = text.match(/#define\s+ENCODER_RESOLUTION\s+(\d+)/);
    if (encAMatch && encBMatch) {
        document.getElementById('enableEncoders').checked = true;
        const pinsA = parsePins(encAMatch[1]);
        const pinsB = parsePins(encBMatch[1]);
        document.getElementById('encCount').value = pinsA.length;
        if (encResMatch) document.getElementById('encResolution').value = encResMatch[1];
        toggleEncoderOptions();
        pinsA.forEach((p, idx) => {
            const elA = document.getElementById(`enc_${idx}_pinA`);
            if (elA) elA.value = p;
        });
        pinsB.forEach((p, idx) => {
            const elB = document.getElementById(`enc_${idx}_pinB`);
            if (elB) elB.value = p;
        });
    }

    // 8. LAYOUT coordinate matrix
    const layoutMatch = text.match(/#define\s+LAYOUT\b[\s\S]*?\{([\s\S]*?)\}\s*(?:#define|\/\/|$)/);
    if (layoutMatch) {
        const coordPairs = layoutMatch[1].match(/\{\s*(\d+)\s*,\s*(\d+)\s*\}/g);
        if (coordPairs && coordPairs.length > 0) {
            let reqRows = 0;
            let reqCols = 0;
            const numRowsMatch = text.match(/#define\s+NUM_ROWS\s+(\d+)/);
            const numColsMatch = text.match(/#define\s+NUM_COLS\s+(\d+)/);
            if (numRowsMatch) reqRows = parseInt(numRowsMatch[1], 10);
            if (numColsMatch) reqCols = parseInt(numColsMatch[1], 10);

            const parsedCoords = [];
            coordPairs.forEach(cp => {
                const m = cp.match(/\{\s*(\d+)\s*,\s*(\d+)\s*\}/);
                if (m) {
                    const r = parseInt(m[1], 10);
                    const c = parseInt(m[2], 10);
                    parsedCoords.push({ r, c });
                    if (r + 1 > reqRows) reqRows = r + 1;
                    if (c + 1 > reqCols) reqCols = c + 1;
                }
            });

            // Update rowPins and colPins inputs according to reqRows and reqCols
            if (reqRows > 0) {
                document.getElementById('rowPins').value = padPins(parsePins(document.getElementById('rowPins').value), reqRows);
            }
            if (reqCols > 0) {
                document.getElementById('colPins').value = padPins(parsePins(document.getElementById('colPins').value), reqCols);
            }

            const { totalRows, totalCols } = getParsedPins();
            configState.activeKeys = [];
            for (let r = 0; r < totalRows; r++) {
                configState.activeKeys[r] = Array(totalCols).fill(false);
            }
            parsedCoords.forEach(p => {
                if (p.r < totalRows && p.c < totalCols) {
                    configState.activeKeys[p.r][p.c] = true;
                }
            });

            // Restore or generate keyGeometry in new layout order
            const newPairs = getActiveLayoutPairs();
            configState.keyGeometry = newPairs.map(p => oldGeomMap[`${p.r},${p.c}`] || { w: 1, h: 1, x: 0, y: 0 });

            loadedSomething = true;
        }
    }

    // 9. Keymap layers
    const keymapMatch = text.match(/const\s+uint32_t\s+keymap\[\]\[NUM_KEYS\]\s*=\s*\{([\s\S]*?)\};/);
    if (keymapMatch) {
        const layerBlocks = keymapMatch[1].match(/\[([A-Za-z0-9_]+)\]\s*=\s*\{([\s\S]*?)\}/g);
        if (layerBlocks) {
            configState.layers = [];
            configState.keymaps = {};
            layerBlocks.forEach(lb => {
                const m = lb.match(/\[([A-Za-z0-9_]+)\]\s*=\s*\{([\s\S]*?)\}/);
                if (m) {
                    const lName = m[1];
                    configState.layers.push(lName);
                    const rawKeys = parsePins(m[2].replace(/\/\/.*/g, '').replace(/\\\n/g, ' '));
                    configState.keymaps[lName] = rawKeys;
                }
            });
            if (configState.layers.length > 0) {
                configState.currentLayer = configState.layers[0];
            }
        }
    }

    // 10. Encoder keymaps
    const encKeymapMatch = text.match(/const\s+uint32_t\s+encoder_keymap\[\]\[\d+\]\[2\]\s*=\s*\{([\s\S]*?)\};/);
    if (encKeymapMatch) {
        configState.encoderKeymaps = {};
        const layerBlocks = encKeymapMatch[1].match(/\[([A-Za-z0-9_]+)\]\s*=\s*\{([\s\S]*?)\}/g);
        if (layerBlocks) {
            layerBlocks.forEach(lb => {
                const m = lb.match(/\[([A-Za-z0-9_]+)\]\s*=\s*\{([\s\S]*?)\}/);
                if (m) {
                    const lName = m[1];
                    const pairs = m[2].match(/\{\s*([^,]+)\s*,\s*([^}]+)\s*\}/g);
                    if (pairs) {
                        configState.encoderKeymaps[lName] = pairs.map(p => {
                            const pm = p.match(/\{\s*([^,]+)\s*,\s*([^}]+)\s*\}/);
                            return { cw: pm[1].trim(), ccw: pm[2].trim() };
                        });
                    }
                }
            });
        }
    }

    if (loadedSomething) {
        markDirty();
        initMatrixGrid();
        renderVisualKeymap();
        gotoStep(1);
        applyLanguage();
        alert(currentLanguage === 'en' ? `Successfully loaded config from ${filename}!` : `Файл ${filename} успешно загружен и применен в конфигураторе!`);
    } else {
        alert(currentLanguage === 'en' ? `Could not parse config structures from ${filename}.` : `Не удалось распознать структуру DMK в файле ${filename}.`);
    }
}

function parseAndApplyVialJson(json, filename) {
    markDirty();
    if (json.name) {
        document.getElementById('kbName').value = json.name;
        document.getElementById('vialName').value = json.name;
    }
    if (json.vendorId) document.getElementById('vialVid').value = json.vendorId;
    if (json.productId) document.getElementById('vialPid').value = json.productId;

    let maxRows = 0;
    let maxCols = 0;

    if (json.matrix) {
        if (json.matrix.rows) maxRows = json.matrix.rows;
        if (json.matrix.cols) maxCols = json.matrix.cols;
    }

    if (json.features && json.features.encoder) {
        document.getElementById('enableEncoders').checked = true;
        toggleEncoderOptions();
    }

    if (json.lighting) {
        document.getElementById('enableRgb').checked = true;
        toggleRgbOptions();
    }

    if (json.layouts && json.layouts.keymap) {
        const km = json.layouts.keymap;
        configState.keyGeometry = [];
        const activePairs = [];
        let detectedEncCount = 0;

        let currentKLE_Y = 0;
        let currentKLE_X = 0;

        km.forEach((row, rowIdx) => {
            if (rowIdx > 0) {
                currentKLE_Y += 1;
                currentKLE_X = 0;
            }
            let curW = 1;
            let curH = 1;

            row.forEach(item => {
                if (typeof item === 'object') {
                    if (item.x !== undefined) currentKLE_X += item.x;
                    if (item.y !== undefined) currentKLE_Y += item.y;
                    if (item.w !== undefined) curW = item.w;
                    if (item.h !== undefined) curH = item.h;
                } else if (typeof item === 'string') {
                    if (item.includes('\ne') || item.endsWith('e')) {
                        const clean = item.split('\n')[0];
                        const parts = clean.split(',').map(s => parseInt(s.trim(), 10));
                        if (parts.length === 2) {
                            detectedEncCount++;
                        }
                    } else {
                        const parts = item.split(',').map(s => parseInt(s.trim(), 10));
                        if (parts.length === 2 && !isNaN(parts[0]) && !isNaN(parts[1])) {
                            const r = parts[0];
                            const c = parts[1];
                            activePairs.push({ r, c, w: curW, h: curH, x: currentKLE_X, y: currentKLE_Y });
                            if (r + 1 > maxRows) maxRows = r + 1;
                            if (c + 1 > maxCols) maxCols = c + 1;
                        }
                    }
                    currentKLE_X += curW;
                    curW = 1;
                    curH = 1;
                }
            });
        });

        if (detectedEncCount > 0) {
            document.getElementById('enableEncoders').checked = true;
            document.getElementById('encCount').value = detectedEncCount;
            toggleEncoderOptions();
        }

        // Set row and col pins placeholder count if needed
        if (maxRows > 0 && maxCols > 0) {
            document.getElementById('rowPins').value = padPins(parsePins(document.getElementById('rowPins').value), maxRows);
            document.getElementById('colPins').value = padPins(parsePins(document.getElementById('colPins').value), maxCols);

            configState.activeKeys = [];
            for (let r = 0; r < maxRows; r++) {
                configState.activeKeys[r] = Array(maxCols).fill(false);
            }

            const geomByCoord = {};
            activePairs.forEach(p => {
                configState.activeKeys[p.r][p.c] = true;
                geomByCoord[`${p.r},${p.c}`] = { w: p.w, h: p.h, x: p.x, y: p.y };
            });

            const pairs = getActiveLayoutPairs();
            configState.keyGeometry = pairs.map(p => geomByCoord[`${p.r},${p.c}`] || { w: 1, h: 1, x: 0, y: 0 });
        }
    }

    initMatrixGrid();
    renderVisualKeymap();
    gotoStep(1);
    applyLanguage();
    alert(currentLanguage === 'en' ? `Successfully loaded Vial layout from ${filename}!` : `Файл ${filename} успешно загружен! Геометрия и матрица Vial применены.`);
}
