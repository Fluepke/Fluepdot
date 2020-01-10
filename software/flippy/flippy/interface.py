"""This module defines all Interface classes.
Interfaces are used for interfacing with the art installation"""
from __future__ import annotations
from socket import socket, AF_UNIX, SOCK_STREAM
from select import select
import os
from queue import Queue
from abc import ABCMeta, abstractmethod
from threading import Thread
import time

from cobs import cobs
import serial
import crcmod

from flippy.command import Command


class Interface(metaclass=ABCMeta):
    """An interface transmits / receives @see Commands"""

    @classmethod
    def get_interface_classes(cls):
        """Returns all subclasses of @see Interface"""
        for subclass in cls.__subclasses__():
            yield from subclass.get_interface_classes()
            yield subclass

    @staticmethod
    def from_config(config: dict):
        """Load interface configuration"""
        for interface_class in Interface.get_interface_classes():
            if interface_class.__name__ == config["type"]:
                return interface_class(config)
        raise ValueError(f"Interface type '{config['type']}' not known")

    @abstractmethod
    def get_config(self) -> dict:
        """Gets the current interface configuration"""

    @abstractmethod
    def is_up(self) -> bool:
        """Returns the current interface status"""

    @abstractmethod
    def set_up(self):
        """Power on the interface"""

    @abstractmethod
    def down(self):
        """Interface shutdown"""


class RxInterface(Interface):
    """An interface that receives data. Used in the simulator"""
    def __init__(self):
        self.rx_queue = None
        self.rx_count = 0

    @abstractmethod
    def is_up(self) -> bool:
        """Returns the current interface status"""

    @abstractmethod
    def set_up(self):
        """Power on the interface"""

    @abstractmethod
    def down(self):
        """Interface shutdown"""

    def receive_raw(self, data):
        """Used internally to deserialize received frames"""
        for chunk in data.split(b'\x00'):
            if len(chunk) == 0:
                continue
            decoded = cobs.decode(chunk)
            if self.rx_queue is None:
                return
            command = Command.from_raw(decoded)
            command.rx_interface = self
            self.rx_queue.put(command)
            self.rx_count += 1

    def set_rx_queue(self, value):
        """Python does not allow to wait / select from multiple Queues, thus
        a master thread can set the interface's queue externally"""
        self.rx_queue = value

    def get_rx_count(self) -> int:
        """Get the receive count"""
        return self.rx_count


class TxInterface(Interface):
    """An interface that transmits data"""

    def __init__(self):
        self.crc_fun = crcmod.mkCrcFun(0x18005, rev=True, initCrc=0x0, xorOut=0x0)

    def send(self, command: Command):
        """Send a command"""
        data = command.serialize()
        data = cobs.encode(data) + b'\x00'
        self.send_bytes(data)

    @abstractmethod
    def send_bytes(self, data: bytes):
        """Send raw data"""

    @abstractmethod
    def get_tx_count(self) -> int:
        """Get the transmit count"""


class MirrorTxInterface(TxInterface):
    """Mirrors the data to send on all subinterfaces"""

    def __init__(self, config: dict):
        super().__init__()
        self.tx_count = config["tx_count"]
        self.__interfaces = dict((
            interface_name,
            Interface.from_config(interface_config))
                                 for interface_name, interface_config
                                 in config["interfaces"].items())

    def get_config(self) -> dict:
        interfaces = dict((
            interface_name,
            interface.get_config())
                          for interface_name, interface
                          in self.__interfaces.items())
        return {
            "type": type(self).__name__,
            "tx_count": self.get_tx_count(),
            "interfaces": interfaces,
        }

    def is_up(self) -> bool:
        return any(interface.is_up() for interface in self.__interfaces)

    def set_up(self):
        for interface_name, interface in self.__interfaces.items():
            print(f"Bringing interface '{interface_name}' up")
            interface.set_up()

    def down(self):
        for interface_name, interface in self.__interfaces.items():
            print(f"Shutting down interface '{interface_name}'")
            interface.down()

    def send_bytes(self, data: bytes):
        for interface in self.__interfaces.values():
            interface.send_bytes(data)
        self.tx_count += 1

    def get_tx_count(self):
        return self.tx_count


