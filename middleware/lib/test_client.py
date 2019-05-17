import socket
import struct


def tcp_request(string):
    return struct.pack('i', len(string)) + string.encode('utf-8')

# Create a TCP/IP socket
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = ("127.0.0.1", 4234)
print('connecting to {} port {}'.format(*server_address))
client.connect(server_address)

message = 'open&data'
print('sending {!r}'.format(message))
client.send(tcp_request(message))
print('closing socket')
client.close()


server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(("127.0.0.1", 9001))

server.listen(5)

sock, _ = server.accept()
print("Connected")
data = sock.recv(1024)
print(data)
