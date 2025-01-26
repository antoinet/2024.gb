from pyboy import PyBoy
from typing import Iterable, Any
from progress.bar import Bar
import time

class Emu:
    def __init__(self, rom, turbo=False):
        self.emu = PyBoy(rom)
        if turbo:
            self.emu.set_emulation_speed(0) # no speed limit
        self.current_digit = '0'
        self.shift_reg = ['0'] * 4
    
    def shift(self, n):
        self.shift_reg.pop(0)
        self.shift_reg.append(n)
    
    def run(self):
        # boot up
        for i in range(100):
            self.emu.tick()

        start_time = time.time()
        min_entropy = (100.0, "")
        dbseq = de_bruijn(10, 4)[3:]
        bar = Bar("Processing", max=len(dbseq))
        for n in dbseq:
            self.shift(n)
            self.move_to(n)
            self.button("a")
            self.button("start")
            self.tick(20, True)
            pin = ''.join(self.shift_reg)
            entropy = self.emu.screen.image.entropy()
            if entropy < min_entropy[0]:
                min_entropy = (entropy, pin)
            self.button("start")
            self.tick(20, False)
            bar.next()

        bar.finish()
        print(f"min_entropy: {min_entropy[0]} for {min_entropy[1]}")
        end_time = time.time()
        print(f"Execution time: {end_time-start_time:.4f} seconds")

        for c in min_entropy[1]:
            self.move_to(c)
            self.button("a")
        self.button("start")

        while self.tick():
            pass

        self.emu.stop()
    
    def button(self, value, ticks=1):
        self.emu.button(value, ticks)
        self.emu.tick(ticks+1, False)
    
    def tick(self, count=1, render=True):
        return self.emu.tick(count, render)
    
    def screenshot(self, name):
        image = self.emu.screen.image
        image.save(name)

    def move_to(self, target_digit):
        pinpad = {
            '7': (0, 3), '8': (1, 3), '9': (2, 3),
            '4': (0, 2), '5': (1, 2), '6': (2, 2),
            '1': (0, 1), '2': (1, 1), '3': (2, 1),
            '0': (1, 0)
        }

        source = pinpad[self.current_digit]
        target = pinpad[target_digit]
        diff = (target[0] - source[0], target[1] - source[1])

        path = []
        if diff[1] > 0:
            path.extend(['up']*abs(diff[1]))
        if diff[0] < 0:
            path.extend(['left']*abs(diff[0]))
        if diff[0] > 0:
            path.extend(['right']*abs(diff[0]))
        if diff[1] < 0:
            path.extend(['down']*abs(diff[1]))
        self.current_digit = target_digit

        for move in path:
            self.button(move)


def de_bruijn(k: Iterable[str] | int, n: int) -> str:
    """de Bruijn sequence for alphabet k
    and subsequences of length n.
    """
    # Two kinds of alphabet input: an integer expands
    # to a list of integers as the alphabet..
    if isinstance(k, int):
        alphabet = list(map(str, range(k)))
    else:
        # While any sort of list becomes used as it is
        alphabet = k
        k = len(k)

    a = [0] * k * n
    sequence = []

    def db(t, p):
        if t > n:
            if n % p == 0:
                sequence.extend(a[1 : p + 1])
        else:
            a[t] = a[t - p]
            db(t + 1, p)
            for j in range(a[t - p] + 1, k):
                a[t] = j
                db(t + 1, t)

    db(1, 1)
    return "".join(alphabet[i] for i in sequence)

if __name__ == "__main__":
    emu = Emu("2024.gb", True)
    emu.run()
