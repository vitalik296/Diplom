import os
import struct
import uuid
from functools import reduce
from threading import Timer, Thread

from celery import Celery
from kombu import Exchange, Queue

from lib.Storage import StorageMapper
from utils import Config, TaskThread, Interaction

CF = Config()


def wrap(payload):
    format, value = payload
    return struct.pack(format, value)


def pack(*args):
    res = map(lambda x: wrap(x), args)
    return reduce(lambda x, y: x + y, res)


class Storage(object):
    # For test
    node_id = str(uuid.uuid4().hex)
    celery = Celery(node_id, broker="pyamqp://")

    def __init__(self, **kwargs):
        print(self.node_id)
        super().__init__()

        self.celery.conf.task_queues = (
            Queue(self.node_id, exchange=Exchange(name=self.node_id, type="topic"),
                  routing_key=f'cluster.{self.node_id}.*'),
        )

        self.__udp_sender = Interaction("udp_sender")

        self._mapper = StorageMapper()

        self._block_size = int(CF.get("Storage", "block_size"))
        self._filed_memory = 0
        self._size = int(CF.get("Storage", "size"))
        self._seek = 0

        self._node_id = None

        self.__init__storage_file()

        self._handlers = {"write": TaskThread(target=kwargs.get("write", self.udp_write), name="write")}

        self.celery.send_task(name="health_monitor.init",
                              args=(self.node_id,
                                    CF.get("Receiver", "ip"),
                                    CF.get("Receiver", "udp_port"),
                                    self._size - self._filed_memory),
                              exchange="health_monitor",
                              routing_key="health_monitor.init")

    def __del__(self):
        self._file.close()

    def __start_thread(self):
        self.celery.worker_main()

    def start(self):
        Thread(name=self.node_id, target=self.__start_thread).start()

    def __init__storage_file(self):
        if os.path.isfile(CF.get("Storage", "file_path")):
            self._file = open(CF.get("Storage", "file_path"), 'r+b')
            self._filed_memory = int(self._mapper.query("select_filled_package")[0][0])
        else:
            self._file = open(CF.get("Storage", "file_path"), 'w+b')
            self._file.seek(self._size)
            self._file.write(b'\n')
            self._file.flush()
            print("Initial Load...")
            for i in range(self._size // self._block_size):
                print(str(100 * i / self._size * self._block_size) + "%")
                self._mapper.query("initial_insert", i)

    @celery.task(name="node.read", queue=f"cluster.{node_id}", routing_key=f"cluster.{node_id}.read", )
    def tcp_read(self, package_id):

        result = self._mapper.query("get_package", package_id)

        if result:
            block_offset, inblock_offset, real_size = result[0]
            self._read(package_id, buffer_size=real_size, block_offset=block_offset)
        else:
            # TODO add exception
            pass

    def _read(self, pack_id, buffer_size=int(CF.get("Storage", "block_size")), block_offset=0, in_block_offset=0):
        print(block_offset, self._block_size + in_block_offset)
        print(block_offset * self._block_size + in_block_offset)
        self._file.seek(block_offset * self._block_size + in_block_offset)
        buffer = self._file.read(buffer_size - in_block_offset)
        buffer = pack(("i", -1), ("I", int(pack_id)), ("I", int(buffer_size)), (str(self._block_size) + "s", buffer),
                      ("H", 0))  # TODO add checksum

        self.__udp_sender.insert((buffer, (CF.get("Middleware", "ip"), int(CF.get("Middleware", "udp_"
                                                                                                "port")))))

    def udp_write(self, data):
        payload, address = data

        package = payload

        package_id = struct.unpack("I", package[4:8])[0]
        size = struct.unpack("I", package[8:12])[0]
        data = package[12:12 + size]

        free_pack = self._mapper.query("get_free_package")

        if free_pack:
            id, block_offset, inblock_offset = free_pack[0]
            self._write(data, block_offset, inblock_offset)
            self._mapper.query("update_package", (package_id, 0, size, id))

            self.celery.send_task(name="health_monitor.status",
                                  args=(str(size), self.node_id, str(package_id), "True"),
                                  exchange="health_monitor",
                                  routing_key="health_monitor.status")
        else:
            # TODO add else statement and handler
            pass

    def _write(self, data_to_write, block_offset=0, in_block_offset=0):
        self._file.seek(block_offset * self._block_size + in_block_offset)
        self._filed_memory += len(data_to_write)
        self._file.write(data_to_write)
        self._file.flush()

    def __is_alive_request(self):
        print("here")
        self.celery.send_task(name="health_monitor.alive",
                              args=(self.node_id, "True"),
                              exchange="health_monitor",
                              routing_key="health_monitor.alive")
        Timer(int(CF.get("HealthMonitor", "repeat_timeout")), self.__is_alive_request).start()

