#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <ir.h>


//static int curtime=0;

struct {
  int start;
  int bites;
  char byte[10];
} nec;

enum {
  PROTO_DETECT,
  PROTO_NEC,
  PROTO_PANASONIC,
};

struct {
  int head_up;
  int head_down;
  int parsetype;
  int lastbit;
} detect;


static void detect_reset()
{
  detect.parsetype=PROTO_DETECT;
  detect.lastbit=0;
  detect.head_up=0;
  detect.head_down=0;
}

int ir_init()
{
  detect_reset();
}


static int in_range(int value, int etalon, int delta)
{
  if ((value>(etalon-delta)) &&
      (value<(etalon+delta)))
    return 1;
  return 0;
}

static int parse_nec_init(){
  nec.start=0;
  nec.bites=0;
  memset(nec.byte, 0, sizeof(nec.byte));
}

static int parse_nec_bite(ir_event* ev, int delta)
{
  if(ev->stat==0){
    if(in_range(delta, 560, 50)){
      nec.start=1;
    }else{
      printf("BROKEN PACKAGE\n");
      return -1;
    }
  }else{
    if (in_range(delta, 1120-560, 50)){
      nec.bites++;
    }else{
      if (in_range(delta, 2240-560, 50)){
        int byte=nec.bites/8;
        int bite=nec.bites%8;
        nec.byte[byte]|=1<<bite;
        //printf("BYTE: %i, BITE: %i, CHR: %x\n", byte, bite, nec.byte[byte]);
        nec.bites++;
      }else{
        printf("BROKEN PACKAGE2\n");
      }
    }
  }
  return 0;
}

int ir_set_event(ir_event* ev)
{
  static int oldtime=0;
  static int oldstat=0;

  if (ev->stat == oldstat){
    printf("TIME OUT\n");

    switch(detect.parsetype){
      case PROTO_NEC:
        ir_dumpresult(nec.byte, (nec.bites+1)/8);
        parse_nec_init();
        break;
    }
    detect_reset();
  }
  oldstat=ev->stat;

  if (ev->time < oldtime){
    //DROP PACKAGE
    printf("PARSE init \n");
    detect_reset();
    return 0;
  }

  int delta = ev->time-oldtime;
  oldtime=ev->time;
  //printf("Event: %i, %i, delta: %i\n", ev->stat, ev->time, delta);

  if (detect.parsetype==PROTO_DETECT) {
    do{
      if (ev->stat == 0) {
        if (detect.head_up==0){
          detect.head_up=delta;
          return 0;
        }else{
          printf("HEADER head PARSE ERROR\n");
          detect_reset();
        }
      }else{
        if (detect.head_up!=0) {
          if (detect.head_down == 0){
            detect.head_down = delta;
            break;
          }else{
            printf("HEADER tail parse error\n");
            detect_reset();
            return 0;
          }
        }else{
          if (detect.head_up==0){
            //First package
            return 0;
          }else{
            printf("HEADER tailtail parse error\n");
            detect_reset();
            return 0;
          }
        }

      }
    }while(0);
    if (in_range(detect.head_up, 9000, 100) )
      if (in_range(detect.head_down, 4400, 100) ){
        detect.parsetype=PROTO_NEC;
        printf("FINDED NEC PROTOCOL\n");
      }
  }

  if(detect.parsetype==PROTO_NEC){
    parse_nec_bite(ev, delta);
  }


  return 0;
}

