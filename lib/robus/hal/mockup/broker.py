import zmq

from threading import Thread, Lock
from collections import defaultdict


BROKER_PORT = 9001

ctx = zmq.Context()


class Broker(object):
    def __init__(self):
        self.broker = ctx.socket(zmq.REP)
        self.broker.bind('tcp://*:{}'.format(BROKER_PORT))

        self.lp = BROKER_PORT + 1

        self.conn = {}
        self.new_conn_lock = Lock()
        self.conn_lock = defaultdict(lambda: Lock())

        self.neighbors = defaultdict(lambda: {'left': None,
                                              'right': None})
        self.left, self.right = None, None

        self.running = True

    def send(self, id, wire, msg):
        with self.conn_lock[id]:
            sock = self.conn[id][1]

            sock.send_string(wire)
            sock.send(msg)

    def broadcast(self, msg):
        with self.new_conn_lock:
            for id in self.conn.keys():
                self.send(id, 'broadcast', msg)

    def serve(self):
        def listener(my_id, sock):
            while True:
                wire = sock.recv_string()
                msg = sock.recv()

                if wire == 'broadcast':
                    self.broadcast(msg)

                elif wire == 'ptp left':
                    id = self.neighbors[my_id]['left']
                    if id is not None:
                        self.send(id, 'ptp right', msg)

                elif wire == 'ptp right':
                    id = self.neighbors[my_id]['right']
                    if id is not None:
                        self.send(id, 'ptp left', msg)

        while self.running:
            req = self.broker.recv()
            req = req.decode('utf-8')

            if req.startswith('register'):
                plug = req.split(' ')[1]

                rep = '{} {}'.format(self.lp,
                                     self.lp + 1)
                self.broker.send_string(rep)

                recv_sock = ctx.socket(zmq.PAIR)
                recv_sock.bind('tcp://*:{}'.format(self.lp))

                send_sock = ctx.socket(zmq.PAIR)
                send_sock.bind('tcp://*:{}'.format(self.lp + 1))

                with self.new_conn_lock:
                    self.conn[self.lp] = (recv_sock,
                                          send_sock)

                if plug == 'none':
                    self.left, self.right = self.lp, self.lp

                elif plug == 'left':
                    self.neighbors[self.lp]['right'] = self.left
                    self.neighbors[self.left]['left'] = self.lp
                    self.left = self.lp

                elif plug == 'right':
                    self.neighbors[self.lp]['left'] = self.right
                    self.neighbors[self.right]['right'] = self.lp
                    self.right = self.lp

                t = Thread(target=lambda: listener(self.lp, recv_sock))
                t.daemon = True
                t.start()

                self.print_topo()

                self.lp += 2

    def print_topo(self):
        def id4mod(mod):
            from string import ascii_lowercase
            id = (mod - 9000) // 2

            if id == 1:
                return 'gate'
            else:
                return ascii_lowercase[id - 2]

        topo = 'Topology: '
        mod = self.left

        while mod is not None:
            if self.left == self.right:
                topo += '{}'.format(id4mod(mod))
                break
            topo += '{} <--> '.format(id4mod(mod))
            mod = self.neighbors[mod]['right']

            if mod == self.right:
                topo += '{}'.format(id4mod(mod))
                break
        print(topo)


if __name__ == '__main__':
    broker = Broker()
    broker.serve()
