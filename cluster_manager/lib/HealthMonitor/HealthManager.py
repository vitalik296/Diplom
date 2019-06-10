from lib.HealthMonitor.HealthMonitor.HealthDispatcher import HealthDispatcher
from utils import Transmitter, Singleton, Worker


class HealthManager(metaclass=Singleton):

    def __init__(self):
        self.__transmitter = Transmitter(HealthWorker)

        self.__health_monitor = None

    def start(self):
        self.__transmitter.start()
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
