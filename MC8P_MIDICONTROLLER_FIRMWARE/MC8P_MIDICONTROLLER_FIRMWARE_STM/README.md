# MC8P MkII firmware

STM32F411 Black Pill firmware for the MC8P 8-pot MIDI controller, built with the Arduino IDE (STM32duino core). Open `MC8P_MIDICONTROLLER_FIRMWARE_STM.ino`; the Arduino IDE opens every `.h` in this folder as a tab.

## How the code is laid out

The `.ino` only holds `setup()` and `loop()`. Everything else is in a header per job. The headers are only included by the `.ino`, so the whole firmware is still one translation unit: globals are defined once in `state.h` / `hardware.h` and every module can see them.

`prototypes.h` declares every function before any module is compiled, because the Arduino IDE only writes prototypes for functions in the `.ino` itself, and the modules call each other in both directions. **When you add a function, add its declaration to `prototypes.h` under its file.**

`loop()` each pass:

1. `TinyUSBDevice.task()` and drain USB MIDI in (web editor SysEx → `onUsbSysEx()`)
2. `readButtons()`: buttons → screens and settings
3. `readAllPotentiometers()` then `processPotentiometers()`: pots → MIDI CCs
4. `handleParameterEditWithPot()`: the edit pot changes values in the menus
5. `drawCurrentScreen()`

## Files

| File | What it does |
|---|---|
| `MC8P_MIDICONTROLLER_FIRMWARE_STM.ino` | Version, web editor protocol notes, licence, `setup()` and `loop()` |
| `hardware.h` | Pin map, OLED object, DIN and USB MIDI instances, button pins |
| `state.h` | Shared types (curves, screen states, scene modes) and every global |
| `prototypes.h` | Declarations of every function, grouped by file |
| `display_assets.h` | Screen layout tables and PROGMEM bitmaps |
| `curves.h` | Pot position → CC value through the NOR / INV / LOG / ILG curve and min/max range |
| `spi_flash.h` | W25Q64 SPI flash driver: erase, program, read, struct read/write |
| `storage.h` | Settings structs and versions, save / load to flash, factory reset |
| `midi_out.h` | `sendControlChange()` to DIN MIDI and USB MIDI |
| `editor_sysex.h` | USB SysEx protocol for the web editor (get / set pots and globals, save, revert, reset) |
| `assign.h` | Add / remove a MIDI message on the selected pot |
| `scenes.h` | OP-XY scene navigation (STEP and CUSTOM order) |
| `temp_override.h` | Hold ENTER for temporary pot moves, then catch-up |
| `pots.h` | Multiplexer reads and pot → MIDI processing |
| `pot_edit.h` | Editing menu values with the edit pot |
| `buttons.h` | ASSIGN / ENTER / PREV / NEXT handling for every screen |
| `display_main.h` | Main screen: MkII, MkI and YX themes, scene overlay |
| `display_assign.h` | ASSIGN screen |
| `display_menus.h` | Menu, scene, settings, display, about and confirm popups |
| `startup.h` | Boot sequence: splash, load settings, first pot read, initial MIDI |
| `ui.h` | `drawCurrentScreen()`: draws whichever screen is current |
| `build_opt.h` | Build flags: vector table / flash offset `0x4000` (bootloader) |
