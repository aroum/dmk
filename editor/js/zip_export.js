// =========================================================================
// Standalone GitHub Repository ZIP Builder (Pure JS, zero external deps)
// =========================================================================

function buildZipFile(files) {
    function crc32(buf) {
        let table = crc32.table;
        if (!table) {
            table = new Uint32Array(256);
            for (let i = 0; i < 256; i++) {
                let c = i;
                for (let k = 0; k < 8; k++) {
                    c = (c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1);
                }
                table[i] = c >>> 0;
            }
            crc32.table = table;
        }
        let crc = 0xFFFFFFFF;
        for (let i = 0; i < buf.length; i++) {
            crc = table[(crc ^ buf[i]) & 0xFF] ^ (crc >>> 8);
        }
        return (crc ^ 0xFFFFFFFF) >>> 0;
    }

    const enc = new TextEncoder();
    const entries = [];
    let offset = 0;

    for (const f of files) {
        const nameBytes = enc.encode(f.name);
        const dataBytes = (typeof f.data === 'string') ? enc.encode(f.data) : f.data;
        const crc = crc32(dataBytes);
        const size = dataBytes.length;

        // Local file header (30 bytes + name + data)
        const localHeader = new Uint8Array(30 + nameBytes.length);
        const lView = new DataView(localHeader.buffer);
        lView.setUint32(0, 0x04034b50, true);
        lView.setUint16(4, 20, true); // version needed
        lView.setUint16(6, 0x0800, true); // utf-8 flag
        lView.setUint16(8, 0, true); // no compression (stored)
        lView.setUint16(10, 0, true); // mod time
        lView.setUint16(12, 0, true); // mod date
        lView.setUint32(14, crc, true);
        lView.setUint32(18, size, true);
        lView.setUint32(22, size, true);
        lView.setUint16(26, nameBytes.length, true);
        lView.setUint16(28, 0, true); // extra length
        localHeader.set(nameBytes, 30);

        entries.push({
            nameBytes,
            dataBytes,
            crc,
            size,
            offset,
            localHeader,
        });

        offset += localHeader.length + dataBytes.length;
    }

    let centralDirSize = 0;
    const centralRecords = [];

    for (const e of entries) {
        // Central directory file header (46 bytes + name)
        const cHeader = new Uint8Array(46 + e.nameBytes.length);
        const cView = new DataView(cHeader.buffer);
        cView.setUint32(0, 0x02014b50, true);
        cView.setUint16(4, 20, true); // version made by
        cView.setUint16(6, 20, true); // version needed
        cView.setUint16(8, 0x0800, true); // utf-8 flag
        cView.setUint16(10, 0, true); // no compression
        cView.setUint16(12, 0, true);
        cView.setUint16(14, 0, true);
        cView.setUint32(16, e.crc, true);
        cView.setUint32(20, e.size, true);
        cView.setUint32(24, e.size, true);
        cView.setUint16(28, e.nameBytes.length, true);
        cView.setUint16(30, 0, true); // extra
        cView.setUint16(32, 0, true); // comment length
        cView.setUint16(34, 0, true); // disk start
        cView.setUint16(36, 0, true); // internal attr
        cView.setUint32(38, 0, true); // external attr
        cView.setUint32(42, e.offset, true); // relative offset of local header
        cHeader.set(e.nameBytes, 46);

        centralRecords.push(cHeader);
        centralDirSize += cHeader.length;
    }

    // End of central directory record (22 bytes)
    const eocd = new Uint8Array(22);
    const eView = new DataView(eocd.buffer);
    eView.setUint32(0, 0x06054b50, true);
    eView.setUint16(4, 0, true); // disk #
    eView.setUint16(6, 0, true); // disk with CD
    eView.setUint16(8, entries.length, true); // entries on disk
    eView.setUint16(10, entries.length, true); // total entries
    eView.setUint32(12, centralDirSize, true); // central dir size
    eView.setUint32(16, offset, true); // offset of central dir
    eView.setUint16(20, 0, true); // comment len

    const parts = [];
    for (const e of entries) {
        parts.push(e.localHeader);
        parts.push(e.dataBytes);
    }
    for (const cr of centralRecords) {
        parts.push(cr);
    }
    parts.push(eocd);

    return new Blob(parts, { type: "application/zip" });
}

