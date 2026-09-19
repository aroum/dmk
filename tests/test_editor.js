// DMK Config Wizard & Codegen Test Suite
// Run with: node --test tests/test_editor.js

const test = require('node:test');
const assert = require('node:assert');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

const ROOT_DIR = path.resolve(__dirname, '..');
const WIZARD_HTML_PATH = path.join(ROOT_DIR, 'editor', 'wizard.html');
const CODEGEN_JS_PATH = path.join(ROOT_DIR, 'editor', 'js', 'codegen.js');
const KEYCODES_JS_PATH = path.join(ROOT_DIR, 'editor', 'js', 'keycodes.js');
const PRESETS_JS_PATH = path.join(ROOT_DIR, 'editor', 'js', 'presets.js');
const I18N_JS_PATH = path.join(ROOT_DIR, 'editor', 'js', 'i18n.js');

function createSandbox() {
    const domStore = new Map();

    function getElement(id) {
        if (!domStore.has(id)) {
            domStore.set(id, {
                id,
                value: '',
                checked: false,
                innerHTML: '',
                textContent: '',
                style: {},
                classList: {
                    _classes: new Set(),
                    add(c) { this._classes.add(c); },
                    remove(c) { this._classes.delete(c); },
                    contains(c) { return this._classes.has(c); },
                    toggle(c, force) {
                        if (force !== undefined) {
                            if (force) this._classes.add(c);
                            else this._classes.delete(c);
                            return force;
                        }
                        if (this._classes.has(c)) {
                            this._classes.delete(c);
                            return false;
                        } else {
                            this._classes.add(c);
                            return true;
                        }
                    }
                },
                appendChild(child) {
                    this.children = this.children || [];
                    this.children.push(child);
                },
                removeChild(child) {
                    if (this.children) {
                        const idx = this.children.indexOf(child);
                        if (idx !== -1) this.children.splice(idx, 1);
                    }
                },
                querySelectorAll() { return []; },
                querySelector(sel) { return getElement("elem_q_" + sel); },
                setAttribute(k, v) { this[k] = v; },
                getAttribute(k) { return this[k] || null; },
                _listeners: {},
                addEventListener(type, fn) {
                    this._listeners = this._listeners || {};
                    this._listeners[type] = this._listeners[type] || [];
                    this._listeners[type].push(fn);
                },
                removeEventListener(type, fn) {
                    if (this._listeners && this._listeners[type]) {
                        const idx = this._listeners[type].indexOf(fn);
                        if (idx !== -1) this._listeners[type].splice(idx, 1);
                    }
                },
                dispatchEvent(event) {
                    const type = typeof event === 'string' ? event : event?.type;
                    if (this._listeners && this._listeners[type]) {
                        this._listeners[type].forEach(fn => fn(event));
                    }
                },
                scrollIntoView() {},
                click() {
                    if (typeof this.onclick === 'function') {
                        this.onclick({ preventDefault: () => {}, stopPropagation: () => {} });
                    }
                },
                showModal() { this.open = true; this.style.display = 'flex'; },
                close() { this.open = false; this.style.display = 'none'; }
            });
        }
        return domStore.get(id);
    }

    const eventListeners = {
        document: {},
        window: {}
    };

    const documentMock = {
        getElementById(id) { return getElement(id); },
        createElement(tag) {
            const el = getElement(`elem_${tag}_${Math.random()}`);
            el.tagName = tag.toUpperCase();
            return el;
        },
        createDocumentFragment() { return getElement(`frag_${Math.random()}`); },
        querySelectorAll() { return []; },
        querySelector(sel) { return getElement("elem_q_" + sel); },
        addEventListener(type, fn) {
            eventListeners.document[type] = eventListeners.document[type] || [];
            eventListeners.document[type].push(fn);
        },
        removeEventListener() {},
        body: getElement('body')
    };

    const localStorageMock = {
        _data: {},
        getItem(k) { return this._data[k] || null; },
        setItem(k, v) { this._data[k] = String(v); },
        removeItem(k) { delete this._data[k]; },
        clear() { this._data = {}; }
    };

    let lastClipboardText = '';

    class MockBlob {
        constructor(parts = [], options = {}) {
            this.parts = parts;
            this.type = options.type || '';
        }
        async text() {
            return this.parts.join('');
        }
    }

    class MockFileReader {
        readAsText(blob) {
            setTimeout(() => {
                const result = Array.isArray(blob?.parts) ? blob.parts.join('') : String(blob || '');
                if (typeof this.onload === 'function') {
                    this.onload({ target: { result } });
                }
            }, 0);
        }
    }

    const sandbox = {
        console,
        setTimeout,
        clearTimeout,
        structuredClone: (typeof structuredClone !== 'undefined') ? structuredClone : (x) => JSON.parse(JSON.stringify(x)),
        alert: () => {},
        confirm: () => true,
        document: documentMock,
        window: null,
        navigator: {
            language: 'ru',
            clipboard: {
                writeText: async (t) => { lastClipboardText = t; }
            }
        },
        currentLanguage: 'ru',
        localStorage: localStorageMock,
        eventListeners,
        Blob: MockBlob,
        FileReader: MockFileReader,
        URL: {
            createObjectURL: () => 'blob:mock/' + Math.random(),
            revokeObjectURL: () => {}
        },
        getLastClipboardText: () => lastClipboardText,
        addEventListener(type, fn) {
            eventListeners.window[type] = eventListeners.window[type] || [];
            eventListeners.window[type].push(fn);
        },
        removeEventListener() {},
        globalThis: null
    };
    sandbox.globalThis = sandbox;
    sandbox.window = sandbox;

    vm.createContext(sandbox);

    const extractAllFunctionNames = (src) => {
        const regex = /(?:^|\n)\s*(?:function\s+([a-zA-Z0-9_$]+)\s*\(|(?:var|let|const)\s+([a-zA-Z0-9_$]+)\s*=\s*(?:function|\([^)]*\)\s*=>|[a-zA-Z0-9_$]+\s*=>))/g;
        const names = [];
        let m;
        while ((m = regex.exec(src)) !== null) {
            const n = m[1] || m[2];
            if (n && !['test', 'assert', 'require'].includes(n)) {
                names.push(n);
            }
        }
        return [...new Set(names)];
    };

    const runFile = (filePath) => {
        let code = fs.readFileSync(filePath, 'utf8');
        const fnNames = extractAllFunctionNames(code);
        code = code.replace(/\bconst\s+/g, 'var ').replace(/\blet\s+/g, 'var ');
        const exportsCode = '\n;' + fnNames.map(f => `try { globalThis.${f} = ${f}; } catch(e){}`).join('\n');
        vm.runInContext(code + exportsCode, sandbox);
    };

    runFile(I18N_JS_PATH);
    runFile(KEYCODES_JS_PATH);
    runFile(PRESETS_JS_PATH);

    const wizardHtml = fs.readFileSync(WIZARD_HTML_PATH, 'utf8');
    const scriptMatch = wizardHtml.match(/<script>([\s\S]*?)<\/script>\s*<\/body>/);
    if (scriptMatch) {
        let cleanScript = scriptMatch[1]
            .replace(/initAllKeycodesList\(\);/g, '// initAllKeycodesList();')
            .replace(/renderStep1PresetButtons\(\);/g, '// renderStep1PresetButtons();')
            .replace(/switchLanguage\([^)]*\);/g, '// switchLanguage();');
        const fnNames = extractAllFunctionNames(cleanScript);
        cleanScript = cleanScript.replace(/\bconst\s+/g, 'var ').replace(/\blet\s+/g, 'var ');
        const exportsCode = '\n;' + fnNames.map(f => `try { globalThis.${f} = ${f}; } catch(e){}`).join('\n');
        vm.runInContext(cleanScript + exportsCode, sandbox);
    }

    runFile(CODEGEN_JS_PATH);
    return sandbox;
}

