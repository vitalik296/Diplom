from .Receiver import Receiver, base_tcp, base_udp
from .Sender import Sender
from .Worker import Worker
from .InteractionListener import InteractionListener


class Transmitter(object):

    def __init__(self, worker=Worker, tcp_receiver=None, udp_receiver=None, name="receive"):
        if udp_receiver:
            if tcp_receiver:
                self.__receiver = Receiver(tcp_address=tcp_receiver, udp_address=udp_receiver)
            else:
                self.__receiver = Receiver(udp_address=udp_receiver)
        else:
            if tcp_receiver:
                self.__receiver = Receiver(tcp_address=tcp_receiver, udp_address=None)
            else:
                self.__receiver = Receiver(udp_address=None)

        self.__sender = Sender()

        self.__handler = InteractionListener(1, name=name)

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
