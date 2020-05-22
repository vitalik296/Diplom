from lib.UDPHandler.UDPMapper import UDPMapper
from utils import Cache, Interaction, Singleton


class UDPHandler(metaclass=Singleton):

    def __init__(self):
        super().__init__()
        self.receiver = Interaction("receive")
        self.udp_sender_inter = Interaction("udp_sender")
        self.cache = Cache("middleware")

        self.cache['fd'] = {}
        self.cache['pathname'] = {}
        self.cache['package'] = {}
        self.cache['user'] = {}

        self.mapper = UDPMapper()
