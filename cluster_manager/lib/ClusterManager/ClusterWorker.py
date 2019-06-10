from utils import Worker, Singleton
from lib.ClusterManager.ClusterDispatcher import ClusterManagerDispatcher


class ClusterWorker(Worker, metaclass=Singleton):

    def __init__(self):
        file_system_worker = ClusterManagerDispatcher()
        file_system_worker.start()

        super().__init__(tcp_worker=file_system_worker, udp_worker=None)

    def run(self, _, data, address):
        self._tcp_worker.dispatch(data, address)

    def stop(self):
        self._tcp_worker.stop()
