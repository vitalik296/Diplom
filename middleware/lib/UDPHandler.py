import struct
from functools import reduce

from lib.UDPMapper import UDPMapper
from utils import BaseHandler, Cache, TaskThread, Interaction, Config

CF = Config()


def wrap(payload):
    format, value = payload
    return struct.pack(format, value)


def pack(*args):
    res = map(lambda x: wrap(x), args)
    return reduce(lambda x, y: x + y, res)


class UDPHandler(BaseHandler):

    def __init__(self, **kwargs):
        super().__init__()
        self._request_inter = Interaction("receive")
        self._udp_sender_inter = Interaction("udp_sender")
        self._cache = Cache("middleware")

        self._cache['fd'] = {}
        self._cache['pathname'] = {}
        self._cache['package'] = {}
        self._cache['user'] = {}

        self._mapper = UDPMapper()

        self._handlers = {"udp_read": TaskThread(target=kwargs.get("read", self._read), name="read"),
                          "udp_write": TaskThread(target=kwargs.get("write", self._write), name="write")}

    def execute(self, data, address):

        fd = struct.unpack("i", data[:4])[0]

        if fd == -1:
            self._handlers['udp_read'].add_task((data, address))
        else:
            self._handlers['udp_write'].add_task((data, address))

    def start(self):
        for handler in self._handlers.values():
            handler.start()

    def stop(self):
        for handler in self._handlers.values():
            handler.stop()
            handler.join()

    def _write(self, data, *args, **kwargs):
        data, address = data

        fd = struct.unpack("i", data[:4])[0]
        number = struct.unpack("I", data[4:8])[0]
        data_size = struct.unpack("I", data[8:12])[0]

        node_id, package_id = self._cache['package'].get((fd, number), (None, None))

        if node_id:

            del self._cache['package'][(fd, number)]

            buffer = pack(("i", -1), ("I", int(package_id)), ("I", data_size), (CF.get("Package", "data") + "s", data[12: -2]), ("H", 0))

            ip, udp_port = self._mapper.query("get_node_address_by_node_id", node_id)[0]

            self._udp_sender_inter.insert((buffer, (ip, int(udp_port))))
        else:
            self._request_inter.insert(("udp", data, address), 0)

    def _read(self, data, *args, **kwargs):
        data, _ = data

        pack_id = struct.unpack("I", data[4:8])[0]

        pathname = self._mapper.query("get_pathname_by_pack_id", pack_id)[0][0]

        fd = int(self._cache["pathname"][pathname])

        order_num = self._cache['package'].get(pack_id, -1)

        if order_num != -1:
            del self._cache['package'][pack_id]

        data = pack(("i", fd), ("I", order_num)) + data[8:-2] + pack(('H', 0))

        client_address = self._cache['user'].get(fd, None)

        if client_address:
            # if
                # del self._cache['user'][fd]
            self._udp_sender_inter.insert((data, (client_address['ip'], client_address['udp_port'])))
