; Listing generated by Microsoft (R) Optimizing Compiler Version 14.00.50727.762 

	TITLE	D:\CLEARSKY\sources\engine\xrEngine\Device_create.cpp
	.686P
	.XMM
	include listing.inc
	.model	flat

INCLUDELIB OLDNAMES

PUBLIC	??$xr_new@VCStats@@@@YAPAVCStats@@XZ		; xr_new<CStats>
PUBLIC	??_C@_07CJAIGBMI@?9gpu_sw?$AA@			; `string'
PUBLIC	??_C@_0M@HDMHJLNP@?9gpu_nopure?$AA@		; `string'
PUBLIC	??_C@_08BCPJOLJ@?9gpu_ref?$AA@			; `string'
PUBLIC	??_C@_0BK@KNICGKPM@Starting?5RENDER?5device?4?4?4?$AA@ ; `string'
PUBLIC	??_C@_0L@ICBKKMF@shaders?4xr?$AA@		; `string'
;	COMDAT ??_C@_0L@ICBKKMF@shaders?4xr?$AA@
CONST	SEGMENT
??_C@_0L@ICBKKMF@shaders?4xr?$AA@ DB 'shaders.xr', 00H	; `string'
CONST	ENDS
;	COMDAT ??_C@_0BK@KNICGKPM@Starting?5RENDER?5device?4?4?4?$AA@
CONST	SEGMENT
??_C@_0BK@KNICGKPM@Starting?5RENDER?5device?4?4?4?$AA@ DB 'Starting RENDE'
	DB	'R device...', 00H				; `string'
CONST	ENDS
;	COMDAT ??_C@_08BCPJOLJ@?9gpu_ref?$AA@
CONST	SEGMENT
??_C@_08BCPJOLJ@?9gpu_ref?$AA@ DB '-gpu_ref', 00H	; `string'
CONST	ENDS
;	COMDAT ??_C@_0M@HDMHJLNP@?9gpu_nopure?$AA@
CONST	SEGMENT
??_C@_0M@HDMHJLNP@?9gpu_nopure?$AA@ DB '-gpu_nopure', 00H ; `string'
CONST	ENDS
;	COMDAT ??_C@_07CJAIGBMI@?9gpu_sw?$AA@
CONST	SEGMENT
??_C@_07CJAIGBMI@?9gpu_sw?$AA@ DB '-gpu_sw', 00H	; `string'
_MACRO_ADD DB	0ecH
	DB	0afH
	DB	0caH
	DB	036H
	DB	0b7H
	DB	0dH
	DB	059H
	DB	0c1H
	DB	0faH
	DB	0a7H
	DB	071H
	DB	0b1H
	DB	023H
	DB	0c5H
	DB	08eH
	DB	065H
	DB	0d1H
	DB	036H
	DB	03H
	DB	012H
	DB	07aH
	DB	02cH
	DB	031H
	DB	03aH
	DB	03eH
	DB	014H
	DB	05dH
	DB	0d2H
	DB	0cH
	DB	051H
	DB	066H
	DB	0c7H
	DB	0e1H
	DB	060H
	DB	098H
	DB	06aH
	DB	016H
	DB	09dH
	DB	0a8H
	DB	080H
	DB	0e4H
	DB	07aH
	DB	094H
	DB	0b2H
	DB	0a2H
	DB	0f5H
	DB	06eH
	DB	027H
	DB	0f5H
	DB	0aH
	DB	03fH
	DB	0c5H
	DB	0ffH
	DB	020H
	DB	04cH
	DB	0b8H
	DB	0c5H
	DB	072H
	DB	071H
	DB	09aH
	DB	0a9H
	DB	0a0H
	DB	0ddH
	DB	04fH
	DB	068H
	DB	053H
	DB	0b1H
	DB	0faH
	DB	026H
	DB	014H
	DB	085H
	DB	094H
	DB	021H
	DB	014H
	DB	080H
	DB	05cH
	DB	070H
	DB	0baH
	DB	0c5H
	DB	0f6H
	DB	058H
	DB	0dbH
	DB	0aeH
	DB	0e5H
	DB	08dH
	DB	0cdH
	DB	0dcH
	DB	06aH
	DB	09cH
	DB	011H
	DB	0fcH
	DB	06dH
	DB	033H
	DB	017H
	DB	0fbH
	DB	0f2H
	DB	030H
	DB	0f2H
	DB	0e0H
	DB	0d3H
	DB	0e3H
	DB	092H
	DB	0acH
	DB	046H
	DB	0c7H
	DB	086H
	DB	09bH
	DB	06eH
	DB	074H
	DB	05dH
	DB	0b1H
	DB	05H
	DB	0b9H
	DB	0faH
	DB	075H
	DB	09fH
	DB	084H
	DB	057H
	DB	02aH
	DB	040H
	DB	072H
	DB	0e7H
	DB	0c9H
	DB	0cbH