class SocketTxInterface(TxInterface):
    """Sends data to a unix socket"""

    def __init__(self, config: dict):
        super().__init__()
        socket_path = config["socket_path"]
        if os.path.exists(socket_path):
            os.remove(socket_path)
        self.socket_path = socket_path
        self.server_socket = socket(AF_UNIX, SOCK_STREAM)
        self.sockets = [self.server_socket]
        self.socket_thread = Thread(
            daemon=True,
            target=self.handle_socket,
            name=f"handle_socket '{socket_path}'",
            args=())
        self.tx_thread = Thread(
            daemon=True,
            target=self.tx_task,
            name=f"tx '{socket_path}'",
            args=())
        self.tx_queue = Queue()
        self.tx_count = config.get("tx_count", 0)
        self.wants_shutdown = False

    def get_config(self) -> dict:
        return {
            "type": type(self).__name__,
            "tx_count": self.get_tx_count(),
            "socket_path": self.socket_path
        }

    def set_up(self):
        if self.is_up():
            return
        self.wants_shutdown = False
        self.server_socket.bind(self.socket_path)
        self.server_socket.listen(23)
        self.socket_thread.start()
        self.tx_thread.start()

    def down(self):
        self.wants_shutdown = True
        self.tx_queue.put(None)
        # try to connect to the server socket to notify the handle_socket thread
        temp_socket = socket(AF_UNIX, SOCK_STREAM)
        temp_socket.connect(self.socket_path)
        temp_socket.close()
        while self.wants_shutdown:
            time.sleep(0.01)

    def is_up(self):
        return self.socket_thread.is_alive() and self.tx_thread.is_alive()

    def send_bytes(self, data: bytes):
        self.tx_queue.put(data)

    def get_tx_count(self):
        return self.tx_count

    def handle_socket(self):
        """Handles socket (dis)connects"""
        while True:
            readable, _, errored = select(
                self.sockets, [], self.sockets)
            if self.wants_shutdown:
                print("Shutdown requested")
                for shutdown_socket in self.sockets:
                    shutdown_socket.close()
                self.wants_shutdown = False
                return
            for notified_socket in readable:
                try:
                    if notified_socket is self.server_socket:
                        client, _ = self.server_socket.accept()
                        self.sockets.append(client)
                        print("client connected")
                    elif not notified_socket.recv(1):
                        self.sockets.remove(notified_socket)
                        notified_socket.close()
                        print("client disconnected")
                except ConnectionResetError:
                    self.sockets.remove(notified_socket)
                    notified_socket.close()

            for notified_socket in errored:
                self.sockets.remove(notified_socket)
                notified_socket.close()
                print("client disconnected unexpectedly")

    def tx_task(self):
        """Transmits packets"""
        while True:
            if self.wants_shutdown:
                print("Shutdown requested")
                return
            data = self.tx_queue.get()
            if data is None:
                continue
            for _socket in self.sockets:
                if _socket is not self.server_socket:
                    _socket.sendall(data)
            self.tx_count += 1
            self.tx_queue.task_done()


class SerialTxInterface(TxInterface):
    """Sends data to a serial interface"""

    def __init__(self, config: dict):
        super().__init__()
        self.tx_count = config.get("tx_count", 0)
        self.port_path = config["port_path"]
        self.baudrate = config["baudrate"]
        self.parity = config["parity"]
        self.stopbits = config["stopbits"]
        self.tx_queue = Queue()
        self.tx_thread = Thread(
            daemon=True,
            target=self.tx_task,
            name=f"tx '{self.port_path}'",
            args=())
        self.port = None

    def get_config(self):
        return {
            "tx_count": self.get_tx_count(),
            "port_path": self.port_path,
            "baudrate": self.baudrate,
            "parity": self.parity,
            "stopbits": self.stopbits,
        }

    def set_up(self):
        if self.is_up():
            return
        self.port = serial.Serial(
            port=self.port_path,
            baudrate=self.baudrate,
            parity=self.parity,
            stopbits=self.stopbits)
        self.tx_thread.start()

    def down(self):
        self.port.close()
        self.tx_queue.put(None)

    def is_up(self):
        return self.port is not None and self.port.open

    def send_bytes(self, data):
        self.tx_queue.put(data)

    def get_tx_count(self):
        return self.tx_count

    def tx_task(self):
        """Transmits data"""
        while True:
            if not self.port.open:
                return
            data = self.tx_queue.get()
            if data is None:
                continue
            self.port.write(data)
            #self.port.flush()
            self.tx_count += 1


class SocketRxInterface(RxInterface):
    """Receives data from a unix socket"""

    socket_path: str
    """The socket path to connect to"""

    notify_socket: socket
    """Used to notify the RX thread to shutdown"""

    client_socket: socket
    """Used to connect to a UNIX socket"""

    rx_count: int
    """Received count"""

    wants_shutdown: bool
    """Used to signal shutdown request"""

    def __init__(self, config: dict):
        super().__init__()
        self.socket_path = config["socket_path"]
        self.notify_socket = socket(AF_UNIX, SOCK_STREAM)
        self.client_socket = socket(AF_UNIX, SOCK_STREAM)
        self.rx_count = config["rx_count"]
        self.rx_thread = Thread(
            daemon=True,
            target=self.rx_task,
            name=f"rx '{self.socket_path}'")
        self.wants_shutdown = False

    def get_config(self):
        return {
            "type": type(self).__name__,
            "socket_path": self.socket_path,
            "rx_count": self.rx_count
        }

    def set_up(self):
        if self.is_up():
            return
        self.wants_shutdown = False
        notify_socket_path = f"{self.socket_path}.notify.sock"
        if os.path.exists(notify_socket_path):
            os.remove(notify_socket_path)
        self.notify_socket.bind(notify_socket_path)
        self.notify_socket.listen(1)
        self.client_socket.connect(self.socket_path)
        self.rx_thread.start()

    def down(self):
        self.wants_shutdown = True
        temp_socket = socket(AF_UNIX, SOCK_STREAM)
        temp_socket.connect(self.notify_socket.getsockname())
        temp_socket.close()
        while self.wants_shutdown:
            time.sleep(0.1)

    def is_up(self):
        return self.rx_thread.is_alive()

    def rx_task(self):
        """Receives data"""
        chunk = bytearray()
        while True:
            readable, _, _ = select([self.client_socket, self.notify_socket], [], [])
            if self.wants_shutdown:
                print("Shutdown requested")
                self.client_socket.close()
                self.notify_socket.close()
                self.wants_shutdown = False
                return
            for readable_socket in readable:
                if readable_socket == self.client_socket:
                    chunk += bytearray(self.client_socket.recv(1024))
            if len(chunk) > 0 and chunk[-1] == 0:
                self.receive_raw(chunk)
                chunk = bytearray()
