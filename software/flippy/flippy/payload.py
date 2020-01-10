"""This module defines all payloads used by commands"""
from __future__ import annotations
from typing import Tuple
from abc import ABC, abstractmethod
import os

from bitarray import bitarray

from flippy.constants import DEFAULT_CLEAR_TIME


class CommandPayload(ABC):
    """Abstract command payload class"""

    @abstractmethod
    def serialize(self) -> bytearray:
        """Serializes the payload for transport over RS485 / sockets"""

    @abstractmethod
    def deserialize(self, payload: bytearray):
        """Deserialize the payload and provider higher means of abstraction"""


class EmptyPayload(CommandPayload):
    """Dummy payload. Required for the RenderCommand"""

    def __init__(self, payload: bytearray = None):
        if payload is not None:
            self.deserialize(payload)

    def serialize(self) -> bytearray:
        return bytearray([])

    def deserialize(self, payload: bytearray):
        if len(payload) != 0:
            raise ValueError("An empty payload is expected to be empty")

class StringPayload(CommandPayload):
    """String payload"""
    def __init__(self, payload: bytearray = None):
        if payload is not None:
            self.deserialize(payload)

    def serialize(self) -> bytearray:
        return self.buf

    def deserialize(self, payload: bytearray):
        self.buf = payload

    def from_string(value: str) -> StringPayload:
        return StringPayload(bytearray(value.encode("ascii") + b'\x00'))

class BooleanPayload(CommandPayload):
    """Payload that carries a single boolean value"""
    value: bool

    def __init__(self, payload: bytearray = None, value: bool = False):
        self.value = value
        if payload is not None:
            self.deserialize(payload)

    def serialize(self) -> bytearray:
        return bytearray([self.value])

    def deserialize(self, payload: bytearray):
        if len(payload) != 1:
            raise ValueError("Expecting length of 1 for a BooleanPayload")
        self.value = bool(payload[0])

class Framebuffer(CommandPayload):
    """Framebuffer for a single fluepdot display
    """

    raw: bitarray

    def __init__(self, payload: bytearray = None):
        self.raw = bitarray(endian='little')
        if payload is None:
            self.raw.frombytes(bytes(230))
        else:
            self.deserialize(payload)

    def __getitem__(self, key: Tuple[int, int]) -> bool:
        """Returns if the pixel at the given coordinate (x, y) is set
        """
        offset_x, offset_y = key
        if offset_x < 0 or offset_x >= 115 or offset_y < 0 or offset_y >= 16:
            raise IndexError()
        return self.raw[offset_x * 16 + offset_y]

    def __setitem__(self, key: Tuple[int, int], value: bool):
        offset_x, offset_y = key
        if offset_x < 0 or offset_x >= 115 or offset_y < 0 or offset_y >= 16:
            raise IndexError()
        self.raw[offset_x * 16 + (16 - offset_y)] = value

    def __str__(self):
        ret = ""
        for offset_y in range(0, 16):
            for offset_x in range(0, 115):
                ret += "█" if self[(offset_x, offset_y)] else " "
            ret += "\n"
        return ret

    def serialize(self) -> bytearray:
        return bytearray(self.raw.tobytes())

    def deserialize(self, payload: bytearray):
        if len(payload) != 230:
            raise ValueError(
                f"malformed payload was {len(payload)} expected 230")
        self.raw = bitarray(endian="little")
        self.raw.frombytes(bytes(payload))

    @staticmethod
    def create_empty() -> Framebuffer:
        """Creates an empty framebuffer"""
        return Framebuffer()

    @staticmethod
    def create_random() -> Framebuffer:
        """Creates a random framebuffer"""
        return Framebuffer(bytes(os.urandom(230)))


class RenderingOptions(CommandPayload):
    """Each fluepdot's column has an H-Bridge, and two cycles:
    1. clear, the H bridge supplies +12V. With a common clear channel, the column can be cleared
    2. set, the H bridge supplies GND. Pixels can be set individualy.
    Timings can be supplied for both cycles. 0 results in skipping the cycle.
    Skipping cycles can be used to achieve higher framerates.
    Different timings can be used to create sounds / music.

    Keep in mind, that below a certain threshold pixels will not be set / cleared, because the coils
    are slow, even though they still produce sound ;)
    """

    __options: bytearray

    def __init__(self, payload: bytearray = None,
                 duration: int = DEFAULT_CLEAR_TIME):
        if payload is None:
            self.__options = bytearray([duration >> 8, duration & 0xFF] * 115)
        else:
            if len(payload) != 230:
                raise ValueError("malformed packet")
            self.__options = payload

    def __getitem__(self, key: int) -> int:
        if key < 0 or key >= 115:
            raise IndexError()
        return ((self.__options[key * 2] << 8) |
                (self.__options[key * 2 + 1])) * 2

    def __setitem__(self, key: int, value: int):
        if key < 0 or key >= 115:
            raise IndexError()
        if value < 0 or value > 65535 * 2:
            raise ValueError()
        value = value // 2
        self.__options[key * 2] = value >> 8
        self.__options[key * 2 + 1] = value & 0xFF

    def serialize(self) -> bytearray:
        return self.__options

    def deserialize(self, payload: bytearray):
        if len(payload) != 230:
            raise ValueError("malformed packet")
        self.__options = payload


class ClearOptions(RenderingOptions):
    """How long to power the coils for each column to clear pixels"""


class FluepOptions(RenderingOptions):
    """How long to power the coils for each column to set pixels"""
