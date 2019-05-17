from threading import Thread

from .utilities import StoppedThread
from .Interaction import Interaction
from .Worker import Worker


class InteractionListener(object):
    def __init__(self, max_process_count):
        self.__interaction = Interaction("receive")
        self.__proc_count = max_process_count
        self._threads = []

    def __start_thread(self, callback, args=None):
        self._threads.append(StoppedThread(name="listener", target=callback, args=args))
        # self._threads[-1].daemon = True
        self._threads[-1].start()

    @staticmethod
    def _handle(is_alive, worker):
        interaction = Interaction("receive")

        while is_alive():
            data = interaction.remove()

            if data:
                worker.run(*data)

    def start(self, worker=None):

        if worker is None:
            worker = Worker()

        for _ in range(self.__proc_count):
            self.__start_thread(self._handle, args=(worker,))

    def stop(self):
        for thread in self._threads:
            thread.stop()
            thread.join()
