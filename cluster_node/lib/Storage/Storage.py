import os
import struct
from threading import Timer
from functools import reduce

from lib.Storage import StorageMapper
from utils import Config, BaseHandler, TaskThread, Interaction

CF = Config()


def wrap(payload):
    format, value = payload
    return struct.pack(format, value)


def pack(*args):
    return reduce(lambda x, y: wrap(x) + wrap(y), args)


class Storage(BaseHandler):

    def __init__(self, **kwargs):
        super().__init__()

        self.__tcp_sender = Interaction("tcp_sender")
        self.__udo_sender = Interaction("udp_sender")

        self._mapper = StorageMapper()

        self._block_size = int(CF.get("Storage", "block_size"))
        self._filed_memory = 0
        self._size = int(CF.get("Storage", "size"))
        self._seek = 0

        self._node_id = None

        if os.path.isfile(CF.get("Storage", "file_path")):
            self._file = open(CF.get("Storage", "file_path"), 'r+b')
            self._filed_memory = int(self._mapper.query("select_filled_package")[0][0])
        else:
            self._file = open(CF.get("Storage", "file_path"), 'w+b')
            self._file.seek(self._size)
            self._file.write(b'\n')
            for i in range(self._size // self._block_size):
                self._mapper.query("initial_insert", i)

        self._handlers = {"read": TaskThread(target=kwargs.get("read", self.tcp_read), name="read"),
                          "write": TaskThread(target=kwargs.get("write", self.udp_write), name="write"),
                          "init": TaskThread(target=kwargs.get("init", self._init), name="init")}

    def __del__(self):
        self._file.close()

    def execute(self, data, address):
        print(data)
        command, *payload = data.decode("utf-8").split('&')
        if not payload:
            payload = command
            command = "write"
        handler = self._handlers.get(command, None)

        if handler:
            handler.add_task((payload, address))
        else:
            print("Bad command: ", command)

    def start(self):
        for handler in self._handlers.values():
            handler.start()

        message = "&".join(("init", CF.get("Receiver", "tcp_port"), CF.get("Receiver", "udp_port"),
                            str(self._size - self._filed_memory)))

        self.__tcp_sender.insert((message, (CF.get("HealthMonitor", "ip"), int(CF.get("HealthMonitor", "port")))))

    def stop(self):
        for handler in self._handlers.values():
            handler.stop()
            handler.join()

    def tcp_read(self, data, *args, **kwargs):
        payload, address = data

        package_id = payload[0]

        result = self._mapper.query("get_package", package_id)

        if result:
            block_offset, inblock_offset, real_size = result[0]
            self._read(package_id, buffer_size=real_size, block_offset=block_offset)
        else:
            # TODO add exception
            pass

    def _read(self, pack_id, buffer_size=int(CF.get("Storage", "block_size")), block_offset=0, in_block_offset=0):
        self._file.seek(block_offset * self._block_size + in_block_offset)
        buffer = self._file.read(buffer_size - in_block_offset)
        buffer = pack((("i", -1), ("I", pack_id), ("I", buffer_size), (str(self._block_size) + "s", buffer), ("H", 0)))  # TODO add checksum
        print(buffer)
        self.__udo_sender.insert((buffer, (CF.get("Middleware", "ip"), int(CF.get("Middleware", "port")))))

    def udp_write(self, data, *args, **kwargs):
        payload, address = data

        package = payload[0]

        print(package)

        package_id = struct.unpack("I", package[4:8])[0]
        size = struct.unpack("I", package[8:12])[0]
        data = package[12:12 + size]

        free_pack = self._mapper.query("get_free_package")

        if free_pack:
            id, block_offset, inblock_offset = free_pack[0]
            self._write(data, block_offset, inblock_offset)
            self._mapper.query("update_package", (package_id, 0, size, id))

            message = "&".join(("status", str(self._node_id), str(package_id), "True"))

            self.__tcp_sender.insert(message, (CF.get("Cluster Manager", "ip"), int(CF.get("Cluster Manager", "port"))))
        else:
            # TODO add else statement and handler
            pass

    def _write(self, data_to_write, block_offset=0, in_block_offset=0):
        self._file.seek(block_offset * self._block_size + in_block_offset)
        self._filed_memory += len(data_to_write)
        self._file.write(data_to_write)

    def __is_alive_request(self):
        self.__tcp_sender.insert(
            ("alive&" + self._node_id + "&True", (CF.get("HealthMonitor", "ip"), int(CF.get("HealthMonitor", "port")))))
        Timer(int(CF.get("HealthMonitor", "repeat_timeout")), self.__is_alive_request).start()

    def _init(self, data, *args, **kwargs):
        payload, _ = data
        self._node_id = int(payload[0])

        self.__is_alive_request()
