[CmdletBinding()]
param(
    [Alias("c")][switch]$Clean,
    [Alias("f")][switch]$Flash,
    [Alias("u")][switch]$UsbFlash,
    [switch]$Uf2,
    [Alias("h")][switch]$Help,
    [Alias("b")][string]$Keyboard = "corne",
    [string]$Mcu = "milandr",
    [Alias("p")][string]$Probe = "j-link",
    [string]$Memory = "",
    [string]$NrfPort = "",
    [string]$Lang = ""
)

$ErrorActionPreference = "Stop"

# --- LANGUAGE CONFIG ---
$CURRENT_LANG = "ru"
# -----------------------

$ScriptDir = $PSScriptRoot

# Обработка смены языка (перезапись самого файла)
if ($Lang) {
    if ($Lang -notin @("ru", "en")) {
        Write-Host "Invalid language / Неверный язык: $Lang. Use 'ru' or 'en'." -ForegroundColor Red
        exit 1
    }
    
    $scriptPath = $MyInvocation.MyCommand.Path
    $content = Get-Content -Path $scriptPath -Raw
    $content = $content -replace '(?m)^\$CURRENT_LANG\s*=\s*".*"', "`$CURRENT_LANG = `"$Lang`""
    Set-Content -Path $scriptPath -Value $content
    
    if ($Lang -eq "en") {
        Write-Host "Language successfully changed to English." -ForegroundColor Green
    } else {
        Write-Host "Язык успешно изменен на русский." -ForegroundColor Green
    }
    exit 0
}

# --- LOCALIZATION STRINGS ---
if ($CURRENT_LANG -eq "en") {
    $MSG_ERR_KBD_NOT_FOUND = "Keyboard not found in"
    $MSG_AVAIL_KBD = "Available keyboards:"
    $MSG_CLEAN_DIR = "=== Cleaning build directory ==="
    $MSG_INIT_CMAKE = "=== Initializing CMake ==="
    $MSG_BUILD = "=== Building"
    $MSG_BUILD_SUCCESS = "=== Build successful:"
    $MSG_BIN = "Binary:"
    $MSG_UF2_READY_CMAKE = "UF2 is ready (generated via CMake):"
    $MSG_ERR_ELF2UF2 = "elf2uf2 not found! Please install pico-sdk."
    $MSG_ERR_ELF_NOT_FOUND = "ELF not found:"
    $MSG_GEN_UF2 = "=== Generating UF2 ==="
    $MSG_UF2_READY = "UF2 is ready:"
    $MSG_ERR_UF2_NOT_FOUND = "UF2 not found:"
    $MSG_UF2_UNSUPPORTED = "UF2 is not supported for"
    $MSG_ERR_PROBE_CONF = "Probe config not found:"
    $MSG_FLASH_DEBUG = "=== Flashing via debugger ==="
    $MSG_FLASH_SUCCESS = "=== Flash successful ==="
    $MSG_ERR_HEX_NOT_FOUND = "HEX not found:"
    $MSG_ERR_NRF_SCRIPT = "Flash script not found:"
    $MSG_FLASH_USB_NRF = "=== Flashing via USB (adafruit-nrfutil) ==="
    $MSG_NRF_DFU_HINT = "Make sure the board is in DFU mode (double click Reset button)."
    $MSG_FLASH_FAILED = "=== Flash failed ==="
    $MSG_ERR_PICOTOOL = "picotool not found! Please install it."
    $MSG_ERR_DFU_UTIL = "dfu-util not found! Please install it."
    $MSG_FLASH_USB = "=== Flashing via USB ==="
    $MSG_FLASH_USB_DFU = "=== Flashing via USB (dfu-util) ==="
    $MSG_MILANDR_DFU_HINT = "Make sure the board is in DFU mode (hold PB6/button, press Reset, then release PB6)."
    $MSG_PICO_BOOTSEL_ERR = "Device not found in BOOTSEL mode or USB-ready mode."
    $MSG_PICO_BOOTSEL_ACT = "Action Required: Hold BOOTSEL, press Reset, then release BOOTSEL."
    $MSG_PICO_PRESS_KEY = "Press any key when device is ready..."
    $MSG_PICO_CONT = "Continuing..."
    $MSG_PICO_UPLOAD = "Uploading"
    $MSG_PICO_SUCCESS = "Flash Successful! Program started."
    $MSG_PICO_FAIL = "Flash failed! Check connection."
    $MSG_USB_UNSUPPORTED = "USB-flash is not supported for"
} else {
    $MSG_ERR_KBD_NOT_FOUND = "Клавиатура не найдена в"
    $MSG_AVAIL_KBD = "Доступные клавиатуры:"
    $MSG_CLEAN_DIR = "=== Очистка директории сборки ==="
    $MSG_INIT_CMAKE = "=== Инициализация CMake ==="
    $MSG_BUILD = "=== Сборка"
    $MSG_BUILD_SUCCESS = "=== Сборка успешна:"
    $MSG_BIN = "Бинарник:"
    $MSG_UF2_READY_CMAKE = "UF2 готов (сгенерирован через CMake):"
    $MSG_ERR_ELF2UF2 = "elf2uf2 не найден! Установите pico-sdk."
    $MSG_ERR_ELF_NOT_FOUND = "ELF не найден:"
    $MSG_GEN_UF2 = "=== Генерация UF2 ==="
    $MSG_UF2_READY = "UF2 готов:"
    $MSG_ERR_UF2_NOT_FOUND = "UF2 не найден:"
    $MSG_UF2_UNSUPPORTED = "UF2 не поддерживается для"
    $MSG_ERR_PROBE_CONF = "Конфиг отладчика не найден:"
    $MSG_FLASH_DEBUG = "=== Прошивка через отладчик ==="
    $MSG_FLASH_SUCCESS = "=== Прошивка успешна ==="
    $MSG_ERR_HEX_NOT_FOUND = "HEX не найден:"
    $MSG_ERR_NRF_SCRIPT = "Скрипт прошивки не найден:"
    $MSG_FLASH_USB_NRF = "=== Прошивка через USB (adafruit-nrfutil) ==="
    $MSG_NRF_DFU_HINT = "Убедитесь, что плата переведена в режим DFU (двойное нажатие кнопки Reset)."
    $MSG_FLASH_FAILED = "=== Прошивка не удалась ==="
    $MSG_ERR_PICOTOOL = "picotool не найден! Установите его."
    $MSG_ERR_DFU_UTIL = "dfu-util не найден! Установите его."
    $MSG_FLASH_USB = "=== Прошивка через USB ==="
    $MSG_FLASH_USB_DFU = "=== Прошивка через USB (dfu-util) ==="
    $MSG_MILANDR_DFU_HINT = "Убедитесь, что плата переведена в режим DFU (зажав кнопку PB6 при сбросе)."
    $MSG_PICO_BOOTSEL_ERR = "Устройство не найдено в режиме BOOTSEL или режиме USB."
    $MSG_PICO_BOOTSEL_ACT = "Необходимое действие: Удерживайте BOOTSEL, нажмите Reset, затем отпустите BOOTSEL."
    $MSG_PICO_PRESS_KEY = "Нажмите любую клавишу, когда устройство будет готово..."
    $MSG_PICO_CONT = "Продолжаем..."
    $MSG_PICO_UPLOAD = "Загрузка"
    $MSG_PICO_SUCCESS = "Прошивка успешна! Программа запущена."
    $MSG_PICO_FAIL = "Прошивка не удалась! Проверьте подключение."
    $MSG_USB_UNSUPPORTED = "USB-flash не поддерживается для"
}

# Print RAM/FLASH usage table (same format as Milandr CMake output)
function Show-MemoryUsage {
    param([string]$ElfFile, [string]$Mcu, [string]$MemoryOverride)

    if (-not (Get-Command arm-none-eabi-size -ErrorAction SilentlyContinue)) { return }
    if (-not (Test-Path $ElfFile)) { return }

    $sizeOut = & arm-none-eabi-size $ElfFile 2>$null | Select-Object -Last 1
    $parts = ($sizeOut -split '\s+').Where({ $_ -ne '' })
    if ($parts.Count -lt 3 -or $parts[0] -eq 'text') { return }

    $text = [int64]$parts[0]
    $data = [int64]$parts[1]
    $bss  = [int64]$parts[2]
    $flashUsed = $text + $data
    $ramUsed   = $data + $bss

    # Default sizes per MCU
    switch ($Mcu) {
        "rp2040"   { $flashSize = 2 * 1024 * 1024; $ramSize = 264 * 1024 }
        "rp2350"   { $flashSize = 4 * 1024 * 1024; $ramSize = 520 * 1024 }
        "nrf52840" { $flashSize = 1 * 1024 * 1024; $ramSize = 256 * 1024 }
        "baikal"   { $flashSize = 1 * 1024 * 1024; $ramSize = 256 * 1024 }
        default    { return } # milandr already printed by CMake
    }

    # Override flash size if --Memory was given (e.g. "2MB", "512KB")
    if ($MemoryOverride -match '^(\d+)(KB|MB)$') {
        $num = [int64]$Matches[1]
        if ($Matches[2] -eq 'KB') { $flashSize = $num * 1024 }
        else                      { $flashSize = $num * 1024 * 1024 }
    }

    function Format-Size([int64]$b) {
        if ($b -ge 1MB)   { return "$([int]($b/1MB)) MB" }
        elseif ($b -ge 1KB) { return "$([int]($b/1KB)) KB" }
        else              { return "$b B" }
    }

    $flashPct = [math]::Round($flashUsed * 100.0 / $flashSize, 2)
    $ramPct   = [math]::Round($ramUsed   * 100.0 / $ramSize,   2)

    Write-Host ("Memory region         Used Size  Region Size  %age Used")
    Write-Host ("           FLASH: {0,11}  {1,11}  {2,8:F2}%" -f "$flashUsed B", (Format-Size $flashSize), $flashPct)
    Write-Host ("             RAM: {0,11}  {1,11}  {2,8:F2}%" -f "$ramUsed B",   (Format-Size $ramSize),   $ramPct)
}

function Show-Help {
    if ($CURRENT_LANG -eq "en") {
        Write-Host "Usage: .\build_all.ps1 [OPTIONS]"
        Write-Host "`nMain parameters:"
        Write-Host "  -b, -Keyboard [NAME]     Keyboard selection (default: corne)"
        Write-Host "  -Mcu [MCU]               MCU selection (milandr/rp2040/rp2350/nrf52840/baikal, default: milandr)"
        Write-Host "  -p, -Probe [PROBE]       Debugger selection (j-link, default: j-link)"
        Write-Host "  -Memory [SIZE]           Memory size selection (e.g., 256KB, 512KB, 2MB, 4MB, 16MB)"
        Write-Host "  -NrfPort [PORT]          Port for flashing nRF52840 (e.g., COM3)"
        Write-Host "  -Lang [LANG]             Set script language (ru or en) and save it"
        Write-Host "`nCombined flags (use spaces in PS!):"
        Write-Host "  -c, -Clean               Perform a clean build"
        Write-Host "  -f, -Flash               Flash via debugger"
        Write-Host "  -u, -UsbFlash            Flash via USB (for RP2040/RP2350/nRF52840/milandr)"
        Write-Host "  -Uf2                     Generate UF2 file (for RP2040/RP2350/nRF52840)"
        Write-Host "  -h, -Help                Show this help"
        Write-Host "`nExamples:"
        Write-Host "  .\build_all.ps1 -Keyboard corne -Clean -Flash"
        Write-Host "  .\build_all.ps1 -Keyboard corne -Clean -UsbFlash -Uf2 -Mcu rp2040"
        Write-Host "  .\build_all.ps1 -Lang en"
    } else {
        Write-Host "Usage: .\build_all.ps1 [OPTIONS]"
        Write-Host "`nОсновные параметры:"
        Write-Host "  -b, -Keyboard [NAME]     Выбор клавиатуры (по умолчанию: corne)"
        Write-Host "  -Mcu [MCU]               Выбор микроконтроллера (milandr/rp2040/rp2350/nrf52840/baikal, по умолчанию: milandr)"
        Write-Host "  -p, -Probe [PROBE]       Выбор отладчика (j-link, по умолчанию: j-link)"
        Write-Host "  -Memory [SIZE]           Выбор размера памяти (e.g., 256KB, 512KB, 2MB, 4MB, 16MB)"
        Write-Host "  -NrfPort [PORT]          Порт для прошивки nRF52840 (например COM3)"
        Write-Host "  -Lang [LANG]             Установить язык скрипта (ru или en) и сохранить"
        Write-Host "`nКомбинированные флаги (в PS пишутся через пробел!):"
        Write-Host "  -c, -Clean               Выполнить чистую сборку"
        Write-Host "  -f, -Flash               Прошить через отладчик"
        Write-Host "  -u, -UsbFlash            Прошить через USB (для RP2040/RP2350/nRF52840/milandr)"
        Write-Host "  -Uf2                     Сгенерировать UF2-файл (для RP2040/RP2350/nRF52840)"
        Write-Host "  -h, -Help                Показать справку"
        Write-Host "`nПримеры:"
        Write-Host "  .\build_all.ps1 -Keyboard corne -Clean -Flash"
        Write-Host "  .\build_all.ps1 -Keyboard corne -Clean -UsbFlash -Uf2 -Mcu rp2040"
        Write-Host "  .\build_all.ps1 -Lang ru"
    }
}

