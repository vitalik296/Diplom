import select
import struct
import uuid

from celery import Celery
from kombu import Queue, Exchange

from utils import Interaction, Config, StoppedThread
from lib.UDPHandler.Tasks import udp_read, udp_write

CF = Config()

PACKAGE_SIZE = int(CF.get("Package", "size"))
SOCKET_TIMEOUT = int(CF.get("Receiver", "socket_timeout"))


# def crc16(buffer, size):
#     crc = 0
#
#     for i in range(0, size):
#         crc ^= buffer[i]
#         for j in range(0, 8):
#             if (crc & 1) > 0:
#                 crc = (crc >> 1) ^ 0x8005
#             else:
#                 crc = crc >> 1
#
#     return crc


def checksum_compare(package, checksum):
    # return crc16(package, PACKAGE_SIZE-2) == checksum
    return True


def udp_receiver(client_socket, cluster_socket):
    interaction = Interaction("receive")

    inputs_list = [client_socket, cluster_socket]

    def receive():
        readable, *_ = select.select(inputs_list, [], [], SOCKET_TIMEOUT)

        for s in readable:
            package, address = s.recvfrom(PACKAGE_SIZE)

            *_, checksum = struct.unpack('<III' + str(512) + 'sH', package)

            if checksum_compare(package, checksum):
                interaction.insert(("udp", package, address), 0)

                if s is client_socket:
                    udp_write.delay(queue="udp_queue")
                else:
                    udp_read.delay(queue="udp_queue")

                s.sendto(struct.pack('h', 1), address)
            else:
                s.sendto(struct.pack('h', 0), address)

    StoppedThread(name="udp_receiver", target=receive).start()


middleware = Celery("middleware", broker="pyamqp://", backend="rpc://",
                    include=['lib.TCPHandler.Tasks', 'lib.UDPHandler.Tasks'])

middleware.conf.task_queues = (
    Queue('tcp_queue', exchange=Exchange(name="tcp_queue", type="topic"), routing_key='middleware.tcp_queue.*'),
    Queue('udp_queue', exchange=Exchange(name="udp_queue", type="topic"), routing_key='middleware.udp_queue.*'),
)
