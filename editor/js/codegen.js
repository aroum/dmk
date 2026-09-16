function generateVialJson() {
            const kbName = document.getElementById('kbName').value || "DMK Keyboard";
            const vialVid = document.getElementById('vialVid').value || "0xCAFE";
            const vialPid = document.getElementById('vialPid').value || "0x4001";
            const { totalRows, totalCols } = getParsedPins();
            const enableEnc = document.getElementById('enableEncoders')?.checked;
            const encCount = parseInt(document.getElementById('encCount')?.value, 10) || 0;
            const enableRgb = document.getElementById('enableRgb')?.checked;

            const matrixMap = [];

            // Add encoders row at the top if enabled
            if (enableEnc && encCount > 0) {
                const encRow = [];
                for (let e = 0; e < encCount; e++) {
                    const eGeom = (configState.encoderGeometry && configState.encoderGeometry[e]) ? configState.encoderGeometry[e] : { x: 0, y: 0 };
                    const props = {};
                    if (eGeom.x !== 0) props.x = eGeom.x;
                    if (eGeom.y !== 0) props.y = eGeom.y;
                    if (Object.keys(props).length > 0) {
                        encRow.push(props);
                    }
                    encRow.push(`${e},0\n\n\n\n\ne`);
                    encRow.push(`${e},1\n\n\n\n\ne`);
                }
                matrixMap.push(encRow);
            }

            // Collect all active keys with their absolute visual coordinates
            const allKeys = [];
            let keySeqIndex = 0;
            for (let r = 0; r < totalRows; r++) {
                for (let c = 0; c < totalCols; c++) {
                    if (configState.activeKeys[r] && configState.activeKeys[r][c]) {
                        const geom = (configState.keyGeometry && configState.keyGeometry[keySeqIndex]) ? configState.keyGeometry[keySeqIndex] : { w: 1, h: 1, x: 0, y: 0 };
                        const vX = Math.round((c + (geom.x || 0)) * 1000) / 1000;
                        const vY = Math.round((r + (geom.y || 0)) * 1000) / 1000;
                        allKeys.push({
                            r, c,
                            w: geom.w || 1,
                            h: geom.h || 1,
                            vX, vY
                        });
                        keySeqIndex++;
                    }
                }
            }

            // Group keys into visual rows by vY (KLE standard)
            const yValues = [...new Set(allKeys.map(k => k.vY))].sort((a, b) => a - b);
            let currentKLE_Y = 0;

            yValues.forEach((yVal, rowIdx) => {
                const rowArr = [];
                const keysInRow = allKeys.filter(k => k.vY === yVal).sort((a, b) => a.vX - b.vX);

                const defaultY = (rowIdx === 0) ? 0 : (currentKLE_Y + 1);
                const deltaY = Math.round((yVal - defaultY) * 1000) / 1000;

                let currentKLE_X = 0;

                keysInRow.forEach((k, kIdx) => {
                    const props = {};
                    if (kIdx === 0 && deltaY !== 0) {
                        props.y = deltaY;
                    }

                    const deltaX = Math.round((k.vX - currentKLE_X) * 1000) / 1000;
                    if (deltaX !== 0) {
                        props.x = deltaX;
                    }

                    if (k.w > 1) props.w = k.w;
                    if (k.h > 1) props.h = k.h;

                    if (Object.keys(props).length > 0) {
                        rowArr.push(props);
                    }
                    rowArr.push(`${k.r},${k.c}`);

                    currentKLE_X = k.vX + k.w;
                });

                currentKLE_Y = yVal;
                matrixMap.push(rowArr);
            });

            const vialObj = {
                "name": kbName,
                "vendorId": vialVid,
                "productId": vialPid,
                ...(enableRgb ? { "lighting": "qmk_rgblight" } : {}),
                "matrix": {
                    "rows": totalRows,
                    "cols": totalCols
                },
                ...(enableEnc && encCount > 0 ? { "features": { "encoder": true } } : {}),
                "layouts": {
                    "keymap": matrixMap
                }
            };

            document.getElementById('vialJsonOutput').textContent = JSON.stringify(vialObj, null, 2);
        }

        function generateConfigCode() {
            const kbName = document.getElementById('kbName').value || "DMK Custom";
            const isSplit = configState.isSplit;
            const masterSide = document.getElementById('masterSide').value;
            const matrixType = configState.matrixType;
            const { rows, cols, totalCols, totalRows } = getParsedPins();
            const debounce = document.getElementById('debounceMs').value || 10;
            const polarity = document.getElementById('matrixPolarity').value;

            const enableRgb = document.getElementById('enableRgb').checked;
            const rgbCount = document.getElementById('rgbCount').value || 0;
            const rgbPin = document.getElementById('rgbPin').value || "GPIO0";

            const enableEnc = document.getElementById('enableEncoders').checked;
            const encCount = parseInt(document.getElementById('encCount').value, 10) || 0;
            const encResolution = document.getElementById('encResolution').value || 2;
            const encA = [];
            const encB = [];
            const encKeymapEntries = [];

            if (enableEnc) {
                for (let i = 0; i < encCount; i++) {
                    const pA = document.getElementById(`enc_${i}_pinA`)?.value?.trim() || `GPIO${2+i*2}`;
                    const pB = document.getElementById(`enc_${i}_pinB`)?.value?.trim() || `GPIO${3+i*2}`;
                    encA.push(pA);
                    encB.push(pB);
                }
            }

            const enableVial = document.getElementById('enableVial').checked;
            const vialName = document.getElementById('vialName').value || kbName;
            const vialVid = document.getElementById('vialVid').value || "0xCAFE";
            const vialPid = document.getElementById('vialPid').value || "0x4001";

            // Count Active Keys & Build Layout Array
            const activeCoords = [];
            const layoutPairs = [];
            for (let r = 0; r < totalRows; r++) {
                for (let c = 0; c < totalCols; c++) {
                    if (configState.activeKeys[r] && configState.activeKeys[r][c]) {
                        activeCoords.push({ r, c });
                        layoutPairs.push(`{${r}, ${c}}`);
                    }
                }
            }
            const numKeys = layoutPairs.length;

            let out = `#ifndef CONFIG_H\n#define CONFIG_H\n\n`;

            if (configState.mcu && configState.mcu !== 'all') {
                out += `// Default target microcontroller for DMK build system\n`;
                out += `#define DEFAULT_MCU ${configState.mcu}\n\n`;
            }

            // Modular Subsystem Disables
            const featureDisables = [];
            if (configState.enableMouse === false) featureDisables.push('#define NO_MOUSE');
            if (configState.enableGamepad === false) featureDisables.push('#define NO_GAMEPAD');
            if (configState.enableMidi === false) featureDisables.push('#define NO_MIDI');
            if (configState.enableLed === false) featureDisables.push('#define NO_LED');
            if (!enableRgb) featureDisables.push('#define NO_RGB');

            if (featureDisables.length > 0) {
                out += `/* --- Modular Subsystem Options --- */\n`;
                out += featureDisables.join('\n') + `\n\n`;
            }

            if (enableVial) {
                const proto = configState.protocolMode || configState.vialProtocol || document.getElementById('vialProtocolMode')?.value || 'vial';
                out += `// === Vial / VIA GUI Configuration ===\n`;
                out += `#ifndef VIAL\n`;
                out += `#define VIAL\n`;
                out += `#endif\n`;
                if (proto === 'via_v3') {
                    out += `// Protocol 12 for usevia.app\n`;
                    out += `#define VIA_V3\n`;
                }
                out += `#define VIAL_KEYBOARD_NAME "${vialName}"\n`;
                out += `#define VIAL_VENDOR_ID ${vialVid}\n`;
                out += `#define VIAL_PRODUCT_ID ${vialPid}\n\n`;
            }

            out += `#include "pin_defs.h"\n\n`;

            // Helper to get platform pins with fallback to defaults
            const getPlatformPins = (pKey) => {
                const multi = (configState.multiMcuPins && configState.multiMcuPins[pKey]) ? configState.multiMcuPins[pKey] : {};
                const def = MCU_DEFAULTS[pKey] || MCU_DEFAULTS.rp2040;
                return {
                    rows: (multi.rows && multi.rows.length) ? multi.rows : def.rows,
                    cols: (multi.cols && multi.cols.length) ? multi.cols : def.cols,
                    direct: (multi.direct && multi.direct.length) ? multi.direct : def.direct,
                    splitTx: multi.splitTx || def.splitTx || def.serial || 'GPIO0',
                    splitRx: multi.splitRx || def.splitRx || 'GPIO1',
                    rgb: multi.rgb || def.rgb || 'GPIO0',
                    encA: (multi.encA && multi.encA.length) ? multi.encA : def.encA,
                    encB: (multi.encB && multi.encB.length) ? multi.encB : def.encB
                };
            };

            // Split settings
            if (isSplit) {
                out += `/* --- Split Keyboard Settings --- */\n`;
                out += `#define MASTER_SIDE ${masterSide}\n`;
                if (configState.mcu === 'all') {
                    const rpP = getPlatformPins('rp2040');
                    const milP = getPlatformPins('milandr');
                    const nrfP = getPlatformPins('nrf52840');
                    const baiP = getPlatformPins('baikal');
                    out += `// Communication modes per platform (HW Half-Duplex, Full-Duplex UART, Soft Bit-Bang)\n`;
                    out += `#if defined(MCU_rp2040) || defined(MCU_rp2350)\n`;
                    out += `    #define SPLIT_CONNECTION_TYPE HW_HALF_DUPLEX\n`;
                    out += `    #define SPLIT_TX_PIN          ${rpP.splitTx}\n`;
                    out += `#elif defined(MCU_milandr)\n`;
                    out += `    #define SPLIT_CONNECTION_TYPE HW_FULL_DUPLEX\n`;
                    out += `    #define SPLIT_TX_PIN          ${milP.splitTx}\n`;
                    out += `    #define SPLIT_RX_PIN          ${milP.splitRx}\n`;
                    out += `#elif defined(MCU_nrf52840)\n`;
                    out += `    #define SPLIT_CONNECTION_TYPE HW_HALF_DUPLEX\n`;
                    out += `    #define SPLIT_TX_PIN          ${nrfP.splitTx}\n`;
                    out += `#elif defined(MCU_baikal)\n`;
                    out += `    #define SPLIT_CONNECTION_TYPE HW_HALF_DUPLEX\n`;
                    out += `    #define SPLIT_TX_PIN          ${baiP.splitTx}\n`;
                    out += `#endif\n\n`;
                } else if (configState.mcu === 'milandr') {
                    out += `#define SPLIT_CONNECTION_TYPE HW_FULL_DUPLEX\n`;
                    out += `#define SPLIT_TX_PIN PF1\n`;
                    out += `#define SPLIT_RX_PIN PF0\n\n`;
                } else {
                    out += `#define SPLIT_CONNECTION_TYPE HW_HALF_DUPLEX\n`;
                    out += `#define SPLIT_TX_PIN ${document.getElementById('serialPinRP').value || 'GPIO0'}\n\n`;
                }
            }

            // Matrix Settings
            out += `/* --- Matrix Settings --- */\n`;
            out += `#define MATRIX_TYPE ${matrixType}\n`;
            if (polarity === 'ACTIVE_LOW') {
                out += `#define MATRIX_ACTIVE_LOW\n`;
            }
            out += `#define DEBOUNCE ${debounce}\n\n`;

            if (matrixType === 'DIRECT') {
                out += `// Direct connection pins\n`;
                if (configState.mcu === 'all') {
                    const rpP = getPlatformPins('rp2040');
                    const milP = getPlatformPins('milandr');
                    const nrfP = getPlatformPins('nrf52840');
                    const baiP = getPlatformPins('baikal');
                    out += `#if defined(MCU_rp2040) || defined(MCU_rp2350)\n`;
                    out += `#define DIRECT_PINS { ${rpP.direct.join(', ')} }\n`;
                    out += `#elif defined(MCU_milandr)\n`;
                    out += `#define DIRECT_PINS { ${milP.direct.join(', ')} }\n`;
                    out += `#elif defined(MCU_nrf52840)\n`;
                    out += `#define DIRECT_PINS { ${nrfP.direct.join(', ')} }\n`;
                    out += `#elif defined(MCU_baikal)\n`;
                    out += `#define DIRECT_PINS { ${baiP.direct.join(', ')} }\n`;
                    out += `#endif\n\n`;
                } else {
                    out += `#define DIRECT_PINS { ${document.getElementById('directPins').value} }\n\n`;
                }
            } else {
                out += `// Row and column pins\n`;
                if (configState.mcu === 'all') {
                    const rpP = getPlatformPins('rp2040');
                    const milP = getPlatformPins('milandr');
                    const nrfP = getPlatformPins('nrf52840');
                    const baiP = getPlatformPins('baikal');
                    out += `#if defined(MCU_rp2040) || defined(MCU_rp2350)\n`;
                    out += `#define ROW_PINS { ${rpP.rows.join(', ')} }\n`;
                    out += `#define COL_PINS { ${rpP.cols.join(', ')} }\n`;
                    out += `#elif defined(MCU_milandr)\n`;
                    out += `#define ROW_PINS { ${milP.rows.join(', ')} }\n`;
                    out += `#define COL_PINS { ${milP.cols.join(', ')} }\n`;
                    out += `#elif defined(MCU_nrf52840)\n`;
                    out += `#define ROW_PINS { ${nrfP.rows.join(', ')} }\n`;
                    out += `#define COL_PINS { ${nrfP.cols.join(', ')} }\n`;
                    out += `#elif defined(MCU_baikal)\n`;
                    out += `#define ROW_PINS { ${baiP.rows.join(', ')} }\n`;
                    out += `#define COL_PINS { ${baiP.cols.join(', ')} }\n`;
                    out += `#endif\n\n`;
                } else {
                    out += `#define ROW_PINS { ${rows.join(', ')} }\n`;
                    out += `#define COL_PINS { ${cols.join(', ')} }\n\n`;
                }
            }

            // RGB
            if (enableRgb && rgbCount > 0) {
                out += `/* --- RGB Backlight --- */\n`;
                out += `#define RGB_NUM ${rgbCount}\n`;
                if (configState.mcu === 'all') {
                    const rpP = getPlatformPins('rp2040');
                    const milP = getPlatformPins('milandr');
                    const nrfP = getPlatformPins('nrf52840');
                    const baiP = getPlatformPins('baikal');
                    out += `#if defined(MCU_rp2040) || defined(MCU_rp2350)\n`;
                    out += `#define RGB_PIN ${rpP.rgb}\n`;
                    out += `#elif defined(MCU_milandr)\n`;
                    out += `#define RGB_PIN ${milP.rgb}\n`;
                    out += `#elif defined(MCU_nrf52840)\n`;
                    out += `#define RGB_PIN ${nrfP.rgb}\n`;
                    out += `#elif defined(MCU_baikal)\n`;
                    out += `#define RGB_PIN ${baiP.rgb}\n`;
                    out += `#endif\n\n`;
                } else {
                    out += `#define RGB_PIN ${rgbPin}\n\n`;
                }
            }

            // Encoders
            if (enableEnc) {
                out += `/* --- Rotary Encoders --- */\n`;
                if (configState.mcu === 'all') {
                    const rpP = getPlatformPins('rp2040');
                    const milP = getPlatformPins('milandr');
                    const nrfP = getPlatformPins('nrf52840');
                    const baiP = getPlatformPins('baikal');
                    out += `#if defined(MCU_rp2040) || defined(MCU_rp2350)\n`;
                    out += `#define ENCODER_PINS_A { ${rpP.encA.slice(0, encCount).join(', ')} }\n`;
                    out += `#define ENCODER_PINS_B { ${rpP.encB.slice(0, encCount).join(', ')} }\n`;
                    out += `#elif defined(MCU_milandr)\n`;
                    out += `#define ENCODER_PINS_A { ${milP.encA.slice(0, encCount).join(', ')} }\n`;
                    out += `#define ENCODER_PINS_B { ${milP.encB.slice(0, encCount).join(', ')} }\n`;
                    out += `#elif defined(MCU_nrf52840)\n`;
                    out += `#define ENCODER_PINS_A { ${nrfP.encA.slice(0, encCount).join(', ')} }\n`;
                    out += `#define ENCODER_PINS_B { ${nrfP.encB.slice(0, encCount).join(', ')} }\n`;
                    out += `#elif defined(MCU_baikal)\n`;
                    out += `#define ENCODER_PINS_A { ${baiP.encA.slice(0, encCount).join(', ')} }\n`;
                    out += `#define ENCODER_PINS_B { ${baiP.encB.slice(0, encCount).join(', ')} }\n`;
                    out += `#endif\n`;
                } else if (encA.length > 0) {
                    out += `#define ENCODER_PINS_A { ${encA.join(', ')} }\n`;
                    out += `#define ENCODER_PINS_B { ${encB.join(', ')} }\n`;
                }
                out += `#define ENCODER_RESOLUTION ${encResolution}\n\n`;
            }

            // Keymap Matrix & LAYOUT
            out += `/* --- Keymap Matrix --- */\n`;
            out += `#define NUM_ROWS ${totalRows}\n`;
            out += `#define NUM_COLS ${totalCols}\n`;
            if (isSplit) {
                out += `#define NUM_ROWS_SPLIT ${rows.length}\n`;
                out += `#define NUM_COLS_SPLIT ${cols.length}\n`;
            }
            out += `#define NUM_KEYS ${numKeys}\n\n`;

            out += `// {ROW, COL} coordinate mapping\n`;
            out += `// clang-format off\n`;
            out += `#define LAYOUT { \\\n`;

            // Break layout into logical rows matching the matrix rows
            const rowsMap = new Map();
            activeCoords.forEach((coord, idx) => {
                if (!rowsMap.has(coord.r)) {
                    rowsMap.set(coord.r, []);
                }
                rowsMap.get(coord.r).push({ ...coord, idx, pairStr: layoutPairs[idx] });
            });

            const rowChunks = [];
            rowsMap.forEach((keysInRow) => {
                rowChunks.push("    " + keysInRow.map(k => k.pairStr).join(', '));
            });

            out += rowChunks.join(', \\\n') + ` \\\n}\n`;
            out += `#define LAYOUT_DEFAULT LAYOUT\n`;
            out += `// clang-format on\n\n`;

            // Layers definition and Keymap
            out += `#ifdef DEFINE_KEYMAP\n`;
            configState.layers.forEach((l, idx) => {
                out += `#define ${l} ${idx}\n`;
            });
            out += `\n// Flat keymaps: one entry per key in LAYOUT order\n`;
            out += `// clang-format off\n`;
            out += `const uint32_t keymap[][NUM_KEYS] = {\n`;

            configState.layers.forEach((l, lIdx) => {
                out += `    [${l}] = {\n`;
                const keys = configState.keymaps[l] || [];
                const keyLines = [];
                rowsMap.forEach((keysInRow) => {
                    const rowKeycodes = keysInRow.map(k => keys[k.idx] || "K_TRNS");
                    keyLines.push("        " + rowKeycodes.join(', '));
                });
                out += keyLines.join(', \\\n') + `\n    }${lIdx < configState.layers.length - 1 ? ',' : ''}\n`;
            });
            out += `};\n`;
            out += `\nconst size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);\n\n`;

            if (enableEnc && encA.length > 0) {
                out += `// Rotary encoder actions per layer: { CW, CCW }\n`;
                out += `const uint32_t encoder_keymap[][${encA.length}][2] = {\n`;
                configState.layers.forEach((l, lIdx) => {
                    out += `    [${l}] = {\n`;
                    const layerEncActs = (configState.encoderKeymaps && configState.encoderKeymaps[l]) ? configState.encoderKeymaps[l] : [];
                    const lines = [];
                    for (let i = 0; i < encA.length; i++) {
                        const act = layerEncActs[i] || (lIdx === 0 ? { cw: "K_VOLU", ccw: "K_VOLD" } : { cw: "K_TRNS", ccw: "K_TRNS" });
                        lines.push(`        { ${act.cw || 'K_TRNS'}, ${act.ccw || 'K_TRNS'} } // Enc ${i+1}`);
                    }
                    out += lines.join(',\n') + `\n`;
                    out += `    }${lIdx < configState.layers.length - 1 ? ',' : ''}\n`;
                });
                out += `};\n\n`;
            }

            // Macros: Layer Trigger Macros & Custom User Macros
            const enableLayerHotkeys = document.getElementById('enableLayerHotkeys')?.checked;
            const enableCustomMacros = document.getElementById('enableCustomMacros')?.checked;
            const enableChords = document.getElementById('enableChords')?.checked;

            const macroAliases = [];
            const macroDefinitions = [];
            const macroEntries = [];

            // 1. Custom User Macros (M(0), M(1)...)
            if (enableCustomMacros && configState.customMacros && configState.customMacros.length > 0) {
                configState.customMacros.forEach((macro, idx) => {
                    const aliasName = macro.name ? macro.name.trim().toUpperCase() : `MC_MACRO_${idx}`;
                    macroAliases.push(`#define ${aliasName} M(${macroEntries.length})`);

                    let stepsCode = `const MacroStep ${aliasName}_STEPS[] = { ${macro.steps || 'M_DN(K_NO), M_UP(K_NO)'} };`;
                    macroDefinitions.push(stepsCode);

                    macroEntries.push(
                        `    // Macro ${idx}: ${aliasName}\n` +
                        `    [${macroEntries.length}] = {\n` +
                        `        .steps         = ${aliasName}_STEPS,\n` +
                        `        .count         = sizeof(${aliasName}_STEPS) / sizeof(${aliasName}_STEPS[0]),\n` +
                        `        .layer_trigger = MACRO_NO_LAYER,\n` +
                        `    }`
                    );
                });
            }

            // 2. Layer Trigger Macros
            if (enableLayerHotkeys && configState.layerTriggerMacros) {
                configState.layers.forEach((l, idx) => {
                    const stepsArray = configState.layerTriggerMacros[l] || [];
                    if (stepsArray.length > 0) {
                        const stepCodes = [];
                        stepsArray.forEach(st => {
                            if (st.type === 'TAP') {
                                stepCodes.push(`M_DN(${st.key || 'K_NO'})`);
                                stepCodes.push(`M_UP(${st.key || 'K_NO'})`);
                            } else if (st.type === 'DOWN') {
                                stepCodes.push(`M_DN(${st.key || 'K_NO'})`);
                            } else if (st.type === 'UP') {
                                stepCodes.push(`M_UP(${st.key || 'K_NO'})`);
                            } else if (st.type === 'DELAY') {
                                stepCodes.push(`M_D(${st.delay || 100})`);
                            }
                        });

                        let macroStepsCode = `const MacroStep STEPS_LAYER_TRIGGER_${l}[] = { ${stepCodes.join(', ')} };`;
                        macroDefinitions.push(macroStepsCode);

                        macroEntries.push(
                            `    // Automatically fires when layer ${l} (${idx}) becomes active\n` +
                            `    [${macroEntries.length}] = {\n` +
                            `        .steps         = STEPS_LAYER_TRIGGER_${l},\n` +
                            `        .count         = sizeof(STEPS_LAYER_TRIGGER_${l}) / sizeof(STEPS_LAYER_TRIGGER_${l}[0]),\n` +
                            `        .layer_trigger = ${l},\n` +
                            `    }`
                        );
                    }
                });
            }

            if (macroDefinitions.length > 0) {
                out += `/* --- Custom Macros & Layer Triggers --- */\n`;
                if (macroAliases.length > 0) {
                    out += macroAliases.join('\n') + `\n\n`;
                }
                out += macroDefinitions.join('\n') + `\n\n`;
                out += `const Macro keyboard_macros[] = {\n`;
                out += macroEntries.join(',\n') + `\n`;
                out += `};\n\n`;
                out += `const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);\n\n`;
            } else {
                out += `const Macro keyboard_macros[] = {};\n`;
                out += `const uint8_t keyboard_macros_count = 0;\n\n`;
            }

            // 3. Chords / Combos
            if (enableChords && configState.customChords && configState.customChords.length > 0) {
                out += `/* --- Chords / Combos Configuration --- */\n`;
                out += `// Macro helper for chord key coordinates: K(Row, Col, Layer)\n`;
                out += `#ifndef K\n#define K(r, c, l) {r, c, l}\n#endif\n\n`;

                // Function declarations & generated action functions for chord actions
                const actionSet = new Set();
                const chordActionFuncMap = {};

                configState.customChords.forEach((chord, cIdx) => {
                    const actRaw = (chord.action || 'LC(K_C)').trim();
                    if (/^[a-zA-Z_][a-zA-Z0-9_]*$/.test(actRaw) && !actRaw.startsWith('K_')) {
                        // User C function name
                        chordActionFuncMap[cIdx] = actRaw;
                        actionSet.add(`void ${actRaw}(void) { /* Custom function */ }`);
                    } else {
                        // Keycode, modifier, or layer expression (e.g. LC(K_C), MO(1), K_ESC)
                        const fnName = `chord_action_${cIdx + 1}`;
                        chordActionFuncMap[cIdx] = fnName;
                        actionSet.add(`void ${fnName}(void) {\n    // Generated chord action for: ${actRaw}\n    action_exec(${actRaw});\n}`);
                    }
                });

                actionSet.forEach(fnCode => {
                    out += `${fnCode}\n\n`;
                });

                out += `const Chord my_chords[] = {\n`;
                const chordEntries = [];
                configState.customChords.forEach((chord, idx) => {
                    const keyList = parseChordKeys(chord);
                    const layersList = (Array.isArray(chord.targetLayers) && chord.targetLayers.length > 0) ? chord.targetLayers : [0];
                    const fnName = chordActionFuncMap[idx] || 'do_action';

                    layersList.forEach(layerIdx => {
                        const kFormatted = keyList.map(kItem => {
                            let r = kItem.r !== undefined ? kItem.r : 0;
                            let c = kItem.c !== undefined ? kItem.c : 0;
                            return `K(${r}, ${c}, ${layerIdx})`;
                        });

                        chordEntries.push(
                            `    // Chord ${idx + 1} (Layer ${layerIdx})\n` +
                            `    {\n` +
                            `        .keys      = { ${kFormatted.join(', ')} },\n` +
                            `        .key_count = ${kFormatted.length},\n` +
                            `        .action    = ${fnName}\n` +
                            `    }`
                        );
                    });
                });
                out += chordEntries.join(',\n') + `\n`;
                out += `};\n\n`;
                out += `#define CHORDS_COUNT (sizeof(my_chords) / sizeof(my_chords[0]))\n`;
            }

            out += `// clang-format on\n`;
            out += `#endif\n\n`;

            out += `#endif // CONFIG_H\n`;

            document.getElementById('configCodeOutput').textContent = out;
        }
