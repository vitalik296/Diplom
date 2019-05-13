import random
import select
import socket
import struct

from threading import Thread
from Interaction import Interaction

DEFAULT_TCP = ("127.0.0.1", 4234)
DEFAULT_UDP = ("127.0.0.1", 4238)

TCP = 0
UDP = 1

TCP_BUFFER_SIZE = 2048

PACKAGE_SIZE = 66


def checksum_compare(package):
    return random.choices([True, False], [0.5, 0.5])


def base_tcp(tcp_socket):
    interaction = Interaction("request")

    tcp_socket.listen(15)
    inputs = [tcp_socket]

    while inputs:
        readable, *_ = select.select(inputs, [], [])

        for s in readable:
            if s is tcp_socket:
                connection, client_address = s.accept()
                connection.setblocking(0)
                inputs.append(connection)
            else:
                data_size = s.recv(struct.calcsize("i"))

                if data_size:
                    data_size = struct.unpack("i", data_size)[0]
                else:
                    inputs.remove(s)
                    s.close()
                    continue

                data = s.recv(data_size)

                if data:
                    interaction.insert((s, data, "tcp"), 1)
                else:
                    inputs.remove(s)
                    s.close()


def base_udp(udp_socket):
    interaction = Interaction("request")

    while True:
        package, address = udp_socket.recvfrom(PACKAGE_SIZE)

        if checksum_compare(package):
            interaction.insert((address, package, "udp"), 0)
            udp_socket.sendto(struct.pack('h', 1), address)
        else:
            udp_socket.sendto(struct.pack('h', 0), address)


class Request(object):

    def __init__(self, tcp_address=DEFAULT_TCP, udp_address=DEFAULT_UDP):
        self._threads = []
        self._tcp_socket = Request.socket_create(tcp_address, TCP)
        self._udp_socket = Request.socket_create(udp_address, UDP)

    @staticmethod
    def socket_create(socket_address, socket_type):
        if socket_type == TCP:
            new_socket = socket.socket(type=socket.SOCK_STREAM)
            new_socket.setblocking(False)
        else:
            new_socket = socket.socket(type=socket.SOCK_DGRAM)

        new_socket.bind(socket_address)

        return new_socket

    def __start_thread(self, name, callback, args=None):
        self._threads.append(Thread(name=name, target=callback, args=args))
        self._threads[-1].start()

    def start(self, tcp_request=base_tcp, udp_request=base_udp):
        self.__start_thread(name="tcp", callback=tcp_request, args=(self._tcp_socket,))
        self.__start_thread(name="udp", callback=udp_request, args=(self._udp_socket,))

    def stop(self):
        for thread in self._threads:
            thread.join()

        self._tcp_socket.close()
        self._udp_socket.close()
