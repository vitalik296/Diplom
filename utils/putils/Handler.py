from threading import Thread
from Interaction import Interaction
from Worker import Worker


class Handler(object):
    def __init__(self, max_process_count):
        self.__interaction = Interaction("request")
        self.__proc_count = max_process_count
        self._threads = []

    def __start_thread(self, callback, kwargs=None):
        self._threads.append(Thread(target=callback, kwargs=kwargs))
        self._threads[-1].daemon = True
        self._threads[-1].start()

    def _handle(self, **kwargs):
        worker = Worker(**kwargs)

        while True:
            data = self.__interaction.remove()

            if data:
                status = worker.run(*data)

                if not status:
                    self.__interaction.insert(data, -1)

    def start(self, **kwargs):
        for _ in range(self.__proc_count):
            self.__start_thread(self._handle, kwargs)
