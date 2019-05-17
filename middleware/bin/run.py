from threading import Timer

from lib.Middleware import Middleware

middle = Middleware()

middle.start()

# Timer(5, middle.stop).start()