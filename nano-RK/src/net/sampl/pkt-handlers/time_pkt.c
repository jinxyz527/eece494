#include <globals.h>
#include <sampl.h>
#include <time_pkt.h>

#define MAX_MSGS	50

#ifdef NANORK
#include <nrk.h>
#include <nrk_error.h>
#else
#define my_mac 0
#endif


uint32_t time;
nrk_time_t t;

int8_t time_generate (SAMPL_UPSTREAM_PKT_T * pkt)
{
  TIME_PKT_T p;
  p.mac_addr = my_mac;
  pkt->payload_len = time_pkt_add (&p, pkt->payload, 0);
  pkt->num_msgs = 1;
  pkt->pkt_type = TIME_PKT;
  return NRK_OK;
}

int8_t time_p2p_generate (SAMPL_PEER_2_PEER_PKT_T * pkt)
{
  TIME_PKT_T p;
  p.mac_addr = my_mac;
  pkt->payload_len = time_pkt_add (&p, pkt->payload, 0);
  return NRK_OK;
}


int8_t time_aggregate (SAMPL_UPSTREAM_PKT_T * in, SAMPL_UPSTREAM_PKT_T * out)
{
  uint8_t len, i, j, k, dup,t;
  TIME_PKT_T p1, p2;

  if (in->num_msgs > MAX_MSGS || out->num_msgs > MAX_MSGS) {
#ifdef NANORK
    nrk_kprintf (PSTR ("MAX messages exceeded in aggregate!\r\n"));
#endif
    return NRK_ERROR;
  }

  for (i = 0; i < in->num_msgs; i++) {
    dup = 0;
    // get next ping packet to compare against current outgoing list
    t=time_pkt_get (&p1, in->payload, i);
    if(t==0) return NRK_ERROR;
    for (k = 0; k < out->num_msgs; k++) {
      // get packet from outgoing list and compare against incomming packet
      t=time_pkt_get (&p2, out->payload, k);
      if(t==0) return NRK_ERROR;
      if (p1.mac_addr == p2.mac_addr)
        dup = 1;
    }
    if (dup == 0) {
      // if packet is unique, add to outgoing packet
      t = time_pkt_add (&p1, out->payload, out->num_msgs);
      if(t>0)
	{
	out->payload_len = t;
      	out->num_msgs++;
	} else out->error_code|=OVERFLOW_ERROR_MASK;
    }
  }
  return NRK_OK;
}


uint8_t time_pkt_get (TIME_PKT_T * p, uint8_t * buf, uint8_t index)
{
  if((index+1)*TIME_PKT_SIZE > MAX_PKT_PAYLOAD) return 0;
  p->mac_addr = buf[index * TIME_PKT_SIZE];
  p->time[0] = buf[index * TIME_PKT_SIZE + 1];
  p->time[1] = buf[index * TIME_PKT_SIZE + 2];
  p->time[2] = buf[index * TIME_PKT_SIZE + 3];
  p->time[3] = buf[index * TIME_PKT_SIZE + 4];
  return 1;
}


uint8_t time_pkt_add (TIME_PKT_T * p, uint8_t * buf, uint8_t index)
{
  if(((index+1) * TIME_PKT_SIZE)> MAX_PKT_PAYLOAD) return 0;

if(local_epoch==0) time=0;
else
{
  nrk_time_get(&t);
  time=local_epoch+t.secs;
}

  buf[index * TIME_PKT_SIZE] = p->mac_addr;
  buf[index * TIME_PKT_SIZE+1 ] = time & 0xff;
  buf[index * TIME_PKT_SIZE+2 ] = (time>>8) & 0xff;
  buf[index * TIME_PKT_SIZE+3 ] = (time>>16) & 0xff;
  buf[index * TIME_PKT_SIZE+4 ] = (time>>24) & 0xff;
  return ((index + 1) * TIME_PKT_SIZE);
}


