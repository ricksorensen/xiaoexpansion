import sys

_use_stdin_query = False
try:
    import uselect

    _use_stdin_query = True
except ImportError:
    pass


class StdinQuery:
    def __init__(self, useSelect=None):
        self.init(useSelect=useSelect)

    def deinit(self):
        if self._kpoll is not None:
            self._kpoll.unregister(sys.stdin)
        self._kpoll = None

    def init(self, useSelect=None):
        global _use_stdin_query
        self._kpoll = None
        if useSelect is None:
            useSelect = _use_stdin_query
        if useSelect:
            # print("Select Available")
            self._kpoll = uselect.poll()
            self._kpoll.register(sys.stdin, uselect.POLLIN)

    def kbin(self):
        rv = None
        if self._kpoll is not None:
            rv = sys.stdin.read(1) if self._kpoll.poll(0) else None
        else:
            rv = sys.stdin.read(1)
        return rv

    def kbLine(self, start="", echo=True):
        rv = start
        x = None
        if echo:
            print(start, end="")
        while x != "\n":
            while (x := self.kbin()) is None:
                pass
            if echo:
                print(x, end="")
            rv = rv + x
        return rv


def chkit(useSelect=True):
    n = 0
    qin = StdinQuery(useSelect)
    while True:
        if (x := qin.kbin()) is not None:
            if x == "x":
                print("\n count = " + str(n))
            elif x == "Q":
                qin.deinit()
                break
            else:
                print(qin.kbLine(x))
            n = 0
        else:
            n = n + 1


if __name__ == "__main__":
    chkit()
