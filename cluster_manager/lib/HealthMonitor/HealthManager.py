import struct

import select

from lib.HealthMonitor.HealthMonitor.HealthDispatcher import HealthDispatcher
from utils import Transmitter, Singleton, Worker, Config, Interaction

CF = Config()


def base_tcp(is_alive, tcp_socket):
    interaction = Interaction("health")

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


class HealthManager(metaclass=Singleton):

    def __init__(self):
        self.__transmitter = Transmitter(HealthWorker, tcp_receiver=(CF.get("HealthMonitor", 'ip'), int(CF.get("HealthMonitor", 'tcp_port'))), name="health")

        self.__health_monitor = None

    def start(self):
        self.__transmitter.start(receivers=(base_tcp, None))
        # self.__health_monitor.start()

    def stop(self):
        self.__transmitter.stop()
        # self.__health_monitor.stop()


class HealthWorker(Worker, metaclass=Singleton):

    def __init__(self):
        file_system_worker = HealthDispatcher()
        file_system_worker.start()

        super().__init__(tcp_worker=file_system_worker, udp_worker=None)

    def run(self, _, data, address):
        self._tcp_worker.dispatch(data, address)

    def stop(self):
        self._tcp_worker.stop()
