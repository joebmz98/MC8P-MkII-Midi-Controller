# Legacy firmware builds

Older MC8P firmware builds, kept for reference and for rolling back. The
current release is the build in `MC8P_MIDICONTROLLER_FIRMWARE_STM/build/`,
which is what the [web flasher](https://joebmz98.github.io/MC8P-MkII-Flasher/)'s
**Latest release** option flashes.

To flash one of these, open the web flasher, choose **Custom file**
and pick the `.hex`, which carries its own address. If you use the `.bin`,
enter the start address from the table below.

| Folder | Version | Board | Start address | Notes |
| --- | --- | --- | --- | --- |
| `v3.1/` | 3.1 | MC8P MkII (STM32F411 Black Pill) | `0x08004000` | First web flasher release. Linked to run after the WeAct HID bootloader at `0x08000000`, so it won't start on its own without it. No editor support. |
| `v3.0/` | 3.0 | MC8P MkII (STM32F411 Black Pill) | `0x08000000` | Built with the **MC8P MkII MIDI Controller** board option (modified STM32 core). No editor support. |
| `v3.0-stock-core/` | 3.0 | MC8P MkII (STM32F411 Black Pill) | `0x08000000` | First STM32 build, made with the stock STM32 core (generic F411 board option) before the MC8P board option existed. |

The MC8P Editor needs firmware 3.2 or later, so it can't connect to any of
these.

## Where they came from

All were taken unchanged from the build folders in this repo's
history:

| Folder | Commit |
| --- | --- |
| `v3.1/` | `9156036` "firmware 3.1" |
| `v3.0/` | `b2107a6` "firmware binary" |
| `v3.0-stock-core/` | `54c8468` "STM32 FW" |

The `.elf` and `.map` debug files are left out. They're still in this repo's
history if you need them.

## Adding a build here

When a new release replaces the build in
`MC8P_MIDICONTROLLER_FIRMWARE_STM/build/`, copy the old `.hex` and `.bin`
into a new `vX.Y/` folder here, then add a row to both tables above.
