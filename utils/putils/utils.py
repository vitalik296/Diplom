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
