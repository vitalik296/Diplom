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
            "mkdir": TaskThread(target=kwargs.get("mkdir", self._mkdir), name="mkdir"),
            "load": TaskThread(target=kwargs.get("load", self._load), name="load"),
            "cache_add": TaskThread(target=kwargs.get("cache_add", self._cache_add), name="cache_add")
        }

        self._cache = Cache("middleware")

        self._cache['fd'] = {}
        self._cache['pathname'] = {}
        self._cache['package'] = {}
        self._cache['user'] = {}

        self._mapper = TCPMapper()

    def __create_fd(self):
        """0...127"""
        keys = self._cache['fd'].keys()

        for i in range(128):
            if i not in keys:
                return i

        return -1

    def execute(self, data, address):
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
    def _cache_add(self, data):
        payload, address = data

        keys, values = payload

        keys = keys.split('|')
        values = values.split('|')

        for key in keys:
            pathname, order_num = key.split(',')
            fd = int(self._cache['pathname'][pathname])
            order_num = int(order_num)
            self._cache['package'].update({(fd, order_num): values.pop(0).split(',')})

    # FUSE OPERATION
    def _open(self, data):

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

        self._cache['fd'].update({fd: {'type': file_type, 'size': size, 'order': order, 'pathname': pathname}})
        self._cache['pathname'].update({pathname: fd})

        res = "&".join((str(1), str(fd), str(order)))

        self._tcp_sender_inter.insert((res, (response_ip, int(response_port))))

    def _flush(self, data):
        payload, address = data

        fd, response_port = payload

        pathname = self._cache['fd']['pathname']

        del self._cache['pathname'][pathname]
        del self._cache['fd'][int(fd)]

        # TODO add synchronization

        self._tcp_sender_inter.insert(('1', (address[0], int(response_port))))

    def _getattr(self, data):

        payload, address = data
        path_name, response_port = payload

        res = self._mapper.query("get_file_attr_by_pathname", path_name)
        if not res:
            return self._tcp_sender_inter.insert(("0&File Doesn't Exists", (address[0], int(response_port))))

        file_type, size = res[0]

        res = "&".join((str(1), file_type, "777", str(size)))

        self._tcp_sender_inter.insert((res, (address[0], int(response_port))))

    def _load(self, data):
        payload, address = data

        fd = int(payload.pop(0))

        pack_count = len(payload)

        client_address = self._cache["user"][fd]

        message = "&".join(("1", str(pack_count)))

        self._tcp_sender_inter.insert((message, (client_address['ip'], client_address['tcp_port'])))

        for value in payload:
            node_id, pack_id, order_num = value.split(",")
            ip, tcp_port = self._mapper.query("get_node_ip_tcp_port", int(node_id))[0]

            self._cache["package"].update({int(pack_id): int(order_num)})

            message = "&".join(("read", pack_id))

            self._tcp_sender_inter.insert((message, (ip, int(tcp_port))))

    def _read(self, data):
        payload, address = data
        fd, max_package_count, offset, udp_port, tcp_port = payload
        fd = int(fd)
        self._cache["user"].update({fd: {"ip": address[0], "tcp_port": int(tcp_port), "udp_port": int(udp_port)}})

        pathname = self._cache['fd'].get(fd, None)

        status = self._mapper.query("get_file_status", pathname['pathname'])[0]

        if not status or status == "False":
            self._tcp_sender_inter.insert(("0&File isn't ready", (address[0], int(tcp_port))))

        message = "&".join(('read', str(fd), pathname['pathname'], str(max_package_count), str(offset)))

        self._tcp_sender_inter.insert((message, CLUSTER_MANAGER_ADDRESS))

    def _write(self, data):

        payload, address = data

        fd, package_count, _ = payload

        pathname = self._cache['fd'][int(fd)]['pathname']

        # TODO add exists check

        # res = self._mapper.query("get_file_by_pathname", pathname)
        #
        # if not res:
        #     return self._tcp_sender_inter.insert(("0&File Doesn't Exists", (address[0], int(response_port))))

        self._tcp_sender_inter.insert(("&".join(("write", pathname, package_count)), CLUSTER_MANAGER_ADDRESS))

    def _readdir(self, data):
        payload, address = data
        path_name, response_port = payload

        res = self._mapper.query("get_direct_child", path_name)

        if not res:
            return self._tcp_sender_inter.insert(("0&Directory Doesn't Exists", (address[0], int(response_port))))

        res = [element[0].rsplit('/', 1)[1] for element in res]

        res = "&".join((str(1), *res))

        self._tcp_sender_inter.insert((res, (address[0], int(response_port))))

    def _create(self, data):

        payload, address = data
        pathname, response_port = payload

        if self._mapper.query('get_file_attr_by_pathname', pathname):
            return self._tcp_sender_inter.insert(("0&File Already Exists", (address[0], int(response_port))))
        else:
            request = "&".join(("create", pathname, address[0], response_port))
            self._tcp_sender_inter.insert((request, CLUSTER_MANAGER_ADDRESS))

    def _mkdir(self, data):
        payload, address = data
        pathname, response_port = payload

        if self._mapper.query('get_file_attr_by_pathname', pathname):
            return self._tcp_sender_inter.insert(("0&Directory Already Exists", (address[0], int(response_port))))
        else:
            request = "&".join(("mkdir", pathname))
            self._tcp_sender_inter.insert((request, CLUSTER_MANAGER_ADDRESS))
            return self._tcp_sender_inter.insert(('1', (address[0], int(response_port))))
