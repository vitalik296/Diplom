from .MiddleWorker import MiddleWorker
from utils import Transmitter, Singleton


class Middleware(metaclass=Singleton):

    def __init__(self):
        self.__transmitter = Transmitter(MiddleWorker)

        # self.__db_mapper = None

    def start(self):
        self.__transmitter.start()

    def stop(self):
        self.__transmitter.stop()
