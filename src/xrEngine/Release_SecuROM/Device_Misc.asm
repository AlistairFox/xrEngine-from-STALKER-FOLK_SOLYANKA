; Listing generated by Microsoft (R) Optimizing Compiler Version 14.00.50727.762 

	TITLE	D:\CLEARSKY\sources\engine\xrEngine\Device_Misc.cpp
	.686P
	.XMM
	include listing.inc
	.model	flat

INCLUDELIB OLDNAMES

PUBLIC	??_C@_0M@DBMIEFHH@rsWireframe?$AA@		; `string'
PUBLIC	??_C@_07BGOCBDGE@rsVSync?$AA@			; `string'
PUBLIC	??_C@_09EAOPACIJ@rsClearBB?$AA@			; `string'
PUBLIC	??_C@_0N@DBBDFLM@rsFullscreen?$AA@		; `string'
PUBLIC	??_C@_0BH@CLBOKMHM@?9?5Dumping?5device?5flags?$AA@ ; `string'
PUBLIC	??_C@_09GIDPMCMI@?$CK?5?$CF20s?5?$CFs?$AA@	; `string'
;	COMDAT ??_C@_09GIDPMCMI@?$CK?5?$CF20s?5?$CFs?$AA@
CONST	SEGMENT
??_C@_09GIDPMCMI@?$CK?5?$CF20s?5?$CFs?$AA@ DB '* %20s %s', 00H ; `string'
CONST	ENDS
;	COMDAT ??_C@_0BH@CLBOKMHM@?9?5Dumping?5device?5flags?$AA@
CONST	SEGMENT
??_C@_0BH@CLBOKMHM@?9?5Dumping?5device?5flags?$AA@ DB '- Dumping device f'
	DB	'lags', 00H					; `string'
CONST	ENDS
;	COMDAT ??_C@_0M@DBMIEFHH@rsWireframe?$AA@
CONST	SEGMENT
??_C@_0M@DBMIEFHH@rsWireframe?$AA@ DB 'rsWireframe', 00H ; `string'
CONST	ENDS
;	COMDAT ??_C@_07BGOCBDGE@rsVSync?$AA@
CONST	SEGMENT
??_C@_07BGOCBDGE@rsVSync?$AA@ DB 'rsVSync', 00H		; `string'
CONST	ENDS
;	COMDAT ??_C@_09EAOPACIJ@rsClearBB?$AA@
CONST	SEGMENT
??_C@_09EAOPACIJ@rsClearBB?$AA@ DB 'rsClearBB', 00H	; `string'
CONST	ENDS
;	COMDAT ??_C@_0N@DBBDFLM@rsFullscreen?$AA@
CONST	SEGMENT
??_C@_0N@DBBDFLM@rsFullscreen?$AA@ DB 'rsFullscreen', 00H ; `string'
_DF	DD	FLAT:??_C@_0N@DBBDFLM@rsFullscreen?$AA@
	DD	01H
	DD	FLAT:??_C@_09EAOPACIJ@rsClearBB?$AA@
	DD	02H
	DD	FLAT:??_C@_07BGOCBDGE@rsVSync?$AA@
	DD	04H
	DD	FLAT:??_C@_0M@DBMIEFHH@rsWireframe?$AA@
	DD	08H
	DD	00H
	DD	00H
PUBLIC	?DumpFlags@CRenderDevice@@QAEXXZ		; CRenderDevice::DumpFlags
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrengine\device_misc.cpp
;	COMDAT ?DumpFlags@CRenderDevice@@QAEXXZ
_TEXT	SEGMENT
?DumpFlags@CRenderDevice@@QAEXXZ PROC			; CRenderDevice::DumpFlags, COMDAT
; _this$ = ecx

; 20   : {

	push	esi

; 21   : 	Log("- Dumping device flags");

	push	OFFSET ??_C@_0BH@CLBOKMHM@?9?5Dumping?5device?5flags?$AA@
	call	DWORD PTR __imp_?Log@@YAXPBD@Z
	add	esp, 4

; 22   : 	_DF *p = DF;
; 23   : 	while (p->name) {

	cmp	DWORD PTR _DF, 0
	mov	esi, OFFSET _DF
	je	SHORT $LN1@DumpFlags
	push	edi
	mov	edi, DWORD PTR __imp_?Msg@@YAXPBDZZ
$LL2@DumpFlags:

; 24   : 		Msg("* %20s %s",p->name,psDeviceFlags.test(p->mask)?"on":"off");

	mov	eax, DWORD PTR [esi+4]
	push	eax
	mov	ecx, OFFSET ?psDeviceFlags@@3U?$_flags@I@@A ; psDeviceFlags
	call	?test@?$_flags@I@@QBEHI@Z		; _flags<unsigned int>::test
	test	eax, eax
	mov	eax, OFFSET ??_C@_02LIELOMNJ@on?$AA@
	jne	SHORT $LN6@DumpFlags
	mov	eax, OFFSET ??_C@_03MCADLMAF@off?$AA@
$LN6@DumpFlags:
	mov	ecx, DWORD PTR [esi]
	push	eax
	push	ecx
	push	OFFSET ??_C@_09GIDPMCMI@?$CK?5?$CF20s?5?$CFs?$AA@
	call	edi

; 25   : 		p++;

	add	esi, 8
	add	esp, 12					; 0000000cH
	cmp	DWORD PTR [esi], 0
	jne	SHORT $LL2@DumpFlags
	pop	edi
$LN1@DumpFlags:
	pop	esi

; 26   : 	}
; 27   : }

	ret	0
?DumpFlags@CRenderDevice@@QAEXXZ ENDP			; CRenderDevice::DumpFlags
END
