from lib.StorageCelery import storage, node_id, udp_receiver
from utils import Socket, Config

CF = Config()

DEFAULT_UDP = (CF.get("Receiver", "ip"), int(CF.get("Receiver", "udp_port")))

udp_receiver(Socket.create_and_bind_udp(DEFAULT_UDP))

storage.worker_main()