_MACRO_SUB DB	0ecH
	DB	0afH
	DB	0caH
	DB	036H
	DB	0b7H
	DB	0dH
	DB	059H
	DB	0c1H
	DB	0faH
	DB	0a7H
	DB	071H
	DB	0b1H
	DB	023H
	DB	0c5H
	DB	08eH
	DB	065H
	DB	0d1H
	DB	036H
	DB	03H
	DB	012H
	DB	07aH
	DB	02cH
	DB	031H
	DB	03aH
	DB	03eH
	DB	014H
	DB	05dH
	DB	0d2H
	DB	0cH
	DB	051H
	DB	066H
	DB	0c7H
	DB	0e1H
	DB	060H
	DB	098H
	DB	06aH
	DB	016H
	DB	09dH
	DB	0a8H
	DB	080H
	DB	0e4H
	DB	07aH
	DB	094H
	DB	0b2H
	DB	0a2H
	DB	0f5H
	DB	06eH
	DB	027H
	DB	0f5H
	DB	0aH
	DB	03fH
	DB	0c5H
	DB	0ffH
	DB	020H
	DB	04cH
	DB	0b8H
	DB	0c5H
	DB	072H
	DB	071H
	DB	09aH
	DB	0a9H
	DB	0a0H
	DB	0ddH
	DB	04fH
	DB	068H
	DB	053H
	DB	0b1H
	DB	0faH
	DB	026H
	DB	014H
	DB	085H
	DB	094H
	DB	021H
	DB	014H
	DB	080H
	DB	05cH
	DB	070H
	DB	0baH
	DB	0c5H
	DB	0f6H
	DB	058H
	DB	0dbH
	DB	0aeH
	DB	0e5H
	DB	08dH
	DB	0cdH
	DB	0dcH
	DB	06aH
	DB	09cH
	DB	011H
	DB	0fcH
	DB	06dH
	DB	033H
	DB	017H
	DB	0fbH
	DB	0f2H
	DB	022H
	DB	0ecH
	DB	0b5H
	DB	0fdH
	DB	075H
	DB	0caH
	DB	02bH
	DB	055H
	DB	0ccH
	DB	02bH
	DB	0beH
	DB	0fcH
	DB	0c6H
	DB	073H
	DB	03cH
	DB	0efH
	DB	0fdH
	DB	0d7H
	DB	08bH
	DB	086H
	DB	069H
	DB	04aH
	DB	06aH
	DB	080H
	DB	0f0H
	DB	06eH
	DB	0a7H
	DB	09H
