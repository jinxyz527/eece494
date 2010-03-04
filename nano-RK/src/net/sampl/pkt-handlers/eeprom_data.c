#include <globals.h>
#include <sampl.h>
#include <eeprom_data.h>

#ifdef NANORK
#include <nrk.h>
#include <nrk_error.h>
#else
#define my_mac 0
#define MAX_PKT_PAYLOAD  113
#endif


#ifdef NANORK

int8_t eeprom_storage_p2p_generate (SAMPL_PEER_2_PEER_PKT_T * p2p_pkt_in,
                                    SAMPL_PEER_2_PEER_PKT_T * p2p_pkt_out)
{

  EEPROM_STORAGE_PKT_T p;
  uint8_t i;

  eeprom_storage_pkt_unpack(&p, p2p_pkt_in->payload);
  // set reply mac
  p.mac = my_mac;
  if (p.addr > EEPROM_APP_PROTECTION_BOUNDARY) {
    //write operation
    if (p.mode == EE_WRITE) {
      for (i = 0; i < p.data_len; i++)
        nrk_eeprom_write_byte (p.addr + i, p.eeprom_payload[i]);
	// This will get copied on the write by default
    }
    //READ operation
    if (p.mode == EE_READ) {
      // Read data from eeprom 
      p.eeprom_payload=&(p2p_pkt_out->payload[EE_PAYLOAD_START_INDEX]);
      for (i = 0; i < p.data_len; i++)
        p.eeprom_payload[i] = nrk_eeprom_read_byte (p.addr + i);
    }
    p.mode = EE_REPLY;
  }
  else {
    p.mode = EE_ERROR;
  }

  p2p_pkt_out->payload_len = eeprom_storage_pkt_pack (&p, p2p_pkt_out->payload);

  return NRK_OK;
}



int8_t eeprom_storage_generate (SAMPL_UPSTREAM_PKT_T * pkt,
                                SAMPL_DOWNSTREAM_PKT_T * ds_pkt)
{
  EEPROM_STORAGE_PKT_T p;
  uint8_t i;

  //nrk_kprintf( PSTR( "EEPROM packet: " ));
  eeprom_storage_pkt_unpack(&p, ds_pkt->payload);
  p.mac = my_mac;
  if (p.addr > EEPROM_APP_PROTECTION_BOUNDARY) {
    //write operation
    if (p.mode == EE_WRITE) {
      //nrk_kprintf( PSTR( "write" ));
      for (i = 0; i < p.data_len; i++)
        nrk_eeprom_write_byte (p.addr + i, p.eeprom_payload[i]);
       // This will get copied by pack for the reply
    }
    //READ operation
    if (p.mode == EE_READ) {
      //nrk_kprintf( PSTR( "read" ));
      // Read data from eeprom 
      p.eeprom_payload=&(ds_pkt->payload[EE_PAYLOAD_START_INDEX]);
      for (i = 0; i < p.data_len; i++)
        p.eeprom_payload[i] = nrk_eeprom_read_byte (p.addr + i);
    }
    p.mode = EE_REPLY;
  }
  else {
    //nrk_kprintf( PSTR( "error" ));
    p.mode = EE_ERROR;
  }

  //nrk_kprintf( PSTR( "\r\n" ));
  pkt->payload_len = eeprom_storage_pkt_pack(&p, pkt->payload);
  pkt->num_msgs = 1;

  return NRK_OK;
}


int8_t eeprom_storage_aggregate (SAMPL_UPSTREAM_PKT_T * in,
                                 SAMPL_UPSTREAM_PKT_T * out)
{
// DATA STORAGE currently does not support aggregation
  return NRK_ERROR;
}
#endif

void eeprom_storage_pkt_unpack(EEPROM_STORAGE_PKT_T * p, char *buf)
{
  p->mode = buf[0];
  p->addr = buf[1];
  p->addr = p->addr << 8;
  p->addr |= buf[2];
  p->data_len = buf[3];
  p->mac = buf[4];
  // point to the start of the correct index
  p->eeprom_payload = &buf[EE_PAYLOAD_START_INDEX];
}


uint8_t eeprom_storage_pkt_pack(EEPROM_STORAGE_PKT_T * p, char *buf)
{
  uint8_t i;

  buf[0] = p->mode;
  buf[1] = (p->addr >> 8) & 0xff;
  buf[2] = (p->addr & 0xff);
  buf[3] = p->data_len;
  buf[4] = p->mac;
  i=0;
  if(p->mode!=EE_READ)
  {
  for (i = 0; i < p->data_len; i++)
    buf[EE_PAYLOAD_START_INDEX + i] = p->eeprom_payload[i];
  }
  return (EE_PAYLOAD_START_INDEX + i);
}
