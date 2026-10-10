import ctypes as C
from pathlib import Path
lib=C.CDLL(str(Path('build-host/clock_test.dll').resolve()))
lib.face_name.restype=C.c_char_p
assert lib.face_count()==25
names=[lib.face_name(i) for i in range(25)]
assert len(set(names))==25
for i,name in enumerate(names):
    assert lib.test_twelve_hour_dynamic(i),name
print('PASS: 25 unique faces; every dynamic clock renders identical hands at 10:09 and 22:09')