_MACRO_IF_EQL DB 0ecH
	DB	0afH
	DB	0aaH
	DB	093H
	DB	02dH
	DB	04H
	DB	0a3H
	DB	08fH
	DB	08aH
	DB	010H
	DB	067H
	DB	091H
	DB	0c9H
	DB	0e1H
	DB	0daH
	DB	02cH
	DB	031H
	DB	01dH
	DB	082H
	DB	0e2H
	DB	07H
	DB	0f9H
	DB	0e8H
	DB	024H
	DB	075H
	DB	0e9H
	DB	09dH
	DB	054H
	DB	070H
	DB	036H
	DB	04fH
	DB	0ffH
	DB	0ddH
	DB	04aH
	DB	082H
	DB	014H
	DB	088H
	DB	048H
	DB	074H
	DB	051H
	DB	03fH
	DB	0baH
	DB	051H
	DB	098H
	DB	0e9H
	DB	01dH
	DB	027H
	DB	0b1H
	DB	07bH
	DB	097H
	DB	027H
	DB	046H
	DB	09aH
	DB	046H
	DB	064H
	DB	0d4H
	DB	0efH
	DB	092H
	DB	0f0H
	DB	05H
	DB	05eH
	DB	079H
	DB	0edH
	DB	08dH
	DB	034H
	DB	052H
	DB	08aH
	DB	090H
	DB	0e9H
	DB	0baH
	DB	03cH
	DB	0beH
	DB	08bH
	DB	0e5H
	DB	0caH
	DB	028H
	DB	0b0H
	DB	092H
	DB	0deH
	DB	04cH
	DB	0d8H
	DB	031H
	DB	0ceH
	DB	0ceH
	DB	08eH
	DB	022H
	DB	078H
	DB	0f8H
	DB	07cH
	DB	0ceH
	DB	0f6H
	DB	04eH
	DB	084H
	DB	0d0H
	DB	0eeH
	DB	0b5H
	DB	0bH
	DB	08eH
	DB	053H
	DB	0d5H
	DB	0ceH
	DB	07cH
	DB	04fH
	DB	02fH
	DB	0faH
	DB	0f6H
	DB	0bfH
	DB	0c8H
	DB	070H
	DB	05aH
	DB	0eH
	DB	00H
	DB	0dbH
	DB	093H
	DB	044H
	DB	083H
	DB	0f4H
	DB	0fcH
	DB	0ebH
	DB	02fH
	DB	031H
	DB	01dH
	DB	02H
	DB	023H
	DB	09eH
	DB	08aH
	DB	0b5H
	DB	013H
	DB	068H
	DB	048H
	DB	03dH
	DB	076H
	DB	01H
	DB	0feH
	DB	040H
	DB	078H
	DB	02cH
	DB	0b7H
	DB	09H
	DB	039H
	DB	0cfH
	DB	0a4H
	DB	058H
	DB	087H
	DB	072H
	DB	0f8H
	DB	0a0H
	DB	0bcH
	DB	0e9H
	DB	0a3H
	DB	0d0H
	DB	0c0H
	DB	078H
	DB	080H
	DB	0faH
	DB	0dcH
	DB	05H
	DB	09aH
	DB	0cH
	DB	040H
	DB	09cH
	DB	0b6H
	DB	062H
	DB	0d4H
	DB	06eH
	DB	0a0H
	DB	018H
	DB	09aH
	DB	0eeH
	DB	085H
	DB	04aH
	DB	0b0H
	DB	063H
	DB	0d3H
	DB	02aH
	DB	084H
	DB	0caH
	DB	03cH
	DB	055H
	DB	0a9H
	DB	0d2H
	DB	018H
	DB	0aeH
	DB	04dH
	DB	040H
	DB	080H
	DB	0e5H
	DB	07aH
	DB	095H
	DB	033H
	DB	090H
	DB	0f2H
	DB	0f6H
	DB	06H
	DB	0d9H
	DB	065H
	DB	04dH
	DB	02bH
	DB	095H
	DB	029H
	DB	02H
	DB	0b9H
	DB	0f9H
	DB	0a6H
	DB	05H
	DB	056H
	DB	013H
	DB	02bH
	DB	09fH
	DB	069H
	DB	0faH
	DB	0f1H
	DB	0bbH
	DB	028H
	DB	0a6H
	DB	071H
	DB	040H
	DB	011H
	DB	0d7H
	DB	086H
	DB	04H
	DB	0eeH
	DB	0b6H
	DB	0eeH
	DB	0baH
	DB	0c6H
	DB	045H
	DB	07H
	DB	075H
	DB	0ecH
	DB	0c5H
	DB	0d0H
	DB	07dH
	DB	08cH
	DB	0c5H
	DB	051H
	DB	063H
	DB	0b4H
	DB	0eaH
	DB	051H
	DB	063H
	DB	0b4H
	DB	0eaH
	DB	0d6H
	DB	061H
	DB	0b1H
	DB	0e1H
	DB	0bfH
	DB	0d4H
	DB	05eH
	DB	0adH
	DB	07dH
	DB	0d5H
	DB	03H
	DB	053H
	DB	050H
	DB	09cH
	DB	068H
	DB	01bH
	DB	0a5H
	DB	0b4H
	DB	0f8H
	DB	066H
	DB	01eH
	DB	0c6H
	DB	076H
	DB	019H
	DB	0a9H
	DB	02fH
	DB	054H
	DB	0e6H
	DB	0c1H
	DB	041H
	DB	047H
	DB	0f1H
	DB	01aH
	DB	037H
	DB	047H
	DB	078H
	DB	0c5H
	DB	040H
	DB	07aH
	DB	028H
	DB	0a6H
	DB	0f1H
	DB	044H
	DB	014H
	DB	08eH
	DB	088H
	DB	015H
	DB	0aeH
	DB	0f0H
	DB	091H
	DB	0fH
	DB	06aH
	DB	0e3H
	DB	09dH
	DB	09bH
	DB	071H
	DB	099H
	DB	02H
	DB	034H
	DB	0e8H
	DB	05fH
	DB	02H
	DB	0b5H
	DB	06fH
	DB	045H
	DB	02aH
	DB	05aH
	DB	0efH
	DB	04bH
	DB	0d2H
	DB	07aH
	DB	0efH
	DB	04bH
	DB	053H
	DB	058H
	DB	073H
	DB	0a4H
	DB	01aH
	DB	01bH
	DB	015H
	DB	0e8H
	DB	04H
	DB	0c2H
	DB	0c3H
	DB	065H
	DB	0a5H
	DB	0bH
	DB	016H
	DB	0ccH
	DB	026H
	DB	0f0H
	DB	026H
	DB	0dbH
	DB	0caH
	DB	04aH
	DB	037H
	DB	03H
	DB	079H
	DB	064H
	DB	0dH
	DB	02aH
	DB	0baH
	DB	057H
	DB	061H
	DB	0bdH
	DB	0fcH
	DB	0e7H
	DB	0dH
	DB	0cdH
	DB	072H
	DB	0f9H
	DB	0deH
	DB	0abH
	DB	087H
	DB	054H
	DB	08fH
	DB	0eH
	DB	026H
	DB	0d5H
	DB	08aH
	DB	078H
	DB	015H
	DB	06cH
	DB	04bH
	DB	0dbH
	DB	015H
	DB	06cH
	DB	04bH
	DB	0dbH
	DB	015H
	DB	06cH
	DB	04bH
	DB	0dbH
	DB	095H
	DB	012H
	DB	0f0H
	DB	0f9H
	DB	01dH
	DB	010H
	DB	09fH
	DB	0c3H
	DB	0a8H
	DB	084H
	DB	0b0H
	DB	047H
	DB	058H
	DB	067H
	DB	094H
	DB	058H
	DB	06dH
	DB	08aH
	DB	0feH
	DB	064H
	DB	0daH
	DB	071H
	DB	0cfH
	DB	02dH
	DB	0daH
	DB	0d3H
	DB	06dH
	DB	083H
	DB	0e2H
	DB	07H
	DB	01fH
	DB	047H
	DB	0b7H
	DB	01bH
	DB	0afH
	DB	088H
	DB	021H
	DB	0deH
	DB	08cH
	DB	098H
	DB	052H
	DB	0a8H
	DB	070H
	DB	032H
	DB	092H
	DB	080H
	DB	01bH
	DB	00H
	DB	0fdH
	DB	049H
	DB	032H
	DB	0f0H
	DB	01eH
	DB	0b8H
	DB	0dbH
	DB	065H
	DB	033H
	DB	093H
	DB	093H
	DB	08eH
	DB	0a0H
	DB	097H
	DB	0efH
	DB	0a7H
	DB	0e3H
	DB	0d2H
	DB	084H
	DB	049H
	DB	02cH
	DB	0e0H
	DB	0d9H
	DB	0c0H
	DB	059H
	DB	0f8H
	DB	0cdH
	DB	062H
	DB	083H
	DB	0efH
	DB	07bH
	DB	055H
	DB	05dH
	DB	06bH
	DB	07fH
