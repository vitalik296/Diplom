from threading import Thread

from Interaction import Interaction


def base_processor():
    pass


def base_answer_creator():
    pass


class Processor(object):
    def __init__(self, max_process_count):
        self.__request_interaction = Interaction("request")
        self.__response_interaction = Interaction("response")
        self.__proc_count = max_process_count
        self._threads = []

    def __start_thread(self, callback, args=None):
        self._threads.append(Thread(target=callback, args=args))
        self._threads[-1].start()

    def _processor(self, worker):
        while True:
            data = self.__request_interaction.remove()

            if data:
                result = worker(data)

                if not result:
                    self.__request_interaction.insert(data, -1)
                else:
                    self.__response_interaction.insert(result)

    def start(self, worker):
        for _ in range(self.__proc_count):
            self.__start_thread(self._processor, (worker,))
