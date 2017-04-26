import os

from time import sleep
from random import randint
from subprocess import Popen, DEVNULL

root = os.path.join(os.path.abspath(os.path.dirname(__file__)), '..', '..', '..')
broker_path = os.path.join(root, 'lib/robus/hal/mockup/broker.py')
module_path = os.path.join(root, '.pioenvs/native/program')


if __name__ == '__main__':
    broker = Popen(['python', broker_path])
    sleep(1)

    M = Popen([module_path, 'M', str(randint(1, 5)), 'none'])
    sleep(1)

    N = Popen([module_path, 'N', str(randint(1, 5)), 'right'])
    sleep(1)
    O = Popen([module_path, 'O', str(randint(1, 5)), 'right'])
    sleep(1)
    P = Popen([module_path, 'P', str(randint(1, 5)), 'right'])
    sleep(1)
    Q = Popen([module_path, 'Q', str(randint(1, 5)), 'left'])
    sleep(1)

    modules = [M, N, O, P, Q]

    for m in modules:
        m.wait()

    broker.terminate()
