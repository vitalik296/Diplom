from lib.Storage import Storage
from utils import Transmitter, Singleton, Worker


class StorageManager(metaclass=Singleton):

    def __init__(self):
        self.__transmitter = Transmitter(StorageWorker)

        self.__health_monitor = None

    def start(self):
        self.__transmitter.start()
        # self.__health_monitor.start()

    def stop(self):
        self.__transmitter.stop()
        # self.__health_monitor.stop()


class StorageWorker(Worker, metaclass=Singleton):

    def __init__(self):
        file_system_worker = Storage()
        file_system_worker.start()

        super().__init__(tcp_worker=file_system_worker, udp_worker=None)

    def run(self, _, data, address):
        self._tcp_worker.execute(data, address)

    def stop(self):
        self._tcp_worker.stop()