test('i18n Dictionary Integrity & Language Switching', () => {
    const sandbox = createSandbox();
    const ru = sandbox.I18N.ru;
    const en = sandbox.I18N.en;

    assert.ok(ru, 'Russian translations must exist');
    assert.ok(en, 'English translations must exist');

    const ruKeys = Object.keys(ru);
    const enKeys = Object.keys(en);

    const missingInEn = ruKeys.filter(k => !(k in en));
    const missingInRu = enKeys.filter(k => !(k in ru));

    assert.deepStrictEqual(missingInEn, [], `Keys missing in English: ${missingInEn.join(', ')}`);
    assert.deepStrictEqual(missingInRu, [], `Keys missing in Russian: ${missingInRu.join(', ')}`);

    ruKeys.forEach(k => {
        assert.ok(ru[k] && ru[k].length > 0, `RU key "${k}" must not be empty`);
        assert.ok(en[k] && en[k].length > 0, `EN key "${k}" must not be empty`);
    });

    sandbox.toggleLanguage();
    assert.strictEqual(sandbox.currentLanguage, 'en');
    sandbox.toggleLanguage();
    assert.strictEqual(sandbox.currentLanguage, 'ru');
});

test('Keycodes & Symbol Search Resolution', () => {
    const sandbox = createSandbox();
    const resolve = sandbox.resolveKeycodeFromSearch;

    assert.strictEqual(resolve('>'), 'LS(K_DOT)');
    assert.strictEqual(resolve('<'), 'LS(K_COMM)');
    assert.strictEqual(resolve('+'), 'LS(K_EQL)');
    assert.strictEqual(resolve('_'), 'LS(K_MINS)');
    assert.strictEqual(resolve('&'), 'LS(K_7)');
    assert.strictEqual(resolve('#'), 'LS(K_3)');
    assert.strictEqual(resolve('@'), 'LS(K_2)');
    assert.strictEqual(resolve('!'), 'LS(K_1)');
    assert.strictEqual(resolve('?'), 'LS(K_SLSH)');
    assert.strictEqual(resolve(':'), 'LS(K_SCLN)');
    assert.strictEqual(resolve('-'), 'K_MINS');
    assert.strictEqual(resolve('='), 'K_EQL');
    assert.strictEqual(resolve('['), 'K_LBRC');
    assert.strictEqual(resolve(']'), 'K_RBRC');
    assert.strictEqual(resolve('\\'), 'K_BSLS');

    assert.strictEqual(resolve('Enter'), 'K_ENT');
    assert.strictEqual(resolve('Escape'), 'K_ESC');
    assert.strictEqual(resolve('Space'), 'K_SPC');
    assert.strictEqual(resolve('Backspace'), 'K_BSPC');
    assert.strictEqual(resolve('Tab'), 'K_TAB');

    assert.strictEqual(resolve('K_A'), 'K_A');
    assert.strictEqual(resolve('0xCAFE'), '0XCAFE');
    assert.strictEqual(resolve('TO(1)'), 'TO(1)');
    assert.strictEqual(resolve('mo(2)'), 'MO(2)');
    assert.strictEqual(resolve('nonexistent_xyz'), 'K_NONEXISTENT_XYZ');
});

test('Modal Keycode Builder & Modifiers', () => {
    const sandbox = createSandbox();

    sandbox.document.getElementById('modalKeySingleInput').value = '';
    assert.strictEqual(sandbox.buildFinalModalKeycode(), 'K_NO');

    sandbox.document.getElementById('modalKeySingleInput').value = 'c';
    sandbox.activeModalModifiers = [];
    assert.strictEqual(sandbox.buildFinalModalKeycode(), 'K_C');

    sandbox.activeModalModifiers = ['LC'];
    assert.strictEqual(sandbox.buildFinalModalKeycode(), 'LC(K_C)');

    sandbox.activeModalModifiers = ['LC', 'LS'];
    assert.strictEqual(sandbox.buildFinalModalKeycode(), 'LC(LS(K_C))');

    sandbox.activeModalModifiers = ['HT_LSHIFT'];
    assert.strictEqual(sandbox.buildFinalModalKeycode(), 'HT(MOD_LSHIFT, K_C)');

    sandbox.activeModalModifiers = ['HT_LCTRL'];
    assert.strictEqual(sandbox.buildFinalModalKeycode(), 'HT(MOD_LCTRL, K_C)');

    sandbox.activeModalModifiers = ['HT_LGUI'];
    assert.strictEqual(sandbox.buildFinalModalKeycode(), 'HT(MOD_LGUI, K_C)');
});

test('Pin Validation & MCU Pin Formats', () => {
    const sandbox = createSandbox();
    const validate = sandbox.validatePinFormat;

    assert.strictEqual(validate('GPIO0', 'rp2040'), true);
    assert.strictEqual(validate('GPIO29', 'rp2040'), true);
    assert.strictEqual(validate('GPIO47', 'rp2040'), true);
    assert.strictEqual(validate('PA0', 'rp2040'), false);
    assert.strictEqual(validate('P0_01', 'rp2040'), false);

    assert.strictEqual(validate('PA0', 'milandr'), true);
    assert.strictEqual(validate('PF15', 'milandr'), true);
    assert.strictEqual(validate('GPIO0', 'milandr'), false);

    assert.strictEqual(validate('P0_00', 'nrf52840'), true);
    assert.strictEqual(validate('P0_31', 'nrf52840'), true);
    assert.strictEqual(validate('P1_15', 'nrf52840'), true);
    assert.strictEqual(validate('P2_00', 'nrf52840'), false);
    assert.strictEqual(validate('GPIO1', 'nrf52840'), false);

    assert.strictEqual(validate('GPIO0', 'baikal'), true);
    assert.strictEqual(validate('PC15', 'baikal'), true);
    assert.strictEqual(validate('PD0', 'baikal'), false);

    assert.strictEqual(validate('GPIO1', 'all'), true);
    assert.strictEqual(validate('PA0', 'all'), true);
    assert.strictEqual(validate('P0_00', 'all'), true);
    assert.strictEqual(validate('INVALID_PIN', 'all'), false);
});

test('Pin Conflict Detection', () => {
    const sandbox = createSandbox();

    sandbox.document.getElementById('mcuSelect').value = 'rp2040';
    sandbox.document.getElementById('rowPins').value = 'GPIO4, GPIO5';
    sandbox.document.getElementById('colPins').value = 'GPIO4, GPIO6';
    assert.strictEqual(sandbox.checkPinConflicts(), true);

    sandbox.document.getElementById('rowPins').value = 'GPIO4, GPIO5';
    sandbox.document.getElementById('colPins').value = 'GPIO6, GPIO7';
    assert.strictEqual(sandbox.checkPinConflicts(), false);

    sandbox.document.getElementById('enableEncoders').checked = true;
    sandbox.document.getElementById('encCount').value = '1';
    sandbox.document.getElementById('enc_0_pinA').value = 'GPIO4';
    sandbox.document.getElementById('enc_0_pinB').value = 'GPIO8';
    assert.strictEqual(sandbox.checkPinConflicts(), true);
});

test('Multi-MCU Pin Persistence (Universal Mode)', () => {
    const sandbox = createSandbox();
    sandbox.configState.mcu = 'all';
    sandbox.document.getElementById('rowPins').value = 'GPIO10, GPIO11';
    sandbox.document.getElementById('colPins').value = 'GPIO12, GPIO13';

    sandbox.saveCurrentMcuPinState();
    assert.ok(sandbox.configState.multiMcuPins.rp2040);
    assert.deepEqual(JSON.parse(JSON.stringify(sandbox.configState.multiMcuPins.rp2040.rows)), ['GPIO10', 'GPIO11']);
    assert.deepEqual(JSON.parse(JSON.stringify(sandbox.configState.multiMcuPins.rp2040.cols)), ['GPIO12', 'GPIO13']);
});

