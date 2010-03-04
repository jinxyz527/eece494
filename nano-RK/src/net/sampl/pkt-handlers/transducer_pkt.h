#ifndef _TRANSDUCER_PKT_H
#define _TRANSDUCER_PKT_H

#include <sampl.h>


#define TRANSDUCER_PKT_HEADER_SIZE	2
#define TRANSDUCER_MSG_HEADER_SIZE	3

#define GLOBAL_DEBUG_MASK	0x01	

typedef struct transducer_msg
{
	uint8_t mac_addr;   
	uint8_t type;   
	uint8_t len; 
	uint8_t *payload;
} TRANSDUCER_MSG_T;

typedef struct transducer_pkt 
{
	uint8_t checksum;   // Byte 0 
	uint8_t num_msgs;  // Byte 1
	uint8_t *msgs_payload;   
} TRANSDUCER_PKT_T;




int8_t transducer_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out);
int8_t transducer_p2p_generate(SAMPL_PEER_2_PEER_PKT_T *p2p_pkt_in, SAMPL_PEER_2_PEER_PKT_T *p2p_pkt_out);
int8_t transducer_generate(SAMPL_UPSTREAM_PKT_T *pkt, SAMPL_DOWNSTREAM_PKT_T *ds_pkt);
// This function returns a computed checksum to compare against the normal checksum
uint8_t transducer_pkt_unpack( TRANSDUCER_PKT_T *p, uint8_t *payload_buf);
uint8_t transducer_pkt_pack( TRANSDUCER_PKT_T *p, uint8_t *payload_buf);

// NOTE: for upstream packets, build a dummy transducer_pkt and set msg pointer to payload and set
//       num_msgs to upstream packet num_msgs....
uint8_t transducer_msg_add( TRANSDUCER_PKT_T *p, TRANSDUCER_MSG_T *m );
uint8_t transducer_msg_get( TRANSDUCER_PKT_T *p,TRANSDUCER_MSG_T *m, uint8_t index );

#endif
