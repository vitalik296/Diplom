import socket
from queue import PriorityQueue, Queue


class NamedSingleton(type):
    _instances = {}

    def __call__(cls, name, *args, **kwargs):
        if name not in cls._instances:
            cls._instances[name] = super(NamedSingleton, cls).__call__(name, *args, **kwargs)
        return cls._instances[name]


class BaseQueue(object):
    def __init__(self):
        self.__queue = Queue()

    def insert(self, item):
        self.__queue.put(item)

    def remove(self):
        if not self.__queue.empty():
            return self.__queue.get()
        return None

    def empty(self):
        return self.__queue.empty()


class MaxPriorityItem(object):
    def __init__(self, item, priority):
        self.item = item
        self.priority = priority

    def __lt__(self, other):
        return self.priority > other.priority


class MaxPriorityQueue(BaseQueue):
    def __init__(self):
        self.__queue = PriorityQueue()

    def insert(self, item, priority=0):
        self.__queue.put(MaxPriorityItem(item, priority))

    def remove(self):
        if not self.__queue.empty():
            return self.__queue.get().item
        return None


class Socket(object):
    @staticmethod
    def create_tcp(blocking=True):
        new_socket = socket.socket(type=socket.SOCK_STREAM)
        new_socket.setblocking(blocking)
        return new_socket

    @staticmethod
    def create_udp():
        new_socket = socket.socket(type=socket.SOCK_DGRAM)
        return new_socket

    @staticmethod
    def create_and_bind_tcp(server_address, blocking=False):
        server_socket = Socket.create_tcp(blocking)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(server_address)
        return server_socket

    @staticmethod
    def create_and_bind_udp(server_address):
        server_socket = Socket.create_udp()
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(server_address)
        return server_socket
