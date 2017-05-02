import zmq
import time

from threading import Thread

BROKER_PORT = 9001

ctx = zmq.Context()


class Module(object):
    def __init__(self, alias, plug):
        self.alias = alias

        self.server = ctx.socket(zmq.REQ)
        self.server.connect('tcp://localhost:{}'.format(BROKER_PORT))

        self.server.send_string('register {}'.format(plug))
        msg = self.server.recv().decode('utf-8')
        send, recv = map(int, msg.split(' '))

        self.recv_sock = ctx.socket(zmq.PAIR)
        self.recv_sock.connect('tcp://localhost:{}'.format(recv))

        self.send_sock = ctx.socket(zmq.PAIR)
        self.send_sock.connect('tcp://localhost:{}'.format(send))

        self.poll_t = Thread(target=self.poll)
        self.poll_t.daemon = True
        self.poll_t.start()

    def send(self, wire, msg):
        self.send_sock.send_string(wire)
        self.send_sock.send(msg)

    def poll(self, *args, **kwargs):
        self.i = 0
        self.start = time.time()

        while True:
            wire = self.recv_sock.recv_string()
            msg = self.recv_sock.recv()
            self.i += 1


if __name__ == '__main__':
    import sys
    alias = sys.argv[1]
    plug = sys.argv[2]

    module = Module(alias, plug)

    def say_goodbye(signum, frame):
        print('Terminating with {} msg per sec received'.format(module.i / (time.time() - module.start)))
        sys.exit(0)

    import signal
    signal.signal(signal.SIGTERM, say_goodbye)

    while True:
        module.send('broadcast', b'hello')
        module.send('ptp left', b'et a gauche')
        module.send('ptp right', b'et a droite')
