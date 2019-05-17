import select
import socket
import struct

from .Interaction import Interaction
# from  import Socket
from .utilities import Socket, StoppedThread

DEFAULT_TCP = ("127.0.0.1", 4234)
DEFAULT_UDP = ("127.0.0.1", 4235)

PACKAGE_SIZE = 66


def base_tcp(is_alive, tcp_socket):
    interaction = Interaction("receive")

    tcp_socket.listen(15)
    inputs = [tcp_socket]

    while is_alive():
        readable, *_ = select.select(inputs, [], [], 5)

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


def checksum_compare(package):
    return True


def base_udp(is_alive, udp_socket):
    interaction = Interaction("receive")

    # udp_socket.settimeout(5)

    iputs_list = [udp_socket]

    while is_alive():

        readable, *_ = select.select(iputs_list, [], [], 5)

        for s in readable:
            package, address = s.recvfrom(PACKAGE_SIZE)

            if checksum_compare(package):
                interaction.insert(("udp", package, address), 0)
                # udp_socket.sendto(struct.pack('h', 1), address)
            else:
                pass
            s.sendto(struct.pack('h', 0), address)


class Receiver(object):
    def __init__(self, tcp_address=DEFAULT_TCP, udp_address=DEFAULT_UDP):
        self._threads = []
        self._tcp_socket = Socket.create_and_bind_tcp(tcp_address)
        self._udp_socket = Socket.create_and_bind_udp(udp_address)

    def __start_thread(self, name, callback, args=None):
        self._threads.append(StoppedThread(name=name, target=callback, args=args))
        # self._threads[-1].daemon = True
        self._threads[-1].start()

    def start(self, tcp_request=base_tcp, udp_request=base_udp):
        self.__start_thread(name="request-tcp", callback=tcp_request, args=(self._tcp_socket,))
        self.__start_thread(name="request-udp", callback=udp_request, args=(self._udp_socket,))

    # def join(self):
    #     for thread in self._threads:
    #         thread.stop()
    #         thread.join()

    def stop(self):
        for thread in self._threads:
            thread.stop()
            thread.join()

        self._tcp_socket.close()
        self._udp_socket.close()
