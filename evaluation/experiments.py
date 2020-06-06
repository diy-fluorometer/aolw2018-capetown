#!/usr/bin/env python

import serial
import os
import sys
import json
import time

if len(sys.argv) < 2:
    print("usage: " + sys.argv[0] + " <experiment.json>")
    sys.exit(0)

ser = serial.Serial("/dev/ttyUSB1", 9600, timeout=0)

ex_spec = {}
# open experiment description
with open(sys.argv[1], "r") as fp:
    ex_spec = json.loads(fp.read())

fp = open(sys.argv[1] + ".out", "w")

# submit commands and read output

for cmd in ex_spec['commands']:
    print(cmd)
    if cmd.startswith("local"):
        if cmd.startswith("local wait"):
            t = int(cmd[11:])
            time.sleep(t)
        continue
    ser.write(bytes(cmd + "\n", "utf-8"));
    if cmd.startswith("read"):
        n = int(cmd[5:])
        time.sleep((n + 1) * 0.6 + 1)
    else:
        time.sleep(1) # give it some time
    data = ser.read(2000)
    fp.write(data.decode("utf-8"))

fp.close()
ser.close()