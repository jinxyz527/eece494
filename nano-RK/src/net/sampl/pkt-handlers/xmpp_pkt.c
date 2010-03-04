#include <sampl.h>
#include <xmpp_pkt.h>
#include <ack_pkt.h>

#ifdef NANORK
#include <nrk.h>
#include <nrk_error.h>
#else
#define my_mac 0
#endif



int8_t xmpp_generate (SAMPL_UPSTREAM_PKT_T * pkt,
                      SAMPL_DOWNSTREAM_PKT_T * ds_pkt)
{
  // For downstream messages, generate an ACK reply to show
  // where the message has traveled 

  return ack_generate (pkt);
}


void xmpp_pkt_unpack (XMPP_PKT_T * p, uint8_t * buf, uint8_t index)
{
  //p->passwd_size = strlen (&buf[XMPP_PASSWD_START_OFFSET]) + 1;
  //p->jid_size = strlen (&buf[XMPP_PASSWD_START_OFFSET + p->passwd_size]) + 1;
  //p->msg_size =
  //  strlen (&buf[XMPP_PASSWD_START_OFFSET + p->passwd_size + p->jid_size]);
  p->passwd_size = buf[XMPP_PWD_SIZE_OFFSET];
  p->dst_jid_size = buf[XMPP_DST_JID_SIZE_OFFSET];
  p->src_jid_size = buf[XMPP_SRC_JID_SIZE_OFFSET];
  p->msg_size = buf[XMPP_MSG_SIZE_OFFSET];
  p->timeout = ((buf[XMPP_CTRL_TIMEOUT_BYTE_MSB]&0x0f)<<8);
  p->timeout |= buf[XMPP_CTRL_TIMEOUT_BYTE_LSB];
  p->binary_flag = !!(buf[XMPP_CTRL_TIMEOUT_BYTE_MSB]&XMPP_BINARY_MASK);
  p->pub_sub_flag= !!(buf[XMPP_CTRL_TIMEOUT_BYTE_MSB]&XMPP_PUB_SUB_MASK);
  p->explicit_src_jid_flag = !!(buf[XMPP_CTRL_TIMEOUT_BYTE_MSB]&XMPP_EXPLICIT_SRC_JID_MASK);
  p->passwd = &(buf[XMPP_VAR_START_OFFSET]);
  if(p->explicit_src_jid_flag==1)
  	p->src_jid = &(buf[XMPP_VAR_START_OFFSET + p->passwd_size ]);
  else p->src_jid=0;
  
  p->dst_jid = &(buf[XMPP_VAR_START_OFFSET + p->passwd_size + p->src_jid_size]);
  p->msg = &(buf[XMPP_VAR_START_OFFSET + p->passwd_size + p->src_jid_size + p->dst_jid_size]);
}


uint8_t xmpp_pkt_pack (XMPP_PKT_T * p, uint8_t * buf, uint8_t index)
{
  uint8_t i;
  //uint16_t size;
  //size=XMPP_PASSWD_START_OFFSET + (uint16_t)strlen(p->passwd) + (uint16_t)strlen (p->jid) + (uint16_t)strlen (p->msg); 

  buf[XMPP_PWD_SIZE_OFFSET] = p->passwd_size;

  if(p->explicit_src_jid_flag==1)
  	buf[XMPP_SRC_JID_SIZE_OFFSET] = p->src_jid_size;
  else 
  	buf[XMPP_SRC_JID_SIZE_OFFSET] = 0;

  buf[XMPP_DST_JID_SIZE_OFFSET] = p->dst_jid_size;

  buf[XMPP_MSG_SIZE_OFFSET] = p->msg_size;
  buf[XMPP_CTRL_TIMEOUT_BYTE_MSB] =0; 
  buf[XMPP_CTRL_TIMEOUT_BYTE_LSB] =0; 
  if( p->binary_flag ) buf[XMPP_CTRL_TIMEOUT_BYTE_MSB] |= XMPP_BINARY_MASK; 
  if( p->pub_sub_flag) buf[XMPP_CTRL_TIMEOUT_BYTE_MSB] |= XMPP_PUB_SUB_MASK; 
  if( p->explicit_src_jid_flag) buf[XMPP_CTRL_TIMEOUT_BYTE_MSB] |= XMPP_EXPLICIT_SRC_JID_MASK; 
  buf[XMPP_CTRL_TIMEOUT_BYTE_MSB] |= ((p->timeout>>8)&0x0f);
  buf[XMPP_CTRL_TIMEOUT_BYTE_LSB] = (p->timeout & 0xff);
  buf[XMPP_PWD_SIZE_OFFSET] = p->passwd_size;
  buf[XMPP_SRC_JID_SIZE_OFFSET] = p->src_jid_size;
  buf[XMPP_DST_JID_SIZE_OFFSET] = p->dst_jid_size;
  buf[XMPP_MSG_SIZE_OFFSET] = p->msg_size;

if(((uint16_t)XMPP_VAR_START_OFFSET + (uint16_t)p->passwd_size + (uint16_t)p->src_jid_size + (uint16_t)p->msg_size + (uint16_t)p->dst_jid_size)>=MAX_PKT_PAYLOAD)
	  return 0;


  for (i = 0; i < p->passwd_size; i++)
    buf[XMPP_VAR_START_OFFSET + i] = p->passwd[i];

  for (i = 0; i < p->src_jid_size; i++)
    buf[XMPP_VAR_START_OFFSET + p->passwd_size + i] = p->src_jid[i];

  for (i = 0; i < p->dst_jid_size; i++)
    buf[XMPP_VAR_START_OFFSET + p->passwd_size + p->src_jid_size +i] = p->dst_jid[i];

  for (i = 0; i < p->msg_size; i++)
    buf[XMPP_VAR_START_OFFSET + p->passwd_size + p->src_jid_size + p->dst_jid_size + i] =
      p->msg[i];

  return (XMPP_VAR_START_OFFSET + p->passwd_size + p->src_jid_size + p->dst_jid_size +
          p->msg_size);
}