PUBLIC	?ConnectToRender@CRenderDevice@@QAEXXZ		; CRenderDevice::ConnectToRender
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrengine\device_create.cpp
;	COMDAT ?ConnectToRender@CRenderDevice@@QAEXXZ
_TEXT	SEGMENT
?ConnectToRender@CRenderDevice@@QAEXXZ PROC		; CRenderDevice::ConnectToRender, COMDAT
; _this$ = ecx

; 158  : {

	push	esi
	mov	esi, ecx

; 159  : 	if (!m_pRender)

	cmp	DWORD PTR [esi+868], 0
	jne	SHORT $LN1@ConnectToR

; 160  : 		m_pRender			= RenderFactory->CreateRenderDeviceRender();

	mov	eax, DWORD PTR __imp_?RenderFactory@@3PAVIRenderFactory@@A
	mov	ecx, DWORD PTR [eax]
	mov	edx, DWORD PTR [ecx]
	mov	eax, DWORD PTR [edx+32]
	call	eax
	mov	DWORD PTR [esi+868], eax
$LN1@ConnectToR:
	pop	esi

; 161  : }

	ret	0
?ConnectToRender@CRenderDevice@@QAEXXZ ENDP		; CRenderDevice::ConnectToRender
PUBLIC	?_SetupStates@CRenderDevice@@AAEXXZ		; CRenderDevice::_SetupStates
; Function compile flags: /Ogtpy
;	COMDAT ?_SetupStates@CRenderDevice@@AAEXXZ
_TEXT	SEGMENT
?_SetupStates@CRenderDevice@@AAEXXZ PROC		; CRenderDevice::_SetupStates, COMDAT
; _this$ = ecx

