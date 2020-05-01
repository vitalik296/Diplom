from threading import Timer

from lib.HealthMonitor.HealthMonitor.HealthMapper import HealthMapper
from utils import Interaction, TaskThread, Cache, Config

CF = Config()


class HealthDispatcher(object):
    def __init__(self, **kwargs):
        super().__init__()
        self._request_inter = Interaction("receive")
        self._sender_inter = Interaction("sender")

        self._handlers = {
            "alive": TaskThread(target=kwargs.get("alive", self._alive), name="alive"),
            "status": TaskThread(target=kwargs.get("status", self._status), name="status"),
            "init": TaskThread(target=kwargs.get("init", self._init), name="init"),
        }

        self._cache = Cache("cluster_manager")
        self._cache['node_load'] = {}

        self._mapper = HealthMapper()
        # self._cache['fd'] = {}

    def dispatch(self, data, address):
        command, *payload = data.decode("utf-8").split('&')
        print(command, payload)
        handler = self._handlers.get(command, None)

        if handler:
            handler.add_task((payload, address))
        else:
            print("Bad command: ", command)
        
        #handler.delay((payload, address))

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

    def _alive(self, data, *args, **kwargs):
        payload, _ = data
        node_id, status = payload

        node_id = int(node_id)

        if self._cache.get(node_id, None):
            if self._cache[node_id]['timer']:
                self._cache[node_id]['timer'].cancel()

        if status == "True":
            self._cache[node_id]['timer'] = Timer(int(CF.get("Node", "node_live_timeout")), self._alive,
                                                  args=[((node_id, "False"), ())])
            self._cache[node_id]['timer'].start()
        elif self._cache.get(node_id, None):
            del self._cache[node_id]
            del self._cache['node_load'][node_id]

    def _status(self, data, *args, **kwargs):
        payload, _ = data
        new_size, node_id, pack_id, status = payload

        node_id = int(node_id)

        self._mapper.query("update_package_status", (status, pack_id))

        res, file_id = self._mapper.query("get_unready_package", (pack_id, pack_id))

        res = res[0]
        file_id = file_id[0]

        self._mapper.query("update_file_size", (new_size, file_id))

        self._cache[node_id]['pack_idle_count'] -= 1
        self._cache['node_load'][node_id] = self._cache[node_id]['pack_idle_count']*self._cache[node_id]['free_size']

        if not res:
            message = "&".join(("status", str(file_id), "True"))
            self._sender_inter.insert((message, (CF.get("Receiver", "ip"), int(CF.get("Receiver", "tcp_port")))))

    def _init(self, data, *args, **kwargs):
        payload, address = data
        tcp_port, udp_port, free_size = payload

        res = self._mapper.query("get_node_by_address", (address[0], int(tcp_port)))

        if not res:
            node_id = self._mapper.query("insert_new_node", (address[0], tcp_port, udp_port))[0][0]
        else:
            node_id = int(res[0][0])

        res = "&".join(("init", str(node_id)))
        self._sender_inter.insert((res, (address[0], int(tcp_port))))

        self._cache[node_id] = {'timer': None,
                                'ip': address[0],
                                'tcp_port': int(tcp_port),
                                'udp_port': int(udp_port),
                                'free_size': int(free_size),
                                'pack_idle_count': 0}
        self._cache['node_load'][node_id] = 0
