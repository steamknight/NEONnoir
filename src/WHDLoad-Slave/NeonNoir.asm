;*---------------------------------------------------------------------------
;  :Program.	NeonNoir.asm
;  :Contents.	Slave for "NeonNoir By Mass"
;  :Author.		Vairn
;  :Original	v1 
;  :History.	02.11.22 started
;  :Requires.	-
;  :Language.	68000 Assembler
;  :Translator.	vasm
;---------------------------------------------------------------------------*

	INCDIR	Includes:
	INCLUDE	whdload.i
	INCLUDE	whdmacros.i
	
	IFD BARFLY
	OUTPUT	"NeonNoir.slave"
	BOPT	O+				;enable optimizing
	BOPT	OG+				;enable optimizing
	BOPT	ODd-				;disable mul optimizing
	BOPT	ODe-				;disable mul optimizing
	BOPT	w4-				;disable 64k warnings
	BOPT	wo-				;disable optimize warnings
	SUPER
	ENDC

;============================================================================

CHIPMEMSIZE	= $1ff000	;size of chip memory
FASTMEMSIZE	= $00000	;size of fast memory
NUMDRIVES	= 1		;amount of floppy drives to be configured
WPDRIVES	= %0000		;write protection of floppy drives

BLACKSCREEN			;set all initial colors to black
BOOTDOS				;enable _bootdos routine
DEBUG				;enable additional internal checks
FONTHEIGHT	= 8		;enable 80 chars per line
HDINIT				;initialize filesystem handler
HRTMON				;add support for HrtMON
INITAGA			;enable AGA features
IOCACHE		= 10000		;cache for the filesystem handler (per fh)
POINTERTICKS	= 1		;set mouse speed
SEGTRACKER			;add segment tracker
SETKEYBOARD			;activate host keymap
WHDCTRL				;add WHDCtrl resident command

;============================================================================

slv_Version	= 16
slv_Flags	= WHDLF_NoError|WHDLF_Examine
slv_keyexit	= $5D

;============================================================================

	INCDIR	Sources:
	INCLUDE	whdload/kick31.s

;============================================================================

	IFD BARFLY
	IFND	.passchk
	DOSCMD	"WDate  >T:date"
.passchk
	ENDC
	ENDC

slv_CurrentDir	dc.b	"data",0
slv_name	dc.b	"NeonNoir",0
slv_copy	dc.b	"2022 Mass Produced Games",0
slv_info	dc.b	"AmiBlitzJam",10
			dc.b	"Install by Vairn"
	IFD BARFLY
		INCBIN	"T:date"
	ENDC
		dc.b	0
	EVEN

;============================================================================

_bootdos	lea	(_saveregs,pc),a0
		movem.l	d1-d3/d5-d7/a1-a2/a4-a6,(a0)
		move.l	(a7)+,(11*4,a0)
		move.l	(_resload,pc),a2	;A2 = resload

	;open doslib
		lea	(_dosname,pc),a1
		move.l	(4),a6
		jsr	(_LVOOldOpenLibrary,a6)
		lea	(_dosbase,pc),a0
		move.l	d0,(a0)
		move.l	d0,a6			;A6 = dosbase


	;load exe
		lea	(_program,pc),a0
		move.l	a0,d1
		jsr	(_LVOLoadSeg,a6)
		move.l	d0,d7			;D7 = segment
		beq	_noseg

	;call
		move.l	d7,d1
		move.l	d3,d0
		move.l	a3,a0
		bsr	.call
		move.l	(_resload,pc),a2

	;wait a bit
		moveq	#50,d0
		jsr	(resload_Delay,a2)

	;quit
		pea	TDREASON_OK
		jmp	(resload_Abort,a2)

; D0 = ULONG arg length
; D1 = BPTR  segment
; A0 = CPTR  arg string

.call		lea	(_callregs,pc),a1
		movem.l	d2-d7/a2-a6,(a1)
		move.l	(a7)+,(11*4,a1)
		move.l	d0,d4
		lsl.l	#2,d1
		move.l	d1,a3
		move.l	a0,a4
	;create longword aligend copy of args
		lea	(_callargs,pc),a1
		move.l	a1,d2
.callca		move.b	(a0)+,(a1)+
		subq.w	#1,d0
		bne	.callca
	;set args
		move.l	(_dosbase,pc),a6
		jsr	(_LVOInput,a6)
		lsl.l	#2,d0		;BPTR -> APTR
		move.l	d0,a0
		lsr.l	#2,d2		;APTR -> BPTR
		move.l	d2,(fh_Buf,a0)
		clr.l	(fh_Pos,a0)
		move.l	d4,(fh_End,a0)
	;call
		move.l	d4,d0
		move.l	a4,a0
		movem.l	(_saveregs,pc),d1-d3/d5-d7/a1-a2/a4-a6
		jsr	(4,a3)
	;return
		movem.l	(_callregs,pc),d2-d7/a2-a6
		move.l	(_callrts,pc),a0
		jmp	(a0)

	CNOP 0,4
_saveregs	ds.l	11
_saverts	dc.l	0
_dosbase	dc.l	0
_callregs	ds.l	11
_callrts	dc.l	0
_callargs	ds.b	208
STR_BUF_SIZE = 128
_program	dc.b	"neonnoir",0

	EVEN

_noseg		pea	(_program,pc)
		pea	205			; file not found
		pea	TDREASON_DOSREAD
		jmp	(resload_Abort,a2)


;============================================================================

	END

