;===================================================================
; code to detect mockingboard
;===================================================================
; this isn't always easy
; my inclination is to just assume slot #4 but that isn't always realistic

; code below based on "hw.mockingboard.a" from "Total Replay"

;license:MIT
; By Andrew Roughan
;   in the style of 4am for Total Replay
;
; Mockingboard support functions
;

;------------------------------------------------------------------------------
; HasMockingboard or Echo+
; detect Mockingboard card by searching for one 6522 timer across all slots
; access 6522 timer with deterministic cycle counts. Check for only one since 
; the Echo+ only features one 6522.
;
;   based on prior art in Mockingboard Developers Toolkit
;   with optimisation from deater/french touch
;   also takes into account FastChip //e clock difference
;
; in:    none
;        accelerators should be off
; out:   C set if Mockingboard found in any slot
;        if card was found, X = #$Cn where n is the slot number of the card
;        C clear if no Mockingboard found
;        other flags clobbered
;        zp $65-$67 clobbered
;        A/Y clobbered
;------------------------------------------------------------------------------

mockingboard_detect:
	lda	#$00
	sta	MB_ADDR_L
	ldx	#$C7			; start at slot #7
mb_slot_loop:
	stx	MB_ADDR_H
	ldy	#$04			; 6522 #1 $Cx04
	jsr	mb_timer_check
	bne	mb_next_slot
mb_found:
	sec				; found
	rts

mb_next_slot:
	dex
	cpx	#$C0
	bne	mb_slot_loop

	clc				; not found
	rts

mb_timer_check:
	lda	(MB_ADDR_L),Y		; read 6522 timer low byte
	sta	MB_VALUE
	lda	(MB_ADDR_L),Y		; second time
	sec
	sbc	MB_VALUE
	cmp	#$F8			; looking for (-)8 cycles between reads
	beq	mb_timer_check_done
	cmp	#$F7			; FastChip //e clock is different
mb_timer_check_done:
	rts


;===================================================================
; dangerous code to patch mockingboard if not in slot#4
;===================================================================
; this code patches any $C4 value to the proper slot# if not slot4
; this can be dangerous, it might over-write other important values
;	that should be $C4

; safer ways to do this:
;	only do this if 2 bytes after a LDA/STA/LDX/STX
;	count total and if not 39 then print error message

mockingboard_patch:
	; from mockingboard_init 	$1BBF
	;   to done_pt3_irq_handler	$1D85

	ldx	MB_ADDR_H
	ldy	#0

	lda	#<mockingboard_init
	sta	MB_ADDR_L
	lda	#>mockingboard_init
	sta	MB_ADDR_H

mb_patch_loop:
	lda	(MB_ADDR_L),Y
	cmp	#$C4
	bne	mb_patch_nomatch

	txa
	sta	(MB_ADDR_L),Y
mb_patch_nomatch:

	inc	MB_ADDR_L
	lda	MB_ADDR_L
	bne	mb_patch_oflo
	inc	MB_ADDR_H

mb_patch_oflo:
	lda	MB_ADDR_H
	cmp	#>done_pt3_irq_handler
	bne	mb_patch_loop
	lda	MB_ADDR_L
	cmp	#<done_pt3_irq_handler
	bne	mb_patch_loop

mb_patch_done:
	rts
