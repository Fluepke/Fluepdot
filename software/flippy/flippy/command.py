"""Contains all commands that can be transmitted to a Fluepdot"""
from __future__ import annotations
from abc import ABCMeta, abstractmethod
import crcmod

from bitarray import bitdiff

from flippy.payload import CommandPayload, EmptyPayload, Framebuffer, FluepOptions, ClearOptions
from flippy.constants import LATENCY
from flippy.fluepdot import Fluepdot


class Command(metaclass=ABCMeta):
    """Commands perform actions on a single fluepdot"""

    crc_fun = crcmod.mkCrcFun(0x18005, rev=True, initCrc=0x0, xorOut=0x0)

    """Used in serial communication to tell different types of commands apart"""
    nr: int = -1

    """Command's payload"""
    payload: CommandPayload = None

    """The interface this command was received from"""
    rx_interface: "RxInterface" = None

    """The address to send this command to"""
    address: int

    def __init__(
            self,
            payload: CommandPayload = None,
            raw_payload: bytearray = None,
            address: int = 0):
        self.address = address
        if payload is not None and raw_payload is not None:
            raise ValueError("mutually exclusive")
        if payload is not None:
            self.payload = payload
        elif raw_payload is not None:
            self.address = raw_payload[0]
            expected_crc = Command.crc_fun(raw_payload[:-2])
            received_crc = raw_payload[-2] << 8 | raw_payload[-1]
            if expected_crc != received_crc:
                raise ChecksumError(raw_payload)
            self.payload = self.get_payload_type()(raw_payload[2:-2])

    @staticmethod
    def get_payload_type():
        """Returns the type of the payload"""

    @staticmethod
    def from_raw(raw_payload: bytearray) -> Command:
        """Derserializes a received command
        *Note*: This does not perform the COBS decode
        """
        for subclass in Command.__subclasses__():
            if subclass.nr == raw_payload[1]:
                return subclass(raw_payload=raw_payload)
        raise ValueError(f"command nr {raw_payload[1]} not known")

    def serialize(self) -> bytearray:
        """Serializes the command for further use in serial communications.
        *Note*: This does not perform the COBS encode
        """
        data = bytearray([self.address, self.nr]) + self.payload.serialize()
        crc16 = Command.crc_fun(data)
        return data + bytearray([crc16 >> 8, crc16 & 0xFF]) 

    @abstractmethod
    def simulate(self, fluepdot: Fluepdot) -> float:
        """Simulate the execution of this command on a fluepdot and return the execution time"""


class FramebufferCommand(Command):
    """Transmit a 115x16 framebuffer, but do not yet render it"""

    nr: int = 2

    @staticmethod
    def get_payload_type():
        return Framebuffer

    def simulate(self, fluepdot: Fluepdot):
        fluepdot.old_framebuffer = fluepdot.framebuffer
        fluepdot.framebuffer = self.payload
        return LATENCY


class RenderCommand(Command):
    """Render the current framebuffer"""

    nr: int = 3

    @staticmethod
    def get_payload_type():
        return EmptyPayload

    def simulate(self, fluepdot: Fluepdot):
        fluepdot.redraw_curses()
        fluepdot.pixels_flipped += bitdiff(
            fluepdot.old_framebuffer.raw,
            fluepdot.framebuffer.raw)
        return LATENCY + \
            sum(fluepdot.clear_options[x] + fluepdot.fluep_options[x] for x in range(0, 115)) / 1000


class ClearOptionsCommand(Command):
    """Transmit ClearOptions"""

    nr: int = 4

    @staticmethod
    def get_payload_type():
        return ClearOptions

    def simulate(self, fluepdot: Fluepdot):
        fluepdot.clear_options = self.payload
        return LATENCY


class FluepOptionsCommand(Command):
    """Transmit FluepOptions"""

    nr: int = 5

    @staticmethod
    def get_payload_type():
        return FluepOptions

    def simulate(self, fluepdot: Fluepdot):
        fluepdot.fluep_options = self.payload
        return LATENCY

class ChecksumError(ValueError):
    def __init__(self, raw_payload):
        super().__init__(f"{raw_payload} has an invalid checksum")
