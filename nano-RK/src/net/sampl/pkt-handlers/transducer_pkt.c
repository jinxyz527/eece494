#include <globals.h>
#include <sampl.h>
#include <control_pkt.h>
#include <ack_pkt.h>
#include <transducer_pkt.h>
#include <stdio.h>

#ifdef NANORK
#include <sampl_tasks.h>
#include <transducer_handler.h>
#include <nrk.h>
#include <nrk_error.h>

static uint8_t tran_last_seq_num;
static  TRANSDUCER_PKT_T p1, p2, p2p_tran_pkt_out, p2p_tran_pkt_in;
static  TRANSDUCER_MSG_T m1, m2, p2p_tran_msg_in, p2p_tran_msg_out;

int8_t transducer_generate (SAMPL_UPSTREAM_PKT_T * pkt,
                            SAMPL_DOWNSTREAM_PKT_T * ds_pkt)
{
  ACK_PKT_T a;
  uint8_t i,total_len;
  uint8_t status;
  uint8_t pkt_generated,t,v;


  if(ds_pkt->seq_num==tran_last_seq_num) return NRK_ERROR;
  tran_last_seq_num=ds_pkt->seq_num;


  pkt_generated = 0;
  pkt->payload_len = 0;
  pkt->num_msgs = 0;
  status = 0;
  t = transducer_pkt_unpack (&p1, ds_pkt->payload);
/*
  for(i=0;i <ds_pkt->payload_len; i++ )
  	printf( "%d ",ds_pkt->payload[i] );

  nrk_kprintf( PSTR(")\n" ));
*/
  total_len=0;

  p2.num_msgs=0;
  p2.checksum=0;
  p2.msgs_payload=&(pkt->payload[TRANSDUCER_PKT_HEADER_SIZE]);

  if (t) {
    for (i = 0; i < p1.num_msgs; i++) {
      v=transducer_msg_get(&p1, &m1, i);
      if (v==1 && (m1.mac_addr == my_mac || m1.mac_addr == 255)) {
        pkt_generated = 1;

	m2.len=0;
	m2.mac_addr = my_mac;
	m2.type=0;
	m2.payload =  &(pkt->payload[total_len+TRANSDUCER_MSG_HEADER_SIZE+TRANSDUCER_PKT_HEADER_SIZE]);

        // Call application transducer handler
        status = transducer_handler (&m1, &m2);

	if(status!=0)
	{
		total_len+=(m2.len+TRANSDUCER_MSG_HEADER_SIZE+TRANSDUCER_PKT_HEADER_SIZE);
		if(total_len>=(MAX_PKT_PAYLOAD-US_PAYLOAD_START-TRANSDUCER_PKT_HEADER_SIZE))
		{
          		pkt->error_code |= OVERFLOW_ERROR_MASK;
	  		total_len=MAX_PKT_PAYLOAD-US_PAYLOAD_START-TRANSDUCER_PKT_HEADER_SIZE;	
		}
		if (status == 0)
          		pkt->error_code |= INVALID_DATA_ERROR_MASK;
	
	
        	// Copy header elements into packet
		transducer_msg_add(&p2, &m2);

		t=transducer_pkt_pack( &p2, pkt->payload );

        	// Update new length of packet
        	pkt->payload_len =  t;
		// Multiple msg can be in the packet, but there is only 1 packet
        	pkt->num_msgs=1;  
        	pkt->pkt_type = TRANSDUCER_PKT;
	}
      }
    }
  }
  else {
    pkt_generated = 1;
    // build NCK reply packet
    a.mac_addr = my_mac;
    pkt->payload_len = ack_pkt_add (&a, pkt->payload, 0);
    pkt->num_msgs = 1;
    pkt->pkt_type = ACK_PKT;
    pkt->error_code |= NCK_ERROR_MASK;        // set error type for NCK
  }

  if (pkt_generated == 0) {
    pkt->pkt_type = EMPTY_PKT;
    pkt->num_msgs = 0;
    pkt->payload_len = 0;
  }


  return NRK_OK;
}

