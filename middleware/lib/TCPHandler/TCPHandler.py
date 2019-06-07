from lib.TCPHandler.TCPMapper import TCPMapper
from utils import BaseHandler, Interaction, TaskThread, Cache, Config

CF = Config()

CLUSTER_MANAGER_ADDRESS = (CF.get("Cluster Manager", "ip"), int(CF.get("Cluster Manager", "port")))


class TCPHandler(BaseHandler):

    def __init__(self, **kwargs):
        super().__init__()
        self._request_inter = Interaction("receive")
        self._tcp_sender_inter = Interaction("tcp_sender")
        self._udp_sender_inter = Interaction("udp_sender")
        self._handlers = {
            "open": TaskThread(target=kwargs.get("open", self._open), name="open"),
            "flush": TaskThread(target=kwargs.get("flush", self._flush), name="flush"),
            "read": TaskThread(target=kwargs.get("read", self._read), name="read"),
            "write": TaskThread(target=kwargs.get("write", self._write), name="write"),
            "getattr": TaskThread(target=kwargs.get("getattr", self._getattr), name="getattr"),
            "readdir": TaskThread(target=kwargs.get("readdir", self._readdir), name="readdir"),
            "create": TaskThread(target=kwargs.get("create", self._create), name="create"),
            "mkdir": TaskThread(target=kwargs.get("mkdir", self._mkdir), name="mkdir")
        }

        self._cache = Cache("middleware")

        self._cache['fd'] = {}
        self._cache['package'] = {}

        self._mapper = TCPMapper()
        # self._cache['fd'] = {}

    def __create_fd(self):
        """0...127"""
        keys = self._cache.keys()

        for i in range(128):
            if i not in keys:
                return i

        return -1

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

    # CACHE OPERATION
    def _cache_add(self, data, *args, **kwargs):
        print("cache_add method: ", data)

    def _cache_del(self, data, *args, **kwargs):
        print("cache_del method: ", data)

    # FUSE OPERATION
    def _open(self, data, *args, **kwargs):

        payload, address = data
        pathname, *response_address = payload

        if len(response_address) == 2:
            response_ip = response_address[0]
            response_port = response_address[1]
        else:
            response_ip = address[0]
            response_port = response_address[0]

        res = self._mapper.query("get_file_by_pathname", pathname)[0]

        if not res:
            return self._tcp_sender_inter.insert(("0&File Doesn't Exists", (response_ip, int(response_port))))

        file_type, size, order = res

        fd = self.__create_fd()

        # TODO add exception
        if fd == -1:
            return 0

        self._cache['fd'].update({fd: {'type': file_type, 'size': size, 'order': order}})

        res = "&".join((str(1), str(fd), str(order)))

        self._tcp_sender_inter.insert((res, (address[0], int(response_port))))

    def _flush(self, data, *args, **kwargs):
        payload, address = data

        fd, response_port = payload
        del self._cache[int(fd)]

        # TODO add synchronization

        self._tcp_sender_inter.insert(('1', (address[0], int(response_port))))

    def _getattr(self, data, *args, **kwargs):

        payload, address = data
        path_name, response_port = payload

        res = self._mapper.query("get_file_attr_by_pathname", path_name)[0]
        if not res:
            return self._tcp_sender_inter.insert(("0&File Doesn't Exists", (address[0], int(response_port))))

        res = "&".join((1, *res, "777"))

        self._tcp_sender_inter.insert((res, (address[0], int(response_port))))

    def _read(self, data, *args, **kwargs):
        print("read function: ", data)

    def _write(self, data, *args, **kwargs):

        payload, address = data

        # TODO add exist condition

        self._tcp_sender_inter.insert(("&".join(("write", *payload)), CLUSTER_MANAGER_ADDRESS))

    def _readdir(self, data, *args, **kwargs):
        payload, address = data
        path_name, response_port = payload

        res = self._mapper.query("get_direct_child", path_name)

        if not res:
            return self._tcp_sender_inter.insert(("0&Directory Doesn't Exists", (address[0], int(response_port))))

        res = [element[0].rsplit('/', 1)[1] for element in res]

        res = "&".join((str(1), *res))

        self._tcp_sender_inter.insert((res, (address[0], int(response_port))))

    def _create(self, data, *args, **kwargs):

        payload, address = data
        pathname, response_port = payload

        if self._mapper.query('get_file_attr_by_pathname', pathname):
            return self._tcp_sender_inter.insert(("0&File Already Exists", (address[0], int(response_port))))
        else:
            request = "&".join(("create", pathname, address[0], response_port))
            self._tcp_sender_inter.insert((request, CLUSTER_MANAGER_ADDRESS))

    def _mkdir(self, data, *args, **kwargs):
        payload, address = data
        pathname, response_port = payload

        if self._mapper.query('get_file_attr_by_pathname', pathname):
            return self._tcp_sender_inter.insert(("0&Directory Already Exists", (address[0], int(response_port))))
        else:
            request = "&".join(("mkdir", pathname))
            self._tcp_sender_inter.insert((request, CLUSTER_MANAGER_ADDRESS))
            return self._tcp_sender_inter.insert(('1', (address[0], int(response_port))))
