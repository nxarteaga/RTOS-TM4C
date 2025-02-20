	.def setPsp
	.def setAspBit
	.def setUnprivileged
	.def getPsp
	.def getMsp
	.def executeBusFaultError
	.def executeHardFault
	.def triggerFault
    .def getXPSR	;This are all to get stack info
    .def getPC
    .def getLR
    .def getR12
    .def getR3
    .def getR2
    .def getR1
    .def getR0
    .def pop_R4_R11
    .def push_R4_R11
	.def getSVC



.thumb
.const

EXC_RETURN	.field 0xFFFFFFFD ;Page 111 Datasheet

setPsp:
	MSR PSP, R0
	ISB
	BX LR

;MRS Move to ARM Register from System Register
setAspBit:
	MRS R0, CONTROL ;Move especial register CONTROL to R0ISB				;Instruction Barrier
	ORR	R0, R0, #2  ;OR R0(contains CONTROL) with bit 2
	MSR CONTROL, R0	; Loads result from R0 into CONTROL
	ISB
	BX LR

setUnprivileged:
	MRS R0, CONTROL ;Move especial register CONTROL to R				;Instruction Barrier
	ORR	R0, R0, #1  ;OR R0(contains CONTROL) with bit 0
	MSR CONTROL, R0	; Loads result from R0 into CONTROL
	ISB
	BX LR

getPsp:
	MRS R0, PSP
	BX LR

getMsp:
	MRS R0, MSP
	BX LR

executeBusFaultError:
    MOVW R1, #0x0000         ; Load lower 16 bits of 0x20000000
    MOVT R1, #0x2000         ; Load upper 16 bits of 0x20000000
    LDR R2, [R1]             ; Load value from address in R1
    ADD R3, R2, #2           ; Add 2 to the value in R2 and store result in R3
    LDR R4, [R3]             ; Load value from address in R3
    BX LR                    ; Return from function

executeHardFault:		; Corrupts the Link Register = Hard Fault
	MOVW R1, #0xFFFF
	MOVT R1, #0xFFFF
	MOV  LR, R1
	BX LR

triggerFault:			; triggers a bus fault
	.word 0xDEADAAAA
	BX LR


getXPSR:
	MRS R0, PSP
    ADD	R0, R0, #28	; PSP move ptr 7*4
    LDR R1, [R0]
    BX LR

getPC:
    MRS R0, PSP
    ADD	R0, R0, #24	;PSP move ptr 6*4
    LDR R1, [R0]
    BX LR

getLR:
	MRS R0, PSP
    ADD	R0, R0, #20	; PSP move ptr 5*4
    LDR R1, [R0]
    BX LR

getR12:
	MRS R0, PSP
    ADD	R0, R0, #16	; PSP move ptr 4*4
    LDR R1, [R0]
    BX LR

getR3:
    MRS R0, PSP
    ADD	R0, R0, #12	;PSP move ptr 3*4
    LDR R1, [R0]
    BX LR

getR2:
	MRS R0, PSP
    ADD	R0, R0, #8	; PSP move ptr 2*4
    LDR R1, [R0]
    BX LR

getR1:
    MRS R0, PSP
    ADD	R0, R0, #4	;PSP move ptr 1*4
    LDR R1, [R0]
    BX LR

getR0:
	MRS R0, PSP
    ADD	R0, R0, #20	; PSP move ptr 0*4
    LDR R1, [R0]
    BX LR

getR13:
	MRS R0, PSP
    ADD	R0, R0, #14	; PSP move ptr 0*4
	BX LR

pop_R4_R11:
	MRS R0, PSP
	LDR LR, [R0]
	ADD R0, R0, #4
	LDR R11, [R0]
	ADD R0, R0, #4
	LDR R10, [R0]
	ADD R0, R0, #4
	LDR R9, [R0]
	ADD R0, R0, #4
	LDR R8, [R0]
	ADD R0, R0, #4
	LDR R7, [R0]
	ADD R0, R0, #4
	LDR R6, [R0]
	ADD R0, R0, #4
	LDR R5, [R0]
	ADD R0, R0, #4
	LDR R4, [R0]
	ADD R0, R0, #4
	MSR PSP, R0
	ISB
	BX LR

push_R4_R11:
    MRS R0, PSP            ; Load current PSP into R0

    SUB R0, R0, #4         ; Make space for R4
    STR R4, [R0]           ; Store R4 at the top of the stack

    SUB R0, R0, #4         ; Make space for R5
    STR R5, [R0]           ; Store R5

    SUB R0, R0, #4         ; Make space for R6
    STR R6, [R0]           ; Store R6

    SUB R0, R0, #4         ; Make space for R7
    STR R7, [R0]           ; Store R7

    SUB R0, R0, #4         ; Make space for R8
    STR R8, [R0]           ; Store R8

    SUB R0, R0, #4         ; Make space for R9
    STR R9, [R0]           ; Store R9

    SUB R0, R0, #4         ; Make space for R10
    STR R10, [R0]          ; Store R10

    SUB R0, R0, #4         ; Make space for R11
    STR R11, [R0]          ; Store R11

	SUB R0, R0, #4
	STR R12, [R0]

    MSR PSP, R0            ; Update PSP to new stack location
    ISB
    BX LR                  ; Return to caller

getSVC:
    MRS R0, PSP              ;
    LDR R0, [R0, #24]        ;
    SUB R0, R0, #2           ;
    LDRB R0, [R0]            ;
    AND R0, R0, #0xFF        ;
    BX LR




