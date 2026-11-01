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

| Reference | Qty | Name | Price | Buy link |
|---|---:|---|---:|---|
| D1, D2, D3 | 3 | 5mm LED | $0.30 | [Search](https://www.aliexpress.com/w/wholesale-5mm-LED.html) |
| J1 | 1 | 01x04 Pin Header | $0.20 | [Search](https://www.aliexpress.com/w/wholesale-01x04-Pin-Header.html) |
| J2, J3, J4 | 3 | 01x02 JST PH Connector | $0.45 | [Search](https://www.aliexpress.com/w/wholesale-01x02-JST-PH-Connector.html) |
| R1, R2, R3 | 3 | 220 Film Resistor | $0.15 | [Search](https://www.aliexpress.com/w/wholesale-220-Film-Resistor.html) |
| SW1, SW2, SW3, SW4 | 4 | Cherry MX keyboard switch | $1.00 | [Search](https://www.aliexpress.com/w/wholesale-Cherry-MX-keyboard-switch.html) |
| SW5, SW7, SW8 | 3 | 6 mm THT switch | $0.30 | [Search](https://www.aliexpress.com/w/wholesale-6-mm-THT-switch.html) |
| SW6 | 1 | Rotary Encoder | $0.30 | [Search](https://www.aliexpress.com/w/wholesale-Rotary-Encoder.html) |
| U1, U2, U3 | 3 | XIAO-nRF52840 | $5.00 | [Search](https://www.aliexpress.com/w/wholesale-XIAO-nRF52840.html) |
| — | 1 | 1.3 Inch OLED Screen | $10.00 | [Search](https://www.aliexpress.com/w/wholesale-1.3-Inch-OLED-Screen.html) |
| — | 1 | PCB | $12.00 | — |
| — | 1 | 3D Printed Case | ~$15.00 | — |
|  |  | **Estimated total** | **~$44.70** |  |

## CAD

[Open the LonoMacro Onshape document](https://cad.onshape.com/documents/b238660379d2ad484aa23eea/w/9873678c9d89b1b19152732d/e/ca0627a1691ab6fd1505dec4).
