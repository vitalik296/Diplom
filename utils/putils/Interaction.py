from threading import Lock
from queue import PriorityQueue, Queue

from utils import TypedSingleton


class QueueItem(object):
    def __init__(self, priority, data):
        self.priority = priority
        self.data = data

    def __lt__(self, other):
        return self.priority > other.priority


class Interaction(metaclass=TypedSingleton):

    def __init__(self, inter_type):
        self.__queue = PriorityQueue() if inter_type == "request" else Queue()
        self.__lock = Lock()

    def insert(self, priority = 0, data):
        self.__lock.acquire()

        if type(self.__queue) == PriorityQueue:
            self.__queue.put(QueueItem(priority, data))
        else:
            self.__queue.put(data)

        self.__lock.release()

    def remove(self):
        self.__lock.acquire()

        if self.__queue.empty():
            result = None
        else:
            result = self.__queue.get()

        self.__lock.release()

        return result

    def is_empty(self):
        self.__lock.acquire()
        result = self.__queue.empty()
        self.__lock.release()
        return result