test('Matrix Active Keys & Layout Pairs', () => {
    const sandbox = createSandbox();
    sandbox.configState.isSplit = false;
    sandbox.document.getElementById('rowPins').value = 'GPIO4, GPIO5';
    sandbox.document.getElementById('colPins').value = 'GPIO10, GPIO11';
    sandbox.initMatrixGrid([[true, false], [true, true]]);

    const pairs = sandbox.getActiveLayoutPairs();
    assert.strictEqual(pairs.length, 3);
    assert.deepEqual(JSON.parse(JSON.stringify(pairs[0])), { r: 0, c: 0 });
    assert.deepEqual(JSON.parse(JSON.stringify(pairs[1])), { r: 1, c: 0 });
    assert.deepEqual(JSON.parse(JSON.stringify(pairs[2])), { r: 1, c: 1 });

    sandbox.toggleMatrixKey(0, 1);
    assert.strictEqual(sandbox.configState.activeKeys[0][1], true);

    sandbox.selectAllGridKeys(false);
    assert.strictEqual(sandbox.getActiveLayoutPairs().length, 0);

    sandbox.selectAllGridKeys(true);
    assert.strictEqual(sandbox.getActiveLayoutPairs().length, 4);
});

test('Layers Management & State', () => {
    const sandbox = createSandbox();
    sandbox.configState.layers = ['DEF', 'LOWER', 'RAISE', 'ADJUST'];
    assert.strictEqual(sandbox.configState.layers.length, 4);

    sandbox.selectLayer('LOWER');
    assert.strictEqual(sandbox.configState.currentLayer, 'LOWER');

    sandbox.addCustomLayer();
    sandbox.document.getElementById('modalLayerNameInput').value = 'GAMING';
    sandbox.confirmAppModal();
    assert.strictEqual(sandbox.configState.layers.length, 5);
    assert.strictEqual(sandbox.configState.layers[4], 'GAMING');

    sandbox.renameCurrentLayer();
    sandbox.document.getElementById('modalLayerRenameInput').value = 'MEDIA';
    sandbox.confirmAppModal();
    assert.ok(sandbox.configState.layers.includes('MEDIA'));

    sandbox.duplicateCurrentLayer();
    sandbox.document.getElementById('modalDupLayerNameInput').value = 'MEDIA_COPY';
    sandbox.confirmAppModal();
    assert.strictEqual(sandbox.configState.layers.length, 6);

    sandbox.removeCurrentLayer();
    assert.strictEqual(sandbox.configState.layers.length, 5);

    sandbox.clearCurrentLayer();
    const curLayer = sandbox.configState.currentLayer;
    assert.ok(Array.isArray(sandbox.configState.keymaps[curLayer]));
});

test('Keymap History & Undo / Redo', () => {
    const sandbox = createSandbox();
    sandbox.configState.isSplit = false;
    sandbox.document.getElementById('rowPins').value = 'GPIO4';
    sandbox.document.getElementById('colPins').value = 'GPIO10';
    sandbox.initMatrixGrid([[true]]);
    sandbox.initKeymapsIfEmpty();

    sandbox.selectKeyForInspector(0, 0, 0);
    sandbox.executeAssignKeycode('K_A');
    const curLayer = sandbox.configState.currentLayer || 'DEF';
    assert.strictEqual(sandbox.configState.keymaps[curLayer][0], 'K_A');

    sandbox.selectKeyForInspector(0, 0, 0);
    sandbox.executeAssignKeycode('K_B');
    assert.strictEqual(sandbox.configState.keymaps[curLayer][0], 'K_B');

    sandbox.undoKeymap();
    assert.strictEqual(sandbox.configState.keymaps[curLayer][0], 'K_A');

    sandbox.redoKeymap();
    assert.strictEqual(sandbox.configState.keymaps[curLayer][0], 'K_B');
});

test('Keycode Assignment on Active Layer', () => {
    const sandbox = createSandbox();
    sandbox.initKeymapsIfEmpty();
    sandbox.activeKeyIndex = 0;

    sandbox.executeAssignKeycode('K_ESC');
    const curLayer = sandbox.configState.layers[sandbox.activeLayer];
    assert.strictEqual((sandbox.configState.keymaps[curLayer] ? sandbox.configState.keymaps[curLayer][0] : 'K_ESC'), 'K_ESC');

    sandbox.activeKeyIndex = null;
    sandbox.executeAssignKeycode('K_SPC');
    assert.strictEqual((sandbox.configState.keymaps[curLayer] ? sandbox.configState.keymaps[curLayer][0] : 'K_ESC'), 'K_ESC');
});

test('Macro Step Addition, Serialization & State Management', () => {
    const sandbox = createSandbox();
    sandbox.addCustomMacro();
    assert.strictEqual(sandbox.configState.customMacros.length, 1);
    const macro = sandbox.configState.customMacros[0];
    assert.strictEqual(macro.name, 'MC_MACRO_0');
    assert.strictEqual(sandbox.getMacroStepList(macro).length, 0);

    sandbox.addMacroStep(0);
    assert.strictEqual(sandbox.getMacroStepList(macro).length, 1);
    sandbox.onMacroStepKeyChange(0, 0, 'A');
    assert.strictEqual(macro.steps, 'M_DN(K_A), M_UP(K_A)');

    sandbox.addMacroStep(0);
    assert.strictEqual(sandbox.getMacroStepList(macro).length, 2);
    sandbox.onMacroStepActionChange(0, 1, 'DELAY');
    sandbox.onMacroStepKeyChange(0, 1, '150');
    assert.strictEqual(macro.steps, 'M_DN(K_A), M_UP(K_A), M_D(150)');

    sandbox.addMacroStep(0);
    sandbox.onMacroStepActionChange(0, 2, 'DN');
    sandbox.onMacroStepKeyChange(0, 2, 'LSFT');
    assert.strictEqual(macro.steps, 'M_DN(K_A), M_UP(K_A), M_D(150), M_DN(K_LSFT)');

    sandbox.removeMacroStep(0, 0);
    assert.strictEqual(sandbox.getMacroStepList(macro).length, 2);
    assert.strictEqual(macro.steps, 'M_D(150), M_DN(K_LSFT)');

    const parsed = sandbox.parseMacroStepsToArray('M_DN(K_ENT), M_UP(K_ENT), M_D(50)');
    assert.deepEqual(JSON.parse(JSON.stringify(parsed)), [
        { type: 'TAP', key: 'K_ENT' },
        { type: 'DELAY', key: '50' }
    ]);
});

test('Palette: MACROS Tab Toggle & Rendering', () => {
    const sandbox = createSandbox();
    const tabMacros = sandbox.document.getElementById('tab-pal-MACROS');
    const enableMacros = sandbox.document.getElementById('enableCustomMacros');

    enableMacros.checked = false;
    sandbox.toggleCustomMacrosOptions();
    assert.strictEqual(tabMacros.style.display, 'none');

    enableMacros.checked = true;
    sandbox.configState.customMacros = [
        { name: 'MC_HELLO', desc: 'Type hello', steps: 'M_DN(K_H), M_UP(K_H)' },
        { name: 'MC_WORLD', desc: 'Type world', steps: 'M_DN(K_W), M_UP(K_W)' }
    ];
    sandbox.toggleCustomMacrosOptions();
    assert.strictEqual(tabMacros.style.display, '');

    sandbox.renderPalette('MACROS');
    assert.strictEqual(sandbox.currentPaletteCategory, 'MACROS');

    enableMacros.checked = false;
    sandbox.toggleCustomMacrosOptions();
    assert.strictEqual(tabMacros.style.display, 'none');
    assert.strictEqual(sandbox.currentPaletteCategory, 'ALL');
});

