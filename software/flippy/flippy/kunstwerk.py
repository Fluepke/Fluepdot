"""This module defines the Kunstwerk class"""
import json
import time
import curses
import operator
from queue import Queue
from threading import Thread
from typing import Dict, Tuple

from prometheus_client import start_http_server
from prometheus_client.core import GaugeMetricFamily, CounterMetricFamily, REGISTRY

from flippy.fluepdot import Fluepdot
from flippy.interface import Interface, RxInterface
from flippy.program import Program
from flippy.constants import BROADCAST_ADDRESS
from flippy.command import Command, PowerCommand
from flippy.payload import BooleanPayload

class Kunstwerk:
    """A kunstwerk (english: artwork) is a combination of several fluepdots"""

    fluepdots: Dict[int, Fluepdot]
    interfaces: Dict[str, Interface]
    interface_mapping: Dict[int, Interface]

    def __init__(self, config: dict):
        self.enable_curses = config['kunstwerk']['enable_curses']
        self.enable_metrics = config['kunstwerk']['enable_metrics']
        self.__init_interfaces(
            config['kunstwerk']['interfaces'])
        self.__init_fluepdots(
            config['kunstwerk']['fluepdots'])
        self.interface_mapping = self.__init_interface_mapping(
            config['kunstwerk']['fluepdots'])
        self.wants_shutdown = False
        self.curses_thread = Thread(
            daemon=True,
            target=self.curses,
            name='curses interface',
            args=())
        self.rx_queue = Queue()

    def __init_interfaces(self, config: dict):
        """Setup interfaces from the given configuration"""
        self.interfaces = dict((
            interface_name,
            Interface.from_config(interface_config))
                               for interface_name, interface_config
                               in config.items())

    def __init_fluepdots(self, config: dict):
        """Setup fluepdots from the given configuration"""
        self.fluepdots = dict((
            int(address),
            Fluepdot.from_config(fluepdot_config))
                              for address, fluepdot_config
                              in config.items())

    def __init_interface_mapping(self, config: dict):
        return dict((
            int(address),
            self.interfaces[fluepdot_config["interface"]])
                    for address, fluepdot_config
                    in config.items())

    def translate_coordinate(self, coordinate):
        if any(map(operator.gt, coordinate, self.get_dimensions())):
            raise IndexError()
        for address,fluepdot in self.fluepdots.items():
            if fluepdot.contains(coordinate):
                print(f"{coordinate} -> {fluepdot.translate_coordinate(coordinate)}")
                return (address, fluepdot.translate_coordinate(coordinate))
        return False

    def execute_command(self, command: Command):
        """Execute a single command"""
        if command.address == BROADCAST_ADDRESS:
            for interface in self.interfaces.values():
                interface.send(command)
        else:
            self.interface_mapping[command.address].send(command)

    def execute(self, program: Program):
        """Execute a given program on the Kunstwerk"""
        last_sleep = time.perf_counter()
        if not program.validated:
            return
        for timestamp, address, preencoded in program.raw:
            if timestamp != 0:
                sleep_time = (timestamp / 1000) - \
                    ((time.perf_counter() - last_sleep) / 1000)
                # should be precise enough on *nix
                time.sleep(max(0, sleep_time))
                last_sleep = time.perf_counter()
            if address == BROADCAST_ADDRESS:
                # wait for all interfaces to finish up sending commands
                waiting = True
                while waiting:
                    done = True
                    for interface in self.interfaces.values():
                        if not interface.tx_queue.empty():
                            time.sleep(0.0001)
                            done = False
                    waiting = not done
                for interface in self.interfaces.values():
                    interface.send_bytes(preencoded)
            else:
                self.interface_mapping[address].send_bytes(preencoded)

    @staticmethod
    def from_file(file_path: str):
        """Restore kunstwerk configuration from a file"""
        with open(file_path) as config_file:
            config = json.load(config_file)
        return Kunstwerk(config)

    def get_config(self):
        """Get the kunstwerk's configuration as a dict, e.g. to store it to a file"""
        fluepdots = {}
        for address, fluepdot in self.fluepdots.items():
            fluepdots[address] = fluepdot.get_config()
        interfaces = {}
        for name, interface in self.interfaces.items():
            interfaces[name] = interface.get_config()
            interface.name = name
        for address, interface in self.interface_mapping.items():
            fluepdots[address]['interface'] = interface.name
        kunstwerk_config = {
            "enable_curses": self.enable_curses,
            "interfaces": interfaces,
            "fluepdots": fluepdots
        }
        return {"kunstwerk": kunstwerk_config}

    def get_dimensions(self) -> Tuple[int, int]:
        """Get the kunstwerk's dimensions. Returns an (width, height) tuple"""
        return (
            max(fluepdot.location_x + fluepdot.get_width()
                for fluepdot in self.fluepdots.values()),
            max(fluepdot.location_y + fluepdot.get_height()
                for fluepdot in self.fluepdots.values()))

    def set_up(self):
        """Up() the interfaces and fluepdots"""
        for interface_name, interface in self.interfaces.items():
            print(f"Bringing interface '{interface_name}' up")
            interface.set_up()
            if isinstance(interface, RxInterface):
                interface.set_rx_queue(self.rx_queue)
        if self.enable_metrics:
            REGISTRY.register(KunstwerkStatisticsCollector(self))
            start_http_server(9101, "")
        if self.enable_curses:
            screen = curses.initscr()
            screen.clear()
            curses.start_color()
            curses.noecho()
            curses.init_color(1, 1000, 1000, 0)
            curses.init_color(2, 0, 0, 0)
            curses.init_color(1, 1000, 1000, 1000)
            curses.init_color(3, 200, 200, 200)
            curses.init_pair(1, 1, 2)
            curses.init_pair(2, 1, 3)
            i = 0
            for fluepdot in self.fluepdots.values():
                fluepdot.create_curses_win(((i + (i // 48)) % 2) + 1)
                i += 1
                fluepdot.redraw_curses()
        self.curses_thread.start()

    def down(self):
        """Down() the interfaces and the fluepdots"""
        for interface in self.interfaces.values():
            interface.down()
        self.wants_shutdown = True
        self.rx_queue.put(None)
        if self.enable_curses:
            curses.endwin()

    def power_on(self):
        """Power on the fluepdots"""
        for address, _ in self.fluepdots.items():
            power_on_cmd = PowerCommand(BooleanPayload(value=True))
            power_on_cmd.address = address
            self.execute_command(power_on_cmd)

    def curses(self):
        """Thread method handling received messages"""
        while True:
            if self.wants_shutdown:
                return
            received_command = self.rx_queue.get()
            if received_command is None:
                continue
            if received_command.address == BROADCAST_ADDRESS:
                for address, interface in self.interface_mapping.items():
                    if interface == received_command.rx_interface:
                        received_command.simulate(self.fluepdots[address])
            else:
                received_command.simulate(
                    self.fluepdots[received_command.address])
            self.rx_queue.task_done()


class KunstwerkStatisticsCollector:
    """Provide prometheus statistics"""
    kunstwerk: Kunstwerk
    power_status: GaugeMetricFamily

    def __init__(self, kunstwerk: Kunstwerk):
        self.kunstwerk = kunstwerk
        self.power_status = GaugeMetricFamily(
            "fluepdot_power_status",
            "1 if the fluepdot is powered on",
            labels=["address"])
        self.pixels_flipped = CounterMetricFamily(
            "fluepdot_pixels_flipped",
            "counts the number of pixels flipped",
            labels=["address"])

    def collect(self):
        """Collect metrics"""
        for address, fluepdot in self.kunstwerk.fluepdots.items():
            self.power_status.add_metric([str(address)], fluepdot.power_status)
            self.pixels_flipped.add_metric([str(address)], fluepdot.pixels_flipped)
        yield self.power_status
        yield self.pixels_flipped
