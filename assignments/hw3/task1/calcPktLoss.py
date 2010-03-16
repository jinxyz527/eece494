#!/usr/bin/python

from __future__ import division
import os

dir = "kaiser"

packet_start = 1
packet_total = 25

try:
    for file in sorted(os.listdir(dir)):
        num_packets = 0
        rssi_total = 0
        for line in open(dir + "/" + file, "r"):
            vals = line.split()

            if len(vals) <= 2:
                continue

            rssi = int(vals[1].strip("rssi="))
            seq = int(vals[2].strip("data="))

            if seq == 0:
                # ignore
                continue

            rssi_total += rssi
            num_packets += 1

        print "%s \t-- avg rssi= %0.2f \tpkt loss= %0.2f" % (file, rssi_total/num_packets, (packet_total-num_packets)/packet_total*100)
except:
    pass