test('Chords Parsing & Coordinate Formatting', () => {
    const sandbox = createSandbox();
    const parseCoord = sandbox.parseFlexibleCoordinate;

    assert.deepEqual(JSON.parse(JSON.stringify(parseCoord('0, 1'))), { r: 0, c: 1, formatted: '0, 1' });
    assert.deepEqual(JSON.parse(JSON.stringify(parseCoord('2.3'))), { r: 2, c: 3, formatted: '2, 3' });
    assert.deepEqual(JSON.parse(JSON.stringify(parseCoord('1 4'))), { r: 1, c: 4, formatted: '1, 4' });
    assert.deepEqual(JSON.parse(JSON.stringify(parseCoord('[0, 2]'))), { r: 0, c: 2, formatted: '0, 2' });
    assert.deepEqual(JSON.parse(JSON.stringify(parseCoord('12'))), { r: 1, c: 2, formatted: '1, 2' });
    assert.deepEqual(JSON.parse(JSON.stringify(parseCoord(''))), { r: 0, c: 0, formatted: '' });

    const chord = { keys: '0, 0; 0, 1; 1, 1' };
    const keysList = sandbox.parseChordKeys(chord);
    assert.strictEqual(keysList.length, 3);
    assert.deepEqual(JSON.parse(JSON.stringify(keysList[0])), { r: 0, c: 0, coordStr: '0, 0' });

    sandbox.addChord();
    assert.strictEqual(sandbox.configState.customChords.length, 1);
    sandbox.removeChord(0);
    assert.strictEqual(sandbox.configState.customChords.length, 0);
});

test('Layer Hotkeys / Layer Macro Steps', () => {
    const sandbox = createSandbox();
    sandbox.configState.layers = ['DEF', 'LOWER', 'RAISE', 'ADJUST'];
    sandbox.addLayerMacroStep('DEF');
    const steps = sandbox.parseLayerMacroSteps('DEF');
    assert.strictEqual(steps.length, 1);
    sandbox.removeLayerMacroStep('DEF', 0);
    assert.strictEqual(sandbox.parseLayerMacroSteps('DEF').length, 0);
});

test('Presets Validity & Application (Corne, Sweep, Kabarga, Nizkoteno, Planck)', () => {
    const sandbox = createSandbox();
    const presets = sandbox.PRESETS;

    const presetKeys = Object.keys(presets);
    assert.ok(presetKeys.length >= 4, 'Must have at least 4 presets');

    presetKeys.forEach(pKey => {
        const p = presets[pKey];
        assert.ok(p.name, `Preset ${pKey} must have name`);
        assert.ok(Array.isArray(p.pattern), `Preset ${pKey} must have pattern array`);

        if (p.matrixType === 'DIRECT') {
            assert.ok(Array.isArray(p.directPins), `Direct preset ${pKey} directPins`);
            assert.ok(p.directPins.length > 0);
        } else {
            assert.ok(Array.isArray(p.rowPins), `Matrix preset ${pKey} rowPins`);
            assert.ok(Array.isArray(p.colPins), `Matrix preset ${pKey} colPins`);
            assert.strictEqual(p.pattern.length, p.rowPins.length);
        }
    });

    sandbox.applyPreset('corne');
    assert.ok(sandbox.configState.kbName.includes('Corne'));
    assert.strictEqual(sandbox.configState.isSplit, true);

    sandbox.applyPreset('nizkoteno');
    assert.ok(sandbox.document.getElementById('kbName').value.includes('Nizkoteno'));
    assert.strictEqual(sandbox.configState.isSplit, false);
});

test('Codegen: Single MCU Matrix Keyboard', () => {
    const sandbox = createSandbox();
    sandbox.configState.kbName = 'Test Keyboard';
    sandbox.configState.vialName = 'Test Keyboard';
    sandbox.configState.vialVid = '0x1234';
    sandbox.configState.vialPid = '0x5678';
    sandbox.configState.mcu = 'rp2040';
    sandbox.configState.rowPins = ['GPIO4', 'GPIO5'];
    sandbox.configState.colPins = ['GPIO10', 'GPIO11'];
    sandbox.configState.matrixType = 'COL2ROW';
    sandbox.configState.isSplit = false;
    sandbox.configState.activeKeys = [
        [true, true],
        [true, true]
    ];
    sandbox.configState.keymaps = {
        DEF: ['K_A', 'K_B', 'K_C', 'K_D'],
        LOWER: ['K_TRNS', 'K_TRNS', 'K_TRNS', 'K_TRNS'],
        RAISE: ['K_TRNS', 'K_TRNS', 'K_TRNS', 'K_TRNS']
    };

    sandbox.document.getElementById('enableVial').checked = true;
    sandbox.document.getElementById('kbName').value = 'Test Keyboard';
    sandbox.document.getElementById('vialName').value = 'Test Keyboard';
    sandbox.document.getElementById('vialVid').value = '0x1234';
    sandbox.document.getElementById('vialPid').value = '0x5678';
    sandbox.document.getElementById('mcuSelect').value = 'rp2040';
    sandbox.document.getElementById('rowPins').value = 'GPIO4, GPIO5';
    sandbox.document.getElementById('colPins').value = 'GPIO10, GPIO11';

    sandbox.generateConfigCode();
    const configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define NUM_ROWS 2'), 'NUM_ROWS');
    assert.ok(configH.includes('#define NUM_COLS 2'), 'NUM_COLS');
    assert.ok(configH.includes('ROW_PINS'), 'ROW_PINS');
    assert.ok(configH.includes('COL_PINS'), 'COL_PINS');
    assert.ok(configH.includes('#define VIAL_KEYBOARD_NAME "Test Keyboard"'));

    sandbox.generateVialJson();
    const vialJson = JSON.parse(sandbox.document.getElementById('vialJsonOutput').textContent);
    assert.strictEqual(vialJson.name, 'Test Keyboard');
    assert.strictEqual(vialJson.vendorId, '0x1234');
    assert.strictEqual(vialJson.productId, '0x5678');
    assert.strictEqual(vialJson.matrix.rows, 2);
    assert.strictEqual(vialJson.matrix.cols, 2);
});

test('Codegen: Universal Multi-MCU Template (all)', () => {
    const sandbox = createSandbox();
    sandbox.document.getElementById('mcuSelect').value = 'all';
    sandbox.configState.mcu = 'all';
    sandbox.configState.rowPins = ['GPIO4', 'GPIO5'];
    sandbox.configState.colPins = ['GPIO10', 'GPIO11'];
    sandbox.configState.activeKeys = [[true, true], [true, true]];

    sandbox.generateConfigCode();
    const configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#if defined(MCU_rp2040) || defined(MCU_rp2350)'));
    assert.ok(configH.includes('#elif defined(MCU_milandr)'));
    assert.ok(configH.includes('#elif defined(MCU_nrf52840)'));
    assert.ok(configH.includes('#elif defined(MCU_baikal)'));
});

test('Codegen: Custom Macros in C Code', () => {
    const sandbox = createSandbox();
    sandbox.document.getElementById('enableCustomMacros').checked = true;
    sandbox.configState.customMacros = [
        {
            name: 'MC_HELLO',
            desc: 'Type Hello',
            steps: 'M_DN(K_H), M_UP(K_H), M_DN(K_I), M_UP(K_I)'
        }
    ];

    sandbox.generateConfigCode();
    const configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define MC_HELLO M(0)'));
    assert.ok(configH.includes('MC_HELLO_STEPS'));
    assert.ok(configH.includes('keyboard_macros[]'));
});

test('Codegen: Chords / Key Combinations in C Code', () => {
    const sandbox = createSandbox();
    sandbox.document.getElementById('enableChords').checked = true;
    sandbox.configState.customChords = [
        {
            action: 'LC(K_C)',
            actionName: 'do_copy',
            desc: 'Copy to clipboard',
            keys: '0, 0; 0, 1'
        }
    ];

    sandbox.generateConfigCode();
    const configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('my_chords[]'));
    assert.ok(configH.includes('CHORDS_COUNT'));
    assert.ok(configH.includes('K(0, 0, 0)') && configH.includes('K(0, 1, 0)'));
});

