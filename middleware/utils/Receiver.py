import select
import struct

from .Config import Config
from .Interaction import Interaction
# from  import Socket
from .utilities import Socket, StoppedThread

CF = Config()

DEFAULT_TCP = (CF.get("Receiver", "ip"), int(CF.get("Receiver", "tcp_port")))

PACKAGE_SIZE = int(CF.get("Package", "size"))

SOCKET_TIMEOUT = int(CF.get("Receiver", "socket_timeout"))


def base_tcp(is_alive, tcp_socket):
    interaction = Interaction("receive")

    tcp_socket.listen(15)
    inputs = [tcp_socket]

    while is_alive():
        readable, *_ = select.select(inputs, [], [], SOCKET_TIMEOUT)

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


class Receiver(object):
    def __init__(self, tcp_address=DEFAULT_TCP):
        self._threads = []
        self._tcp_socket = Socket.create_and_bind_tcp(tcp_address)

    def __start_thread(self, name, callback, args=None):
        self._threads.append(StoppedThread(name=name, target=callback, args=args))
        # self._threads[-1].daemon = True
        self._threads[-1].start()

    def start(self, tcp_request=base_tcp):
        self.__start_thread(name="request-tcp", callback=tcp_request, args=(self._tcp_socket,))

    def stop(self):
        for thread in self._threads:
            thread.stop()
            thread.join()

        self._tcp_socket.close()