if ($Help) {
    Show-Help
    exit 0
}

# Проверка существования клавиатуры
$KeyboardDir = Join-Path $ScriptDir "keyboards" $Keyboard
if (-not (Test-Path $KeyboardDir)) {
    Write-Host "$MSG_ERR_KBD_NOT_FOUND $KeyboardDir" -ForegroundColor Red
    Write-Host $MSG_AVAIL_KBD
    Get-ChildItem -Path (Join-Path $ScriptDir "keyboards") -Directory | Select-Object -ExpandProperty Name
    exit 1
}

# Настройка целевого имени
$TargetName = "dmk_${Keyboard}_${Mcu}"
$BuildDir = Join-Path $ScriptDir "build"
$HrdProbe = Join-Path $ScriptDir "platforms\milandr\dep\probe\jlink4swd.cfg"

if ($Clean) {
    Write-Host $MSG_CLEAN_DIR -ForegroundColor Yellow
    if (Test-Path $BuildDir) { Remove-Item -Path $BuildDir -Recurse -Force }
}

if (-not (Test-Path $BuildDir)) {
    Write-Host $MSG_INIT_CMAKE -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
    
    $CmakeArgs = @("-S", $ScriptDir, "-B", $BuildDir, "-DKEYBOARD=$Keyboard", "-DMCU=$Mcu")
    if ($Memory) { $CmakeArgs += "-DMEMORY=$Memory" }
    if ($Mcu -eq "milandr" -and $UsbFlash) { $CmakeArgs += "-DBOOTLOADER=ON" }
    
    # В Windows CMake по умолчанию использует Visual Studio, если он установлен. 
    # Если вы хотите принудительно использовать Unix Makefiles (MinGW) или Ninja, 
    # раскомментируйте одну из строк ниже:
    # $CmakeArgs = @("-G", "Ninja") + $CmakeArgs
    # $CmakeArgs = @("-G", "MinGW Makefiles") + $CmakeArgs

    & cmake $CmakeArgs
}