; 35   : 	// General Render States
; 36   : 	mView.identity			();

	xorps	xmm0, xmm0
	movss	xmm1, DWORD PTR __real@3f800000
	movss	DWORD PTR [ecx+96], xmm1
	movss	DWORD PTR [ecx+100], xmm0
	movss	DWORD PTR [ecx+104], xmm0
	movss	DWORD PTR [ecx+108], xmm0
	movss	DWORD PTR [ecx+112], xmm0
	movss	DWORD PTR [ecx+116], xmm1
	movss	DWORD PTR [ecx+120], xmm0
	movss	DWORD PTR [ecx+124], xmm0
	movss	DWORD PTR [ecx+128], xmm0
	movss	DWORD PTR [ecx+132], xmm0
	movss	DWORD PTR [ecx+136], xmm1
	movss	DWORD PTR [ecx+140], xmm0
	movss	DWORD PTR [ecx+144], xmm0
	movss	DWORD PTR [ecx+148], xmm0
	movss	DWORD PTR [ecx+152], xmm0
	movss	DWORD PTR [ecx+156], xmm1

; 37   : 	mProject.identity		();

	movss	DWORD PTR [ecx+160], xmm1
	movss	DWORD PTR [ecx+164], xmm0
	movss	DWORD PTR [ecx+168], xmm0
	movss	DWORD PTR [ecx+172], xmm0
	movss	DWORD PTR [ecx+176], xmm0
	movss	DWORD PTR [ecx+180], xmm1
	movss	DWORD PTR [ecx+184], xmm0
	movss	DWORD PTR [ecx+188], xmm0
	movss	DWORD PTR [ecx+192], xmm0
	movss	DWORD PTR [ecx+196], xmm0
	movss	DWORD PTR [ecx+200], xmm1
	movss	DWORD PTR [ecx+204], xmm0
	movss	DWORD PTR [ecx+208], xmm0
	movss	DWORD PTR [ecx+212], xmm0
	movss	DWORD PTR [ecx+216], xmm0
	movss	DWORD PTR [ecx+220], xmm1

; 38   : 	mFullTransform.identity	();

	movss	DWORD PTR [ecx+224], xmm1
	movss	DWORD PTR [ecx+228], xmm0
	movss	DWORD PTR [ecx+232], xmm0
	movss	DWORD PTR [ecx+236], xmm0
	movss	DWORD PTR [ecx+240], xmm0
	movss	DWORD PTR [ecx+244], xmm1
	movss	DWORD PTR [ecx+248], xmm0
	movss	DWORD PTR [ecx+252], xmm0
	movss	DWORD PTR [ecx+256], xmm0
	movss	DWORD PTR [ecx+260], xmm0
	movss	DWORD PTR [ecx+264], xmm1
	movss	DWORD PTR [ecx+268], xmm0
	movss	DWORD PTR [ecx+272], xmm0
	movss	DWORD PTR [ecx+276], xmm0
	movss	DWORD PTR [ecx+280], xmm0
	movss	DWORD PTR [ecx+284], xmm1

; 39   : 	vCameraPosition.set		(0,0,0);

	movss	DWORD PTR [ecx+48], xmm0
	movss	DWORD PTR [ecx+52], xmm0
	movss	DWORD PTR [ecx+56], xmm0

; 40   : 	vCameraDirection.set	(0,0,1);

	movss	DWORD PTR [ecx+60], xmm0
	movss	DWORD PTR [ecx+64], xmm0
	movss	DWORD PTR [ecx+68], xmm1

; 41   : 	vCameraTop.set			(0,1,0);

	movss	DWORD PTR [ecx+72], xmm0
	movss	DWORD PTR [ecx+76], xmm1
	movss	DWORD PTR [ecx+80], xmm0

