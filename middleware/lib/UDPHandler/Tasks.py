import struct
from abc import ABC
from functools import reduce

from celery import Task

from lib.MiddlewareCelery import middleware
from lib.UDPHandler import UDPHandler
from utils import Config

CF = Config()


def wrap(payload):
    value_format, value = payload
    return struct.pack(value_format, value)


def pack(*args):
    res = map(lambda x: wrap(x), args)
    return reduce(lambda x, y: x + y, res)


class UDPTask(Task, ABC):
    def __init__(self):
        self.udp_storage = UDPHandler()


@middleware.task(name="middleware.udp_write", queue="udp_queue",
                 routing_key="middleware.udp_queue.write", bind=True, base=UDPTask)
def udp_write(cls):
    data = cls.udp_storage.receiver.remove()
    data, address = data

    fd = struct.unpack("i", data[:4])[0]
    number = struct.unpack("I", data[4:8])[0]
    data_size = struct.unpack("I", data[8:12])[0]

    node_id, package_id = cls.udp_storage.cache['package'].get((fd, number), (None, None))

    if node_id:

        del cls.udp_storage.cache['package'][(fd, number)]

        buffer = pack(("i", -1), ("I", int(package_id)), ("I", data_size),
                      (CF.get("Package", "data") + "s", data[12: -2]), ("H", 0))

        ip, udp_port = cls.udp_storage.mapper.query("get_node_address_by_node_id", node_id)[0]

        cls.udp_storage.udp_sender_inter.insert((buffer, (ip, int(udp_port))))
    else:
        cls.udp_storage.receiver.insert((data, address), 0)
        udp_write.apply_async(countdown=3)


@middleware.task(name="middleware.udp_read", queue="udp_queue",
                 routing_key="middleware.udp_queue.read", bind=True, base=UDPTask)
def udp_read(cls):
    data = cls.udp_storage.udp_sender_inter.remove()
    data, address = data

    pack_id = struct.unpack("I", data[4:8])[0]

    pathname = cls.udp_storage.mapper.query("get_pathname_by_pack_id", pack_id)[0][0]

    fd = int(cls.udp_storage.cache["pathname"][pathname])

    order_num = cls.udp_storage.cache['package'].get(pack_id, -1)

    if order_num != -1:
        del cls.udp_storage.cache['package'][pack_id]

    data = pack(("i", fd), ("I", order_num)) + data[8:-2] + pack(('H', 0))

    client_address = cls.udp_storage.cache['user'].get(fd, None)

    if client_address:
        cls.udp_storage.udp_sender_inter.insert((data, (client_address['ip'], client_address['udp_port'])))
