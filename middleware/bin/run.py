from lib.MiddlewareCelery import middleware, udp_receiver
from utils import Config, Socket

CF = Config()

DEFAULT_IP = CF.get("Receiver", "ip")
CLIENT_UDP_PORT = CF.get("Receiver", "udp_client_port")
CLUSTER_UDP_PORT = CF.get("Receiver", "udp_cluster_port")


udp_receiver(Socket.create_and_bind_udp((DEFAULT_IP, CLIENT_UDP_PORT)),
             Socket.create_and_bind_udp((DEFAULT_IP, CLUSTER_UDP_PORT)))

middleware.worker_main()
