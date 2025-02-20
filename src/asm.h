#ifndef ASM_H
#define ASM_H

extern void setPsp(uint32_t *p);
extern void setAspBit();
extern void setUnprivileged();
extern uint32_t *getPsp();
extern uint32_t *getMsp();
extern void executeBusFaultError();
extern void executeHardFault();
extern void triggerFault();
extern uint32_t *getXPSR();
extern uint32_t *getPC();
extern uint32_t *getPC();
extern uint32_t *getLR();
extern uint32_t *getR12();
extern uint32_t *getR3();
extern uint32_t *getR2();
extern uint32_t *getR1();
extern uint32_t *getR0();
extern void pop_R4_R11();
extern void push_R4_R11();
extern uint32_t getSVC(void);






#endif
