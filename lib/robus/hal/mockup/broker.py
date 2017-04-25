import zmq

from threading import Thread
from collections import defaultdict

BROKER_PORT = 9001

ctx = zmq.Context()


class Broker(object):
    def __init__(self):
        self.last_port = BROKER_PORT + 1
        self.conn = {}

        self.neighbors = defaultdict(lambda: dict(left= None, right=None))

        self.left, self.right = None, None

        self.server = ctx.socket(zmq.REP)
        self.server.bind('tcp://*:{}'.format(BROKER_PORT))

    def serve(self):
        def listen(sock):
            while True:
                wire = sock.recv()
                robus_msg = sock.recv()

                msg = list(map(hex, robus_msg)) if wire == b'broadcast' else robus_msg

                if wire == b'broadcast':
                    for out in self.conn.values():
                        out.send(wire)
                        out.send(robus_msg)

                elif wire == b'ptp left':
                    left = self.neighbors[sock]['left']

                    if left is None:
                        continue

                    left = self.conn[left]

                    left.send(b'ptp right')
                    left.send(robus_msg)

                elif wire == b'ptp right':
                    right = self.neighbors[sock]['right']

                    if right is None:
                        continue

                    right = self.conn[right]

                    right.send(b'ptp left')
                    right.send(robus_msg)

        while True:
            req = self.server.recv()

            if req.startswith(b'plug'):
                side = req[len('plug '):]

                sock = ctx.socket(zmq.PAIR)
                sock.bind('tcp://*:{}'.format(self.last_port))

                t = Thread(target=lambda: listen(sock))
                t.daemon = True
                t.start()

                if side == b'left':
                    left, right = None, self.left
                    self.neighbors[sock] = {'left': left, 'right': right}
                    self.neighbors[self.conn[right]]['left'] = self.last_port
                    self.left = self.last_port

                elif side == b'right':
                    left, right = self.right, None
                    self.neighbors[sock] = {'left': left, 'right': right}
                    self.neighbors[self.conn[left]]['right'] = self.last_port
                    self.right = self.last_port

                else:
                    left, right = None, None
                    self.left, self.right = self.last_port, self.last_port

                self.conn[self.last_port] = sock

                rep = '{}'.format(self.last_port)
                self.last_port += 1

            self.server.send_string(rep)

            print('Broker > New module plugged:')
            for d, v in self.neighbors.items():
                p = next((p for p, s in self.conn.items() if s == d), None)
                print('Broker > {} <--> {} <--> {}'.format(v['left'], p, v['right']))
            print()

if __name__ == '__main__':
    broker = Broker()
    broker.serve()
