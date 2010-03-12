Task 1
======

In order to measure the signal strength and packet loss rate at various locations, two motes are required. 
One mote was deemed the receiver (or gateway) whereas the other was the transmitter.

The transmitter has one task and that is to constantly broadcast a message.
Similarly, the receiver has one running task which listens for messages and prints the message contents. The receiver is also known as a gateway as it should be connected to a computer.

In order to accurately measure packet loss, there needed to be a mechanism to determine the number of lost packets. As such, each message that the transmitter sends is prepended with a sequence number that is incremented each time a message is sent. With the assumption that the packets arrive in-order, if the receiver receives a packet in which the sequence number is not as expected, it can then be determined how many packets were loss in transmission.

Although the packet loss calculation may have been performed on the node themselves, it was decided that in order to ensure that no packets are lost due to buffer issues or the calculation task pre-empting the receiving task, the calculation of the packet loss is instead done in a Python script (task1.py).
The Python script accepts the output of the gateway's print statements and processes them calculating packet loss with the formula:

packet loss rate = (num packets lost)/(num packets total) * 100

