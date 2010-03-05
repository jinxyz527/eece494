#!/usr/bin/python

from __future__ import division

filename = "task1_output"

rssi_dict = {}

packet_loss = 0
packet_total = 0

for line in open(filename, "r"):
    vals = line.split()

    if len(vals) <= 2:
        continue 

    rssi = int(vals[1].strip("rssi="))
    seq = int(vals[2].strip("data="))
    packet_total += 1

    try:
        # sequence ranges from [0-99]
        expected_seq += 1
        expected_seq %= 100

        num_pkt_missing = seq - expected_seq
        if num_pkt_missing >= 0:
            pass
        else:
            # roll over
            num_pkt_missing = 100 + num_pkt_missing + seq

#        print "%d %d = %d" % (seq, expected_seq, num_pkt_missing)
        packet_loss += num_pkt_missing
        expected_seq = seq
    except NameError:
        # first packet
        expected_seq = seq

    rate = (packet_loss/packet_total)*100

    try:
        rate_list = rssi_dict[rssi]
        rate_list.append(rate)
    except:
        rssi_dict[rssi] = [rate]

for rssi, packet_loss_list in rssi_dict.items():
    item_cnt = 0
    packet_loss_total = 0
    for item in packet_loss_list:
        item_cnt += 1
        packet_loss_total += item
    avg_packet_loss = packet_loss_total/item_cnt    

    print "rssi=", rssi, " avg pkt loss=", avg_packet_loss
