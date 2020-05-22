from celery import Celery
from kombu import Queue, Exchange


cluster_manager = Celery("cluster_manager", broker="pyamqp://", backend="rpc://",
                         include=['lib.ClusterManager.ClusterManager', 'lib.HealthMonitor.HealthMonitor'])


cluster_manager.conf.task_queues = (
    Queue('cluster_manager', exchange=Exchange(name="cluster_manager", type="topic"), routing_key='cluster_manager.*'),
    Queue('health_monitor', exchange=Exchange(name="health_monitor", type="topic"), routing_key='health_monitor.#'),
)
