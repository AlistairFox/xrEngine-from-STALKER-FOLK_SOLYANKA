; Listing generated by Microsoft (R) Optimizing Compiler Version 14.00.50727.762 

	TITLE	D:\CLEARSKY\sources\engine\xrEngine\line_editor.cpp
	.686P
	.XMM
	include listing.inc
	.model	flat

INCLUDELIB OLDNAMES

PUBLIC	?IR_OnKeyboardHold@line_editor@text_editor@@MAEXH@Z ; text_editor::line_editor::IR_OnKeyboardHold
PUBLIC	?IR_OnKeyboardRelease@line_editor@text_editor@@MAEXH@Z ; text_editor::line_editor::IR_OnKeyboardRelease
PUBLIC	?IR_OnKeyboardPress@line_editor@text_editor@@MAEXH@Z ; text_editor::line_editor::IR_OnKeyboardPress
PUBLIC	??_R4line_editor@text_editor@@6B@		; text_editor::line_editor::`RTTI Complete Object Locator'
PUBLIC	??_R3line_editor@text_editor@@8			; text_editor::line_editor::`RTTI Class Hierarchy Descriptor'
PUBLIC	??_R2line_editor@text_editor@@8			; text_editor::line_editor::`RTTI Base Class Array'
PUBLIC	??_R1A@?0A@EA@line_editor@text_editor@@8	; text_editor::line_editor::`RTTI Base Class Descriptor at (0,-1,0,64)'
PUBLIC	??_R0?AVline_editor@text_editor@@@8		; text_editor::line_editor `RTTI Type Descriptor'
PUBLIC	??_Gline_editor@text_editor@@UAEPAXI@Z		; text_editor::line_editor::`scalar deleting destructor'
PUBLIC	??_7line_editor@text_editor@@6B@		; text_editor::line_editor::`vftable'
EXTRN	??_Eline_editor@text_editor@@UAEPAXI@Z:PROC	; text_editor::line_editor::`vector deleting destructor'
;	COMDAT ??_7line_editor@text_editor@@6B@
CONST	SEGMENT
??_7line_editor@text_editor@@6B@ DD FLAT:??_R4line_editor@text_editor@@6B@ ; text_editor::line_editor::`vftable'
	DD	FLAT:?IR_OnDeactivate@IInputReceiver@@UAEXXZ
	DD	FLAT:?IR_OnActivate@IInputReceiver@@UAEXXZ
	DD	FLAT:?IR_OnMousePress@IInputReceiver@@UAEXH@Z
	DD	FLAT:?IR_OnMouseRelease@IInputReceiver@@UAEXH@Z
	DD	FLAT:?IR_OnMouseHold@IInputReceiver@@UAEXH@Z
	DD	FLAT:?IR_OnMouseWheel@IInputReceiver@@UAEXH@Z
	DD	FLAT:?IR_OnMouseMove@IInputReceiver@@UAEXHH@Z
	DD	FLAT:?IR_OnMouseStop@IInputReceiver@@UAEXHH@Z
	DD	FLAT:?IR_OnKeyboardPress@line_editor@text_editor@@MAEXH@Z
	DD	FLAT:?IR_OnKeyboardRelease@line_editor@text_editor@@MAEXH@Z
	DD	FLAT:?IR_OnKeyboardHold@line_editor@text_editor@@MAEXH@Z
	DD	FLAT:??_Eline_editor@text_editor@@UAEPAXI@Z
CONST	ENDS
;	COMDAT ??_R4line_editor@text_editor@@6B@
rdata$r	SEGMENT
??_R4line_editor@text_editor@@6B@ DD 00H		; text_editor::line_editor::`RTTI Complete Object Locator'
	DD	00H
	DD	00H
	DD	FLAT:??_R0?AVline_editor@text_editor@@@8
	DD	FLAT:??_R3line_editor@text_editor@@8
