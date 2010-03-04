#include <globals.h>
#include <nrk.h>
#include <nrk_error.h>
#include <sampl.h>
#include <trace.h>
#include <route_table.h>


int8_t trace_generate (SAMPL_UPSTREAM_PKT_T * pkt,
                       SAMPL_DOWNSTREAM_PKT_T * ds_pkt)
{
  TRACE_PKT_T p;
  p.mac_addr = my_mac;
  //p.parent_mac=ds_pkt->last_hop_mac;
  p.parent_mac = route_table_get (GATEWAY_MAC);
  p.ds_rssi = ds_pkt->rssi;
  p.us_rssi = 255;
  pkt->payload_len = trace_pkt_add (&p, pkt->payload, 0);
  pkt->num_msgs = 1;
  return NRK_OK;
}

int8_t trace_aggregate (SAMPL_UPSTREAM_PKT_T * in, SAMPL_UPSTREAM_PKT_T * out)
{
  uint8_t i, k, dup,t;
  TRACE_PKT_T p1, p2;

// if(in->next_hop_dst_mac!=my_mac ) nrk_kprintf( PSTR( "aggregating bad packet!\r\n" ));
  for (i = 0; i < in->num_msgs; i++) {
    dup = 0;
    // get next ping packet to compare against current outgoing list
    t=trace_pkt_get (&p1, in->payload, i);
    if(t==0) return NRK_ERROR;
    for (k = 0; k < out->num_msgs; k++) {
      // get packet from outgoing list and compare against incomming packet
      t=trace_pkt_get (&p2, out->payload, k);
      if(t==0) return NRK_ERROR;
      if (p1.mac_addr == p2.mac_addr)
        dup = 1;
    }
    if (dup == 0) {
      if (p1.parent_mac == my_mac)
        p1.us_rssi = in->rssi;
      // if packet is unique, add to outgoing packet
      t = trace_pkt_add (&p1, out->payload, out->num_msgs);
      if(t>0)
	{
	out->payload_len = t;
      	out->num_msgs++;
	} else out->error_code|=OVERFLOW_ERROR_MASK;
    }
  }
  return NRK_OK;
}


uint8_t trace_pkt_get (TRACE_PKT_T * p, uint8_t * buf, uint8_t index)
{
  if((index+1)*TRACE_PKT_SIZE > MAX_PKT_PAYLOAD) return 0;
  p->mac_addr = buf[index * TRACE_PKT_SIZE];
  p->parent_mac = buf[index * TRACE_PKT_SIZE + 1];
  p->ds_rssi = buf[index * TRACE_PKT_SIZE + 2];
  p->us_rssi = buf[index * TRACE_PKT_SIZE + 3];
  return 1;
}


uint8_t trace_pkt_add (TRACE_PKT_T * p, uint8_t * buf, uint8_t index)
{
  if ((index+1) * TRACE_PKT_SIZE > MAX_PKT_PAYLOAD) return 0;
  buf[index * TRACE_PKT_SIZE] = p->mac_addr;
  buf[index * TRACE_PKT_SIZE + 1] = p->parent_mac;
  buf[index * TRACE_PKT_SIZE + 2] = p->ds_rssi;
  buf[index * TRACE_PKT_SIZE + 3] = p->us_rssi;
  return ((index + 1) * TRACE_PKT_SIZE);
}
