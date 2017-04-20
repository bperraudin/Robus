import socket

if __name__ == '__main__':
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(('', 9010))

    server.listen(1)
    (client, address) = server.accept()
    print('Connected')

    while True:
        data = client.recv(1)
        if data:
            client.send(data)
