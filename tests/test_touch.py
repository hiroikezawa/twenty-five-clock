import ctypes as C
from pathlib import Path
lib=C.CDLL(str(Path('build-host/clock_test.dll').resolve()))
lib.test_gesture.argtypes=[C.c_int,C.c_int,C.c_int,C.c_int64]
def run(sequence):
    lib.test_gesture_reset()
    for down,x,y,ms,expected in sequence:
        got=lib.test_gesture(down,x,y,ms*1000)
        assert got==expected,(down,x,y,ms,expected,got)
# NONE=0 TAP=1 DOUBLE=2 LEFT=3 RIGHT=4 LONG=5.
run([(1,230,230,0,0),(0,0,0,100,0),(0,0,0,450,1),(0,0,0,900,0)])
run([(1,230,230,0,0),(0,0,0,100,0),(1,232,231,200,0),(0,0,0,260,2),(0,0,0,900,0)])
run([(1,230,230,0,0),(1,231,232,710,5),(1,231,232,800,0),(0,0,0,900,0),(0,0,0,1300,0)])
run([(1,300,200,0,0),(1,140,208,100,0),(0,0,0,200,3),(0,0,0,800,0)])
run([(1,140,200,0,0),(1,300,208,100,0),(0,0,0,200,4)])
run([(1,200,140,0,0),(1,208,300,100,0),(0,0,0,200,0),(0,0,0,800,0)])
run([(1,230,230,0,0),(1,280,230,300,0),(1,230,230,750,0),(0,0,0,800,0)])
print('PASS: tap, double tap suppression, long hold suppression, left/right swipe, vertical drag, moved hold')
