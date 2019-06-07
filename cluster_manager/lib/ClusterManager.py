from lib.ClusterWorker import ClusterWorker
from utils import Transmitter, Singleton


class ClusterManager(metaclass=Singleton):

    def __init__(self):
        self.__transmitter = Transmitter(ClusterWorker)

        self.__health_monitor = None

    def start(self):
        self.__transmitter.start()
        # self.__health_monitor.start()

    def stop(self):
        self.__transmitter.stop()
        # self.__health_monitor.stop()
