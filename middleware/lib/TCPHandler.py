from utils import BaseHandler, Interaction, TaskThread


class TCPHandler(BaseHandler):

    def __init__(self, **kwargs):
        super().__init__()
        self._request_inter = Interaction("receive")
        # self._tcp_inter = Interaction("tcp_request")
        # self._udp_inter = Interaction("udp_request")
        self._handlers = {
            "open": TaskThread(target=kwargs.get("open", self._open), name="open"),
            "flush": TaskThread(target=kwargs.get("flush", self._flush), name="flush"),
            "read": TaskThread(target=kwargs.get("read", self._read), name="read"),
            "write": TaskThread(target=kwargs.get("write", self._write), name="write"),
            "getattr": TaskThread(target=kwargs.get("getattr", self._getattr), name="getattr"),
            "readdir": TaskThread(target=kwargs.get("readdir", self._readdir), name="readdir"),
            "create": TaskThread(target=kwargs.get("create", self._create), name="create")
        }

    def execute(self, data, address):
        print(data)
        command, *payload = data.decode("utf-8").split('&')
        print(command, payload)
        handler = self._handlers.get(command, None)

        if handler:
            handler.add_task((payload, address))
        else:
            print("Bad command: ", command)

    def add_handler(self, key, handler_func):
        self._handlers[key] = TaskThread(target=handler_func, name=key)
        self._handlers[key].start()

    def start(self):
        for handler in self._handlers.values():
            handler.start()

    def stop(self):
        for handler in self._handlers.values():
            handler.stop()
            handler.join()

    def _open(self, data, *args, **kwargs):
        payload, address = data

        # some_var, *_ = payload

        print("open function: ", data)
        inter = Interaction('tcp_response')
        inter.insert((payload[0], ("127.0.0.1", 9001)))

    def _flush(self, data, *args, **kwargs):
        print("flush function: ", data)

    def _read(self, data, *args, **kwargs):
        print("read function: ", data)

    def _write(self, data, *args, **kwargs):
        print("write function: ", data)

    def _getattr(self, data, *args, **kwargs):
        print("getattr function: ", data)

    def _readdir(self, data, *args, **kwargs):
        print("readdir function: ", data)

    def _create(self, data, *args, **kwargs):
        print("create function: ", data)
