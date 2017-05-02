import os

from time import sleep
from random import randint
from subprocess import Popen, DEVNULL

root = os.path.join(os.path.abspath(os.path.dirname(__file__)), '..', '..', '..', '..')
broker_path = os.path.join(root, 'lib/robus/hal/mockup/broker.py')
module_path = os.path.join(root, '.pioenvs/mockup_stress/program')


if __name__ == '__main__':
    import sys
    N = int(sys.argv[1])

    from string import ascii_lowercase
    from random import choice

    broker = Popen(['python', broker_path])
    sleep(1)

    gate = Popen([module_path, 'gate', 'none'])
    sleep(1)

    modules = []
    for c in ascii_lowercase[:N]:
        side = choice(('left', 'right'))
        mod = Popen([module_path, c, side])
        modules.append(mod)

    sleep(60)

    for mod in modules:
        mod.terminate()

    broker.terminate()
