.syntax unified
.thumb

.cpu cortex-m4
.arch armv7e-m
.fpu fpv4-sp-d16

.global MSP
.global PSP

.thumb_func
.global SVC_Handler
SVC_Handler: 
	TST lr, #4
	ITE EQ
	MRSEQ r0, MSP
	MRSNE r0, PSP
	B SVC_Handler_Main

.thumb_func
.global PendSV_Handler
PendSV_Handler:
	MRS R0, PSP
	STMDB R0!, {R4-R11}
	MSR PSP, R0
	BL Context_Switch
	MRS R0, PSP
	LDMIA R0!, {R4-R11}
	MSR PSP, R0
	LDR LR, =0xFFFFFFFD
    BX LR


	
