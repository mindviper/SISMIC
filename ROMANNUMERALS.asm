;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer


;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------
NUM			.equ	50 ;indica numero a ser convertido
			mov #NUM, R5 ;guarda numero / contador de unidade para o final
			mov #RESP, R6 ;ponteiro da resposta
			mov #0, R7; 0 no contador de milhar
			mov #0, R12; contador de quinhentos
			mov #0, R8 ; 0 no contador de centena
			mov #0, R9; 0 no contador de cinquenta
			mov #0, R10; 0 no contador de dez
			mov #0, R11 ; 0 no contador de cinco
			call #DIVK ;chama subrot
			jmp $ ;trava execucao
			nop ;exigido pelo montador

; duvisao por milhar
DIVK:		add #-1000, R5 ;subtrai para ver se R5 < 1000
			jn DIVK500 ; se for <1000, pula para a proxima etapa da divisao
			add #1, R7 ;conta +1 milhar
			jmp DIVK ;contnua divisao

;estado intermediario de restauracao do resto
DIVK500:
			add #1000, R5 ;restaura o resto da divisao por 1000
			jmp DIVC ;pula para a divisao por centena

DIVC:
			add #-100, R5 ;subtrai para aver se R5 <100
			jn DIVC50 ;se for <100, pula para a proxima etapa da divisao
			add #1, R8 ;conta +1 centena
			jmp DIVC ;continua divisao

DIVC50:
			add #100, R5 ;restaura o resto da divisao por 100
			jmp DIVD ;pula para a divisao por centena

DIVD:
			add #-10, R5 ;subtrai para aver se R5 <10
			jn DIVD5 ;se for <10, pula para a proxima etapa da divisao
			add #1, R10 ;conta +1 centena
			jmp DIVD ;continua divisao

DIVD5:
			add #10, R5 ;restaura o resto da divisao por 10
			jmp ALG_ROMM ;pula para a divisao por centena

DIV5:
			add #-5, R5 ;subtrai para aver se R5 <5
			jn DIV5U ;se for <5, pula para a proxima etapa da divisao
			add #1, R5 ;conta +1 centena
			jmp DIV5 ;continua divisao

DIV5U:
			add #5, R5 ;restaura o resto da divisao por 5
			jmp ALG_ROMM ;pula para a divisao por centena



ALG_ROMM:
			tst R7
			jz ALG_ROMQ
			dec R7
			mov.b #0x4D, 0(R6) ;insere M em ascii
			inc R6
			jmp ALG_ROMM


ALG_ROMQ:
			cmp #9, R8
			jeq NOVEC
			cmp #5, R8
			jc QUIN
			jeq QUIN
			cmp #4, R8
			jeq QUA
			jmp TRE

NOVEC:
			mov.b #0x43, 0(R6) ;insere C em ascii
			inc R6
			mov.b #0x4D, 0(R6) ; insere M em ascii na memoria
			jmp ALG_ROMD

QUIN:
			mov.b #0x44, 0(R6) ;insere D em ascii na RAM
			inc R6
			add #-5, R8
			tst R8
			jz ALG_ROMD
			mov.b #0x43, 0(R6) ;insere C em ascii na RAM
			inc R6
			dec R8
			tst R8
			jz ALG_ROMD
			mov.b #0x43, 0(R6) ;insere C em ascii na RAM
			inc R6
			dec R8
			tst R8
			jz ALG_ROMD
			mov.b #0x43, 0(R6) ;insere C em ascii na RAM
			inc R6
			jmp ALG_ROMD

QUA:
			mov.b #0x43, 0(R6) ;insere C em ascii na RAM
			inc R6
			mov.b #0x44, 0(R6) ;insere D em ascii na RAM
			inc R6
			jmp ALG_ROMD

