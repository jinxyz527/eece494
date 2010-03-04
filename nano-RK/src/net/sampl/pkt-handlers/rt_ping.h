#ifndef _RT_PING_PKT_H
#define _RT_PING_PKT_H

#include <sampl.h>


#define RT_PING_PKT_SIZE	2

typedef struct rt_ping_pkt 
{
	uint8_t mac_addr;   // Byte 0
	int8_t inbound_rssi;   // Byte 0
} RT_PING_PKT_T;



uint8_t rt_ping_pkt_add( RT_PING_PKT_T *p, uint8_t *buf, uint8_t index );
uint8_t rt_ping_pkt_get( RT_PING_PKT_T *p, uint8_t *buf, uint8_t index );
int8_t rt_ping_p2p_generate(SAMPL_PEER_2_PEER_PKT_T *pkt);

#endif
