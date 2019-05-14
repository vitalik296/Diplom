from threading import Thread
from Interaction import Interaction
from utils import Socket

TCP = 0
UDP = 1

PACKAGE_SIZE = 66


class Sender(object):
    def __init__(self):
        self._threads = []

    @staticmethod
    def __base_tcp():
        tcp_response = Interaction("tcp_response")

        while True:
            item = tcp_response.remove()

            if item:
                data, address = item
                print(data, address)
                tcp_socket = Socket.create_tcp()
                tcp_socket.connect(address)
                tcp_socket.send(data)
                tcp_socket.close()

    @staticmethod
    def __base_udp():
        udp_response = Interaction("udp_response")

        udp_socket = Socket.create_udp()

        while True:
            item = udp_response.remove()

            if item:
                data, address = item

                print('RRRRRRRRRRRRRR', data, address)
                a = udp_socket.sendto(data, address)
                print('EEEEEEEEEEEEEE', a)

        udp_socket.close()

    def __start_thread(self, name, callback, args=None):
        self._threads.append(Thread(name=name, target=callback, args=args))
        self._threads[-1].daemon = True
        self._threads[-1].start()

    def start(self, tcp_request=None, udp_request=None):
        tcp_request = tcp_request if tcp_request else Sender.__base_tcp
        udp_request = udp_request if udp_request else Sender.__base_udp

        self.__start_thread(name="response-tcp", callback=tcp_request, args=())
        self.__start_thread(name="response-udp", callback=udp_request, args=())

    def stop(self):
        for thread in self._threads:
            thread.join()
