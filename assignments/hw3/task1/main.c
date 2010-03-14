#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <bmac.h>
#include <nrk_error.h>
#include <nrk_events.h>
#include <nrk_timer.h>

nrk_task_type RX_TASK;
NRK_STK rx_task_stack[NRK_APP_STACKSIZE];
void rx_task (void);

nrk_task_type TX_TASK;
NRK_STK tx_task_stack[NRK_APP_STACKSIZE];
void tx_task (void);

void nrk_create_taskset ();

uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t startCnt = 0;

nrk_sem_t *lock;

int main ()
{
    nrk_setup_ports ();
    nrk_setup_uart (UART_BAUDRATE_115K2);

    nrk_init ();
    nrk_led_clr(ORANGE_LED);
    nrk_led_clr(GREEN_LED);
    nrk_led_clr(RED_LED);
    nrk_led_clr(BLUE_LED);

    nrk_time_set(0,0);

    bmac_task_config();
    nrk_create_taskset();

    lock = nrk_sem_create(1,2);
    if( lock==NULL ) {
        nrk_kprintf( PSTR("Error creating sem\r\n" ));
    }

    nrk_start();

    return 0;
}

void rx_task ()
{
    uint8_t i,len;
    int8_t rssi, v;
    nrk_status_t ret;
    uint8_t *local_rx_buf;

//    printf( "rx_task: PID=%d\r\n",nrk_get_pid());

    bmac_rx_pkt_set_buffer(rx_buf,RF_MAX_PAYLOAD_SIZE);

    while(!bmac_started()) nrk_wait_until_next_period();

    while(1)
    {
        if( bmac_rx_pkt_ready()==0)
            bmac_wait_until_rx_pkt();  

        v = nrk_sem_pend(lock);
        if( startCnt == 0 )
            startCnt = 1;
        v = nrk_sem_post(lock);

        nrk_led_toggle(GREEN_LED); 
        local_rx_buf = bmac_rx_pkt_get(&len,&rssi);

        printf( "rx_task: rssi=%d data=", rssi);
        for( i=0; i<len; i++ ) {
            printf( "%c", local_rx_buf[i]);
        }
        nrk_kprintf( PSTR("\r\n") );

        bmac_rx_pkt_release();
    }
}

void tx_task()
{
  uint8_t cnt = 0;
  uint8_t val;
  int8_t v;
  
//  printf( "tx_task: PID=%d\r\n",nrk_get_pid());
  
  bmac_init(25);
  while(!bmac_started()) nrk_wait_until_next_period();

  while(1)
  {
        if (cnt > 25) {
            nrk_led_set(BLUE_LED);
            nrk_terminate_task();
        }

        nrk_led_set(RED_LED); 
        sprintf( tx_buf, "%d", cnt );
        val = bmac_tx_pkt(tx_buf, strlen(tx_buf));
        nrk_led_clr(RED_LED); 
//        nrk_kprintf( PSTR("TX task sent data!\r\n") );
        nrk_wait_until_next_period();

        v = nrk_sem_pend(lock);
        if ( startCnt )
            cnt++;
        v = nrk_sem_post(lock);
  }
}

void nrk_create_taskset ()
{
  RX_TASK.task = rx_task;
  nrk_task_set_stk( &RX_TASK, rx_task_stack, NRK_APP_STACKSIZE);
  RX_TASK.prio = 2;
  RX_TASK.FirstActivation = TRUE;
  RX_TASK.Type = BASIC_TASK;
  RX_TASK.SchType = PREEMPTIVE;
  RX_TASK.period.secs = 0;
  RX_TASK.period.nano_secs = 500*NANOS_PER_MS;
  RX_TASK.cpu_reserve.secs = 0;
  RX_TASK.cpu_reserve.nano_secs = 100 * NANOS_PER_MS;
  RX_TASK.offset.secs = 0;
  RX_TASK.offset.nano_secs = 0;
  nrk_activate_task (&RX_TASK);

  TX_TASK.task = tx_task;
  nrk_task_set_stk( &TX_TASK, tx_task_stack, NRK_APP_STACKSIZE);
  TX_TASK.prio = 3;
  TX_TASK.FirstActivation = TRUE;
  TX_TASK.Type = BASIC_TASK;
  TX_TASK.SchType = PREEMPTIVE;
  TX_TASK.period.secs = 0;
  TX_TASK.period.nano_secs = 500*NANOS_PER_MS;
  TX_TASK.cpu_reserve.secs = 0;
  TX_TASK.cpu_reserve.nano_secs = 100 * NANOS_PER_MS;
  TX_TASK.offset.secs = 0;
  TX_TASK.offset.nano_secs = 0;
  nrk_activate_task (&TX_TASK);

//  printf ("Create done\r\n");
}
