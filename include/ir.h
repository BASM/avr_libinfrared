#ifndef _IR_H_
#define _IR_H_

typedef struct _S_IR_event {
  int stat;
  uint32_t time;
} ir_event;

///
// Call it before parse usage..
//
int ir_init(void );
int ir_set_event(ir_event* ev);
int ir_dumpresult(char* array, int size);

#endif /* _IR_H_ */
