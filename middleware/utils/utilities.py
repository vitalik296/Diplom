import socket
import time
from queue import PriorityQueue, Queue
from threading import Thread, Lock, Event, Timer


class Singleton(type):
    _instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]


class NamedSingleton(Singleton):
    def __call__(cls, name, *args, **kwargs):
        if name not in cls._instances:
            cls._instances[name] = super(Singleton, cls).__call__(name, *args, **kwargs)
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


class StoppedThread(Thread):
    def __init__(self, target=None, name=None, args=None, **kwargs):
        super().__init__(target=target, name=name, args=args, kwargs=kwargs)
        self.stopper = Event()

    def is_alive(self):
        return not self.stopper.is_set()

    def run(self):
        while self.is_alive():
            self._target()

    def start(self):
        super().start()
        print(self.name, "started")

    def stop(self):
        print(self.name, "stopping...")
        self.stopper.set()
        print(self.name, "stopped")


class TaskThread(StoppedThread):

    def __init__(self, target=None, name=None, delay=60, *args, **kwargs):
        super().__init__(target=target, name=name, args=args, kwargs=kwargs)
        self.lock = Lock()
        self.task_queue = []
        self.__delay = delay
        self.__timer = None
        self.__is_timer_set = Event()

    def run(self):
        while not self.stopper.is_set():

            if bool(self.task_queue):

                if self.__is_timer_set.is_set():
                    self.__timer_stop()
                    self.__is_timer_set.clear()

                data = self.__get_task()
                self._target(data, *self._args, **self._kwargs)

            elif not self.__is_timer_set.is_set():
                self.__is_timer_set.set()
                self.__timer = Timer(self.__delay, self.stop)
                self.__timer.start()

    def __get_task(self):
        self.lock.acquire()
        data = self.task_queue.pop(0)
        self.lock.release()
        return data

    def __timer_stop(self):
        self.__timer.cancel()

    def stop(self):
        super().stop()
        self.__timer_stop()
        print(self.name, "Sleeping...")

    def _restart(self):
        print("Waking up ...")
        self.stopper.clear()
        self.__is_timer_set.clear()
        self._started.clear()
        self.start()

    def add_task(self, data):
        self.lock.acquire()
        self.task_queue.insert(len(self.task_queue), data)
        self.lock.release()
        if self.stopper.is_set():
            self._restart()


class Cache(metaclass=NamedSingleton):
    def __init__(self, _):
        self.__cache_dict = {}
        self.__mutex = Lock()

    def __setitem__(self, key, item):
        self.__mutex.acquire()
        self.__cache_dict[key] = item
        self.__mutex.release()

    def __delitem__(self, key):
        self.__mutex.acquire()
        del self.__cache_dict[key]
        self.__mutex.release()

    def get(self, key, default=None):
        try:
            res = self.__getitem__(key)
        except KeyError:
            res = default

        return res

    def __getitem__(self, key):
        self.__mutex.acquire()
        res = self.__cache_dict[key]
        self.__mutex.release()
        return res

    def __str__(self):
        return str(self.__cache_dict)

    def keys(self):
        return list(self.__cache_dict.keys())

    def values(self):
        return list(self.__cache_dict.values())
