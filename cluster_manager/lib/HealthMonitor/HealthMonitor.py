from abc import ABC
from threading import Timer

from celery import Task

from lib.ClusterCelery import cluster_manager
from lib.HealthMonitor.HealthMapper import HealthMapper
from utils import Cache, Config

CF = Config()


class HealthMonitor(Task, ABC):

    def __init__(self):
        super().__init__()

        self._cache = Cache("cluster_manager")
        self._cache['node_load'] = {}

        self._mapper = HealthMapper()


@cluster_manager.task(bind=True, base=HealthMonitor, name="health_monitor.alive", queue="health_monitor",
                      routing_key="health_monitor.alive")
def _alive(self, uuid, status):
    print(f"Task: alive. Parameters: uuid={uuid}, status={status}")

    if self._cache.get(uuid, None):
        if self._cache[uuid]['timer']:
            self._cache[uuid]['timer'].cancel()

    if status == "True":
        self._cache[uuid]['timer'] = Timer(int(CF.get("Node", "node_live_timeout")), _alive,
                                           args=[((uuid, "False"), ())])
        self._cache[uuid]['timer'].start()
    elif self._cache.get(uuid, None):
        del self._cache[uuid]
        del self._cache['node_load'][uuid]


@cluster_manager.task(bind=True, base=HealthMonitor, name="health_monitor.status", queue="health_monitor",
                      routing_key="health_monitor.status")
def _status(cls, new_size, uuid, pack_id, status):

    print(f"Task: status. Parameters: new_size={new_size}, uuid={uuid}, pack_id={pack_id}, status={status}")

    cls._mapper.query("update_package_status", (status, pack_id))

    res, file_id = cls._mapper.query("get_unready_package", (pack_id, pack_id))

    res = res[0]
    file_id = file_id[0]

    cls._mapper.query("update_file_size", (new_size, file_id))

    cls._cache[uuid]['pack_idle_count'] -= 1
    cls._cache['node_load'][uuid] = cls._cache[uuid]['pack_idle_count'] * cls._cache[uuid]['free_size']

    if not res:
        cluster_manager.send_task(name="cluster_manager.status",
                                  args=(str(file_id), "True"),
                                  exchange="middleware",
                                  routing_key="middleware.status")


@cluster_manager.task(bind=True, base=HealthMonitor, name="health_monitor.init", queue="health_monitor",
                      routing_key="health_monitor.init")
def _init(cls, uuid, ip, udp_port, free_size):

    print(f"Task: init. Parameters: uuid={uuid}, ip={ip}, udp_port={udp_port}, free_size={free_size}")

    node = cls._mapper.query("get_node_by_uuid", uuid)

    if not node:
        cls._mapper.query("insert_new_node", (uuid, ip, udp_port))

    cls._cache[uuid] = {'timer': None,
                        'ip': ip,
                        'udp_port': int(udp_port),
                        'free_size': int(free_size),
                        'pack_idle_count': 0}
    cls._cache['node_load'][uuid] = 0
