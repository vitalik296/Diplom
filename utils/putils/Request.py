import socket
from threading import Thread

from Interaction import Interaction
from utils import base_tcp, base_udp

DEFAULT_TCP = ("127.0.0.1", 4234)
DEFAULT_UDP = ("127.0.0.1", 4238)

TCP = 0
UDP = 1


class Request(object):

    def __init__(self, tcp_address=DEFAULT_TCP, udp_address=DEFAULT_UDP):
        self._threads = []
        self._tcp_socket = Request.socket_create(tcp_address, TCP)
        self._udp_socket = Request.socket_create(udp_address, UDP)

    @staticmethod
    def socket_create(socket_address, socket_type):
        if socket_type == TCP:
            new_socket = socket.socket(type=socket.SOCK_STREAM)
        else:
            new_socket = socket.socket(type=socket.SOCK_DGRAM)

        new_socket.setblocking(False)

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
