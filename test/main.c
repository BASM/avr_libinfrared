#include <stdio.h>
#include <inttypes.h>
#include <ir.h>

int hexdump(unsigned char* buff, int size)
{
  int i;
  for(i=0; i<size; i++){
    if(((i%16)==0)){
      printf("\n%4.4x ", i); 
    }
    printf("%2.2x ", buff[i]);
  }
  printf("\n");
  return 0;
}


int ir_event_package(char* package, int size)
{
  printf("Get package:\n");
  hexdump(package, size);

  return 0;
}

/////////////////////////////////////////
///########### TESTING ###############///

static void send_fail_package()
{

}

static void send_nec_head(ir_event *ev)
{
  ev->time+=50000; // Drop previes package
  ev->stat=1;

  parce_event(ev);
  ev->time+=9000; // start
  ev->stat=0;

  parce_event(ev);
  ev->time+=4400; // wait after start
  ev->stat=0;
}

static void send_nec_bite(ir_event *ev, int bite)
{
  ev->stat=1;
  parce_event(ev);// 1
  ev->time+=560;
  ev->stat=0;
  parce_event(ev);// 0

  if(bite == 0){
    ev->time+=1120-560;
  }else{
    ev->time+=2240-560;
  }
}

static void send_nec_byte(ir_event *ev, 
    uint8_t chr)
{
  int i;
  for(i=0; i<8; i++)
    send_nec_bite(ev, (chr>>i) & 0x1 );
}

static void send_timeout(ir_event *ev)
{
  ev->stat=0;
  parce_event(ev);// 0
  ev->stat=0;
  parce_event(ev);// 0

}

static void send_nec_package()
{
  ir_event ev;
  ev.time=0;
  ev.stat=0;
  send_nec_head(&ev);

  send_nec_byte(&ev,0x55);
  send_nec_byte(&ev,0xee);
  send_nec_byte(&ev,0x00);
  send_nec_byte(&ev,0xff);
  send_timeout(&ev);

}

int main(void){

  parse_init();
  //ir_event xx;
  //parce_event(&xx);

  printf("Test FAIL PACKAGE\n");
  send_fail_package();

  printf("Test NEC\n");
  send_nec_package();


  return 0;
}
