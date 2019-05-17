from lib.MiddleWorker import MiddleWorker
from utils import Receiver, Sender, Interaction, Worker, InteractionListener
from utils.Receiver import base_tcp, base_udp


class Transmitter(object):

    def __init__(self, worker=Worker):
        self.__receiver = Receiver()
        self.__sender = Sender()

        self.__handler = InteractionListener(1)

        self.__worker = worker()

        # self.__rec_inter = Interaction("receive")
        # self.__tcp_sender_inter = Interaction("tcp_sender")
        # self.__udp_sender_inter = Interaction("udp_sender")

    def start(self, receivers=(base_tcp, base_udp), senders=()):
        self.__receiver.start(*receivers)

        self.__sender.start(*senders)

        self.__handler.start(worker=self.__worker)

    def stop(self):
        self.__receiver.stop()
        self.__handler.stop()
        self.__worker.stop()
        self.__sender.stop()
