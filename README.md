# LonoMacro

LonoMacro is a macro keypad ecosystem with an OLED-equipped BLE master, a
keypad slave, and a rotary-encoder slave.

## Firmware

The three XIAO nRF52840 sketches, pin assignments, and pairing instructions are
in [`firmware/`](firmware/). Connect the master to the computer over USB-C; it
connects to the keypad and rotary slaves over BLE and forwards their inputs to
the computer.

## Bill of materials

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

The total doesnt include shipping

## Control

1. Connect the master to the computer using USB-C.

2. Pressed the 6mm button on the slave to connect to oth