Write-Host "$MSG_BUILD $TargetName ===" -ForegroundColor Yellow
$Jobs = $env:NUMBER_OF_PROCESSORS
if (-not $Jobs) { $Jobs = 4 }

& cmake --build $BuildDir -j $Jobs

if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "$MSG_BUILD_SUCCESS $TargetName ===" -ForegroundColor Green
Write-Host "$MSG_BIN $BuildDir\$TargetName"

# Print memory usage for platforms that don't get it from CMake linker output
if ($Mcu -ne "milandr") {
    $ElfEarly = Join-Path $BuildDir $TargetName
    if (-not (Test-Path $ElfEarly)) { $ElfEarly = Join-Path $BuildDir "${TargetName}.elf" }
    Show-MemoryUsage -ElfFile $ElfEarly -Mcu $Mcu -MemoryOverride $Memory
}

$ElfFile = Join-Path $BuildDir $TargetName
if (-not (Test-Path $ElfFile)) { $ElfFile = Join-Path $BuildDir "${TargetName}.elf" }
$BinFile = Join-Path $BuildDir "${TargetName}.bin"
$Uf2File = Join-Path $BuildDir "${TargetName}.uf2"

# Генерация UF2-файла, если требуется
if ($Uf2) {
    switch ($Mcu) {
        { $_ -in "rp2040", "rp2350" } {
            if (Test-Path $Uf2File) {
                Write-Host "$MSG_UF2_READY_CMAKE $Uf2File" -ForegroundColor Green
            } elseif (-not (Get-Command elf2uf2 -ErrorAction SilentlyContinue)) {
                Write-Host $MSG_ERR_ELF2UF2 -ForegroundColor Red
            } else {
                if (-not (Test-Path $ElfFile)) {
                    Write-Host "$MSG_ERR_ELF_NOT_FOUND $ElfFile" -ForegroundColor Red
                } else {
                    Write-Host $MSG_GEN_UF2 -ForegroundColor Yellow
                    & elf2uf2 $ElfFile $Uf2File
                    Write-Host "$MSG_UF2_READY $Uf2File" -ForegroundColor Green
                }
            }
        }
        "nrf52840" {
            if (Test-Path $Uf2File) {
                Write-Host "$MSG_UF2_READY_CMAKE $Uf2File" -ForegroundColor Green
            } else {
                Write-Host "$MSG_ERR_UF2_NOT_FOUND $Uf2File" -ForegroundColor Red
            }
        }
        default {
            Write-Host "$MSG_UF2_UNSUPPORTED ${Mcu}." -ForegroundColor Yellow
        }
    }
}

