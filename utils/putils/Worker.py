from Interaction import Interaction


class Worker(object):
    def __init__(self, tcp_worker=None, udp_worker=None):
        self.__tcp_worker = tcp_worker if tcp_worker else Worker.__base_tcp_worker
        self.__udp_worker = udp_worker if udp_worker else Worker.__base_udp_worker

    @staticmethod
    def __base_tcp_worker(data, address):
        tcp_response = Interaction("tcp_response")

        # TODO

        tcp_response.insert((data, address))
        return True

    @staticmethod
    def __base_udp_worker(data, address):
        udp_response = Interaction("udp_response")

        # TODO

        udp_response.insert((data, address))
        return True

    def run(self, key, data, address):
        if key == "tcp":
            return self.__tcp_worker(data, address)
        elif key == "udp":
            return self.__udp_worker(data, address)
        else:
            return False
