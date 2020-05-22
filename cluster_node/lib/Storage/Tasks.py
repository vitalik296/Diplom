import struct
from abc import ABC

from celery import Task

from lib.Storage import Storage
from lib.StorageCelery import storage, node_id
from utils import Socket


class StorageTask(Task, ABC):
    def __init__(self):
        self.storage = Storage()


@storage.task(bind=True, base=StorageTask, name="node.read", queue=f"{node_id}",
              routing_key=f"cluster.{node_id}.read")
def tcp_read(cls, package_id):
    print(f"Task: read. Parameters: package_id={package_id}")

    result = cls.storage.mapper.query("get_package", package_id)

    if result:
        block_offset, inblock_offset, real_size = result[0]
        cls.storage.read(package_id, buffer_size=real_size, block_offset=block_offset)
    else:
        # TODO add exception
        pass


@storage.task(bind=True, base=StorageTask, name="node.write", queue=f"{node_id}",
              routing_key=f"cluster.{node_id}.write")
def udp_write(cls):
    data = cls.storage.udp_receiver.remove()
    payload, address = data

    package = payload

    package_id = struct.unpack("I", package[4:8])[0]
    size = struct.unpack("I", package[8:12])[0]
    data = package[12:12 + size]

    free_pack = cls.storage.mapper.query("get_free_package")

    if free_pack:
        id, block_offset, inblock_offset = free_pack[0]
        cls.storage.write(data, block_offset, inblock_offset)
        cls.storage.mapper.query("update_package", (package_id, 0, size, id))

        storage.send_task(name="health_monitor.status",
                          args=(str(size), node_id, str(package_id), "True"),
                          exchange="health_monitor",
                          routing_key="health_monitor.status")
    else:
        # TODO add else statement and handler
        pass


@storage.task(bind=True, base=StorageTask, name="node.send_udp", queue=f"{node_id}",
              routing_key=f"cluster.{node_id}.send_udp")
def base_udp(cls):
    udp_socket = Socket.create_udp()

    item = cls.storage.udp_sender.remove()

    if item:
        data, address = item

        print('Received', data, address)
        a = udp_socket.sendto(data, address)
        print('Sended', a)

    udp_socket.close()
