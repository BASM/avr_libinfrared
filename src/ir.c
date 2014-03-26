//#include <stdio.h>
#define printf(...) {}


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
  PROTO_NECHEAD,
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
    if(in_range(delta, 560, 558)){
      nec.start=1;
    }else{
      printf("BROKEN PACKAGE: %i\n", delta);
      return -1;
    }
  }else{
    if (in_range(delta, 1120-560, 558)){
      nec.bites++;
    }else{
      if (in_range(delta, 2240-560, 558)){
        int byte=nec.bites/8;
        int bite=nec.bites%8;
        nec.byte[byte]|=1<<bite;
        //printf("BYTE: %i, BITE: %i, CHR: %x\n", byte, bite, nec.byte[byte]);
        nec.bites++;
      }else{
        printf("BROKEN PACKAGE2: %i (btes: %i)\n", delta, nec.bites);
      }
    }
  }
  return 0;
}

int ir_set_event(ir_event* ev)
{
  static int oldstat=0;

  if (ev->stat == oldstat){
    //printf("TIME OUT\n");

    switch(detect.parsetype){
      case PROTO_NEC:
        ir_dumpresult(nec.byte, (nec.bites+1)/8);
        break;
    }
    detect_reset();
    return 1;
  }
  oldstat=ev->stat;

  uint32_t delta = ev->time;
  //printf("delta: %i\n", delta);
  if (delta>(9000*2)){
    //printf("START PACKAGE: %i\n", delta);

    if(detect.parsetype==PROTO_NEC){
        ir_dumpresult(nec.byte, (nec.bites+1)/8);
    }
    detect_reset();
    return 1;
  }
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
            return 1;
          }
        }else{
          if (detect.head_up==0){
            //First package
            return 0;
          }else{
            printf("HEADER tailtail parse error\n");
            detect_reset();
            return 1;
          }
        }

      }
    }while(0);
    if ( in_range(detect.head_up, 9000, 300) &&
         in_range(detect.head_down, 4400, 300)) {
         detect.parsetype=PROTO_NEC;
         parse_nec_init();
         return 0;
      }else{
        if( in_range(detect.head_up, 9000, 500) &&
            in_range(detect.head_down, 2250, 500)){
            detect.parsetype=PROTO_NECHEAD;
            parse_nec_init();
        }else{
          printf("UNKNONW HEAD: up: %i, down: %i\n\r", detect.head_up, detect.head_down);
          detect_reset();
        }
        return 1;
      }
  }

  if(detect.parsetype==PROTO_NEC){
    if(parse_nec_bite(ev, delta)<0)
      detect_reset();
  }
  if(detect.parsetype==PROTO_NECHEAD){
    if(in_range(delta, 562, 100)){
        ir_dumpresult(0, 0);//RET
    }
  }


  return 0;
}

