import os
import struct
from functools import reduce
from threading import Timer
from time import sleep

from celery.result import AsyncResult

from lib.Storage import StorageMapper
from lib.StorageCelery import node_id, storage
from utils import Config, Interaction, Singleton

CF = Config()


def wrap(payload):
    format, value = payload
    return struct.pack(format, value)


def pack(*args):
    res = map(lambda x: wrap(x), args)
    return reduce(lambda x, y: x + y, res)


class Storage(metaclass=Singleton):

    def __init__(self):
        self.udp_sender = Interaction("udp_sender")
        self.udp_receiver = Interaction("receive")

        self.mapper = StorageMapper()

        self.block_size = int(CF.get("Storage", "block_size"))
        self.filed_memory = 0
        self.size = int(CF.get("Storage", "size"))
        self.seek = 0

        self.node_id = node_id

        self.__init__storage_file()

        self.__initialize_node()

    def __del__(self):
        self._file.close()

    def __init__storage_file(self):
        if os.path.isfile(CF.get("Storage", "file_path")):
            self._file = open(CF.get("Storage", "file_path"), 'r+b')
            self._filed_memory = int(self.mapper.query("select_filled_package")[0][0])
        else:
            self._file = open(CF.get("Storage", "file_path"), 'w+b')
            self._file.seek(self.size)
            self._file.write(b'\n')
            self._file.flush()
            print("Initial Load...")
            for i in range(self.size // self.block_size):
                print(str(100 * i / self.size * self.block_size) + "%")
                self.mapper.query("initial_insert", i)

    def __initialize_node(self):
        task_id = storage.send_task(name="health_monitor.init",
                                    args=(self.node_id,
                                          CF.get("Receiver", "ip"),
                                          CF.get("Receiver", "udp_port"),
                                          self.size - self._filed_memory),
                                    exchange="health_monitor",
                                    routing_key="health_monitor.init")

        task = AsyncResult(task_id)

        while not task.ready():
            pass

        self.__is_alive_request()

    def read(self, pack_id, buffer_size=int(CF.get("Storage", "block_size")), block_offset=0, in_block_offset=0):
        self._file.seek(block_offset * self.block_size + in_block_offset)
        buffer = self._file.read(buffer_size - in_block_offset)
        buffer = pack(("i", -1), ("I", int(pack_id)), ("I", int(buffer_size)), (str(self.block_size) + "s", buffer),
                      ("H", 0))  # TODO add checksum

        self.udp_sender.insert((buffer, (CF.get("Middleware", "ip"), int(CF.get("Middleware", "udp_port")))))

        storage.send_task(name="node.send_udp",
                          exchange=f"cluster",
                          routing_key=f"cluster.{node_id}.send_udp")

    def write(self, data_to_write, block_offset=0, in_block_offset=0):
        self._file.seek(block_offset * self.block_size + in_block_offset)
        self._filed_memory += len(data_to_write)
        self._file.write(data_to_write)
        self._file.flush()

    def __is_alive_request(self):
        storage.send_task(name="health_monitor.alive",
                          args=(self.node_id, "True"),
                          exchange="health_monitor",
                          routing_key="health_monitor.alive")
        Timer(int(CF.get("HealthMonitor", "repeat_timeout")), self.__is_alive_request).start()
