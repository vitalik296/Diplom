import select
import struct
import uuid

from celery import Celery
from kombu import Queue, Exchange

from utils import Config, StoppedThread, Interaction

CF = Config()

PACKAGE_SIZE = int(CF.get("Package", "size"))
SOCKET_TIMEOUT = int(CF.get("Receiver", "socket_timeout"))

node_id = CF.get("Storage", "uuid") if CF.get("Storage", "uuid") else str(uuid.uuid4().hex)


def checksum_compare(package):
    # TODO add compare
    return True


def udp_receiver(udp_socket, name, queue):
    interaction = Interaction("receive")

    iputs_list = [udp_socket]

    def receive():
        readable, *_ = select.select(iputs_list, [], [], SOCKET_TIMEOUT)

        for s in readable:
            package, address = s.recvfrom(PACKAGE_SIZE)

            if checksum_compare(package):
                interaction.insert((package, address), 0)
                storage.send_task(name=name, queue=queue)
            else:
                # TODO add false handler
                pass
            s.sendto(struct.pack('h', 0), address)

    StoppedThread(name=name, target=receive).start()


storage = Celery(node_id, broker="pyamqp://", include=["lib.Storage.Tasks"], backend="rpc://")
storage.conf.task_queues = (
    Queue(node_id, exchange=Exchange(name="cluster", type="topic"),
          routing_key=f'cluster.{node_id}.*'),
)
