from .Interaction import Interaction


class Worker(object):
    def __init__(self, tcp_worker=None, udp_worker=None):
        self._tcp_worker = tcp_worker if tcp_worker else Worker.__base_tcp_worker
        self._udp_worker = udp_worker if udp_worker else Worker.__base_udp_worker

    @staticmethod
    def __base_tcp_worker(data, address):
        tcp_response = Interaction("tcp_response")

        # TODO

        tcp_response.insert((data, address))

    @staticmethod
    def __base_udp_worker(data, address):
        udp_response = Interaction("udp_response")

        # TODO

        udp_response.insert((data, address))

    def run(self, key, data, address):
        if key == "tcp":
            self._tcp_worker(data, address)
        elif key == "udp":
            self._udp_worker(data, address)

    def stop(self):
        pass
