from copy import copy

from lib.ClusterManager.ClusterDispatcher.ClusterManagerMapper import ClusterManagerMapper
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
            "status": TaskThread(target=kwargs.get("status", self._status), name="status"),
        }

        self._cache = Cache("cluster_manager")
        self._cache['node_load'] = {}

        self._mapper = ClusterManagerMapper()

    def dispatch(self, data, address):
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

    def __recalc_load(self, node_id=None):
        if node_id:
            self._cache['node_load'][node_id] = self._cache[node_id]['free_size'] * self._cache[node_id][
                'pack_idle_count']
        else:
            for key in self._cache.keys():
                self._cache['node_load'][key] = self._cache[key]['free_size'] * self._cache[key]['pack_idle_count']

    def __balance(self, package_count):
        res = []

        for _ in range(package_count):
            node_load = copy(self._cache['node_load'])
            node_to_add = min(node_load.items(), key=lambda item: item[1])[0]
            res.append(node_to_add)
            self._cache[node_to_add]['free_size'] -= int(CF.get("Package", "data"))
            self._cache[node_to_add]['pack_idle_count'] += 1

        return res

    def __serialize_dict(self, dict_to_ser):
        key = []
        values = []

        for item in dict_to_ser.items():
            key.append(','.join(item[0]))
            node, package_id = item[1]
            values.append(','.join((str(node), str(package_id))))

        key = '|'.join(key)
        values = '|'.join(values)

        return "&".join((key, values))

    # FILE SYSTEM OPERATIONS

    def _read(self, data, *args, **kwargs):
        payload, address = data

        fd, pathname, max_package_count, offset = payload

        package_list = []

        packages = self._mapper.query('select_package_by_pathname', pathname)

        searched_next_pack_id = None

        for _ in range(len(packages)):

            for package in packages:
                package = list(package)
                next_pack_id = package[2]
                if not next_pack_id:
                    next_pack_id = None
                else:
                    next_pack_id = int(next_pack_id)
                if next_pack_id == searched_next_pack_id:
                    package[2] = str(len(package_list))
                    searched_next_pack_id = int(package[1])
                    package = map(lambda x: str(x), package)
                    package = ','.join(package)
                    package_list.insert(0, package)
                    break

        message = "&".join(("load", str(fd), *package_list))

        self._sender_inter.insert((message, (CF.get("Middleware", "ip"), int(CF.get("Middleware", "port")))))

    def _write(self, data, *args, **kwargs):
        payload, address = data
        pathname, package_count = payload

        package_count = int(package_count)

        node_list = self.__balance(package_count)

        self._mapper.query("update_file_status", ("False", pathname))

        order_num, file_id = self._mapper.query("select_file_info", pathname)[0]

        order_num = int(order_num)

        pack_id_list = []
        result_dict = {}

        for node in node_list:
            parent_id = self._mapper.query("select_parent_id", file_id)
            if parent_id:
                parent_id = int(parent_id[0][0])

            package_id = int(self._mapper.query("insert_package", (node, file_id))[0][0])

            if parent_id:
                self._mapper.query("update_package", (package_id, parent_id))

            pack_id_list.append(package_id)
            result_dict[(pathname, str(order_num))] = (node, package_id)
            order_num += 1

        if order_num == package_count:
            self._mapper.query("update_file_data", (pack_id_list[0], pathname))

        self._mapper.query("update_file_order_num", (order_num, pathname))

        message = "cache_add&" + self.__serialize_dict(result_dict)

        self._sender_inter.insert((message, (address[0], int(CF.get("Middleware", "port")))))

    def _create(self, data, *args, **kwargs):

        payload, address = data

        pathname, response_ip, response_port = payload

        file_id = self._mapper.query("insert_file", pathname, last_row_id=True)[0][0][0]

        dir_pathname = pathname.rsplit('/', 1)[0]

        if not dir_pathname:
            dir_pathname = '/'

        self._mapper.query("update_directory_data", (file_id, dir_pathname))

        request = "&".join(("open", pathname, response_ip, response_port))

        self._sender_inter.insert((request, (CF.get("Middleware", "ip"), int(CF.get("Middleware", "port")))))

    def _mkdir(self, data, *args, **kwargs):

        payload, _ = data

        pathname = payload[0]

        file_id = self._mapper.query("insert_directory", pathname)[0][0]

        pathname = str(pathname)

        parent_dir_pathname = pathname.rsplit('/', 1)[0]

        if not parent_dir_pathname:
            parent_dir_pathname = '/'

        if parent_dir_pathname != pathname:
            self._mapper.query("update_directory_data", (file_id, parent_dir_pathname))

    def _status(self, data, *args, **kwargs):
        payload, _ = data

        file_id, status = payload

        self._mapper.query("update_file_status_by_file_id", (status, file_id))