; 42   : 	vCameraRight.set		(1,0,0);

	movss	DWORD PTR [ecx+84], xmm1
	movss	DWORD PTR [ecx+88], xmm0
	movss	DWORD PTR [ecx+92], xmm0

; 43   : 
; 44   : 	m_pRender->SetupStates();

	mov	ecx, DWORD PTR [ecx+868]
	mov	eax, DWORD PTR [ecx]
	mov	edx, DWORD PTR [eax+40]
	jmp	edx
?_SetupStates@CRenderDevice@@AAEXXZ ENDP		; CRenderDevice::_SetupStates
_TEXT	ENDS
PUBLIC	?SetupGPU@@YAXPAVIRenderDeviceRender@@@Z	; SetupGPU
; Function compile flags: /Ogtpy
;	COMDAT ?SetupGPU@@YAXPAVIRenderDeviceRender@@@Z
_TEXT	SEGMENT
_pRender$ = 8						; size = 4
?SetupGPU@@YAXPAVIRenderDeviceRender@@@Z PROC		; SetupGPU, COMDAT

; 15   : {

	push	ebx

; 16   : 	// Command line
; 17   : 	char *lpCmdLine		= Core.Params;

	mov	ebx, DWORD PTR __imp_?Core@@3VxrCore@@A
	push	ebp

; 18   : 
; 19   : 	BOOL bForceGPU_SW;
; 20   : 	BOOL bForceGPU_NonPure;
; 21   : 	BOOL bForceGPU_REF;
; 22   : 
; 23   : 	if (strstr(lpCmdLine,"-gpu_sw")!=NULL)		bForceGPU_SW		= TRUE;

	mov	ebp, DWORD PTR __imp__strstr
	push	esi
	push	edi
	add	ebx, 1232				; 000004d0H
	push	OFFSET ??_C@_07CJAIGBMI@?9gpu_sw?$AA@
	push	ebx
	call	ebp
	mov	esi, eax
	neg	esi
	sbb	esi, esi

; 24   : 	else										bForceGPU_SW		= FALSE;
; 25   : 	if (strstr(lpCmdLine,"-gpu_nopure")!=NULL)	bForceGPU_NonPure	= TRUE;

	push	OFFSET ??_C@_0M@HDMHJLNP@?9gpu_nopure?$AA@
	push	ebx
	neg	esi
	call	ebp
	mov	edi, eax
	neg	edi
	sbb	edi, edi

; 26   : 	else										bForceGPU_NonPure	= FALSE;
; 27   : 	if (strstr(lpCmdLine,"-gpu_ref")!=NULL)		bForceGPU_REF		= TRUE;

	push	OFFSET ??_C@_08BCPJOLJ@?9gpu_ref?$AA@
	push	ebx
	neg	edi
	call	ebp

; 28   : 	else										bForceGPU_REF		= FALSE;
; 29   : 
; 30   : 	pRender->SetupGPU(bForceGPU_SW, bForceGPU_NonPure, bForceGPU_REF);

	mov	ecx, DWORD PTR _pRender$[esp+36]
	mov	edx, DWORD PTR [ecx]
	add	esp, 24					; 00000018H
	neg	eax
	sbb	eax, eax
	neg	eax
	push	eax
	mov	eax, DWORD PTR [edx+52]
	push	edi
	push	esi
	call	eax
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx

; 31   : }

	ret	0
?SetupGPU@@YAXPAVIRenderDeviceRender@@@Z ENDP		; SetupGPU
PUBLIC	?_Create@CRenderDevice@@AAEXPBD@Z		; CRenderDevice::_Create
; Function compile flags: /Ogtpy
;	COMDAT ?_Create@CRenderDevice@@AAEXPBD@Z
_TEXT	SEGMENT
_shName$ = 8						; size = 4
?_Create@CRenderDevice@@AAEXPBD@Z PROC			; CRenderDevice::_Create, COMDAT
; _this$ = ecx

; 114  : {

	push	esi
	mov	esi, ecx

; 115  : 	Memory.mem_compact			();

	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	call	DWORD PTR __imp_?mem_compact@xrMemory@@QAEXXZ

; 116  : 
; 117  : 	// after creation
; 118  : 	b_is_Ready					= TRUE;
; 119  : 	_SetupStates				();

	mov	ecx, esi
	mov	DWORD PTR [esi+16], 1
	call	?_SetupStates@CRenderDevice@@AAEXXZ	; CRenderDevice::_SetupStates

; 120  : 
; 121  : 	m_pRender->OnDeviceCreate(shName);

	mov	ecx, DWORD PTR [esi+868]
	mov	eax, DWORD PTR [ecx]
	mov	edx, DWORD PTR _shName$[esp]
	mov	eax, DWORD PTR [eax+44]
	push	edx
	call	eax

; 122  : 
; 123  : 	dwFrame						= 0;

	mov	DWORD PTR [esi+24], 0
	pop	esi

; 124  : }

	ret	4
