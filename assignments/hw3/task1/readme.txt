Task 1
======

In order to measure the signal strength and packet loss rate at various locations, two motes are required. 
Both are capable of transmitting and receiving packets.

In order to accurately measure packet loss, there needed to be a mechanism to determine the number of lost packets. As such, each message that a mote transmits is prepended with a sequence number that is incremented each time a message is sent. With the assumption that the packets arrive in-order, if the receiver receives a packet in which the sequence number is not as expected, it can then be determined how many packets were loss in transmission.

Although the packet loss calculation may have been performed on the node themselves, it was decided that in order to ensure that no packets are lost due to buffer issues or the calculation task pre-empting the receiving task, the calculation of the packet loss is instead done in a Python script (task1.py).
The Python script accepts the output of the gateway's print statements and processes them calculating packet loss with the formula:

packet loss rate = (num packets lost)/(num packets total) * 100

