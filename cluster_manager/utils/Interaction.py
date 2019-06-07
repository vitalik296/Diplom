from threading import Lock

from .utilities import NamedSingleton, MaxPriorityQueue, BaseQueue


class Interaction(metaclass=NamedSingleton):
    def __init__(self, key):
        self.__queue = MaxPriorityQueue() if key == "request" else BaseQueue()
        self.__mutex = Lock()

    def insert(self, data, priority=0):
        self.__mutex.acquire()

        if type(self.__queue) == MaxPriorityQueue:
            self.__queue.insert(data, priority)
        else:
            self.__queue.insert(data)

        self.__mutex.release()

    def remove(self):
        self.__mutex.acquire()
        result = self.__queue.remove()
        self.__mutex.release()
        return result

    def empty(self):
        self.__mutex.acquire()
        result = self.__queue.empty()
        self.__mutex.release()
        return result
