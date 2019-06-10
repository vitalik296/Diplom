class BaseHandler(object):

    def __init__(self):
        self._handlers = {}
        self._cache = {}

    def execute(self, data, address):
        raise NotImplementedError

    def add_handler(self, key, handler_func):
        self._handlers[key] = handler_func

    def start(self):
        raise NotImplementedError

    def stop(self):
        raise NotImplementedError
