from .TCPHandler import TCPHandler
from .UDPHandler import UDPHandler
from utils import Worker, Singleton


class MiddleWorker(Worker, metaclass=Singleton):

    def __init__(self):
        tcp_handler = TCPHandler()
        udp_handler = UDPHandler()

        tcp_handler.start()
        udp_handler.start()
        super().__init__(tcp_worker=tcp_handler, udp_worker=udp_handler)

    def run(self, key, data, address):
        if key == "tcp":
            self._tcp_worker.execute(data, address)
        elif key == "udp":
            self._udp_worker.execute(data, address)

    def stop(self):
        self._tcp_worker.stop()
        # self._udp_worker.stop()