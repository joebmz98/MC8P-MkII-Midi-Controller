# MC8P MkII Standalone MIDI Controller

---

# **8-Pot Standalone MIDI Controller (STM32F411CEU6 BlackPill)**

A customizable, open-source MIDI controller with **8 potentiometers**, each supporting **up to 10 assignable MIDI CCs per potentiometer**, built around the **STM32F411CEU6 BlackPill** for high-performance MIDI control at an improved cost.

<img src="https://github.com/joebmz98/MC8P-Midi-Controller/raw/main/MC8P_MIDI_CONTROLLERS_MEDIA/PHOTOS/P1155438.JPG" alt="Photo of the assembled MC8P MkII MIDI Controller" width="600"/>

---
## Introducing the MC8P Video

[![Watch the introduction video](https://img.youtube.com/vi/sK4E2PJmlkE/0.jpg)](https://www.youtube.com/watch?v=sK4E2PJmlkE)
---

## **Features**
- **8x B100K Pots** with smooth analog control
- **STM32F411CEU6 BlackPill** main microcontroller
- **CD74HC4067M** analog multiplexer for the 8 potentiometers
- **SSD1306 I2C OLED (128x64)** for the UI
- **Per-Pot CC Assignment** – Each pot can send **10 different MIDI CCs**
- **3.5mm MIDI TRS Out** (PJ307 socket for Type A MIDI)
- **4x Kailh Switches** for UI navigation/configuration
- **W25Q64 SPI Flash** for settings storage
- **5V DC Jack Input** (centre positive) or USB power
- **Standalone Operation** – No computer needed after setup
- **Open-Source Firmware** (Arduino IDE + STM32duino compatible)
- **New display themes:** MkI, MkII & YX
- **Scene/Project navigation** with STEP and CUSTOM modes
- **Temporary override mode** with catch-up
- **Improved cost** compared with the Teensy-based design

---

## **Bill of Materials (BOM)**

| Component                  | Qty | Notes                          | Links |
|----------------------------|-----|--------------------------------|-------|
| STM32F411CEU6 BlackPill    | 1   | Main microcontroller           | — |
| SSD1306 OLED (128×64)      | 1   | I²C interface                  | [AliExpress](https://www.aliexpress.com/item/1005006997755041.html) |
| CD74HC4067M                | 1   | 16-channel analog multiplexer  | — |
| W25Q64 / W24Q64 SPI Flash Memory | 1 | SPI settings storage; firmware references W25Q64 | — |
| B100K Vertical Potentiometer | 8  | Linear taper                   | [Example](https://shorturl.at/ZGaz2) |
| 3.5mm Stereo Socket (PJ307) | 1  | MIDI TRS Type A                | [Example](https://shorturl.at/lp2T3) |
| Kailh Switches             | 4   | For menu navigation            | —     |
| Kailh Switch Caps          | 4   | For Kailh switches             | [AliExpress](https://www.aliexpress.com/item/1005006477890497.html) |
| Potentiometer Knobs        | 8   | For B100K potentiometers       | [AliExpress](https://www.aliexpress.com/item/1005006064249289.html) |
| DC-005 Barrel Jack Socket  | 1   | 5V DC power input, centre positive | [AliExpress](https://www.aliexpress.com/item/32839712664.html) |
| 10nF Ceramic Capacitors    | 8   | Potentiometer filtering        | —     |
| 10k Resistors              | 4   | Pull-down for buttons          | —     |
| 220Ω Resistors             | 2   | For MIDI Circuitry             | —     |
| 100nF Ceramic Capacitor    | 1   | Power rail stabilisation       | —     |
| 10uF Ceramic Capacitor     | 1   | Power rail stabilisation       | —     |
| 10uF Electrolytic Capacitor| 1   | 5V input stability             | —     |
| 100uF Electrolytic Capacitor | 1 | Power rail stabilisation       | —     |

---

## **Available Purchase Options**
**PCB Only** – For custom builds → [**Purchase from my Tindie store**](https://www.tindie.com/products/axsinstruments/diy-mc8p-standalone-midi-controller-pcbs-only/)

**Firmware WebFlasher** → [**MC8P MkII WebFlasher**](https://joebmz98.github.io/MC8P-MkII-Flasher/)

---

## **Installation & Setup**
### **1. Flashing the Firmware**
- Easiest method: use the **WebFlasher**  
  https://joebmz98.github.io/MC8P-MkII-Flasher/
- Alternatively, install **Arduino IDE + STM32duino/STM32F4 core** and the dependencies referenced in the Building from Source section
- Clone this repo and open the `.ino` file in the Arduino IDE
- Select **STM32F411CEU6 BlackPill** (or the appropriate STM32F4 board option)
- Upload the firmware

### **2. Hardware Assembly**
- Refer to the *Build Guide*

### **3. First Boot**
- Power via **USB** on the BlackPill or via the **DC-005 5V DC input jack** (centre positive)
- Use the **OLED menu** to assign CCs, configure scenes, and adjust display settings

---

## **Configuration**

### Button Functions

| Button | Primary Function |
|--------|-----------------|
| ASSIGN | Navigate backward, cancel actions, return to previous screens |
| ENTER | Confirm selections, activate temporary override mode |
| PREV | Move up or decrease values; on Main Screen, previous scene |
| NEXT | Move down or increase values; on Main Screen, next scene |

### Screen Hierarchy

The interface is organized into several screens accessible from the main display:

**MAIN SCREEN** → **MENU** → **CC ASSIGN / SCENES / SETTINGS**

**SETTINGS** → **ASSIGN SETTINGS / DISPLAY SETTINGS / ABOUT / FACTORY RESET**

### Main Screen

This is your default operating view. The screen displays eight indicators, one for each potentiometer, showing current MIDI values (0-127). The display style depends on the selected theme:

- **MkII** – vertical bar indicators with scene label/value
- **MkI** – circular pot indicators with scene number
- **YX** – rounded-rectangle bar indicators with scene-change overlay

From the main screen:
- Press **ASSIGN** to open the menu
- Press and hold **ENTER** to activate temporary override mode (the display inverts as visual feedback)
- Press **PREV/NEXT** to change scenes in STEP or CUSTOM scene mode

### Temporary Override Mode

When you press and hold ENTER on the main screen, the controller enters temporary override mode. In this mode, all potentiometer adjustments are temporary. When you release ENTER, the controller returns to the stored values and activates catch-up mode, where potentiometers must pass through their stored positions before taking control again.

### Menu Screen

Press ASSIGN from the main screen to reach the menu. Use PREV and NEXT to scroll through options:

- **CC assign** – Configure MIDI messages for each potentiometer
- **Scenes** – Configure scene/project navigation
- **Settings** – Access device configuration

Press ENTER to select an option, or press ASSIGN to return to the main screen.

### Assign Screen (CC Assign)

This screen lets you configure up to 10 MIDI messages per potentiometer.

**Navigation flow:**
1. Select which potentiometer to edit (flashing number)
2. Select which message slot to edit, or choose Add/Remove
3. Edit individual parameters

**Parameters you can edit for each message:**
- **Channel** – MIDI channel (1-16) or OMNI
- **CC** – Control Change number (0-127)
- **Min Value** – Minimum output value (0-127)
- **Max Value** – Maximum output value (0-127)
- **Curve** – NOR (normal), INV (inverted), LOG (logarithmic), ILG (inverse logarithmic)

**Editing values:** When editing a parameter, use the configured edit potentiometer (default pot 8) to smoothly adjust the value across its full range. The selected parameter flashes to indicate edit mode.

**Navigation within Assign Screen:**
- **PREV/NEXT** – Navigate between pots, messages, or Add/Remove buttons
- **ENTER** – Enter edit mode for a parameter, or cycle to the next parameter
- **ASSIGN** – Return to pot selection, or save and exit (confirmation popup appears)

**Add/Remove messages:**
- Navigate to "+ Add" to create a new message (up to 10 per pot)
- Navigate to "- Remove" to delete the selected message (minimum 1 message per pot)

### Scene Screen

The Scenes screen configures scene/project navigation, including OP-XY-style scene control.

**Parameters:**
- **Scene CC** – MIDI CC used for scene changes (default 82)
- **Mode** – STEP or CUSTOM
- **Custom Length** – Number of scenes in the custom order (1-12, shown in CUSTOM mode)
- **Custom Order** – Editable scene sequence (shown in CUSTOM mode)

**STEP mode:** PREV/NEXT on the Main Screen step through scenes 0-98 (displayed as 1-99).

**CUSTOM mode:** PREV/NEXT step through the configured custom scene order.

**Editing:** Press ENTER to edit the selected parameter or custom sequence position. Use the configured edit potentiometer to change values. Press ENTER again to save, or ASSIGN to cancel/revert.

Scene messages are sent on the configured scene CC and MIDI channel 1. In the YX theme, a temporary scene-change overlay appears on the Main Screen.

### Settings Screen

From the menu, select Settings to access:

**Assign Settings** – Choose which potentiometer (1-8) to use for editing values in the Assign and Scene screens. Use PREV/NEXT to change the selection.

**Display Settings** – Configure:
- **Theme** – MkII, MkI, or YX
- **Invert Display** – Yes/No

Press ENTER to enter edit mode, then use PREV or NEXT to change the value. Press ENTER again to save, or ASSIGN to cancel.

**About** – View firmware version and device information.

**Factory Reset** – Restore all settings to default values. A confirmation screen prevents accidental resets.

### Confirmation Popups

When saving changes or performing a factory reset, a popup screen appears asking for confirmation. Use PREV/NEXT to toggle between Y (Yes) and N (No), then press ENTER to confirm or ASSIGN to cancel.

---

## **MIDI Implementation**
- USB MIDI + TRS MIDI Out (3.5mm Type A)
- Up to **10 CC Messages** per potentiometer
- Per-message MIDI channel 1-16 or OMNI
- Per-message CC number 0-127
- Per-message min/max range 0-127
- Curve types: NOR, INV, LOG, ILG
- Scene/project CC output for STEP and CUSTOM modes
- Settings stored in onboard W25Q64 SPI Flash

---

## **Building from Source**
- Requires **Arduino IDE + STM32duino/STM32F4 core**
- Board: **STM32F411CEU6 BlackPill**
- Libraries:
  - `Adafruit_SSD1306` (OLED)
  - `Adafruit_GFX` (Adafruit UI Design)
  - `MIDI` (MIDI)
  - `Adafruit_TinyUSB` (USB MIDI)
  - `Picopixel` (Picopixel Font)
- Built-in libraries: `Wire`, `SPI`

**Firmware version:** 3.1

**WebFlasher:** https://joebmz98.github.io/MC8P-MkII-Flasher/

**Owner's Manual:** https://docs.google.com/document/d/1LK1hT5nvRXzgEnc-npS-HN6GpmSG35B1l2-KNDyIfV0/edit?usp=sharing

---

## **License**
**MIT License** – Open-source for personal & commercial use

---

## **Support & Contributions**
- **Issues:** Open a GitHub ticket
- **Custom Requests:** Start a thread in the discussions page or send me an E-mail! axs.instruments@gmail.com
- **Want to improve it?** PRs welcome!
- **Special Thanks:** This project is supported by **PCBWay** who have kindly provided the PCB stock available at the moment

---

## **Acknowledgments**

PCBway very kindly offered to sponsor this project and have sent me some more stock for the shop. Thank you to PCBway for supporting this passion project of mine. Emily from the marketing department reached out to me when I first launched the product and store and very kindly offered to sponsor the project. Both working with them and using their services was really easy. All I needed to do was export my PCB gerber files, drag and drop them on the 'PCB Instant Quote' page and send it off for approval. I received my order a week later and was very happy with the quality of the PCBs. Thank you again PCBway for your support.

You can find their website here: https://www.pcbway.com/
