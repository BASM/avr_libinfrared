#ifndef _IR_H_
#define _IR_H_

typedef struct _S_IR_event {
  int stat;
  int time;
} ir_event;

int parse_init(void );
int hexdump(unsigned char* buff, int size);

#endif /* _IR_H_ */
