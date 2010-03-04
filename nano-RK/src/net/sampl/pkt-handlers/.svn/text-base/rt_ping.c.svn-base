#include <globals.h>
#include <sampl.h>
#include "rt_ping.h"

#ifdef NANORK
#include <nrk.h>
#include <nrk_error.h>
#else
#define my_mac 0
#define MAX_PKT_PAYLOAD	113
#endif

#define MAX_MSGS	50

// This file constains the source for the round-trip p2p ping packet. 
// Round-Trip ping returns the RSSI for both directions of the communication.

int8_t rt_ping_p2p_generate (SAMPL_PEER_2_PEER_PKT_T * pkt)
{
  RT_PING_PKT_T p;
  p.mac_addr = my_mac;
  p.inbound_rssi= pkt->rssi;
  pkt->payload_len = rt_ping_pkt_add (&p, pkt->payload, 0);
  return NRK_OK;
}


uint8_t rt_ping_pkt_get (RT_PING_PKT_T * p, uint8_t * buf, uint8_t index)
{
  if (index * RT_PING_PKT_SIZE > MAX_PKT_PAYLOAD)
    return 0;
  p->mac_addr = buf[index * RT_PING_PKT_SIZE];
  p->inbound_rssi = buf[(index * RT_PING_PKT_SIZE)+1];
  return 1;
}


uint8_t rt_ping_pkt_add (RT_PING_PKT_T * p, uint8_t * buf, uint8_t index)
{
  if ((index+1) * RT_PING_PKT_SIZE > MAX_PKT_PAYLOAD)
    return 0;
  buf[index * RT_PING_PKT_SIZE] = p->mac_addr;
  buf[(index * RT_PING_PKT_SIZE)+1] = p->inbound_rssi;
  return ((index + 1) * RT_PING_PKT_SIZE);
}
