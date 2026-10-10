"""Bake OFL fonts into antialiased 4-bit glyphs. No font dependency at runtime."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
chars='0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:-+./% '
sources=['BarlowCondensed-Regular.ttf','Abel-Regular.ttf','CormorantGaramond.ttf']
blob=[];entries=[];fonts=[]
for style,source in enumerate(sources):
    for scale in range(1,5):
        size=[12,23,33,43][scale-1]+(2 if style==2 else 0)
        font=ImageFont.truetype(str(Path('tools/fonts')/source),size)
        top=font.getbbox('0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ')[1]
        bottom=font.getbbox('0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ')[3]
        base=len(entries)
        for ch in chars:
            x,y,r,b=font.getbbox(ch);w,h=r-x,b-y
            im=Image.new('L',(max(w,1),max(h,1)))
            ImageDraw.Draw(im).text((-x,-y),ch,font=font,fill=255)
            offset=len(blob);a=[(v+8)//17 for v in im.getdata()] if w*h else []
            if len(a)%2:a.append(0)
            blob.extend((a[i]<<4)|a[i+1] for i in range(0,len(a),2))
            entries.append((offset,w,h,x,y-top,round(font.getlength(ch))))
        fonts.append((base,bottom-top))
text='/* Generated from OFL fonts. Licenses: tools/fonts directory. */\n#include "font_data.h"\n'
text+='const unsigned char font_alpha[]={\n'+',\n'.join(','.join(map(str,blob[i:i+32])) for i in range(0,len(blob),32))+'\n};\n'
text+='const font_glyph_t font_glyphs[]={\n'+',\n'.join('{'+','.join(map(str,e))+'}' for e in entries)+'\n};\n'
text+='const font_record_t font_records[]={'+','.join('{'+','.join(map(str,e))+'}' for e in fonts)+'};\n'
text+='const char font_chars[]="'+chars+'";\n'
Path('main/display/font_data.c').write_text(text)
print(f'Generated {len(blob)} alpha bytes / {len(entries)} glyphs')
