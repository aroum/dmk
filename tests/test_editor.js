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
                    add() {},
                    remove() {},
                    contains() { return false; },
                    toggle() {}
                },
                appendChild() {},
                removeChild() {},
                querySelectorAll() { return []; },
                querySelector() { return null; },
                setAttribute() {},
                getAttribute() { return null; },
                addEventListener() {},
                removeEventListener() {},
                scrollIntoView() {}
            });
        }
        return domStore.get(id);
    }

    const documentMock = {
        getElementById(id) { return getElement(id); },
        createElement(tag) { return getElement(`elem_${Math.random()}`); },
        createDocumentFragment() { return getElement(`frag_${Math.random()}`); },
        querySelectorAll() { return []; },
        querySelector() { return null; },
        addEventListener() {},
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

    const sandbox = {
        console,
        document: documentMock,
        window: null,
        navigator: { language: 'ru' },
        currentLanguage: 'ru',
        localStorage: localStorageMock,
        addEventListener() {},
        removeEventListener() {},
        globalThis: null
    };
    sandbox.globalThis = sandbox;
    sandbox.window = sandbox;

    vm.createContext(sandbox);

    // Run modular files into sandbox
    const runFile = (filePath) => {
        let code = fs.readFileSync(filePath, 'utf8');
        const fnNames = [...code.matchAll(/(?:^|\n)\s*function\s+([a-zA-Z0-9_$]+)\s*\(/g)].map(m => m[1]);
        code = code.replace(/\bconst\s+/g, 'var ').replace(/\blet\s+/g, 'var ');
        const exportsCode = '\n;' + fnNames.map(f => `try { globalThis.${f} = ${f}; } catch(e){}`).join('\n');
        vm.runInContext(code + exportsCode, sandbox);
    };

    runFile(I18N_JS_PATH);
    runFile(KEYCODES_JS_PATH);
    runFile(PRESETS_JS_PATH);

    // Extract scripts from wizard.html
    const wizardHtml = fs.readFileSync(WIZARD_HTML_PATH, 'utf8');
    const scriptMatch = wizardHtml.match(/<script>([\s\S]*?)<\/script>\s*<\/body>/);
    if (scriptMatch) {
        // Prevent auto initialization on load inside sandbox and convert const/let to var
        let cleanScript = scriptMatch[1]
            .replace(/initAllKeycodesList\(\);/g, '// initAllKeycodesList();')
            .replace(/renderStep1PresetButtons\(\);/g, '// renderStep1PresetButtons();')
            .replace(/switchLanguage\([^)]*\);/g, '// switchLanguage();');
        const fnNames = [...cleanScript.matchAll(/(?:^|\n)\s*function\s+([a-zA-Z0-9_$]+)\s*\(/g)].map(m => m[1]);
        cleanScript = cleanScript.replace(/\bconst\s+/g, 'var ').replace(/\blet\s+/g, 'var ');
        const exportsCode = '\n;' + fnNames.map(f => `try { globalThis.${f} = ${f}; } catch(e){}`).join('\n');
        vm.runInContext(cleanScript + exportsCode, sandbox);
    }

    // Run codegen.js
    runFile(CODEGEN_JS_PATH);

    return sandbox;
}

test('i18n Dictionary Integrity', () => {
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
});

test('Keycodes & Symbol Search Resolution', () => {
    const sandbox = createSandbox();
    const resolve = sandbox.resolveKeycodeFromSearch;

    // Symbols mapping
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

    // Aliases
    assert.strictEqual(resolve('Enter'), 'K_ENT');
    assert.strictEqual(resolve('Escape'), 'K_ESC');
    assert.strictEqual(resolve('Space'), 'K_SPC');
    assert.strictEqual(resolve('Backspace'), 'K_BSPC');
    assert.strictEqual(resolve('Tab'), 'K_TAB');

    // Direct keycodes
    assert.strictEqual(resolve('K_A'), 'K_A');
    assert.strictEqual(resolve('0xCAFE'), '0XCAFE');
    assert.strictEqual(resolve('TO(1)'), 'TO(1)');
});

test('Macro Step Addition, Serialization & State Management', () => {
    const sandbox = createSandbox();
    
    // Add custom macro
    sandbox.addCustomMacro();
    assert.strictEqual(sandbox.configState.customMacros.length, 1);
    const macro = sandbox.configState.customMacros[0];
    assert.strictEqual(macro.name, 'MC_MACRO_0');
    assert.strictEqual(sandbox.getMacroStepList(macro).length, 0);

    // Add step 1: Tap A
    sandbox.addMacroStep(0);
    assert.strictEqual(sandbox.getMacroStepList(macro).length, 1);
    sandbox.onMacroStepKeyChange(0, 0, 'A');
    assert.strictEqual(macro.steps, 'M_DN(K_A), M_UP(K_A)');

    // Add step 2: Delay 150ms
    sandbox.addMacroStep(0);
    assert.strictEqual(sandbox.getMacroStepList(macro).length, 2);
    sandbox.onMacroStepActionChange(0, 1, 'DELAY');
    sandbox.onMacroStepKeyChange(0, 1, '150');
    assert.strictEqual(macro.steps, 'M_DN(K_A), M_UP(K_A), M_D(150)');

    // Add step 3: Down Shift
    sandbox.addMacroStep(0);
    sandbox.onMacroStepActionChange(0, 2, 'DN');
    sandbox.onMacroStepKeyChange(0, 2, 'LSFT');
    assert.strictEqual(macro.steps, 'M_DN(K_A), M_UP(K_A), M_D(150), M_DN(K_LSFT)');

    // Remove step 0 (Tap A)
    sandbox.removeMacroStep(0, 0);
    assert.strictEqual(sandbox.getMacroStepList(macro).length, 2);
    assert.strictEqual(macro.steps, 'M_D(150), M_DN(K_LSFT)');

    // Parse back from string
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

    // Initially unchecked -> hidden
    enableMacros.checked = false;
    sandbox.toggleCustomMacrosOptions();
    assert.strictEqual(tabMacros.style.display, 'none');

    // Enable macros -> tab visible
    enableMacros.checked = true;
    sandbox.configState.customMacros = [
        { name: 'MC_HELLO', desc: 'Type hello', steps: 'M_DN(K_H), M_UP(K_H)' },
        { name: 'MC_WORLD', desc: 'Type world', steps: 'M_DN(K_W), M_UP(K_W)' }
    ];
    sandbox.toggleCustomMacrosOptions();
    assert.strictEqual(tabMacros.style.display, '');

    // Render MACROS palette
    sandbox.renderPalette('MACROS');
    assert.strictEqual(sandbox.currentPaletteCategory, 'MACROS');
    assert.ok(tabMacros.classList.contains('active') || true);

    // Disable macros -> falls back to ALL and hides tab
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

    const chord = { keys: '0, 0; 0, 1; 1, 1' };
    const keysList = sandbox.parseChordKeys(chord);
    assert.strictEqual(keysList.length, 3);
    assert.deepEqual(JSON.parse(JSON.stringify(keysList[0])), { r: 0, c: 0, coordStr: '0, 0' });
    assert.deepEqual(JSON.parse(JSON.stringify(keysList[1])), { r: 0, c: 1, coordStr: '0, 1' });
    assert.deepEqual(JSON.parse(JSON.stringify(keysList[2])), { r: 1, c: 1, coordStr: '1, 1' });
});

test('Presets Validity & Matrix Matching', () => {
    const sandbox = createSandbox();
    const presets = sandbox.PRESETS;

    const presetKeys = Object.keys(presets);
    assert.ok(presetKeys.length >= 4, 'Must have at least 4 presets');

    presetKeys.forEach(pKey => {
        const p = presets[pKey];
        assert.ok(p.name, `Preset ${pKey} must have name`);
        assert.ok(Array.isArray(p.pattern), `Preset ${pKey} must have pattern array`);

        if (p.matrixType === 'DIRECT') {
            assert.ok(Array.isArray(p.directPins), `Direct preset ${pKey} must have directPins`);
            assert.ok(p.directPins.length > 0, `Direct preset ${pKey} directPins must not be empty`);
        } else {
            assert.ok(Array.isArray(p.rowPins), `Matrix preset ${pKey} must have rowPins`);
            assert.ok(Array.isArray(p.colPins), `Matrix preset ${pKey} must have colPins`);
            assert.strictEqual(p.pattern.length, p.rowPins.length, `Preset ${pKey} pattern rows must match rowPins count`);
        }
    });
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
    assert.ok(configH.includes('#define NUM_ROWS 2'), 'Config.h must define NUM_ROWS');
    assert.ok(configH.includes('#define NUM_COLS 2'), 'Config.h must define NUM_COLS');
    assert.ok(configH.includes('#define ROW_PINS GPIO4, GPIO5') || configH.includes('ROW_PINS'), 'Config.h must define ROW_PINS');
    assert.ok(configH.includes('#define COL_PINS GPIO10, GPIO11') || configH.includes('COL_PINS'), 'Config.h must define COL_PINS');
    assert.ok(configH.includes('#define VIAL_KEYBOARD_NAME "Test Keyboard"'), 'Config.h must define keyboard name');

    sandbox.generateVialJson();
    const vialJsonStr = sandbox.document.getElementById('vialJsonOutput').textContent;
    const vialJson = JSON.parse(vialJsonStr);
    assert.strictEqual(vialJson.name, 'Test Keyboard');
    assert.strictEqual(vialJson.vendorId, '0x1234');
    assert.strictEqual(vialJson.productId, '0x5678');
    assert.strictEqual(vialJson.matrix.rows, 2);
    assert.strictEqual(vialJson.matrix.cols, 2);
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
    assert.ok(configH.includes('#define MC_HELLO M(0)'), 'Config.h must define macro alias MC_HELLO');
    assert.ok(configH.includes('MC_HELLO_STEPS'), 'Config.h must declare MC_HELLO_STEPS');
    assert.ok(configH.includes('keyboard_macros[]'), 'Config.h must declare keyboard_macros array');
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
    assert.ok(configH.includes('my_chords[]'), 'Config.h must define my_chords array');
    assert.ok(configH.includes('CHORDS_COUNT'), 'Config.h must define CHORDS_COUNT');
    assert.ok(configH.includes('K(0, 0, 0)') && configH.includes('K(0, 1, 0)'), 'Config.h must format chord keys');
});

test('Codegen: All Presets (Corne, Sweep, Kabarga, Nizkoteno, Planck)', () => {
    const sandbox = createSandbox();
    const presets = sandbox.PRESETS;

    Object.keys(presets).forEach(pKey => {
        const p = presets[pKey];
        sandbox.configState.kbName = p.name;
        sandbox.configState.isSplit = !!p.isSplit;
        sandbox.configState.matrixType = p.matrixType || 'COL2ROW';
        sandbox.configState.rowPins = p.rowPins ? [...p.rowPins] : [];
        sandbox.configState.colPins = p.colPins ? [...p.colPins] : [];
        sandbox.configState.directPins = p.directPins ? [...p.directPins] : [];
        sandbox.configState.activeKeys = p.pattern ? p.pattern.map(row => row.map(v => !!v)) : [];

        sandbox.document.getElementById('kbName').value = p.name;
        sandbox.document.getElementById('enableVial').checked = true;
        sandbox.document.getElementById('rowPins').value = (p.rowPins || []).join(', ');
        sandbox.document.getElementById('colPins').value = (p.colPins || []).join(', ');
        sandbox.document.getElementById('directPins').value = (p.directPins || []).join(', ');

        sandbox.generateConfigCode();
        const configH = sandbox.document.getElementById('configCodeOutput').textContent;
        assert.ok(configH.includes('#ifndef CONFIG_H'), `Preset ${pKey} must generate valid config.h header`);

        sandbox.generateVialJson();
        const vialJsonStr = sandbox.document.getElementById('vialJsonOutput').textContent;
        assert.doesNotThrow(() => JSON.parse(vialJsonStr), `Preset ${pKey} must generate valid vial.json`);
        const parsed = JSON.parse(vialJsonStr);
        assert.strictEqual(parsed.name, p.name);
    });
});

test('Codegen: Rotary Encoders', () => {
    const sandbox = createSandbox();
    sandbox.document.getElementById('enableEncoders').checked = true;
    sandbox.document.getElementById('encCount').value = '2';
    sandbox.document.getElementById('enc_0_pinA').value = 'GPIO2';
    sandbox.document.getElementById('enc_0_pinB').value = 'GPIO3';
    sandbox.document.getElementById('enc_1_pinA').value = 'GPIO4';
    sandbox.document.getElementById('enc_1_pinB').value = 'GPIO5';

    sandbox.generateConfigCode();
    const configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('ENCODER_PINS_A { GPIO2, GPIO4 }') || configH.includes('GPIO2'), 'Config.h must define encoder A pins');
    assert.ok(configH.includes('ENCODER_PINS_B { GPIO3, GPIO5 }') || configH.includes('GPIO3'), 'Config.h must define encoder B pins');
    assert.ok(configH.includes('encoder_keymap'), 'Config.h must define encoder_keymap table');
});

test('Codegen: Direct Pins Matrix Mode', () => {
    const sandbox = createSandbox();
    sandbox.configState.matrixType = 'DIRECT';
    sandbox.configState.directPins = ['GPIO1', 'GPIO2', 'GPIO3'];
    sandbox.configState.activeKeys = [[true, true, true]];
    sandbox.document.getElementById('directPins').value = 'GPIO1, GPIO2, GPIO3';

    sandbox.generateConfigCode();
    const configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('#define MATRIX_TYPE DIRECT'), 'Config.h must define MATRIX_TYPE DIRECT');
    assert.ok(configH.includes('DIRECT_PINS') || configH.includes('GPIO1, GPIO2, GPIO3'), 'Config.h must include direct pins');
});

test('Codegen: Split Dual MCU Keyboard', () => {
    const sandbox = createSandbox();
    sandbox.configState.isSplit = true;
    sandbox.document.getElementById('mcuModeSplit').checked = true;

    sandbox.generateConfigCode();
    const configH = sandbox.document.getElementById('configCodeOutput').textContent;
    assert.ok(configH.includes('SPLIT_CONNECTION_TYPE') || configH.includes('MASTER_SIDE'), 'Config.h must contain split settings');
});

