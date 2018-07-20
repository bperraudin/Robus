from subprocess import Popen
from time import sleep


if __name__ == '__main__':
    import sys
    N = int(sys.argv[1])

    from string import ascii_lowercase
    from random import choice

    broker = Popen(['python', 'broker.py'])
    sleep(1)

    gate = Popen(['python', 'module.py', 'gate', 'none'])
    sleep(1)

    modules = []
    for c in ascii_lowercase[:N]:
        side = choice(('left', 'right'))
        mod = Popen(['python', 'module.py', c, side])
        modules.append(mod)

    sleep(60)

    for mod in modules:
        mod.terminate()

    broker.terminate()
