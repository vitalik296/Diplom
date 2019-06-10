from lib.ClusterManager.ClusterManager import ClusterManager
from lib.HealthMonitor.HealthManager import HealthManager

cluster_manager = ClusterManager()
health_monitor = HealthManager()

cluster_manager.start()
health_monitor.start()