?_Create@CRenderDevice@@AAEXPBD@Z ENDP			; CRenderDevice::_Create
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrcore\xrmemory_subst_msvc.h
;	COMDAT ??$xr_new@VCStats@@@@YAPAVCStats@@XZ
_TEXT	SEGMENT
??$xr_new@VCStats@@@@YAPAVCStats@@XZ PROC		; xr_new<CStats>, COMDAT

; 68   : 	T* ptr	= (T*)Memory.mem_alloc(sizeof(T));

	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	push	3536					; 00000dd0H
	call	DWORD PTR __imp_?mem_alloc@xrMemory@@QAEPAXI@Z

; 69   : 	return new (ptr) T();

	test	eax, eax
	je	SHORT $LN3@xr_new@37
	mov	ecx, eax
	jmp	??0CStats@@QAE@XZ			; CStats::CStats
$LN3@xr_new@37:
	xor	eax, eax

; 70   : }

	ret	0
??$xr_new@VCStats@@@@YAPAVCStats@@XZ ENDP		; xr_new<CStats>
PUBLIC	?Create@CRenderDevice@@QAEXXZ			; CRenderDevice::Create
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrengine\device_create.cpp
;	COMDAT ?Create@CRenderDevice@@QAEXXZ
_TEXT	SEGMENT
_fname$116194 = -520					; size = 520
?Create@CRenderDevice@@QAEXXZ PROC			; CRenderDevice::Create, COMDAT
; _this$ = ecx

