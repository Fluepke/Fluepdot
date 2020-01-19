# Getting started

## Wiring
> Your board should come with a preflashed firmware and most likely you don't need to change anything about the firmware.

1. Connect an RS485 adapter to your computer
2. Wire GND, A(+) and B(-) from the RS485 adapter to the flipdot pcb. (See below for a pin out diagram). 
3. Wire the select lines to the flipdot PCB. Those are the single cables attached to 5 panels. Most likely they are already wired, make sure they didn't become loose. SELECT0 is the one with the outermost panel with the short cable.
4. Wire power supply (12V) to the flipdot PCB. For each flipdot you should calculate with 3 Amps.
5. Attach the flatband cable to the flipdot PCB.

### Pin out
| 1    | 2   | 3   | 4          | 5          | 6        | 7        | 8        | 9        | 10       |
|------|-----|-----|------------|------------|----------|----------|----------|----------|----------|
| V_in | GND | GND | RS485 B(-) | RS485 A(+) | SELECT 0 | SELECT 1 | SELECT 2 | SELECT 3 | SELECT 4 |

## Test run
Power up the flipdot. It should display two test patterns before showing its address.
You can look for debug output on the serial (USB B plug on the flipdot PCB). `115200` baud, 8N1.

## Software
### Requirements
For running the server software you will need `Python 3.8`. It won't work with Python 3.7

### udev
Setup some udev rules, so you have a persistent device name for your RS485 adapter, e.g.

`/etc/udev/rules.d/99-flipdot.rules`:
```
SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", ATTRS{serial}=="41", SYMLINK+="fluepnet0"
```

Reload udev roles: `udevadm control --reload-rules && udevadm trigger`

### Installing
`cd ./software/flippy`, create a virtualenv `virtualenv -p python3 venv && source ./venv/bin/activate`.

Install `flippy`: `python setup.py develop`.

### Configuration
Edit `./examples/configs/server_single.json`, replace the number 37 with the address of your flipdot, replace `/dev/fluepnet0` with your persistent device name.

### Test run
`python flippy/on_off.py examples/configs/server_single.json`.
Enter a number (something within the range 300-5000) and press enter.
