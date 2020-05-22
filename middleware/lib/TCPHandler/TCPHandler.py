from lib.TCPHandler.TCPMapper import TCPMapper
from utils import Interaction, Cache, Config, Singleton

CF = Config()

CLUSTER_MANAGER_ADDRESS = (CF.get("Cluster Manager", "ip"), int(CF.get("Cluster Manager", "port")))


class TCPHandler(metaclass=Singleton):

    def __init__(self):
        super().__init__()
        self.request_inter = Interaction("receive")
        self.tcp_sender_inter = Interaction("tcp_sender")
        self.udp_sender_inter = Interaction("udp_sender")

        self.cache = Cache("middleware")

        self.cache['fd'] = {}
        self.cache['pathname'] = {}
        self.cache['package'] = {}
        self.cache['user'] = {}

        self.mapper = TCPMapper()

    def create_fd(self):
        """0...127"""
        keys = self.cache['fd'].keys()

        for i in range(128):
            if i not in keys:
                return i

        return -1
