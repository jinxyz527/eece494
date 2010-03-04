#ifndef _SAMPL_TASKS_H_
#define _SAMPL_TASKS_H_


uint8_t task_got_pkt_flag;
void sampl_startup();
void sampl_config();
int8_t push_p2p_pkt(char *payload, uint8_t size, uint8_t pkt_type, uint8_t dst_mac );

#endif
