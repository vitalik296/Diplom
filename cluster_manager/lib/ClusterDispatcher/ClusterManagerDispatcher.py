from lib.ClusterDispatcher.ClusterManagerMapper import ClusterManagerMapper
from utils import Interaction, TaskThread, Cache, Config

CF = Config()


class ClusterManagerDispatcher(object):
    def __init__(self, **kwargs):
        super().__init__()
        self._request_inter = Interaction("receive")
        self._sender_inter = Interaction("sender")

        self._handlers = {
            "read": TaskThread(target=kwargs.get("read", self._read), name="read"),
            "write": TaskThread(target=kwargs.get("write", self._write), name="write"),
            "create": TaskThread(target=kwargs.get("create", self._create), name="create"),
            "mkdir": TaskThread(target=kwargs.get("mkdir", self._mkdir), name="mkdir"),
            "init": TaskThread(target=kwargs.get("init", self._init), name="init"),
            "change_node": TaskThread(target=kwargs.get("change_node", self._change_node), name="change_node")
        }

        self._cache = Cache("middleware")

        self._cache['fd'] = {}
        self._cache['package'] = {}

        self._mapper = ClusterManagerMapper()
        # self._cache['fd'] = {}

    def dispatch(self, data, address):
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

    # FILE SYSTEM OPERATIONS

    def _read(self, data, *args, **kwargs):
        print("read function: ", data)

    def _write(self, data, *args, **kwargs):
        print("write function: ", data)

    def _create(self, data, *args, **kwargs):

        print("HERE")

        payload, address = data

        pathname, response_ip, response_port = payload

        file_id = self._mapper.query("insert_file", pathname, last_row_id=True)[0][0][0]

        dir_pathname = pathname.rsplit('/', 1)[0]

        self._mapper.query("update_directory_data", (file_id, dir_pathname))

        request = "&".join(("open", pathname, response_ip, response_port))

        self._sender_inter.insert((request, (CF.get("Middleware", "ip"), int(CF.get("Middleware", "port")))))

    def _mkdir(self, data, *args, **kwargs):

        pathname, _ = data

        self._mapper.query("insert_directory", pathname)
        file_id = self._mapper.query("select_file_id_by_pathname", pathname)[0][0]

        pathname = str(pathname)

        parent_dir_pathname = pathname.rsplit('/', 1)[0]

        if parent_dir_pathname != pathname:
            self._mapper.query("update_directory_data", (file_id, parent_dir_pathname))

    # CLUSTER OPERATIONS

    def _init(self, data, *args, **kwargs):
        print("init node function: ", data)

    def _change_node(self, data, *args, **kwargs):
        print("change_node function: ", data)
