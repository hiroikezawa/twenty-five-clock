"""Exercise the firmware's actual compiled C implementation, not a Python copy."""
import ctypes as C
import datetime as dt
from collections import Counter, deque
from pathlib import Path
lib = C.CDLL(str(Path('build-host/clock_test.dll').resolve()))
lib.test_schedule.argtypes = [C.c_int, C.c_int, C.c_int, C.POINTER(C.c_uint8)]
def schedule(day):
    out = (C.c_uint8 * 24)()
    lib.test_schedule(day.year, day.month, day.day, out)
    return list(out)
day, end = dt.date(2000, 1, 1), dt.date(2100, 1, 1)
previous = schedule(day - dt.timedelta(days=1))
count = 0
window=deque()
usage=Counter()
while day < end:
    s = schedule(day)
    assert s == schedule(day), ('determinism', day)
    assert len(set(s)) == 24 and set(s) <= set(range(25)), ('daily unique', day)
    window.append(s);usage.update(s)
    if len(window)>25: usage.subtract(window.popleft())
    if len(window)==25: assert all(usage[i]==24 for i in range(25)), ('25-day balance',day)
    assert all(a != b for a, b in zip(s, s[1:])), ('adjacent hour', day)
    assert previous[-1] != s[0], ('midnight', day)
    assert previous != s, ('new daily order', day)
    previous = s
    day += dt.timedelta(days=1)
    count += 1
print(f'PASS: {count} days, 2000-2099, determinism/permutations/balance/hour/midnight/daily change')
print('2026-10-07:', [i+1 for i in schedule(dt.date(2026,10,7))])
