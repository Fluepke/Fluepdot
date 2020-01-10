#!/bin/env python
"""Defines the main entry point"""

import time
import sys
from flippy.kunstwerk import Kunstwerk
from flippy.program import Program
from flippy.command import FramebufferCommand, RenderCommand, FluepOptionsCommand, ClearOptionsCommand
from flippy.payload import Framebuffer, EmptyPayload, FluepOptions, ClearOptions

def cli():
    """main entry point"""
    if len(sys.argv) == 1:
        print(f"usage {sys.argv[0]} config_file [program]")
    kunstwerk = Kunstwerk.from_file(sys.argv[1])
    kunstwerk.set_up()
    if len(sys.argv) == 3:
        input()
        kunstwerk.power_on()
        program = Program.from_file(sys.argv[2], kunstwerk)
        clear_cmd = FramebufferCommand(Framebuffer.create_empty())
        clear_cmd.address = 255
        fluep_options_cmd = FluepOptionsCommand(FluepOptions())
        fluep_options_cmd.address = 255
        kunstwerk.execute_command(fluep_options_cmd)
        clear_options_cmd = ClearOptionsCommand(ClearOptions())
        clear_options_cmd.address = 255
        kunstwerk.execute_command(clear_options_cmd)
        render_cmd = RenderCommand(EmptyPayload())
        render_cmd.address = 255
        while True:
            kunstwerk.execute_command(clear_cmd)
            kunstwerk.execute_command(render_cmd)
            kunstwerk.execute(program)
            time.sleep(10)
    input()
    kunstwerk.down()
