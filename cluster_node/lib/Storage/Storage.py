import os
import struct
from threading import Timer
from functools import reduce

from celery import Celery

from lib.Storage import StorageMapper
from utils import Config, BaseHandler, TaskThread, Interaction

CF = Config()


def wrap(payload):
    format, value = payload
    return struct.pack(format, value)


def pack(*args):
    res = map(lambda x: wrap(x), args)
    return reduce(lambda x, y: x + y, res)


class Storage(BaseHandler):
    # For test
    __celery = Celery("cluster_node", broker="pyamqp://")

    def __init__(self, **kwargs):
        super().__init__()

        # self.__tcp_sender = Interaction("tcp_sender")
        self.__udp_sender = Interaction("udp_sender")

        self._mapper = StorageMapper()

        self._block_size = int(CF.get("Storage", "block_size"))
        self._filed_memory = 0
        self._size = int(CF.get("Storage", "size"))
        self._seek = 0

        self._node_id = None

        self.__init__storage_file()

        self._handlers = {"write": TaskThread(target=kwargs.get("write", self.udp_write), name="write")}

    def __del__(self):
        self._file.close()

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

    def execute(self, data, address):
        payload = data
        command = "write"

        print(command, payload)

        handler = self._handlers.get(command, None)

        if handler:
            handler.add_task((payload, address))
        else:
            print("Bad command: ", command)

    # def start(self):
    #     for handler in self._handlers.values():
    #         handler.start()
    #
    #     message = "&".join(("init", CF.get("Receiver", "tcp_port"), CF.get("Receiver", "udp_port"),
    #                         str(self._size - self._filed_memory)))
    #
    #     self.__tcp_sender.insert((message, (CF.get("HealthMonitor", "ip"), int(CF.get("HealthMonitor", "port")))))

    def start(self):
        self.__celery.worker_main()
        self.__celery.send_task("health_monitor.init", args=(CF.get("Receiver", "ip"),
                                                             CF.get("Receiver", "tcp_port"),
                                                             CF.get("Receiver", "udp_port")
                                                             , str(self._size - self._filed_memory)))

    def stop(self):
        for handler in self._handlers.values():
            handler.stop()
            handler.join()

    @__celery.task(name="node.read")
    def tcp_read(self, data):
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

            message = "&".join(("status", str(size), str(self._node_id), str(package_id), "True"))

            self.__tcp_sender.insert((message, (CF.get("HealthMonitor", "ip"), int(CF.get("HealthMonitor", "port")))))
        else:
            # TODO add else statement and handler
            pass

    def _write(self, data_to_write, block_offset=0, in_block_offset=0):
        self._file.seek(block_offset * self._block_size + in_block_offset)
        self._filed_memory += len(data_to_write)
        self._file.write(data_to_write)
        self._file.flush()

    def __is_alive_request(self):
        self.__tcp_sender.insert(
            ("alive&" + str(self._node_id) + "&True",
             (CF.get("HealthMonitor", "ip"), int(CF.get("HealthMonitor", "port")))))
        Timer(int(CF.get("HealthMonitor", "repeat_timeout")), self.__is_alive_request).start()

    @__celery.task(name="node.init")
    def _init(self, data):
        payload, _ = data
        self._node_id = int(payload[0])

        self.__is_alive_request()