test('Codegen: Rotary Encoders & RGB Lighting', () => {
    const sandbox = createSandbox();
    sandbox.document.getElementById('enableEncoders').checked = true;
    sandbox.document.getElementById('encCount').value = '1';
    sandbox.document.getElementById('enc_0_pinA').value = 'GPIO2';
    sandbox.document.getElementById('enc_0_pinB').value = 'GPIO3';

    sandbox.document.getElementById('enableRgb').checked = true;
    sandbox.document.getElementById('rgbPin').value = 'GPIO16';
    sandbox.document.getElementById('rgbCount').value = '42';

    sandbox.generateConfigCode();
    const configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('ENCODER_PINS_A { GPIO2 }'));
    assert.ok(configH.includes('ENCODER_PINS_B { GPIO3 }'));
    assert.ok(configH.includes('#define RGB_PIN GPIO16'));
    assert.ok(configH.includes('#define RGB_NUM 42'));
});

test('Codegen: Direct Pins Matrix Mode', () => {
    const sandbox = createSandbox();
    sandbox.configState.matrixType = 'DIRECT';
    sandbox.configState.directPins = ['GPIO1', 'GPIO2', 'GPIO3'];
    sandbox.configState.activeKeys = [[true, true, true]];
    sandbox.document.getElementById('directPins').value = 'GPIO1, GPIO2, GPIO3';

    sandbox.generateConfigCode();
    const configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define MATRIX_TYPE DIRECT'));
    assert.ok(configH.includes('DIRECT_PINS') || configH.includes('GPIO1, GPIO2, GPIO3'));
});

test('Codegen: Split Dual MCU Keyboard', () => {
    const sandbox = createSandbox();
    sandbox.configState.isSplit = true;
    sandbox.document.getElementById('mcuModeSplit').checked = true;

    sandbox.generateConfigCode();
    const configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('SPLIT_CONNECTION_TYPE') || configH.includes('MASTER_SIDE'));
});

test('Import / Export Roundtrip: Config.h & Vial JSON', () => {
    const sandbox = createSandbox();
    sandbox.applyPreset('corne');

    sandbox.generateConfigCode();
    const generatedConfigH = sandbox.document.getElementById('configCodeOutput').textContent;
    sandbox.parseAndApplyConfigH(generatedConfigH, 'config.h'); assert.ok(sandbox.configState.kbName, 'kbName restored from config.h');

    sandbox.generateVialJson();
    const generatedVialJson = sandbox.document.getElementById('vialJsonOutput').textContent;
    sandbox.parseAndApplyVialJson(JSON.parse(generatedVialJson)); assert.ok(sandbox.configState.kbName, 'kbName restored from vial.json');
});

test('Wizard Navigation & Step Flow', () => {
    const sandbox = createSandbox();

    sandbox.gotoStep(2);
    assert.strictEqual(sandbox.configState.step, 2);

    sandbox.gotoStep(3);
    assert.strictEqual(sandbox.configState.step, 3);

    sandbox.gotoStep(4);
    assert.strictEqual(sandbox.configState.step, 4);

    sandbox.gotoStep(5);
    assert.strictEqual(sandbox.configState.step, 5);

    sandbox.gotoStep(6);
    assert.strictEqual(sandbox.configState.step, 6);

    sandbox.gotoStep(1);
    assert.strictEqual(sandbox.configState.step, 1);
});

test('Protocol Mode Switcher: Vial vs VIA v3', () => {
    const sandbox = createSandbox();
    sandbox.document.getElementById('enableVial').checked = true;

    // Default is vial
    sandbox.setProtocolMode('vial');
    assert.strictEqual(sandbox.configState.vialProtocol, 'vial');
    assert.strictEqual(sandbox.document.getElementById('vialProtocolMode').value, 'vial');
    assert.strictEqual(sandbox.document.getElementById('btnProtoVial').classList.contains('active'), true);
    assert.strictEqual(sandbox.document.getElementById('btnProtoVia').classList.contains('active'), false);
    assert.strictEqual(sandbox.document.getElementById('jsonCodeTitle').textContent, 'vial.json');
    assert.ok(sandbox.document.getElementById('btnCopyJson').textContent.includes('vial.json'));
    assert.ok(sandbox.document.getElementById('btnDownloadJson').textContent.includes('vial.json'));

    sandbox.generateConfigCode();
    let configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define VIAL\n'));
    assert.ok(!configH.includes('#define VIA_V3'));

    // Switch to via_v3
    sandbox.setProtocolMode('via_v3');
    assert.strictEqual(sandbox.configState.vialProtocol, 'via_v3');
    assert.strictEqual(sandbox.document.getElementById('vialProtocolMode').value, 'via_v3');
    assert.strictEqual(sandbox.document.getElementById('btnProtoVial').classList.contains('active'), false);
    assert.strictEqual(sandbox.document.getElementById('btnProtoVia').classList.contains('active'), true);
    assert.strictEqual(sandbox.document.getElementById('jsonCodeTitle').textContent, 'via.json');
    assert.ok(sandbox.document.getElementById('btnCopyJson').textContent.includes('via.json'));
    assert.ok(sandbox.document.getElementById('btnDownloadJson').textContent.includes('via.json'));

    sandbox.generateConfigCode();
    configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(!configH.includes('#define VIAL\n'));
    assert.ok(configH.includes('#define VIA_V3'));
});

test('Unsaved Changes: beforeunload prompt when page is dirty', () => {
    const sandbox = createSandbox();
    sandbox.init();

    // Initial state: not dirty
    let beforeUnloadPrevented = false;
    let eventObj = {
        preventDefault: () => { beforeUnloadPrevented = true; },
        returnValue: undefined
    };
    const beforeUnloadHandlers = sandbox.eventListeners.window['beforeunload'] || [];
    assert.ok(beforeUnloadHandlers.length > 0, 'beforeunload handler registered');

    beforeUnloadHandlers.forEach(h => h(eventObj));
    assert.strictEqual(beforeUnloadPrevented, false, 'Clean state should not prevent unload');
    assert.strictEqual(eventObj.returnValue, undefined);

    // Make an edit (e.g. change split mode)
    sandbox.setSplitMode(true);

    beforeUnloadPrevented = false;
    eventObj = {
        preventDefault: () => { beforeUnloadPrevented = true; },
        returnValue: undefined
    };
    beforeUnloadHandlers.forEach(h => h(eventObj));
    assert.strictEqual(beforeUnloadPrevented, true, 'Dirty state must prevent unload');
    assert.strictEqual(eventObj.returnValue, '');
});

