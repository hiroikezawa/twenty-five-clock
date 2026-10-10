import ctypes as C
from pathlib import Path
lib=C.CDLL(str(Path('build-host/clock_test.dll').resolve()))
lib.test_dirty_restore.argtypes=[C.c_int,C.c_int,C.c_int]
for face in range(lib.face_count()):
    for dx,dy in [(0,0),(-3,-3),(3,3)]:
        result=lib.test_dirty_restore(face,dx,dy)
        assert result==0,(face,dx,dy,result)
print('PASS: 9000 frames, all 25 faces, 3 offsets: dirty restoration equals fresh full rendering pixel for pixel')
