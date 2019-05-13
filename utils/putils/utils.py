import random
import select
import struct

from Interaction import Interaction

TCP_BUFFER_SIZE = 2048

PACKAGE_SIZE = 66


class TypedSingleton(type):
    _instances = {}

    def __call__(cls, class_type, *args, **kwargs):
        if class_type not in cls._instances:
            cls._instances[class_type] = super(TypedSingleton, cls).__call__(class_type, *args, **kwargs)
        return cls._instances[class_type]


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
                    interaction.insert(1, (s, data, "tcp"))
                else:
                    inputs.remove(s)
                    s.close()


def base_udp(udp_socket):
    interaction = Interaction("request")

    while True:
        package, address = udp_socket.recvfrom(PACKAGE_SIZE)

        if checksum_compare(package):
            interaction.insert(0, (address, package, "udp"))
            udp_socket.sendto(struct.pack('h', 1), address)
        else:
            udp_socket.sendto(struct.pack('h', 0), address)