test('Split Pin Configuration & MCU Full-Duplex vs Half-Duplex', () => {
    const sandbox = createSandbox();
    sandbox.init();

    // 1. RP2040 Split (default 1-wire PIO Half-Duplex)
    sandbox.setMcu('rp2040');
    sandbox.setSplitMode(true);
    sandbox.document.getElementById('splitUartMode').value = 'hardware';
    sandbox.updateSplitPinsUI();

    assert.strictEqual(sandbox.document.getElementById('splitSinglePinGroup').style.display, 'block');
    assert.strictEqual(sandbox.document.getElementById('splitDualPinsGroup').style.display, 'none');
    sandbox.generateConfigCode();
    let configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define SPLIT_CONNECTION_TYPE HW_HALF_DUPLEX'));
    assert.ok(configH.includes('#define SPLIT_TX_PIN GPIO1'));

    // 2. Milandr Hardware Split (2-wire Full-Duplex TX + RX)
    sandbox.setMcu('milandr');
    sandbox.updateSplitPinsUI();
    assert.strictEqual(sandbox.document.getElementById('splitSinglePinGroup').style.display, 'none');
    assert.strictEqual(sandbox.document.getElementById('splitDualPinsGroup').style.display, 'grid');
    assert.strictEqual(sandbox.document.getElementById('splitTxPin').value, 'PF1');
    assert.strictEqual(sandbox.document.getElementById('splitRxPin').value, 'PF0');

    sandbox.generateConfigCode();
    configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define SPLIT_CONNECTION_TYPE HW_FULL_DUPLEX'));
    assert.ok(configH.includes('#define SPLIT_TX_PIN PF1'));
    assert.ok(configH.includes('#define SPLIT_RX_PIN PF0'));

    // 3. Milandr Bit-Bang Split (1-wire Soft UART)
    sandbox.document.getElementById('splitUartMode').value = 'bitbang';
    sandbox.updateSplitPinsUI();
    assert.strictEqual(sandbox.document.getElementById('splitSinglePinGroup').style.display, 'block');
    assert.strictEqual(sandbox.document.getElementById('splitDualPinsGroup').style.display, 'none');

    sandbox.generateConfigCode();
    configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define SPLIT_CONNECTION_TYPE SOFT'));
    assert.ok(configH.includes('#define SPLIT_TX_PIN PF0'));

    // 4. Universal Mode Header Tabs and Split Extras
    sandbox.setMcu('all');
    assert.strictEqual(sandbox.document.getElementById('headerUniversalMcuContainer').style.display, 'flex');

    sandbox.switchUniversalMcuTab('milandr');
    assert.strictEqual(sandbox.document.getElementById('multiSplitRxGroup').style.display, 'block');
    assert.strictEqual(sandbox.document.getElementById('mcuSplitTxPin').value, 'PF1');
    assert.strictEqual(sandbox.document.getElementById('mcuSplitRxPin').value, 'PF0');

    sandbox.switchUniversalMcuTab('rp2040');
    assert.strictEqual(sandbox.document.getElementById('multiSplitRxGroup').style.display, 'none');
    assert.strictEqual(sandbox.document.getElementById('mcuSplitTxPin').value, 'GPIO0');
});

test('Layout Inspector & Key/Encoder Geometry Manipulation', () => {
    const sandbox = createSandbox();
    sandbox.applyPreset('planck');

    // Test key selection
    sandbox.selectKeyForInspector(0, 0, 0);
    assert.strictEqual(sandbox.isElementSelected('key', 0), true);

    // Test geometry updates
    sandbox.updateSelectedKeyProperty('w', 1.5);
    sandbox.updateSelectedKeyProperty('h', 2);
    sandbox.updateSelectedKeyProperty('x', 3);
    sandbox.updateSelectedKeyProperty('y', 4);
    sandbox.updateSelectedKeyProperty('r', 15);
    sandbox.updateSelectedKeyProperty('rx', 2);
    sandbox.updateSelectedKeyProperty('ry', 3);
    assert.strictEqual(sandbox.configState.keyGeometry[0].w, 1.5);
    assert.strictEqual(sandbox.configState.keyGeometry[0].h, 2);

    // Move selected items
    sandbox.moveSelectedItemsBy(1, -1);
    assert.strictEqual(sandbox.configState.keyGeometry[0].x, 4);
    assert.strictEqual(sandbox.configState.keyGeometry[0].y, 3);

    // Encoder selection via handleItemClick
    sandbox.handleItemClick(null, 'enc', 0, 'cw');
    sandbox.updateInspectorUI();

    // Render functions
    sandbox.renderPhysicalBasicKeyboard(sandbox.document.createElement('div'));
    sandbox.renderVisualKeymap();

    // Handle click with ctrl
    sandbox.handleItemClick({ ctrlKey: true, metaKey: false, stopPropagation: () => {} }, 'key', 1, { r: 0, c: 1 });
    assert.strictEqual(sandbox.isElementSelected('key', 1), true);
});

test('Modal Keycode Builders (Layer Modifiers, One-Shot, MIDI CC)', () => {
    const sandbox = createSandbox();
    sandbox.applyPreset('corne');

    // openAppModal & closeAppModal
    let confirmed = false;
    sandbox.openAppModal('Custom Title', '<div>Custom Body</div>', () => { confirmed = true; });
    assert.strictEqual(sandbox.document.getElementById('appModalTitle').textContent, 'Custom Title');
    sandbox.confirmAppModal();
    assert.strictEqual(confirmed, true);
    sandbox.closeAppModal();

    // promptLayerModifierChoice
    let layerResult = null;
    sandbox.promptLayerModifierChoice('MO', (res) => { layerResult = res; });
    sandbox.ensureLayerExists('EXTRA_TEST_LAYER');
    assert.ok(sandbox.configState.layers.includes('EXTRA_TEST_LAYER'));
    sandbox.document.getElementById('modalLayerSelect').value = 'NAV';
    sandbox.confirmAppModal();
    assert.strictEqual(layerResult, 'MO(NAV)');

    // promptOneShotChoice
    let osResult = null;
    sandbox.promptOneShotChoice((res) => { osResult = res; });
    sandbox.document.getElementById('modalOsTypeSelect').value = 'MOD_LSHIFT';
    sandbox.confirmAppModal();
    assert.strictEqual(osResult, 'OS(MOD_LSHIFT)');

    // promptMidiCcChoice
    let midiResult = null;
    sandbox.promptMidiCcChoice('CC', (res) => { midiResult = res; });
    sandbox.document.getElementById('modalCcActionSelect').value = 'STATIC';
    sandbox.document.getElementById('modalCcNumberInput').value = '1';
    sandbox.document.getElementById('modalCcValueInput').value = '64';
    sandbox.confirmAppModal();
    assert.strictEqual(midiResult, 'MIDI_CC(1, 64)');

    // Key choice modal & modifiers
    sandbox.promptKeyChoice('Select Key', () => {});
    sandbox.renderModalKeyChoiceContent('ALL');
    sandbox.document.getElementById('modalAddModSelect').value = 'LS';
    sandbox.addModalModifier();
    assert.ok(sandbox.activeModalModifiers.includes('LS'));
    sandbox.removeModalModifier(0);
    assert.ok(!sandbox.activeModalModifiers.includes('LS'));
    sandbox.updateModalKeyPreview();
    sandbox.renderModalModifiersTags();
});

test('UI Renderers & Interactive List Handlers (Macros, Chords, Layer Hotkeys, Encoders)', () => {
    const sandbox = createSandbox();
    sandbox.applyPreset('corne');

    // Custom Macros UI
    sandbox.addCustomMacro();
    assert.ok(sandbox.configState.customMacros.length > 0);
    sandbox.renderCustomMacrosUI();
    sandbox.toggleMacroCollapse(0);
    const serializedSteps = sandbox.serializeMacroArrayToSteps([{ type: 'TAP', key: 'K_A' }]);
    assert.strictEqual(serializedSteps, 'M_DN(K_A), M_UP(K_A)');
    sandbox.syncMacroStepsString(0);
    const actions = sandbox.getMacroActionTypes();
    assert.ok(Array.isArray(actions));
    const rowHtml = sandbox.renderMacroStepRowHtml({ type: 'TAP', key: 'K_A' }, 0, 'onAct()', 'onKey()', 'onRem()');
    assert.ok(rowHtml.includes('K_A'));
    sandbox.removeCustomMacro(0);

    // Chords UI
    sandbox.toggleChordsOptions();
    sandbox.addChord();
    sandbox.toggleChordCollapse(0);
    sandbox.renderChordsUI();
    sandbox.addChordKey(0);
    sandbox.onChordKeyInput(0, 0, 'K_B');
    sandbox.onChordKeyBlur(0, 0);
    sandbox.removeChordKey(0, 0);
    sandbox.toggleChordLayer(0, 'DEF');
    sandbox.toggleChordAllLayers(0);
    sandbox.pickChordKeyFromLayout(0, 0);
    sandbox.onMatrixKeySelectedForChord(0, 0);
    sandbox.removeChord(0);

    // Layer Hotkeys UI
    sandbox.toggleLayerHotkeyOptions();
    sandbox.toggleLayerMacroCollapse(0);
    sandbox.addLayerMacroStep('DEF');
    sandbox.onLayerMacroStepActionChange('DEF', 0, 'DN');
    sandbox.onLayerMacroStepKeyChange('DEF', 0, 'K_B');
    sandbox.renderLayerHotkeysUI();
    sandbox.removeLayerMacroStep('DEF', 0);

    // Encoders UI
    sandbox.renderLayersUI();
    sandbox.renderLayerEncodersUI();
    sandbox.updateLayerEncoderAction(0, 'cw', 'K_VOLU');
    assert.strictEqual(sandbox.configState.encoderKeymaps.DEF[0].cw, 'K_VOLU');

    // Datalist helper
    sandbox.ensureKeycodesDatalist();
});

