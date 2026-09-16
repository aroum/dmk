const PRESETS = {
            corne: {
                name: "Corne DMK",
                isSplit: true,
                matrixType: "COL2ROW",
                rowPins: ["GPIO4", "GPIO5", "GPIO6", "GPIO7"],
                colPins: ["GPIO29", "GPIO28", "GPIO27", "GPIO26", "GPIO22", "GPIO20"],
                enableRgb: true,
                rgbCount: 42,
                rgbPin: "GPIO0",
                enableEncoders: false,
                pattern: [
                    [1,1,1,1,1,1, 1,1,1,1,1,1],
                    [1,1,1,1,1,1, 1,1,1,1,1,1],
                    [1,1,1,1,1,1, 1,1,1,1,1,1],
                    [0,0,0,1,1,1, 1,1,1,0,0,0]
                ]
            },
            sweep: {
                name: "Sweep / Ferris",
                isSplit: true,
                matrixType: "COL2ROW",
                rowPins: ["GPIO4", "GPIO5", "GPIO6", "GPIO7"],
                colPins: ["GPIO29", "GPIO28", "GPIO27", "GPIO26", "GPIO22"],
                enableRgb: false,
                rgbCount: 0,
                pattern: [
                    [1,1,1,1,1, 1,1,1,1,1],
                    [1,1,1,1,1, 1,1,1,1,1],
                    [1,1,1,1,1, 1,1,1,1,1],
                    [0,0,0,1,1, 1,1,0,0,0]
                ]
            },
            kabarga: {
                name: "Kabarga DMK",
                isSplit: false,
                matrixType: "COL2ROW",
                rowPins: ["PB5", "PB4", "PB3", "PA1", "PA0", "PA2", "PA3"],
                colPins: ["PA9", "PA8", "PA7", "PA6", "PA5", "PA4"],
                enableRgb: false,
                rgbCount: 0,
                enableEncoders: false,
                pattern: [
                    [1,1,1,1,1,1],
                    [1,1,1,1,1,1],
                    [1,1,1,1,1,1],
                    [1,1,1,1,1,1],
                    [1,1,1,1,1,1],
                    [1,1,1,1,1,1],
                    [1,1,1,1,1,1]
                ]
            },
            nizkoteno: {
                name: "Nizkoteno 10",
                isSplit: false,
                matrixType: "DIRECT",
                directPins: ["GPIO9", "GPIO8", "GPIO6", "GPIO5", "GPIO10", "GPIO7", "GPIO4", "GPIO2", "GPIO1", "GPIO3"],
                enableRgb: true,
                rgbCount: 10,
                rgbPin: "GPIO29",
                pattern: [[1,1,1,1,1,1,1,1,1,1]]
            },
            planck: {
                name: "Planck 4x12",
                isSplit: false,
                matrixType: "COL2ROW",
                rowPins: ["GPIO4", "GPIO5", "GPIO6", "GPIO7"],
                colPins: ["GPIO10", "GPIO11", "GPIO12", "GPIO13", "GPIO14", "GPIO15", "GPIO16", "GPIO17", "GPIO18", "GPIO19", "GPIO20", "GPIO21"],
                enableRgb: true,
                rgbCount: 48,
                rgbPin: "GPIO0",
                pattern: [
                    [1,1,1,1,1,1,1,1,1,1,1,1],
                    [1,1,1,1,1,1,1,1,1,1,1,1],
                    [1,1,1,1,1,1,1,1,1,1,1,1],
                    [1,1,1,1,1,1,1,1,1,1,1,1]
                ]
            }
        };