rdata$r	ENDS
;	COMDAT ??_R3line_editor@text_editor@@8
rdata$r	SEGMENT
??_R3line_editor@text_editor@@8 DD 00H			; text_editor::line_editor::`RTTI Class Hierarchy Descriptor'
	DD	00H
	DD	02H
	DD	FLAT:??_R2line_editor@text_editor@@8
rdata$r	ENDS
;	COMDAT ??_R2line_editor@text_editor@@8
rdata$r	SEGMENT
??_R2line_editor@text_editor@@8 DD FLAT:??_R1A@?0A@EA@line_editor@text_editor@@8 ; text_editor::line_editor::`RTTI Base Class Array'
	DD	FLAT:??_R1A@?0A@EA@IInputReceiver@@8
rdata$r	ENDS
;	COMDAT ??_R1A@?0A@EA@line_editor@text_editor@@8
rdata$r	SEGMENT
??_R1A@?0A@EA@line_editor@text_editor@@8 DD FLAT:??_R0?AVline_editor@text_editor@@@8 ; text_editor::line_editor::`RTTI Base Class Descriptor at (0,-1,0,64)'
	DD	01H
	DD	00H
	DD	0ffffffffH
	DD	00H
	DD	040H
	DD	FLAT:??_R3line_editor@text_editor@@8
rdata$r	ENDS
;	COMDAT ??_R0?AVline_editor@text_editor@@@8
_DATA	SEGMENT
??_R0?AVline_editor@text_editor@@@8 DD FLAT:??_7type_info@@6B@ ; text_editor::line_editor `RTTI Type Descriptor'
	DD	00H
	DB	'.?AVline_editor@text_editor@@', 00H
PUBLIC	??1line_editor@text_editor@@UAE@XZ		; text_editor::line_editor::~line_editor
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrengine\line_editor.cpp
;	COMDAT ??1line_editor@text_editor@@UAE@XZ
_TEXT	SEGMENT
??1line_editor@text_editor@@UAE@XZ PROC			; text_editor::line_editor::~line_editor, COMDAT
; _this$ = ecx

; 21   : {

	mov	DWORD PTR [ecx], OFFSET ??_7line_editor@text_editor@@6B@

; 22   : }

	add	ecx, 4
	jmp	??1line_edit_control@text_editor@@QAE@XZ ; text_editor::line_edit_control::~line_edit_control
??1line_editor@text_editor@@UAE@XZ ENDP			; text_editor::line_editor::~line_editor
; Function compile flags: /Ogtpy
_TEXT	ENDS
;	COMDAT ??_Gline_editor@text_editor@@UAEPAXI@Z
_TEXT	SEGMENT
___flags$ = 8						; size = 4
??_Gline_editor@text_editor@@UAEPAXI@Z PROC		; text_editor::line_editor::`scalar deleting destructor', COMDAT
; _this$ = ecx
	push	esi
	mov	esi, ecx
	lea	ecx, DWORD PTR [esi+4]
	mov	DWORD PTR [esi], OFFSET ??_7line_editor@text_editor@@6B@
	call	??1line_edit_control@text_editor@@QAE@XZ ; text_editor::line_edit_control::~line_edit_control
	test	BYTE PTR ___flags$[esp], 1
	je	SHORT $LN11@scalar@25
	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	push	esi
	call	DWORD PTR __imp_?mem_free@xrMemory@@QAEXPAX@Z
$LN11@scalar@25:
	mov	eax, esi
	pop	esi
	ret	4
??_Gline_editor@text_editor@@UAEPAXI@Z ENDP		; text_editor::line_editor::`scalar deleting destructor'
; Function compile flags: /Ogtpy
;	COMDAT ?IR_OnKeyboardRelease@line_editor@text_editor@@MAEXH@Z
_TEXT	SEGMENT
_dik$ = 8						; size = 4
?IR_OnKeyboardRelease@line_editor@text_editor@@MAEXH@Z PROC ; text_editor::line_editor::IR_OnKeyboardRelease, COMDAT
; _this$ = ecx

