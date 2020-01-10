"""This module defines the class Program"""
from __future__ import annotations
import os
import csv
from typing import List, Iterable, Tuple

from cobs import cobs

from flippy.command import Command


class Program:
    """An ordered list of timestamped commands to execute on a Kunstwerk"""

    commands: List[Tuple[int, Command]]
    """The deserialized commands"""

    raw: List[Tuple[int, int, bytearray]]
    """The raw and pre-encoded commands are stored for performance reasons"""

    validated: bool
    """Does this program behave well?"""

    execution_time: float
    """Execution time in milliseconds"""

    pixels_flipped: int
    """Pixels flipped by this program"""

    def __init__(self, content: Iterable = None):
        self.commands = []
        self.raw = []
        self.__execution_time = 0

        if content:
            self.parse(content)

    @staticmethod
    def from_file(file_path: str, kunstwerk: "Kunstwerk" = None) -> Program:
        """Loads a program from a file"""
        if not os.path.exists(file_path):
            raise FileNotFoundError(
                f"'{file_path}' does not exist and could not be loaded")
        with open(file_path, 'r') as file_handle:
            program = Program()
            program.parse(file_handle, kunstwerk)
            return program

    def parse(self, content: Iterable, kunstwerk: "Kunstwerk" = None):
        """Parse and validate a program"""
        self.validated = False
        current_time = 0
        current_line = 0
        reader = csv.reader(content)
        for line in reader:
            current_line += 1
            timestamp = int(line[0])
            command_bytes = bytearray.fromhex(line[1])
            address = command_bytes[0]
            if timestamp < 0:
                raise ParseError(
                    current_line,
                    "You cannot travel back in time with this machinery")
            current_time += timestamp
            self.raw.append(
                (timestamp, address, bytearray(
                    cobs.encode(command_bytes) + b'\x00')))
            command = Command.from_raw(command_bytes)
            self.commands += [(timestamp, command)]
            if kunstwerk is None or True:
                continue
            if address == 255:
                for fluepdot in kunstwerk.fluepdots.values():
                    #if current_time + 1000 < fluepdot.last_command_finish_time:
                    #    raise ParseError(
                    #        current_line, "You are trying too fast. Give the kunstwerk some time.")
                    fluepdot.last_command_finish_time += command.simulate(
                        fluepdot)
            else:
                if kunstwerk.fluepdots[address] is None:
                    raise ParseError(
                        current_line,
                        f"Fluepdot with address {address} does not exist")
                fluepdot = kunstwerk.fluepdots[address]
                #if current_time < fluepdot.last_command_finish_time:
                    #raise ValueError(
                    #    current_line,
                    #    "You are trying too fast. Give the kunstwerk some tie.")
                fluepdot.last_command_finish_time += command.simulate(fluepdot)
        if kunstwerk is None: 
            return
        self.execution_time = max(
            fluepdot.last_command_finish_time
            for fluepdot in
            kunstwerk.fluepdots.values())
        self.pixels_flipped = sum(
            fluepdot.pixels_flipped for fluepdot in kunstwerk.fluepdots.values())
        self.validated = True
        print(
            f"Validation success, time {self.execution_time} ms,"
            "pixels flipped {self.pixels_flipped}")

    def write(self, file_obj):
        """Write program to a file object"""
        writer = csv.writer(file_obj)
        for timestamp, command in self.commands:
            writer.writerow([
                timestamp,
                command.serialize().hex()])


class ParseError(Exception):
    """An error that can occur while validating a @see Program"""

    def __init__(self, line_number, message):
        super().__init__(f"Error in line %{line_number}: {message}")
