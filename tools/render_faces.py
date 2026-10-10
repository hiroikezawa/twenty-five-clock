"""Render actual firmware drawing code for visual QA; requires Pillow and host DLL."""
import ctypes as C
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
lib=C.CDLL(str(Path('build-host/clock_test.dll').resolve()))
lib.render_face.argtypes=[C.c_int]*6+[C.c_double,C.POINTER(C.c_uint16)]
lib.face_name.restype=C.c_char_p
names=[lib.face_name(i).decode() for i in range(lib.face_count())]
out=Path('docs/previews');out.mkdir(parents=True,exist_ok=True)
sheet=Image.new('RGB',(5*330,5*360),'#151923');d=ImageDraw.Draw(sheet)
for face,name in enumerate(names):
    pixels=(C.c_uint16*(466*466))()
    lib.render_face(face,2026,10,7,10,9,38.4,pixels)
    data=bytearray()
    for p in pixels:data.extend(((p>>11)*255//31,((p>>5)&63)*255//63,(p&31)*255//31))
    im=Image.frombytes('RGB',(466,466),bytes(data))
    mask=Image.new('L',(466,466));ImageDraw.Draw(mask).ellipse((0,0,465,465),fill=255)
    circ=Image.new('RGB',im.size,'#151923');circ.paste(im,mask=mask)
    circ.save(out/f'{face+1:02d}.png')
    x=(face%5)*330+10;y=(face//5)*360+6;sheet.paste(circ.resize((310,310)),(x,y))
    d.text((x+16,y+323),f'{face+1:02d}  {name}',fill='#dee4ed')
sheet.save(out/'all-faces.png')
print(out/'all-faces.png')
