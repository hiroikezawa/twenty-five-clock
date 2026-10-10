"""Set the clock's system time and battery-backed RTC from this PC over USB."""
import argparse
import time
import serial

p = argparse.ArgumentParser()
p.add_argument('--port', default='COM3')
a = p.parse_args()
with serial.Serial(a.port, 115200, timeout=.2) as s:
    s.rts = False
    s.dtr = False
    s.reset_input_buffer()
    deadline = time.monotonic() + 15
    received = b''
    next_send = 0
    while time.monotonic() < deadline:
        if time.monotonic() >= next_send:
            epoch = int(time.time())
            s.write(f'TIME {epoch}\n'.encode('ascii'))
            next_send = time.monotonic() + 2
        received += s.read(4096)
        if f'TIME OK {epoch} RTC '.encode() in received and b'\n' in received.split(f'TIME OK {epoch}'.encode())[-1]:
            for line in received.decode(errors='replace').splitlines():
                if f'TIME OK {epoch} RTC ' in line:
                    print(line)
                    if 'RTC ESP_OK' not in line:
                        raise SystemExit('System time updated, but RTC write failed')
            break
    else:
        raise SystemExit('No TIME OK received; close serial monitor and check firmware/port')
