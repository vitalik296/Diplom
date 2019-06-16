import struct
from threading import Thread

from .Config import Config
from .Interaction import Interaction
from .utilities import Socket, StoppedThread

CF = Config()

TCP = 0
UDP = 1

PACKAGE_SIZE = int(CF.get("Package", "size"))


def tcp_request(string):
    return struct.pack('i', len(string)) + string.encode('utf-8')


class Sender(object):
    def __init__(self):
        self._threads = []

    @staticmethod
    def __base_tcp(is_alive):
        # tcp_response = Interaction("tcp_response")

        while is_alive():
            item = Interaction("tcp_sender").remove()

            if item:
                data, address = item
                tcp_socket = Socket.create_tcp()
                print(address)
                tcp_socket.connect(address)
                tcp_socket.send(tcp_request(data))
                tcp_socket.close()

    @staticmethod
    def __base_udp(is_alive):
        udp_response = Interaction("udp_sender")

        udp_socket = Socket.create_udp()

        while is_alive():
            item = udp_response.remove()

            if item:
                data, address = item

                print('received\n', data, address)
                a = udp_socket.sendto(data, address)
                print('sended', a)

        # udp_socket.close()

    def __start_thread(self, name, callback, args=None):
        self._threads.append(StoppedThread(name=name, target=callback, args=args))
        # self._threads[-1].daemon = True
        self._threads[-1].start()

    def start(self, tcp_request=None, udp_request=None):
        tcp_request = tcp_request if tcp_request else Sender.__base_tcp
        udp_request = udp_request if udp_request else Sender.__base_udp

        self.__start_thread(name="response-tcp", callback=tcp_request, args=())
        self.__start_thread(name="response-udp", callback=udp_request, args=())

    def stop(self):
        for thread in self._threads:
            thread.stop()
            thread.join()