int8_t transducer_p2p_generate (SAMPL_PEER_2_PEER_PKT_T * p2p_pkt_in,
                            SAMPL_PEER_2_PEER_PKT_T* p2p_pkt_out)
{
  ACK_PKT_T a;
  uint8_t i,total_len;
  uint8_t status,reply;
  uint8_t pkt_generated,t,v;

  if(p2p_pkt_in->seq_num==tran_last_seq_num) return NRK_ERROR;
  tran_last_seq_num=p2p_pkt_in->seq_num;
  pkt_generated = 0;
  //p2p_pkt_out->payload_len = 0;
  status = 0;
  t = transducer_pkt_unpack (&p2p_tran_pkt_in, p2p_pkt_in->payload);

  if(t==0) return NRK_ERROR;

  total_len=0;
  reply=0;

  p2p_tran_pkt_out.num_msgs=0;
  p2p_tran_pkt_out.checksum=0;
  p2p_tran_pkt_out.msgs_payload=&(p2p_pkt_out->payload[TRANSDUCER_PKT_HEADER_SIZE]);

  
    for (i = 0; i < p2p_tran_pkt_in.num_msgs; i++) {
      v=transducer_msg_get(&p2p_tran_pkt_in, &p2p_tran_msg_in, i);
      if (v==1 && (p2p_tran_msg_in.mac_addr == my_mac || p2p_tran_msg_in.mac_addr == 255)) {
        pkt_generated = 1;

	p2p_tran_msg_out.len=0;
	p2p_tran_msg_out.mac_addr = my_mac;
	p2p_tran_msg_out.type=0;
	p2p_tran_msg_out.payload =  &(p2p_pkt_out->payload[total_len+TRANSDUCER_MSG_HEADER_SIZE+TRANSDUCER_PKT_HEADER_SIZE]);

        // Call application transducer handler
        status = transducer_handler (&p2p_tran_msg_in, &p2p_tran_msg_out);

	if(status!=0)
	{
		total_len+=(p2p_tran_msg_out.len+TRANSDUCER_MSG_HEADER_SIZE+TRANSDUCER_PKT_HEADER_SIZE);
		if(total_len>=(MAX_PKT_PAYLOAD-US_PAYLOAD_START-TRANSDUCER_PKT_HEADER_SIZE))
		{
          		//p2p_pkt_out->error_code |= OVERFLOW_ERROR_MASK;
	  		total_len=MAX_PKT_PAYLOAD-US_PAYLOAD_START-TRANSDUCER_PKT_HEADER_SIZE;	
		}
		//if (status == 0)
          	//	p2p_pkt_out->error_code |= INVALID_DATA_ERROR_MASK;
	
	
        	// Copy header elements into packet
		transducer_msg_add(&p2p_tran_pkt_out, &p2p_tran_msg_out);

		t=transducer_pkt_pack( &p2p_tran_pkt_out, p2p_pkt_out->payload );

        	// Update new length of packet
        	p2p_pkt_out->payload_len =  t;
		// Multiple msg can be in the packet, but there is only 1 packet
        	p2p_pkt_out->pkt_type = TRANSDUCER_PKT;
		reply=1;
	}
      }
    }
/*  else {
    pkt_generated = 1;
    // build NCK reply packet
    a.mac_addr = my_mac;
    p2p_pkt_out->payload_len = ack_pkt_add (&a, pkt->payload, 0);
    p2p_pkt_out->num_msgs = 1;
    p2p_pkt_out->pkt_type = ACK_PKT;
    p2p_pkt_out->error_code |= NCK_ERROR_MASK;        // set error type for NCK
  }


  if (pkt_generated == 0) {
    pkt->pkt_type = EMPTY_PKT;
    pkt->num_msgs = 0;
    pkt->payload_len = 0;
  }
*/

if(reply)  return NRK_OK;
else return NRK_ERROR;

}



int8_t transducer_aggregate (SAMPL_UPSTREAM_PKT_T * in,
                             SAMPL_UPSTREAM_PKT_T * out)
{
  uint8_t len, i, k,l, dup;

  if (in->num_msgs > MAX_PKT_PAYLOAD / 4)
    return NRK_ERROR;

  //printf( "%d a[%d %d]\n",my_mac, in->num_msgs, out->num_msgs );

  k = transducer_pkt_unpack(&p1, in->payload );
  k = transducer_pkt_unpack(&p2, out->payload );
  //p1.msgs_payload=in->payload;
  //p2.msgs_payload=out->payload;
  //p2.num_msgs=out->num_msgs;

  for (i = 0; i < p1.num_msgs; i++) {
    dup = 0;
    // get next ping packet to compare against current outgoing list
    transducer_msg_get (&p1,&m1, i);
    for (k = 0; ((k < p2.num_msgs) && (dup==0)); k++) {
      // get packet from outgoing list and compare against incomming packet
      transducer_msg_get (&p2,&m2, k);
      if (m1.mac_addr == m2.mac_addr && m1.type == m2.type && m1.len == m2.len)
        dup = 1;
	// Go through each byte in the pkt
	for(l=0; l<m1.len; l++ ) if(m1.payload[l]!=m2.payload[l]) dup=0;	
    }
    if (dup == 0) {
      // if packet is unique, add to outgoing packet
      //out->payload_len=transducer_reply_pkt_add( &p1, out->payload, out->num_msgs );
      len = transducer_msg_add (&p2, &m1);
     if(len>0) {
        out->payload_len = transducer_pkt_pack(&p2, out->payload );
	//printf( "[%d]: %d %d %d\r\n",my_mac, len,out->payload_len, p2.num_msgs );
	}
      else out->error_code |= OVERFLOW_ERROR_MASK;
    }
  }

  return NRK_OK;
}