function downloadRepositoryZip() {
    generateConfigCode();
    generateVialJson();

    const rawName = document.getElementById("kbName")?.value.trim() || "my_keyboard";
    const cleanName = rawName.toLowerCase().replace(/[^a-z0-9_]/g, "_").replace(/_+/g, "_").replace(/^_|_$/g, "")
        || "my_keyboard";
    const targetMcu = (configState.mcu === "all")
        ? (configState.rpDefaultMcu || "rp2040")
        : configState.mcu;
    const isVia = (configState.vialProtocol === "via_v3" || configState.protocolMode === "via_v3");
    const jsonFileName = isVia ? "via.json" : "vial.json";

    const configHContent = document.getElementById("configCodeOutput").textContent;
    const vialJsonContent = document.getElementById("vialJsonOutput").textContent;

    const extraBuildArgs = (targetMcu === "rp2040" || targetMcu === "rp2350" || targetMcu === "nrf52840")
        ? "-c --uf2"
        : "-c";

    const workflowYaml = `name: Build DMK Firmware (${rawName})

on:
  push:
    branches: [ main, master ]
  pull_request:
    branches: [ main, master ]
  workflow_dispatch:

jobs:
  build:
    name: Build Firmware for ${cleanName} (${targetMcu})
    runs-on: ubuntu-latest
    steps:
      - name: Checkout Repository
        uses: actions/checkout@v4
        with:
          submodules: recursive

      - name: Install Toolchain and Dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential cmake ninja-build gcc-arm-none-eabi libnewlib-arm-none-eabi

      - name: Install uv
        uses: astral-sh/setup-uv@v5

      - name: Build Firmware
        run: |
          chmod +x build_all.sh
          ./build_all.sh -b ${cleanName} --mcu ${targetMcu} ${extraBuildArgs}

      - name: Upload Firmware Artifacts
        uses: actions/upload-artifact@v4
        with:
          name: firmware-${cleanName}-${targetMcu}
          path: |
            build/dmk_*
          if-no-files-found: error
`;

    const readmeContent = `# ${rawName} — DMK Firmware Repository

Сгенерировано с помощью веб-конфигуратора DMK Keyboard Wizard.

## Автоматическая сборка через GitHub Actions
1. Создайте пустой репозиторий на [GitHub](https://github.com/new).
2. Распакуйте этот архив в локальную папку.
3. Инициализируйте и запушьте репозиторий:
\`\`\`bash
git init -b main
git add .
git commit -m "feat: initial keyboard configuration for ${cleanName}"
git remote add origin https://github.com/<YOUR_USERNAME>/<REPO_NAME>.git
git push -u origin main
\`\`\`
4. Перейдите во вкладку **Actions** вашего GitHub репозитория. Сборка запустится автоматически, и готовый файл прошивки появится в разделе **Artifacts** (например \`dmk_${cleanName}.uf2\`).

## Локальная сборка
Если вы компилируете в локальном клоне \`dmk\`:
\`\`\`bash
./build_all.sh -b ${cleanName} --mcu ${targetMcu} ${extraBuildArgs}
\`\`\`
`;

    const files = [
        { name: `.github/workflows/build_${cleanName}.yml`, data: workflowYaml },
        { name: `keyboards/${cleanName}/config.h`, data: configHContent },
        { name: `keyboards/${cleanName}/${jsonFileName}`, data: vialJsonContent },
        { name: `README.md`, data: readmeContent },
    ];

    const zipBlob = buildZipFile(files);
    const url = URL.createObjectURL(zipBlob);
    const a = document.createElement("a");
    a.href = url;
    a.download = `${cleanName}_repo.zip`;
    a.click();
    URL.revokeObjectURL(url);
}
