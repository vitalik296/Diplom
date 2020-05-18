from threading import Lock


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
        try:
            self.__mutex.acquire()
            res = self.__cache_dict[key]
            self.__mutex.release()
            return res
        except Exception as exp:
            print(exp)

    def __str__(self):
        return str(self.__cache_dict)

    def keys(self):
        return list(self.__cache_dict.keys())

    def values(self):
        return list(self.__cache_dict.values())

    def items(self):
        return list(self.__cache_dict.items())
