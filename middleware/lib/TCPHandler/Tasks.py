from abc import ABC

from celery import Task

from lib.MiddlewareCelery import middleware
from lib.TCPHandler import TCPHandler


class TCPTask(Task, ABC):
    def __init__(self):
        self.tcp_storage = TCPHandler()


@middleware.task(name="middleware.cache_add", queue="tcp_queue",
                 routing_key="middleware.tcp_queue.cache_add",
                 bind=True, base=TCPTask)
def _cache_add(cls, new_items):
    print(f"Task: cache_add. Parameters: new_items={new_items}")

    for key, values in new_items.items():
        pathname, order_num = key
        fd = int(cls.tcp_storage.cache['pathname'][pathname])
        cls.tcp_storage.cache['package'].update({(fd, order_num): values})


# FUSE OPERATION
@middleware.task(name="middleware.open", queue="tcp_queue",
                 routing_key="middleware.tcp_queue.open", bind=True, base=TCPTask)
def _open(cls, pathname, response_address, address=None):
    print(f"Task: open. Parameters: pathname={pathname}")

    if len(response_address) == 2:
        response_ip = response_address[0]
        response_port = response_address[1]
    else:
        response_ip = address[0]
        response_port = response_address[0]

    res = cls.tcp_storage.mapper.query("get_file_by_pathname", pathname)[0]

    if not res:
        return cls.tcp_storage.tcp_sender_inter.insert(("0&File Doesn't Exists", (response_ip, int(response_port))))

    file_type, size, order = res

    fd = cls.tcp_storage.create_fd()

    # TODO add exception
    if fd == -1:
        return 0

    cls.tcp_storage.cache['fd'].update({fd: {'type': file_type, 'size': size, 'order': order, 'pathname': pathname}})
    cls.tcp_storage.cache['pathname'].update({pathname: fd})

    res = "&".join((str(1), str(fd), str(order)))

    cls.tcp_storage.tcp_sender_inter.insert((res, (response_ip, int(response_port))))


@middleware.task(name="middleware.flush", queue="tcp_queue",
                 routing_key="middleware.tcp_queue.flush", bind=True, base=TCPTask)
def _flush(cls, fd, address, response_port):
    print(f"Task: flush. Parameters: fd={fd}")

    pathname = cls._cache['fd']['pathname']

    del cls.tcp_storage.cache['pathname'][pathname]
    del cls.tcp_storage.cache['fd'][int(fd)]

    # TODO add synchronization

    cls.tcp_storage.tcp_sender_inter.insert(('1', (address[0], int(response_port))))


@middleware.task(name="middleware.getattr", queue="tcp_queue",
                 routing_key="middleware.tcp_queue.getattr", bind=True, base=TCPTask)
def _getattr(cls, pathname, address, response_port):
    print(f"Task: getattr. Parameters: pathname={pathname}")

    res = cls.tcp_storage.mapper.query("get_file_attr_by_pathname", pathname)
    if not res:
        return cls.tcp_storage.tcp_sender_inter.insert(("0&File Doesn't Exists", (address[0], int(response_port))))

    file_type, size = res[0]

    res = "&".join((str(1), file_type, "777", str(size)))

    cls.tcp_storage.tcp_sender_inter.insert((res, (address[0], int(response_port))))


@middleware.task(name="middleware.load", queue="tcp_queue",
                 routing_key="middleware.tcp_queue.load", bind=True, base=TCPTask)
def _load(cls, fd, packs):
    print(f"Task: load. Parameters: fd={fd}, packs={packs}")
    pack_count = len(packs)

    client_address = cls.tcp_storage.cache["user"][fd]

    message = "&".join(("1", str(pack_count)))

    cls.tcp_storage.tcp_sender_inter.insert((message, (client_address['ip'], client_address['tcp_port'])))

    for value in packs:
        node_id, pack_id, order_num = value.split(",")
        ip, tcp_port = cls.tcp_storage.mapper.query("get_node_ip_tcp_port", int(node_id))[0]

        cls.tcp_storage.cache["package"].update({int(pack_id): int(order_num)})

        message = "&".join(("read", pack_id))

        cls.tcp_storage.tcp_sender_inter.insert((message, (ip, int(tcp_port))))

        middleware.send_task(name="node.read",
                             exchange="cluster",
                             routing_key=f"cluster.{node_id}.read",
                             args=(pack_id,))


