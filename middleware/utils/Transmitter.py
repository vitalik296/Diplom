from .Receiver import Receiver, base_tcp, base_udp
from .Sender import Sender
from .Worker import Worker
from .InteractionListener import InteractionListener


class Transmitter(object):

    def __init__(self, worker=Worker):
        self.__receiver = Receiver()
        self.__sender = Sender()

        self.__handler = InteractionListener(1)

        self.__worker = worker()

    def start(self, receivers=(base_tcp, base_udp), senders=()):
        self.__receiver.start(*receivers)

        self.__sender.start(*senders)

        self.__handler.start(worker=self.__worker)

    def stop(self):
        self.__receiver.stop()
        self.__handler.stop()
        self.__worker.stop()
        self.__sender.stop()
