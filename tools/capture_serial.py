"""Capture a finite boot/benchmark log without leaving COM3 locked."""
import argparse,time
from pathlib import Path
import serial
from esptool.reset import HardReset
p=argparse.ArgumentParser();p.add_argument('--port',default='COM3');p.add_argument('--seconds',type=int,default=60);p.add_argument('--output',default='docs/hardware-boot.log');p.add_argument('--reset',action='store_true');a=p.parse_args()
with serial.Serial(a.port,115200,timeout=0.25) as s:
    s.rts=False
    s.dtr=False
    if a.reset:HardReset(s,uses_usb=True)()
    end=time.monotonic()+a.seconds
    with Path(a.output).open('wb') as out:
        while time.monotonic()<end:
            data=s.read(8192)
            if data:out.write(data);out.flush()
print(a.output)
