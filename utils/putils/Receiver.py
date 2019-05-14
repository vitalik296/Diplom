import select
import struct

from threading import Thread
from Interaction import Interaction
from utils import Socket

DEFAULT_TCP = ("127.0.0.1", 4234)
DEFAULT_UDP = ("127.0.0.1", 4235)

PACKAGE_SIZE = 66


def checksum_compare(package):
    return True


def base_tcp(tcp_socket):
    interaction = Interaction("request")

    tcp_socket.listen(15)
    inputs = [tcp_socket]

    while inputs:
        readable, *_ = select.select(inputs, [], [])

        for s in readable:
            if s is tcp_socket:
                connection, _ = s.accept()
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
                    interaction.insert(("tcp", data, s.getpeername()), 1)
                else:
                    inputs.remove(s)
                    s.close()


def base_udp(udp_socket):
    interaction = Interaction("request")

    print('hereeeeeeeeeeee')
    while True:
        package, address = udp_socket.recvfrom(PACKAGE_SIZE)

        print(package, address)

        if checksum_compare(package):
            interaction.insert(("udp", package, address), 0)
            # udp_socket.sendto(struct.pack('h', 1), address)
        else:
            pass
            # udp_socket.sendto(struct.pack('h', 0), address)


class Receiver(object):
    def __init__(self, tcp_address=DEFAULT_TCP, udp_address=DEFAULT_UDP):
        self._threads = []
        self._tcp_socket = Socket.create_and_bind_tcp(tcp_address)
        self._udp_socket = Socket.create_and_bind_udp(udp_address)

    def __start_thread(self, name, callback, args=None):
        self._threads.append(Thread(name=name, target=callback, args=args))
        self._threads[-1].daemon = True
        self._threads[-1].start()

    def start(self, tcp_request=base_tcp, udp_request=base_udp):
        self.__start_thread(name="request-tcp", callback=tcp_request, args=(self._tcp_socket,))
        self.__start_thread(name="request-udp", callback=udp_request, args=(self._udp_socket,))

    def stop(self):
        for thread in self._threads:
            thread.join()

        self._tcp_socket.close()
        self._udp_socket.close()
