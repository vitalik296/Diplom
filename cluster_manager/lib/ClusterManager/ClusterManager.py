from copy import copy

from kombu import Queue

from lib.ClusterCelery import cluster_manager
from lib.ClusterManager.ClusterManagerMapper import ClusterManagerMapper
from utils import Cache, Config

CF = Config()


class ClusterManager(object):

    def __init__(self):
        super().__init__()

        self._cache = Cache("cluster_manager")
        self._cache['node_load'] = {}

        self._mapper = ClusterManagerMapper()

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

    @staticmethod
    def __serialize_dict(dict_to_ser):
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
    @cluster_manager.task(name="cluster_manager.read", queue="cluster_manager", routing_key="cluster_manager.read")
    def read(self, fd, pathname):
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

        cluster_manager.send_task(name="middleware.load",
                                  args=(str(fd), *package_list),
                                  exchange="middleware",
                                  routing_key="middleware.load")

    @cluster_manager.task(name="cluster_manager.write", queue="cluster_manager", routing_key="cluster_manager.write")
    def write(self, pathname, package_count):
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

        cluster_manager.send_task(name="middleware.cache_add",
                                  args=(self.__serialize_dict(result_dict),),
                                  exchange="middleware",
                                  routing_key="middleware.cache_add")

    @cluster_manager.task(name="cluster_manager.create", queue="cluster_manager", routing_key="cluster_manager.create")
    def create(self, pathname, response_ip, response_port):

        file_id = self._mapper.query("insert_file", pathname, last_row_id=True)[0][0][0]

        dir_pathname = pathname.rsplit('/', 1)[0]

        if not dir_pathname:
            dir_pathname = '/'

        self._mapper.query("update_directory_data", (file_id, dir_pathname))

        cluster_manager.send_task(name="middleware.open",
                                  args=(pathname, response_ip, response_port),
                                  exchange="middleware",
                                  routing_key="middleware.open")

    @cluster_manager.task(name="cluster_manager.create", queue="cluster_manager", routing_key="cluster_manager.mkdir")
    def mkdir(self, pathname):

        file_id = self._mapper.query("insert_directory", pathname)[0][0]

        pathname = str(pathname)

        parent_dir_pathname = pathname.rsplit('/', 1)[0]

        if not parent_dir_pathname:
            parent_dir_pathname = '/'

        if parent_dir_pathname != pathname:
            self._mapper.query("update_directory_data", (file_id, parent_dir_pathname))

    @cluster_manager.task(name="cluster_manager.status", queue="cluster_manager", routing_key="cluster_manager.status")
    def status(self, file_id, status):

        self._mapper.query("update_file_status_by_file_id", (status, file_id))