#else
#define my_mac 0
#define MAX_PKT_PAYLOAD	113


#endif


// This function sets up the values in *p and checks the checksum
uint8_t transducer_pkt_unpack( TRANSDUCER_PKT_T *p, uint8_t *payload_buf)
{
uint8_t checksum,i,offset;
TRANSDUCER_MSG_T m;

if(p==NULL) return 0;

p->checksum=payload_buf[0];
p->num_msgs=payload_buf[1];
p->msgs_payload=payload_buf+2;

offset=TRANSDUCER_PKT_HEADER_SIZE;;
checksum=0;
// compute packet length
for(i=0; i<p->num_msgs; i++ )
{
	transducer_msg_get(p,&m, i);
	offset+=TRANSDUCER_MSG_HEADER_SIZE;
	offset+=m.len;
}
// compute packet checksum
checksum=0;
for(i=1; i<offset; i++ )
	checksum+=payload_buf[i];
if(checksum==payload_buf[0] ) return 1;
return 0;
}


// This function checksums the packet
uint8_t transducer_pkt_pack( TRANSDUCER_PKT_T *p, uint8_t *payload_buf)
{
uint8_t i,j,offset,checksum;
TRANSDUCER_MSG_T m3;

payload_buf[0]=p->checksum;
payload_buf[1]=p->num_msgs;
offset=2;

for(i=0; i<p->num_msgs; i++ )
{
transducer_msg_get(p,&m3, i);
payload_buf[offset]=m3.mac_addr; offset++;
payload_buf[offset]=m3.type; offset++;
payload_buf[offset]=m3.len; offset++;
  for(j=0; j<m3.len; j++ )
    {
	payload_buf[offset]=m3.payload[j]; 
	offset++;
    }
}

// compute packet checksum
checksum=0;
for(i=1; i<offset; i++ )
	checksum+=payload_buf[i];
payload_buf[0]=checksum;
// write checksum back into orignal packet
p->checksum=checksum;
return offset;
}

// NOTE: for upstream packets, build a dummy transducer_pkt and set msg pointer to payload and set
//       num_msgs to upstream packet num_msgs....
uint8_t transducer_msg_add( TRANSDUCER_PKT_T *p, TRANSDUCER_MSG_T *m )
{
uint8_t offset,i;
offset=0;

// find offset for end of current transducer pkt
for(i=0; i<p->num_msgs; i++ )
	offset+=(p->msgs_payload[offset+2]+TRANSDUCER_MSG_HEADER_SIZE);

// cap the packet size...  Don't overflow
if(offset>(MAX_PKT_PAYLOAD-US_PAYLOAD_START-TRANSDUCER_PKT_HEADER_SIZE)) return 0;

// Setup packet specific parameters
// checksum is cleared to remind us to call transducer_pkt_pack() before sending
p->checksum=0;
p->num_msgs++;


// copy over the data
p->msgs_payload[offset]=m->mac_addr;
p->msgs_payload[offset+1]=m->type;
p->msgs_payload[offset+2]=m->len;
offset+=TRANSDUCER_MSG_HEADER_SIZE;

for(i=0; i<m->len; i++ )
{
  p->msgs_payload[offset]=m->payload[i];
  offset++;
}

// Return the length of the message payload alone
return offset;
}



uint8_t transducer_msg_get( TRANSDUCER_PKT_T *p,TRANSDUCER_MSG_T *m, uint8_t index )
{
uint8_t i,offset;
if(index > p->num_msgs) return 0;

offset=0;
for(i=0; i<index; i++ )
	offset+=(p->msgs_payload[offset+2]+TRANSDUCER_MSG_HEADER_SIZE);

m->mac_addr=p->msgs_payload[offset];	
m->type=p->msgs_payload[offset+1];	
m->len=	p->msgs_payload[offset+2];

offset+=TRANSDUCER_MSG_HEADER_SIZE;
m->payload=&(p->msgs_payload[offset]);

return 1;
}