; 41   : 	m_control.on_key_release( dik );

	movss	xmm0, DWORD PTR __real@3f800000
	push	esi
	lea	esi, DWORD PTR [ecx+4]
	movss	DWORD PTR [esi+1072], xmm0
	xorps	xmm0, xmm0
	mov	ecx, esi
	movss	DWORD PTR [esi+1080], xmm0
	movss	DWORD PTR [esi+1084], xmm0
	call	?update_key_states@line_edit_control@text_editor@@AAEXXZ ; text_editor::line_edit_control::update_key_states
	mov	ecx, esi
	call	?update_bufs@line_edit_control@text_editor@@AAEXXZ ; text_editor::line_edit_control::update_bufs
	pop	esi

; 42   : }

	ret	4
?IR_OnKeyboardRelease@line_editor@text_editor@@MAEXH@Z ENDP ; text_editor::line_editor::IR_OnKeyboardRelease
; Function compile flags: /Ogtpy
_TEXT	ENDS
;	COMDAT ?IR_OnKeyboardPress@line_editor@text_editor@@MAEXH@Z
_TEXT	SEGMENT
_dik$ = 8						; size = 4
?IR_OnKeyboardPress@line_editor@text_editor@@MAEXH@Z PROC ; text_editor::line_editor::IR_OnKeyboardPress, COMDAT
; _this$ = ecx

; 31   : 	m_control.on_key_press( dik );

	add	ecx, 4
	jmp	?on_key_press@line_edit_control@text_editor@@QAEXH@Z ; text_editor::line_edit_control::on_key_press
?IR_OnKeyboardPress@line_editor@text_editor@@MAEXH@Z ENDP ; text_editor::line_editor::IR_OnKeyboardPress
_TEXT	ENDS
PUBLIC	?on_frame@line_editor@text_editor@@QAEXXZ	; text_editor::line_editor::on_frame
; Function compile flags: /Ogtpy
;	COMDAT ?on_frame@line_editor@text_editor@@QAEXXZ
_TEXT	SEGMENT
?on_frame@line_editor@text_editor@@QAEXXZ PROC		; text_editor::line_editor::on_frame, COMDAT
; _this$ = ecx

; 26   : 	m_control.on_frame();

	add	ecx, 4
	jmp	?on_frame@line_edit_control@text_editor@@QAEXXZ ; text_editor::line_edit_control::on_frame
?on_frame@line_editor@text_editor@@QAEXXZ ENDP		; text_editor::line_editor::on_frame
; Function compile flags: /Ogtpy
;	COMDAT ?IR_OnKeyboardHold@line_editor@text_editor@@MAEXH@Z
_TEXT	SEGMENT
_dik$ = 8						; size = 4
?IR_OnKeyboardHold@line_editor@text_editor@@MAEXH@Z PROC ; text_editor::line_editor::IR_OnKeyboardHold, COMDAT
; _this$ = ecx

; 36   : 	m_control.on_key_hold( dik );

	add	ecx, 4
	jmp	?on_key_hold@line_edit_control@text_editor@@QAEXH@Z ; text_editor::line_edit_control::on_key_hold
?IR_OnKeyboardHold@line_editor@text_editor@@MAEXH@Z ENDP ; text_editor::line_editor::IR_OnKeyboardHold
_TEXT	ENDS
PUBLIC	??0line_editor@text_editor@@QAE@I@Z		; text_editor::line_editor::line_editor
; Function compile flags: /Ogtpy
;	COMDAT ??0line_editor@text_editor@@QAE@I@Z
_TEXT	SEGMENT
??0line_editor@text_editor@@QAE@I@Z PROC		; text_editor::line_editor::line_editor, COMDAT
; _this$ = esi
; _str_buffer_size$ = eax

; 17   : {

	push	eax
	lea	ecx, DWORD PTR [esi+4]
	mov	DWORD PTR [esi], OFFSET ??_7line_editor@text_editor@@6B@
	call	??0line_edit_control@text_editor@@QAE@I@Z ; text_editor::line_edit_control::line_edit_control

; 18   : }

	mov	eax, esi
	ret	0
??0line_editor@text_editor@@QAE@I@Z ENDP		; text_editor::line_editor::line_editor
END
