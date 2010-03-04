#ifndef _ACK_PKT_H
#define _TIME_PKT_H

#include <../include/sampl.h>


#define TIME_PKT_SIZE	5

typedef struct time_pkt 
{
	uint8_t mac_addr;   // Byte 0
	uint8_t time[4];   // Byte 1-4
} TIME_PKT_T;



int8_t time_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out);
int8_t time_generate(SAMPL_UPSTREAM_PKT_T *pkt);
uint8_t time_pkt_add( TIME_PKT_T *p, uint8_t *buf, uint8_t index );
uint8_t time_pkt_get( TIME_PKT_T *p, uint8_t *buf, uint8_t index );

#endif