@middleware.task(name="middleware.read", queue="tcp_queue",
                 routing_key="middleware.tcp_queue.read", bind=True, base=TCPTask)
def _read(cls, fd, max_package_count, offset, udp_port, tcp_port, address):
    print(f"Task: read. Parameters: fd={fd}, max_package_count={max_package_count}, offset={offset} "
          f"udp_port={udp_port}, tcp_port={tcp_port}")
    fd = int(fd)
    cls.tcp_storage.cache["user"].update({fd: {"ip": address[0], "tcp_port": int(tcp_port), "udp_port": int(udp_port)}})

    pathname = cls.tcp_storage.cache['fd'].get(fd, None)

    status = cls.tcp_storage.mapper.query("get_file_status", pathname['pathname'])[0]

    if not status or status == "False":
        cls.tcp_storage.tcp_sender_inter.insert(("0&File isn't ready", (address[0], int(tcp_port))))

    middleware.send_task(name="cluster_manager.read",
                         exchange="cluster_manager",
                         routing_key="cluster_manager.read",
                         args=(fd, pathname['pathname']))


@middleware.task(name="middleware.write", queue="tcp_queue",
                 routing_key="middleware.tcp_queue.write", bind=True, base=TCPTask)
def _write(cls, fd, package_count):
    print(f"Task: write. Parameters: fd={fd}, package_count={package_count}")
    pathname = cls.tcp_storage.cache['fd'][int(fd)]['pathname']

    # TODO add exists check

    # res = self._mapper.query("get_file_by_pathname", pathname)
    #
    # if not res:
    #     return self._tcp_sender_inter.insert(("0&File Doesn't Exists", (address[0], int(response_port))))

    middleware.send_task(name="cluster_manager.write",
                         excange="cluster_manager",
                         routing_key="cluster_manager.write",
                         args=(pathname, package_count))


@middleware.task(name="middleware.readdir", queue="tcp_queue",
                 routing_key="middleware.tcp_queue.readdir", bind=True, base=TCPTask)
def _readdir(cls, pathname, address, response_port):
    print(f"Task: readdir. Parameters: pathname={pathname}, address={address}, response_port={response_port}")
    res = cls.tcp_storage.mapper.query("get_direct_child", pathname)

    if not res:
        return cls.tcp_storage.tcp_sender_inter.insert(("0&Directory Doesn't Exists", (address[0], int(response_port))))

    res = [element[0].rsplit('/', 1)[1] for element in res]

    res = "&".join((str(1), *res))

    cls.tcp_storage.tcp_sender_inter.insert((res, (address[0], int(response_port))))


@middleware.task(name="middleware.create", queue="tcp_queue",
                 routing_key="middleware.tcp_queue.create", bind=True, base=TCPTask)
def _create(cls, pathname, address, response_port):
    print(f"Task: create. Parameters: pathname={pathname}, address={address}, response_port={response_port}")
    if cls.tcp_storage.mapper.query('get_file_attr_by_pathname', pathname):
        return cls.tcp_storage.tcp_sender_inter.insert(("0&File Already Exists", (address[0], int(response_port))))
    else:
        middleware.send_task(name="cluster_manager.create",
                             exchange="cluster_manager",
                             routing_key="cluster_manager.create",
                             args=(pathname, address[0], response_port))


@middleware.task(name="middleware.mkdir", queue="tcp_queue",
                 routing_key="middleware.tcp_queue.mkdir", bind=True, base=TCPTask)
def _mkdir(cls, pathname, address, response_port):
    print(f"Task: create. Parameters: pathname={pathname}, address={address}, response_port={response_port}")

    if cls.tcp_storage.mapper.query('get_file_attr_by_pathname', pathname):
        return cls.tcp_storage.tcp_sender_inter.insert(("0&Directory Already Exists", (address[0], int(response_port))))
    else:
        middleware.send_task(name="cluster_manager.mkdir",
                             exchange="cluster_manager",
                             routing_key="cluster_manager.mkdir",
                             args=(pathname,))

        return cls.tcp_storage.tcp_sender_inter.insert(('1', (address[0], int(response_port))))