TRE:
			tst R8
			jz ALG_ROMD
			mov.b #0x43, 0(R6) ;insere C em ascii na RAM
			inc R6
			dec R8
			tst R8
			jz ALG_ROMD
			mov.b #0x43, 0(R6) ;insere C em ascii na RAM
			inc R6
			dec R8
			tst R8
			jz ALG_ROMD
			mov.b #0x43, 0(R6) ;insere C em ascii na RAM
			inc R6
			jmp ALG_ROMD

;-------------------------------------------------------------
;-------------------------------------------------------------

ALG_ROMD:

			cmp #9, R10
			jeq NOVEN
			cmp #5, R10
			jc CIN
			jeq CIN
			cmp #4, R10
			jeq QUAR
			jmp TRI

NOVEN:
			mov.b #0x58, 0(R6) ;insere X em ascii
			inc R6
			mov.b #0x43, 0(R6) ; insere C em ascii na memoria
			jmp ALG_ROMU

CIN:
			mov.b #0x4C, 0(R6) ;insere L em ascii na RAM
			inc R6
			add #-5, R10
			tst R8
			jz ALG_ROMU
			mov.b #0x58, 0(R6) ;insere X em ascii na RAM
			inc R6
			dec R10
			tst R10
			jz ALG_ROMU
			mov.b #0x58, 0(R6) ;insere X em ascii na RAM
			inc R6
			dec R10
			tst R10
			jz ALG_ROMU
			mov.b #0x58, 0(R6) ;insere X em ascii na RAM
			inc R6
			jmp ALG_ROMU

QUAR:
			mov.b #0x58, 0(R6) ;insere X em ascii na RAM
			inc R6
			mov.b #0x4C, 0(R6) ;insere L em ascii na RAM
			inc R6
			jmp ALG_ROMU

TRI:
			tst R10
			jz ALG_ROMU
			mov.b #0x58, 0(R6) ;insere X em ascii na RAM
			inc R6
			dec R10
			tst R10
			jz ALG_ROMU
			mov.b #0x58, 0(R6) ;insere X em ascii na RAM
			inc R6
			dec R10
			tst R10
			jz ALG_ROMU
			mov.b #0x58, 0(R6) ;insere X em ascii na RAM
			inc R6
			jmp ALG_ROMU


;--------------------------------------------------------------
;--------------------------------------------------------------

ALG_ROMU:

			cmp #9, R5
			jeq NOVE
			cmp #5, R5
			jc CINC
			jeq CINC
			cmp #4, R5
			jeq QUAT
			jmp TR

NOVE:
			mov.b #0x49, 0(R6) ;insere I em ascii
			inc R6
			mov.b #0x58, 0(R6) ; insere X em ascii na memoria
			jmp FIM

CINC:
			mov.b #0x56, 0(R6) ;insere V em ascii na RAM
			inc R6
			add #-5, R5
			tst R8
			jz FIM
			mov.b #0x49, 0(R6) ;insere I em ascii na RAM
			inc R6
			dec R5
			tst R5
			jz FIM
			mov.b #0x49, 0(R6) ;insere I em ascii na RAM
			inc R6
			dec R5
			tst R5
			jz FIM
			mov.b #0x49, 0(R6) ;insere I em ascii na RAM
			inc R6
			jmp FIM

QUAT:
			mov.b #0x49, 0(R6) ;insere I em ascii na RAM
			inc R6
			mov.b #0x49, 0(R6) ;insere V em ascii na RAM
			inc R6
			jmp FIM

TR:
			tst R5
			jz FIM
			mov.b #0x49, 0(R6) ;insere I em ascii na RAM
			inc R6
			dec R5
			tst R5
			jz FIM
			mov.b #0x49, 0(R6) ;insere I em ascii na RAM
			inc R6
			dec R5
			tst R5
			jz FIM
			mov.b #0x49, 0(R6) ;insere I em ascii na RAM
			inc R6
			jmp FIM




FIM:
			jmp $


			jmp DIVK
			ret




			.data
RESP:		.byte	"RRRRRRRRRRRRRRR",0

;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