# Прошивка через отладчик (OpenOCD)
if ($Flash) {
    if (-not (Test-Path $ElfFile)) {
        Write-Host "$MSG_ERR_ELF_NOT_FOUND $ElfFile" -ForegroundColor Red
        exit 1
    }

    if (-not (Test-Path $BinFile)) {
        & arm-none-eabi-objcopy -O binary $ElfFile $BinFile
    }

    if (-not (Test-Path $HrdProbe)) {
        Write-Host "$MSG_ERR_PROBE_CONF $HrdProbe" -ForegroundColor Red
        exit 1
    }

    Write-Host "$MSG_FLASH_DEBUG ($TargetName)" -ForegroundColor Yellow
    & openocd -f $HrdProbe -c "init" -c "halt" -c "program ${BinFile} verify reset 0x08000000" -c "exit"
    Write-Host $MSG_FLASH_SUCCESS -ForegroundColor Green
}

# Прошивка через USB
if ($UsbFlash) {
    switch ($Mcu) {
        "nrf52840" {
            $NrfFlashScript = Join-Path $ScriptDir "tools\nrfutil\flash_nrf.py"
            $HexFile = Join-Path $BuildDir "${TargetName}.hex"

            if (-not (Test-Path $HexFile)) {
                Write-Host "$MSG_ERR_HEX_NOT_FOUND $HexFile" -ForegroundColor Red
                exit 1
            }

            if (-not (Test-Path $NrfFlashScript)) {
                Write-Host "$MSG_ERR_NRF_SCRIPT $NrfFlashScript" -ForegroundColor Red
                exit 1
            }

            Write-Host "$MSG_FLASH_USB_NRF ($TargetName)" -ForegroundColor Yellow
            Write-Host $MSG_NRF_DFU_HINT -ForegroundColor Yellow
            
            $uvArgs = @("run", "--project", $ScriptDir, $NrfFlashScript, "--hex", $HexFile)
            if ($NrfPort) {
                $uvArgs += "--port"
                $uvArgs += $NrfPort
            }

            & uv $uvArgs
            if ($LASTEXITCODE -ne 0) {
                Write-Host $MSG_FLASH_FAILED -ForegroundColor Red
                exit 1
            }
            Write-Host $MSG_FLASH_SUCCESS -ForegroundColor Green
        }
        { $_ -in "rp2040", "rp2350" } {
            if (-not (Get-Command picotool -ErrorAction SilentlyContinue)) {
                Write-Host $MSG_ERR_PICOTOOL -ForegroundColor Red
                exit 1
            }

            if (-not (Test-Path $BinFile)) {
                & arm-none-eabi-objcopy -O binary $ElfFile $BinFile
            }

            Write-Host "$MSG_FLASH_USB ($TargetName)" -ForegroundColor Yellow
            
            $picotoolInfo = & picotool info 2>&1
            if ($LASTEXITCODE -ne 0) {
                Write-Host $MSG_PICO_BOOTSEL_ERR -ForegroundColor Red
                Write-Host $MSG_PICO_BOOTSEL_ACT -ForegroundColor Yellow
                Write-Host $MSG_PICO_PRESS_KEY -NoNewline
                $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
                Write-Host "`n$MSG_PICO_CONT"
            }

            Write-Host "$MSG_PICO_UPLOAD ${BinFile}..."
            & picotool load $BinFile -x
            if ($LASTEXITCODE -eq 0) {
                Write-Host $MSG_PICO_SUCCESS -ForegroundColor Green
            } else {
                Write-Host $MSG_PICO_FAIL -ForegroundColor Red
                exit 1
            }
        }
        "milandr" {
            if (-not (Get-Command dfu-util -ErrorAction SilentlyContinue)) {
                Write-Host $MSG_ERR_DFU_UTIL -ForegroundColor Red
                exit 1
            }

            if (-not (Test-Path $BinFile)) {
                & arm-none-eabi-objcopy -O binary $ElfFile $BinFile
            }

            Write-Host "$MSG_FLASH_USB_DFU ($TargetName)" -ForegroundColor Yellow
            Write-Host $MSG_MILANDR_DFU_HINT -ForegroundColor Yellow

            & dfu-util -a 0 -d 1209:be92 -D $BinFile
            if ($LASTEXITCODE -ne 0) {
                Write-Host $MSG_FLASH_FAILED -ForegroundColor Red
                exit 1
            }
            Write-Host $MSG_FLASH_SUCCESS -ForegroundColor Green
        }
        default {
            Write-Host "$MSG_USB_UNSUPPORTED ${Mcu}." -ForegroundColor Red
            exit 1
        }
    }
}