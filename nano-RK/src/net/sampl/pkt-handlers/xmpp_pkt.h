#ifndef _XMPP_PKT_H
#define _XMPP_PKT_H

#include <sampl.h>
#define XMPP_BINARY_MASK		0x10
#define XMPP_PUB_SUB_MASK		0x20
// Set to 1 if you wish to send a full source jid instead of using
// the MAC address of the node to convert to a local jid
#define XMPP_EXPLICIT_SRC_JID_MASK	0x40

#define XMPP_CTRL_TIMEOUT_BYTE_MSB	0
#define XMPP_CTRL_TIMEOUT_BYTE_LSB	1
#define XMPP_PWD_SIZE_OFFSET		2
#define XMPP_SRC_JID_SIZE_OFFSET	3
#define XMPP_DST_JID_SIZE_OFFSET	4
#define XMPP_MSG_SIZE_OFFSET		5
#define XMPP_VAR_START_OFFSET		6

typedef struct xmpp_pkt 
{
	uint16_t ctrl_timeout;	// byte 0,1
	uint8_t passwd_size;    // byte 2 
	uint8_t src_jid_size;	// byte 3 
	uint8_t dst_jid_size;	// byte 4 
	uint8_t msg_size;	// byte 5
	uint16_t timeout;
	uint8_t binary_flag;
	uint8_t pub_sub_flag;
	uint8_t explicit_src_jid_flag;
	char *passwd;		
	char *src_jid;
	// This holds the destination of a direct message or an event node name
	char *dst_jid;
	char *msg;
} XMPP_PKT_T;



int8_t xmpp_generate( SAMPL_UPSTREAM_PKT_T *pkt,SAMPL_DOWNSTREAM_PKT_T *ds_pkt);
uint8_t xmpp_pkt_pack( XMPP_PKT_T *p, uint8_t *buf, uint8_t index );
void  xmpp_pkt_unpack( XMPP_PKT_T *p, uint8_t *buf, uint8_t index );

#endif
