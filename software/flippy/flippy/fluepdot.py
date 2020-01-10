"""This module defines the Fluepdot class"""

from __future__ import annotations
import math
import curses

from flippy.payload import ClearOptions, FluepOptions, Framebuffer
from flippy._framebuffer_print import ffi, lib

class Fluepdot:
    """A Fluepdot is a single unit consisting of 115x16 pixels"""
    location_x: int = 0
    location_y: int = 0
    rotation: int = 0
    framebuffer: Framebuffer = Framebuffer.create_empty()
    clear_options: ClearOptions = ClearOptions()
    fluep_options: FluepOptions = FluepOptions()
    window = None
    last_command_finish_time: float = 0
    pixels_flipped: int = 0
    curses_color_pair: int = 1
    power_status: int = 0

    @staticmethod
    def from_config(config: dict) -> Fluepdot:
        """Load configuragion"""
        fluepdot = Fluepdot()
        fluepdot.location_x = config.get("x", 0)
        fluepdot.location_y = config.get("y", 0)
        fluepdot.rotation = config.get("rotation", 0)
        return fluepdot

    def get_config(self):
        """Get fluepdot's configuration"""
        return {
            "x": self.location_x,
            "y": self.location_y,
            "rotation": self.rotation
        }

    def get_width(self):
        """Get fluepdot's width"""
        if self.rotation == 0:
            return 115
        return 16

    def get_height(self):
        """Get fluepdot's height"""
        if self.rotation == 0:
            return 16
        return 115

    def contains(self, coordinate) -> bool:
        x,y = coordinate
        return (x >= self.location_x
                and x < self.location_x + self.get_width()
                and y >= self.location_y
                and y < self.location_y + self.get_height())

    def translate_coordinate(self, coordinate):
        x,y = coordinate
        if self.rotation == 0:
            return (x - self.location_x, y - self.location_y)
        return (y - self.location_y, x - self.location_x)

    def create_curses_win(self, color_pair):
        """Initializes a curses window for drawing the fluepdot into"""
        self.window = curses.newwin(
            math.ceil(self.get_height() / 2),
            math.ceil(self.get_width() / 2),
            (self.location_y // 115) * 58,
            (self.location_x // 16) * 8)
        self.curses_color_pair = color_pair
        return self.window

    def redraw_curses(self):
        """Redraw the fluepdot using curses"""
        if self.window is None:
            print("cannot draw fluepdot")
            return
        framebuffer = bytes(self.framebuffer.serialize() + bytes(2))
        try:
            if self.rotation == 0:
                # pylint: disable=no-member
                res = lib.print_framebuffer_horizontal(framebuffer)
            elif self.rotation == 90:
                # pylint: disable=no-member
                res = lib.print_framebuffer_vertical(framebuffer)
            else:
                raise NotImplementedError()
            self.window.addstr(
                0, 0, ffi.string(res), curses.color_pair(
                    self.curses_color_pair))
            # pylint: disable=no-member
            lib.free(res)
        except curses.error:
            pass
        self.window.refresh()
