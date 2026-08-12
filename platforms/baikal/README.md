# Description

**Board Support Packages (BSP)** were developed to simplify the code of user's projects and unify board-related functions, definitions and constants.\
If the valid development board were specified in the project build script, the corresponding BSP would be used automatically while project building.

# Structure

BSP directory contains packages for several development boards.

```sh
BSP
├── Components
├── BOARD_NAME_1
│   ├── bsp.c
│   ├── bsp.h
│   └── bsp.mk
├── BOARD_NAME_2
│   └── ...
...
├── BOARD_NAME_N
│   └── ...
└── README.md
```

Each BSP consist of several files:

- `bsp.mk`: additional building script to be used in the project build process, if the correct development board name were specified.
  See **BOARD** variable description and available values in the building script template file (_Tools/build/Makefile_template_)
- `bsp.h`: board-specific definitions, constants and function prototypes
- `bsp.c`: board-specific functions

`Components` folder contains software components that are reused across different boards.

Please refer to the desired development board BSP for more information (_<BSP_directory/README.md_)
