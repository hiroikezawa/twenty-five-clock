import ctypes as C
import math
from pathlib import Path
lib=C.CDLL(str(Path('build-host/clock_test.dll').resolve()))
lib.test_rotated_restore.argtypes=[C.c_int,C.c_int,C.c_int,C.c_float]
lib.test_rotate_point.argtypes=[C.c_float,C.c_float,C.c_float]
frames=0
for degrees in [37,90,180,270,-143]:
    angle=math.radians(degrees)
    assert lib.test_rotate_point(angle,233,100)
    for face in range(lib.face_count()):
        for dx,dy in [(0,0),(-3,3),(3,-3)]:
            assert lib.test_rotated_restore(face,dx,dy,angle)==0,(degrees,face,dx,dy)
            frames+=120
print(f'PASS: {frames} rotated frames, 5 angles, 25 faces, 3 offsets; no dirty-cache trails')
