#!/bin/env python
"""Defines the main entry point"""

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
    kunstwerk.power_on()
#    program = Program.from_file(sys.argv[2], kunstwerk)
    clear_cmd = FramebufferCommand(Framebuffer.create_empty())
    clear_cmd.address = 255
    full_cmd = FramebufferCommand(Framebuffer(bytes([255] * 230)))
    full_cmd.address = 255
    render_cmd = RenderCommand(EmptyPayload())
    render_cmd.address = 255
    while True:
        duration = int(input())
        fluep_options_cmd = FluepOptionsCommand(FluepOptions(duration=duration))
        fluep_options_cmd.address = 255
        kunstwerk.execute_command(fluep_options_cmd)
        clear_options_cmd = ClearOptionsCommand(ClearOptions(duration=duration))
        clear_options_cmd.address = 255
        kunstwerk.execute_command(clear_options_cmd)
        
        kunstwerk.execute_command(clear_cmd)
        kunstwerk.execute_command(render_cmd)
        input()
        kunstwerk.execute_command(full_cmd)
        kunstwerk.execute_command(render_cmd)
    input()
    kunstwerk.down()

if __name__ == "__main__":
    print("hello")
    cli()