; 164  : {

	sub	esp, 520				; 00000208H
	push	esi
	mov	esi, ecx

; 165  : 	SECUROM_MARKER_SECURITY_ON(4)

	DB	-21					; ffffffebH
	DB	14					; 0000000eH
	DB	77					; 0000004dH
	DB	97					; 00000061H
	DB	48					; 00000030H
	DB	87					; 00000057H
	DB	121					; 00000079H
	DB	71					; 00000047H
	DB	49					; 00000031H
	DB	107					; 0000006bH
	DB	109					; 0000006dH
	DB	17					; 00000011H
	DB	4
	DB	0
	DB	0
	DB	0

; 166  : 
; 167  : 	if (b_is_Ready)		return;		// prevent double call

	cmp	DWORD PTR [esi+16], 0
	jne	$LN3@Create@3

; 168  : 	Statistic			= xr_new<CStats>();

	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	push	3536					; 00000dd0H
	call	DWORD PTR __imp_?mem_alloc@xrMemory@@QAEPAXI@Z
	test	eax, eax
	je	SHORT $LN7@Create@3
	mov	ecx, eax
	call	??0CStats@@QAE@XZ			; CStats::CStats
	jmp	SHORT $LN8@Create@3
$LN7@Create@3:
	xor	eax, eax
$LN8@Create@3:

; 169  : 
; 170  : #ifdef	DEBUG
; 171  : cdb_clRAY		= &Statistic->clRAY;				// total: ray-testing
; 172  : cdb_clBOX		= &Statistic->clBOX;				// total: box query
; 173  : cdb_clFRUSTUM	= &Statistic->clFRUSTUM;			// total: frustum query
; 174  : cdb_bDebug		= &bDebug;
; 175  : #endif
; 176  : 
; 177  : 	if (!m_pRender)

	cmp	DWORD PTR [esi+868], 0
	mov	DWORD PTR [esi+920], eax
	jne	SHORT $LN1@Create@3

; 178  : 		m_pRender			= RenderFactory->CreateRenderDeviceRender();

	mov	eax, DWORD PTR __imp_?RenderFactory@@3PAVIRenderFactory@@A
	mov	ecx, DWORD PTR [eax]
	mov	edx, DWORD PTR [ecx]
	mov	eax, DWORD PTR [edx+32]
	call	eax
	mov	DWORD PTR [esi+868], eax
$LN1@Create@3:

; 179  : 	SetupGPU(m_pRender);

	mov	ecx, DWORD PTR [esi+868]
	push	ecx
	call	?SetupGPU@@YAXPAVIRenderDeviceRender@@@Z ; SetupGPU

; 180  : 	Log					("Starting RENDER device...");

	push	OFFSET ??_C@_0BK@KNICGKPM@Starting?5RENDER?5device?4?4?4?$AA@
	call	DWORD PTR __imp_?Log@@YAXPBD@Z

; 181  : 
; 182  : #ifdef _EDITOR
; 183  : 	psCurrentVidMode[0]	= dwWidth;
; 184  : 	psCurrentVidMode[1] = dwHeight;
; 185  : #endif // #ifdef _EDITOR
; 186  : 
; 187  : 	fFOV				= 90.f;

	movss	xmm0, DWORD PTR __real@42b40000

; 188  : 	fASPECT				= 1.f;
; 189  : 	m_pRender->Create	(
; 190  : 		m_hWnd,
; 191  : 		dwWidth,
; 192  : 		dwHeight,
; 193  : 		fWidth_2,
; 194  : 		fHeight_2,
; 195  : #ifdef INGAME_EDITOR
; 196  : 		editor() ? false :
; 197  : #endif // #ifdef INGAME_EDITOR
; 198  : 		true
; 199  : 	);

	mov	ecx, DWORD PTR [esi+868]
	add	esp, 8
	push	1
	lea	eax, DWORD PTR [esi+864]
	push	eax
	lea	eax, DWORD PTR [esi+860]
	push	eax
	lea	eax, DWORD PTR [esi+8]
	push	eax
	movss	DWORD PTR [esi+492], xmm0
	movss	xmm0, DWORD PTR __real@3f800000
	lea	eax, DWORD PTR [esi+4]
	push	eax
	mov	eax, DWORD PTR [esi+760]
	movss	DWORD PTR [esi+496], xmm0
	mov	edx, DWORD PTR [ecx]
	mov	edx, DWORD PTR [edx+48]
	push	eax
	call	edx

; 200  : 
; 201  : 	string_path			fname; 
; 202  : 	FS.update_path		(fname,"$game_data$","shaders.xr");

	mov	ecx, DWORD PTR __imp_?xr_FS@@3PAVCLocatorAPI@@A
	mov	ecx, DWORD PTR [ecx]
	push	OFFSET ??_C@_0L@ICBKKMF@shaders?4xr?$AA@
	push	OFFSET ??_C@_0M@FDPKAAFL@$game_data$?$AA@
	lea	eax, DWORD PTR _fname$116194[esp+532]
	push	eax
	call	DWORD PTR __imp_?update_path@CLocatorAPI@@QAEPBDAAY0CAI@DPBD1@Z

; 203  : 
; 204  : 	//////////////////////////////////////////////////////////////////////////
; 205  : 	_Create				(fname);

	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	call	DWORD PTR __imp_?mem_compact@xrMemory@@QAEXXZ
	mov	ecx, esi
	mov	DWORD PTR [esi+16], 1
	call	?_SetupStates@CRenderDevice@@AAEXXZ	; CRenderDevice::_SetupStates
	mov	ecx, DWORD PTR [esi+868]
	mov	edx, DWORD PTR [ecx]
	mov	edx, DWORD PTR [edx+44]
	lea	eax, DWORD PTR _fname$116194[esp+524]
	push	eax
	call	edx

; 206  : 
; 207  : 	PreCache			(0, false, false);

	push	0
	push	0
	push	0
	mov	ecx, esi
	mov	DWORD PTR [esi+24], 0
	call	?PreCache@CRenderDevice@@QAEXI_N0@Z	; CRenderDevice::PreCache

; 208  : 
; 209  : 	SECUROM_MARKER_SECURITY_OFF(4)

	DB	-21					; ffffffebH
	DB	14					; 0000000eH
	DB	77					; 0000004dH
	DB	97					; 00000061H
	DB	48					; 00000030H
	DB	87					; 00000057H
	DB	121					; 00000079H
	DB	71					; 00000047H
	DB	49					; 00000031H
	DB	107					; 0000006bH
	DB	109					; 0000006dH
	DB	18					; 00000012H
	DB	4
	DB	0
	DB	0
	DB	0
$LN3@Create@3:
	pop	esi

; 210  : }

	add	esp, 520				; 00000208H
	ret	0
?Create@CRenderDevice@@QAEXXZ ENDP			; CRenderDevice::Create
END