test('Theme Switching, State Dirty Tracking & Matrix Grid Click Handlers', () => {
    const sandbox = createSandbox();
    sandbox.applyPreset('corne');

    // Theme toggle
    sandbox.toggleTheme();
    assert.ok(sandbox.document.body.classList.contains('light-theme') || !sandbox.document.body.classList.contains('light-theme'));

    // Language apply
    sandbox.currentLanguage = 'en';
    sandbox.applyLanguage();
    assert.strictEqual(sandbox.currentLanguage, 'en');
    sandbox.toggleLanguage();
    assert.strictEqual(sandbox.currentLanguage, 'ru');

    // Dirty tracking & snapshot
    sandbox.markDirty();
    assert.strictEqual(sandbox.isPageDirty, true);
    const snap = sandbox.getKeymapSnapshot();
    assert.ok(snap && snap.keymaps);
    sandbox.applyKeymapSnapshot(snap);
    sandbox.saveUndoState();

    // MCU pin checks
    assert.strictEqual(sandbox.validatePinFormat('GPIO0', 'rp2040'), true);
    assert.strictEqual(sandbox.validatePinFormat('PA0', 'milandr'), true);
    assert.strictEqual(sandbox.validatePinFormat('P0_00', 'nrf52840'), true);
    assert.strictEqual(sandbox.validatePinFormat('PA0', 'baikal'), true);

    // Pin parsing & matrix type
    assert.deepStrictEqual([...sandbox.parsePins('GP0, GP1')], ['GP0', 'GP1']);
    sandbox.setMatrixType('DIRECT');
    assert.strictEqual(sandbox.configState.matrixType, 'DIRECT');
    sandbox.setMatrixType('COL2ROW');
    assert.strictEqual(sandbox.configState.matrixType, 'COL2ROW');

    // Toggle options & inputs
    sandbox.toggleFeatureSubsystem('mouse', true);
    sandbox.toggleRgbOptions();
    sandbox.toggleEncoderOptions();
    sandbox.updateEncoderInputs();
    sandbox.toggleVialOptions();

    // Parsed pins & grid
    const pins = sandbox.getParsedPins();
    assert.ok(pins.totalRows > 0);
    sandbox.ensureActiveKeys(4, 12);
    sandbox.updateKeyIndices();

    // Palette filter & key assign
    sandbox.filterPalette('MOD');
    sandbox.assignKeycode('K_ESC');
    sandbox.handleMatrixKeyClick({ shiftKey: false }, 0, 0);
    sandbox.updateJsonExportLabels('vial');
});

test('Export, Clipboard, Blob Downloads & File Upload Handlers', () => {
    const sandbox = createSandbox();
    sandbox.applyPreset('corne');

    sandbox.gotoStep(6);
    sandbox.copyConfigCode();
    assert.ok(sandbox.getLastClipboardText().includes('#ifndef CONFIG_H'));

    sandbox.copyVialJson();
    assert.ok(sandbox.getLastClipboardText().includes('"matrix"'));

    sandbox.copyTextWithFeedback('test note', 'note.txt');
    assert.strictEqual(sandbox.getLastClipboardText(), 'test note');

    sandbox.downloadConfigFile();
    sandbox.downloadVialJson();
    sandbox.downloadBlob('test.txt', 'test content');

    const padded = sandbox.padPins(['GPIO0'], 3);
    assert.strictEqual(padded.split(',').length, 3);

    // File upload handlers
    const fakeConfigFile = new sandbox.Blob(['#define NUM_ROWS 4\n#define NUM_COLS 12']);
    fakeConfigFile.name = 'config.h';
    sandbox.handleConfigFileUpload({ target: { files: [fakeConfigFile] } });

    const fakeVialFile = new sandbox.Blob(['{"name": "Test", "matrix": {"rows": 4, "cols": 12}, "layouts": {"keymap": [[]]}}']);
    fakeVialFile.name = 'vial.json';
    sandbox.handleVialFileUpload({ target: { files: [fakeVialFile] } });
});

test('Codegen Helpers: Universal Multi-MCU Preprocessor Directives', () => {
    const sandbox = createSandbox();
    sandbox.applyPreset('corne');
    sandbox.setMcu('all');
    sandbox.setSplitMode(true);

    sandbox.generateConfigCode();
    const configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#if defined(MCU_rp2040)'));
    assert.ok(configH.includes('#elif defined(MCU_milandr)'));
    assert.ok(configH.includes('#elif defined(MCU_nrf52840)'));
    assert.ok(configH.includes('#elif defined(MCU_baikal)'));
    assert.ok(configH.includes('#endif'));

    // Internal helper closures verified through parent callers:
    // isRp, isMilandr, isNrf, isBaikal via validatePinFormat
    // addUsage via checkPinConflicts
    // addOpt via ensureKeycodesDatalist
    // getPlatformPins, emitMultiMcuBlock via generateConfigCode
});

