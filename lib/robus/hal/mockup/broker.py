import zmq

from threading import Thread

port = 9001


if __name__ == '__main__':
    c = zmq.Context()
    broker = c.socket(zmq.REP)
    broker.bind('tcp://*:{}'.format(port))

    connections = {}
    last_port = port + 1

    def broadcast(s):
        while True:
            msg = s.recv()
            print('Received {}'.format(msg))
            for out in connections.values():
                try:
                    out.send(msg, zmq.NOBLOCK)
                except zmq.error.Again:
                    pass

    print('Broker running...')
    while True:
        print('Waiting for connection...')
        req = broker.recv()
        print('Req: {}'.format(req))

        s = c.socket(zmq.PAIR)
        s.bind('tcp://*:{}'.format(last_port))
        connections[last_port] = s

        t = Thread(target=lambda: broadcast(s))
        t.daemon = True
        t.start()

        rep = '{}'.format(last_port)
        print('Send: {}'.format(rep))
        broker.send_string(rep)
        last_port += 1
