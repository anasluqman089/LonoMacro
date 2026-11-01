# LonoMacro

LonoMacro is a modular macro keypad built around three XIAO nRF52840 boards:
an OLED-equipped master, a keypad slave, and a rotary-encoder slave. The master
connects to a computer over USB-C and to the slaves over Bluetooth Low Energy
(BLE). Slave inputs are forwarded to the computer as USB keyboard events.

## Project files

- [`PCB/`](PCB/) — KiCad schematic, PCB layout, and project files.
- [`CAD/`](CAD/) — 3D STEP models.
- [`Production/`](Production/) — Gerber, drill, and STL manufacturing files.
- [`firmware/`](firmware/) — Arduino sketches for the master and both slaves.
- [`BOM.csv`](BOM.csv) — Bill of materials, prices, and supplier search links.

## Firmware setup

Install the ArduinoBLE, Adafruit GFX, and Adafruit SSD1306 libraries. Select a
XIAO nRF52840 Arduino board package that supports USB keyboard output for the
master. Compile and upload the corresponding sketch to each board:

- [OLED master](firmware/OLED_Master/OLED_Master.ino)
- [Keypad slave](firmware/Keypad_Slave/Keypad_Slave.ino)
- [Rotary slave](firmware/Rotary_Slave/Rotary_Slave.ino)

The master expects an SSD1306 I2C OLED at address `0x3C`.

## Pairing and controls

1. Connect the OLED master to the computer over USB-C.
2. Press the 6 mm pairing button on each slave: SW5 on the keypad and SW7 on
   the rotary encoder. The corresponding slave LED blinks while it advertises.
3. Press the 6 mm pairing/connect button on the master (SW8). The master LED
   blinks while it searches, and the OLED shows each slave's connection status.
   When both slaves are connected, the master LED and both slave LEDs stay on.
4. Press SW1–SW4 to send the number keys `1`–`4`. Rotate the encoder to send
   Up/Down arrow keys; press the encoder (SW6) to send Enter.

The master LED is solid when both slaves are connected and pulses when it
receives an input. A connected slave LED is solid and pulses when it sends an
input. If a slave disconnects, press its pairing button and then press SW8 on
the master to reconnect.

## OLED and battery status

The OLED-equipped master is intended to show the connected macropads and their
battery levels. A future status screen could include the time and date, for
example:

```text
10:00 PM              1/11/26
----------------------------
Rotary Encoder          50%
Keypad                   5%
```

The current firmware displays connection status and recent input events; it
does not yet report slave battery levels or show a clock. Battery reporting and
time display will require additional firmware and, for the clock, a time source.
The master can be powered from the computer's USB-C connection. The slave
macropads are intended to use lithium-ion batteries; battery and charging
hardware should be selected to match the final design.

## Bill of materials

Prices are the listed prices per BOM row, including grouped quantities. They
are estimates, may change, and **do not include shipping**. The case price is
approximate; purchase links are supplier search pages.

| Component | Qty | Real Unit Price (USD) | Total Market Price (USD) | Reference Links |
| :--- | :---: | :---: | :---: | :--- |
| **XIAO-nRF52840** (Standard) | 3 | \$9.99 | **\$29.97** | [Search AliExpress](https://www.aliexpress.com/w/wholesale-XIAO-nRF52840.html) |
| **Cherry MX Keyboard Switch** | 4 | \$0.60 | **\$2.40** | [Search AliExpress](https://www.aliexpress.com/w/wholesale-Cherry-MX-keyboard-switch.html) |
| **1.3 Inch OLED Screen** | 1 | \$4.00 | **\$4.00** | [Search AliExpress](https://www.aliexpress.com/w/wholesale-1.3-Inch-OLED-Screen.html) |
| **Rotary Encoder EC11** | 1 | \$0.50 | **\$0.50** | [Search AliExpress](https://www.aliexpress.com/w/wholesale-Rotary-Encoder.html) |
| **5mm LED** | 3 | \$0.10 | **\$0.30** | [Search AliExpress](https://www.aliexpress.com/w/wholesale-5mm-LED.html) |
| **01x02 JST PH Connector** | 3 | \$0.15 | **\$0.45** | [Search AliExpress](https://www.aliexpress.com/w/wholesale-01x02-JST-PH-Connector.html) |
| **220 Film Resistor** | 3 | \$0.05 | **\$0.15** | [Search AliExpress](https://www.aliexpress.com/w/wholesale-220-Film-Resistor.html) |
| **6 mm THT Switch** | 3 | \$0.10 | **\$0.30** | [Search AliExpress](https://www.aliexpress.com/w/wholesale-6-mm-THT-switch.html) |
| **01x04 Pin Header** | 1 | \$0.20 | **\$0.20** | [Search AliExpress](https://www.aliexpress.com/w/wholesale-01x04-Pin-Header.html) |
| **PCB Fabrication** | 1 | \$12.00 | **\$12.00** | — |
| **3D Printed Case** | 1 | \$15.00 | **\$15.00** | — |
| **Estimated Real Total** | | | **~\$65.27** | |

## CAD

[Open the LonoMacro Onshape document](https://cad.onshape.com/documents/b238660379d2ad484aa23eea/w/9873678c9d89b1b19152732d/e/ca0627a1691ab6fd1505dec4).

## Image 

### Scematic

<img width="793" height="499" alt="image" src="https://github.com/user-attachments/assets/40ca9d71-f68a-444f-8217-ba86c89d7b63" />

### PCB

<img width="791" height="223" alt="image" src="https://github.com/user-attachments/assets/6e6a18c0-1379-469a-ac1d-6c32d99a5028" />

<img width="1032" height="314" alt="image" src="https://github.com/user-attachments/assets/1248f867-c57b-408c-899a-0973be28b630" />

### CAD

<img width="732" height="504" alt="image" src="https://github.com/user-attachments/assets/7ee86a70-2a26-4a46-a51a-0cd0bf6bc99d" />

## Extra

This project is made for Fabricate, a YSWS program by Hack Club

[Project](https://fabricate.hackclub.com/projects/699).
