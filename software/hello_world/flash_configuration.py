#!/bin/env python

import sys
import os
import struct
import esptool

if len(sys.argv) != 3:
    exit(f"Usage {sys.argv[0]} address baudrate [ESPPORT]")

address = int(sys.argv[1])
baudrate = int(sys.argv[2])
data = struct.pack("<BL", address, baudrate)
port = os.environ.get("ESPPORT", None)
if len(sys.argv) == 4:
    port = sys.argv[3]

with open("config.bin", "wb") as file_handle:
    file_handle.write(data)

if port != None:
    command = ['--chip', 'esp32', '--port', port, '--after', 'hard_reset', 'write_flash', '0x310000', 'config.bin']
else:
    command = ['--chip', 'esp32', '--after', 'hard_reset', 'write_flash', '0x310000', 'config.bin']

print(" ".join(command))
esptool.main(command)