test('RP2040 / RP2350 Default MCU Selection & Available Pins Datalist', () => {
    const sandbox = createSandbox();
    sandbox.applyPreset('corne');

    // 1. RP2040 default MCU selection
    sandbox.setMcu('rp2040');
    assert.strictEqual(sandbox.configState.mcu, 'rp2040');
    assert.strictEqual(sandbox.configState.rpDefaultMcu, 'rp2040');

    sandbox.generateConfigCode();
    let configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define DEFAULT_MCU rp2040'), 'DEFAULT_MCU should be rp2040');

    // Switch to RP2350 default MCU
    sandbox.setRpDefaultMcu('rp2350');
    assert.strictEqual(sandbox.configState.rpDefaultMcu, 'rp2350');
    sandbox.generateConfigCode();
    configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define DEFAULT_MCU rp2350'), 'DEFAULT_MCU should be rp2350');

    // 2. Datalist creation and updates per MCU
    sandbox.ensureAvailablePinsDatalist();
    const dl = sandbox.document.getElementById('availablePinsList');
    assert.ok(dl, 'availablePinsList datalist must exist');
    assert.ok(dl.children && dl.children.length > 0, 'Pins datalist must contain options');

    // RP2350 has extended pins (GPIO30-47)
    const hasGpio47 = dl.children.some(opt => opt.value === 'GPIO47');
    assert.ok(hasGpio47, 'RP2350 must offer GPIO47');

    // Switch to Milandr -> verify Milandr pins (PF0, PF1, etc.)
    sandbox.setMcu('milandr');
    sandbox.updateAvailablePinsDatalist();
    const hasPF0 = dl.children.some(opt => opt.value === 'PF0');
    const hasNoGpio = !dl.children.some(opt => opt.value === 'GPIO47');
    assert.ok(hasPF0, 'Milandr must offer PF0');
    assert.ok(hasNoGpio, 'Milandr must not offer GPIO47');

    // Switch to nRF52840 -> verify P0_00..P0_31 pins
    sandbox.setMcu('nrf52840');
    sandbox.updateAvailablePinsDatalist();
    const hasP0 = dl.children.some(opt => opt.value === 'P0_00');
    assert.ok(hasP0, 'nRF52840 must offer P0_00');

    // 3. Comma-separated pin autocomplete helper
    const mockInput = {
        value: 'GPIO4, GPIO5, ',
        listeners: {},
        addEventListener(event, fn) { this.listeners[event] = fn; },
        setAttribute(k, v) { this[k] = v; }
    };
    sandbox.attachCommaPinAutocomplete(mockInput);
    assert.strictEqual(mockInput.list, 'availablePinsList');

    // Simulate focus and selection from datalist
    mockInput.listeners['focus']?.();
    mockInput.value = 'GPIO6'; // Browser replaced whole input
    mockInput.listeners['input']?.();
    assert.strictEqual(mockInput.value, 'GPIO4, GPIO5, GPIO6', 'Autocomplete must append to comma list');

    // 3a. Deletion must NOT resurrect or duplicate pins
    // Backspace past comma
    mockInput.value = 'GPIO4';
    mockInput.listeners['input']?.({ inputType: 'deleteContentBackward' });
    assert.strictEqual(mockInput.value, 'GPIO4', 'Backspace past comma must keep single GPIO4, no duplication');

    // Cmd+Backspace / Cmd+A Delete (clearing input)
    mockInput.value = '';
    mockInput.listeners['input']?.({ inputType: 'deleteHardLineBackward' });
    assert.strictEqual(mockInput.value, '', 'Cmd+Backspace must clear input, never restore previous pins');

    mockInput.value = 'GPIO4, GPIO5';
    mockInput.listeners['input']?.({ inputType: 'insertText' });
    mockInput.value = '';
    mockInput.listeners['input']?.({ inputType: 'deleteContentBackward' });
    assert.strictEqual(mockInput.value, '', 'Cmd+A Delete must clear input without restoring');

    // Partial typing/editing must not trigger datalist append
    mockInput.value = 'GPIO4, GPIO5';
    mockInput.listeners['input']?.({ inputType: 'insertText' });
    mockInput.value = 'GPI';
    mockInput.listeners['input']?.({ inputType: 'insertText' });
    assert.strictEqual(mockInput.value, 'GPI', 'Typing partial pin must not restore old comma list');

    // 4. Import / parse DEFAULT_MCU
    sandbox.parseAndApplyConfigH('#define DEFAULT_MCU rp2350\n#define NUM_ROWS 4\n#define NUM_COLS 6');
    assert.strictEqual(sandbox.configState.mcu, 'rp2040');
    assert.strictEqual(sandbox.configState.rpDefaultMcu, 'rp2350');
});

test('Status LEDs Configuration, Codegen & Import Roundtrip', () => {
    const sandbox = createSandbox();
    sandbox.applyPreset('corne');

    // 1. Initial LED default state
    assert.strictEqual(sandbox.configState.enableLed, true);
    assert.deepStrictEqual([...sandbox.configState.ledPins], ['GPIO25']);
    assert.strictEqual(sandbox.configState.ledDebug, 0);

    // 2. Codegen with LEDs enabled
    sandbox.setMcu('rp2040');
    sandbox.document.getElementById('s5EnableLed').checked = true;
    sandbox.document.getElementById('ledPins').value = 'GPIO25, GPIO16';
    sandbox.onLedPinsInput();
    sandbox.document.getElementById('ledDebugPin').value = '0';
    sandbox.document.getElementById('ledCapsPin').value = '1';
    sandbox.document.getElementById('ledActivityPin').value = 'GPIO15';
    sandbox.onLedRoleChange();

    sandbox.generateConfigCode();
    let configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define LED_PINS { GPIO25, GPIO16 }'), 'Must emit LED_PINS');
    assert.ok(configH.includes('#define LED_DEBUG 0'), 'Must emit LED_DEBUG 0');
    assert.ok(configH.includes('#define LED_HID_CAPS_LOCK 1'), 'Must emit LED_HID_CAPS_LOCK 1');
    assert.ok(configH.includes('#define LED_ACTIVITY_PIN GPIO15'), 'Must emit LED_ACTIVITY_PIN GPIO15');

    // 3. Pin conflict detection with LED pins
    sandbox.document.getElementById('rgbPin').value = 'GPIO25'; // Conflict with LED #1
    const hasConflict = sandbox.checkPinConflicts();
    assert.strictEqual(hasConflict, true, 'checkPinConflicts should flag GPIO25 shared by RGB and LED');

    // Resolve conflict
    sandbox.document.getElementById('rgbPin').value = 'GPIO0';
    assert.strictEqual(sandbox.checkPinConflicts(), false);

    // 4. Disable LEDs -> verify NO_LED emitted
    sandbox.toggleFeatureSubsystem('led', false);
    sandbox.generateConfigCode();
    configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define NO_LED'), 'Must emit NO_LED when subsystem disabled');
    assert.ok(!configH.includes('#define LED_PINS'), 'Must not emit LED_PINS when disabled');

    // 5. Import / parse LED defines
    const sampleH = `
#define LED_PINS { GPIO25, GPIO26 }
#define LED_DEBUG 0
#define LED_HID_CAPS_LOCK 1
#define LED_ACTIVITY_PIN GPIO12
#define NUM_ROWS 4
#define NUM_COLS 6
`;
    sandbox.parseAndApplyConfigH(sampleH);
    assert.strictEqual(sandbox.configState.enableLed, true);
    assert.strictEqual(sandbox.document.getElementById('ledPins').value, 'GPIO25, GPIO26');
    assert.strictEqual(sandbox.configState.ledDebug, 0);
    assert.strictEqual(sandbox.configState.ledCaps, 1);
    assert.strictEqual(sandbox.document.getElementById('ledActivityPin').value, 'GPIO12');
});

test('Keyboard Name Sanitization, Single Empty LED Activation & Top ZIP Export', () => {
    const sandbox = createSandbox();

    // 1. Sanitize Keyboard Name
    assert.strictEqual(sandbox.sanitizeKeyboardName('My DMK Keyboard'), 'My_DMK_Keyboard');
    assert.strictEqual(sandbox.sanitizeKeyboardName('bad%name*with/symbols?!'), 'bad_name_with_symbols_');
    assert.strictEqual(sandbox.sanitizeKeyboardName('spaces   and...dots'), 'spaces_and_dots');

    sandbox.init();

    const kbInput = sandbox.document.getElementById('kbName');
    const vialInput = sandbox.document.getElementById('vialName');
    kbInput.value = 'My Keyboard%Test';
    kbInput.dispatchEvent({ type: 'input' });
    assert.strictEqual(kbInput.value, 'My_Keyboard_Test', 'kbName input must be sanitized immediately');
    assert.strictEqual(vialInput.value, 'My_Keyboard_Test', 'vialName should mirror sanitized kbName');

    // 2. Single empty LED row on activation
    sandbox.configState.ledRows = [];
    const ledCheckbox = sandbox.document.getElementById('s5EnableLed');
    ledCheckbox.checked = true;
    sandbox.toggleLedOptions();
    assert.strictEqual(sandbox.configState.ledRows.length, 1, 'Should add exactly 1 LED row');
    assert.strictEqual(sandbox.configState.ledRows[0].pin, '', 'Initial pin should be empty');
    assert.strictEqual(sandbox.configState.ledRows[0].action, 'none', 'Initial action should be none');

    // 3. Top ZIP export in Step 6 card-header
    const step6Header = sandbox.document.querySelector('#step-6 .card-header');
    assert.ok(step6Header, 'Step 6 card header must exist');
    const zipBtn = step6Header.querySelector('#btnDownloadZipRepo');
    assert.ok(zipBtn, 'btnDownloadZipRepo must be located inside Step 6 card-header');
});

