; Listing generated by Microsoft (R) Optimizing Compiler Version 14.00.50727.762 

	TITLE	D:\CLEARSKY\sources\engine\xrEngine\cf_dynamic_mesh.cpp
	.686P
	.XMM
	include listing.inc
	.model	flat

INCLUDELIB OLDNAMES

PUBLIC	?_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@3@@Z ; CCF_DynamicMesh::_RayQuery
PUBLIC	??_ECCF_DynamicMesh@@UAEPAXI@Z			; CCF_DynamicMesh::`vector deleting destructor'
PUBLIC	??_R4CCF_DynamicMesh@@6B@			; CCF_DynamicMesh::`RTTI Complete Object Locator'
PUBLIC	??_R3CCF_DynamicMesh@@8				; CCF_DynamicMesh::`RTTI Class Hierarchy Descriptor'
PUBLIC	??_R2CCF_DynamicMesh@@8				; CCF_DynamicMesh::`RTTI Base Class Array'
PUBLIC	??_R1A@?0A@EA@CCF_DynamicMesh@@8		; CCF_DynamicMesh::`RTTI Base Class Descriptor at (0,-1,0,64)'
PUBLIC	??_R0?AVCCF_DynamicMesh@@@8			; CCF_DynamicMesh `RTTI Type Descriptor'
PUBLIC	?size@?$vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@stlp_std@@QBEIXZ ; stlp_std::vector<collide::rq_result,xalloc<collide::rq_result> >::size
PUBLIC	?size@?$xr_vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@@QBEIXZ ; xr_vector<collide::rq_result,xalloc<collide::rq_result> >::size
PUBLIC	?r_count@rq_results@collide@@QAEHXZ		; collide::rq_results::r_count
PUBLIC	?r_results@rq_results@collide@@QAEAAV?$xr_vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@@XZ ; collide::rq_results::r_results
PUBLIC	??$__find_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@priv@stlp_std@@YAPAUrq_result@collide@@PAU23@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@3@AAVrq_results@3@@Z@ABUrandom_access_iterator_tag@1@@Z ; stlp_std::priv::__find_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
PUBLIC	??$find_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z ; stlp_std::find_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
PUBLIC	??$remove_copy_if@PAUrq_result@collide@@PAU12@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@00Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z ; stlp_std::remove_copy_if<collide::rq_result *,collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
PUBLIC	??$remove_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z ; stlp_std::remove_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
PUBLIC	??_7CCF_DynamicMesh@@6B@			; CCF_DynamicMesh::`vftable'
;	COMDAT ??_7CCF_DynamicMesh@@6B@
CONST	SEGMENT
??_7CCF_DynamicMesh@@6B@ DD FLAT:??_R4CCF_DynamicMesh@@6B@ ; CCF_DynamicMesh::`vftable'
	DD	FLAT:??_ECCF_DynamicMesh@@UAEPAXI@Z
	DD	FLAT:?_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@3@@Z
CONST	ENDS
;	COMDAT ??_R4CCF_DynamicMesh@@6B@
rdata$r	SEGMENT
??_R4CCF_DynamicMesh@@6B@ DD 00H			; CCF_DynamicMesh::`RTTI Complete Object Locator'
	DD	00H
	DD	00H
	DD	FLAT:??_R0?AVCCF_DynamicMesh@@@8
	DD	FLAT:??_R3CCF_DynamicMesh@@8
rdata$r	ENDS
;	COMDAT ??_R3CCF_DynamicMesh@@8
rdata$r	SEGMENT
??_R3CCF_DynamicMesh@@8 DD 00H				; CCF_DynamicMesh::`RTTI Class Hierarchy Descriptor'
	DD	00H
	DD	03H
	DD	FLAT:??_R2CCF_DynamicMesh@@8
rdata$r	ENDS
;	COMDAT ??_R2CCF_DynamicMesh@@8
rdata$r	SEGMENT
??_R2CCF_DynamicMesh@@8 DD FLAT:??_R1A@?0A@EA@CCF_DynamicMesh@@8 ; CCF_DynamicMesh::`RTTI Base Class Array'
	DD	FLAT:??_R1A@?0A@EA@CCF_Skeleton@@8
	DD	FLAT:??_R1A@?0A@EA@ICollisionForm@@8
rdata$r	ENDS
;	COMDAT ??_R1A@?0A@EA@CCF_DynamicMesh@@8
rdata$r	SEGMENT
??_R1A@?0A@EA@CCF_DynamicMesh@@8 DD FLAT:??_R0?AVCCF_DynamicMesh@@@8 ; CCF_DynamicMesh::`RTTI Base Class Descriptor at (0,-1,0,64)'
	DD	02H
	DD	00H
	DD	0ffffffffH
	DD	00H
	DD	040H
	DD	FLAT:??_R3CCF_DynamicMesh@@8
rdata$r	ENDS
;	COMDAT ??_R0?AVCCF_DynamicMesh@@@8
_DATA	SEGMENT
??_R0?AVCCF_DynamicMesh@@@8 DD FLAT:??_7type_info@@6B@	; CCF_DynamicMesh `RTTI Type Descriptor'
	DD	00H
	DB	'.?AVCCF_DynamicMesh@@', 00H
PUBLIC	??0spick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@4@@Z@QAE@0ABVCObject@@AAVIKinematics@@@Z ; `CCF_DynamicMesh::_RayQuery'::`8'::spick::spick
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrengine\cf_dynamic_mesh.cpp
;	COMDAT ??0spick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@4@@Z@QAE@0ABVCObject@@AAVIKinematics@@@Z
_TEXT	SEGMENT
_K_$ = 8						; size = 4
??0spick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@4@@Z@QAE@0ABVCObject@@AAVIKinematics@@@Z PROC ; `CCF_DynamicMesh::_RayQuery'::`8'::spick::spick, COMDAT
; _this$ = eax
; _Q_$ = ecx
; _obj_$ = edx

; 32   : 		 {

	mov	DWORD PTR [eax], ecx
	mov	ecx, DWORD PTR _K_$[esp-4]
	mov	DWORD PTR [eax+4], edx
	mov	DWORD PTR [eax+8], ecx

; 33   : 
; 34   : 		 }

	ret	4
??0spick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@4@@Z@QAE@0ABVCObject@@AAVIKinematics@@@Z ENDP ; `CCF_DynamicMesh::_RayQuery'::`8'::spick::spick
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrcdb\xr_collide_defs.h
_TEXT	ENDS
;	COMDAT ?r_results@rq_results@collide@@QAEAAV?$xr_vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@@XZ
_TEXT	SEGMENT
?r_results@rq_results@collide@@QAEAAV?$xr_vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@@XZ PROC ; collide::rq_results::r_results, COMDAT
; _this$ = eax

; 136  : 		IC rqVec		&r_results		()	{ return results; }

	ret	0
?r_results@rq_results@collide@@QAEAAV?$xr_vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@@XZ ENDP ; collide::rq_results::r_results
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\sdk\include\stlport\stl\pointers\_vector.h
;	COMDAT ?size@?$vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@stlp_std@@QBEIXZ
_TEXT	SEGMENT
?size@?$vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@stlp_std@@QBEIXZ PROC ; stlp_std::vector<collide::rq_result,xalloc<collide::rq_result> >::size, COMDAT
; _this$ = eax

; 89   :   size_type size() const        { return _M_impl.size(); }

	mov	ecx, DWORD PTR [eax+4]
	sub	ecx, DWORD PTR [eax]
	mov	eax, 715827883				; 2aaaaaabH
	imul	ecx
	sar	edx, 1
	mov	eax, edx
	shr	eax, 31					; 0000001fH
	add	eax, edx
	ret	0
?size@?$vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@stlp_std@@QBEIXZ ENDP ; stlp_std::vector<collide::rq_result,xalloc<collide::rq_result> >::size
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrcore\_stl_extensions.h
_TEXT	ENDS
;	COMDAT ?size@?$xr_vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@@QBEIXZ
_TEXT	SEGMENT
?size@?$xr_vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@@QBEIXZ PROC ; xr_vector<collide::rq_result,xalloc<collide::rq_result> >::size, COMDAT
; _this$ = eax

; 126  : 	u32		size				() const							{ return (u32)inherited::size();} 

	mov	ecx, DWORD PTR [eax+4]
	sub	ecx, DWORD PTR [eax]
	mov	eax, 715827883				; 2aaaaaabH
	imul	ecx
	sar	edx, 1
	mov	eax, edx
	shr	eax, 31					; 0000001fH
	add	eax, edx
	ret	0
?size@?$xr_vector@Urq_result@collide@@V?$xalloc@Urq_result@collide@@@@@@QBEIXZ ENDP ; xr_vector<collide::rq_result,xalloc<collide::rq_result> >::size
_TEXT	ENDS
PUBLIC	??Rspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@4@@Z@QAE_NAAUrq_result@4@@Z ; `CCF_DynamicMesh::_RayQuery'::`8'::spick::operator()
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrengine\cf_dynamic_mesh.cpp
;	COMDAT ??Rspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@4@@Z@QAE_NAAUrq_result@4@@Z
_TEXT	SEGMENT
_br$ = -52						; size = 52
??Rspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@4@@Z@QAE_NAAUrq_result@4@@Z PROC ; `CCF_DynamicMesh::_RayQuery'::`8'::spick::operator(), COMDAT
; _this$ = edx
; _r$ = esi

; 38   : 			IKinematics::pick_result br;
; 39   : 			VERIFY( r.O == &obj );
; 40   : 			bool  res = K.PickBone( obj.XFORM(), br, Q.range, Q.start, Q.dir, (u16) r.element ) ;

	mov	ecx, DWORD PTR [edx+8]
	mov	eax, DWORD PTR [edx+4]
	mov	edx, DWORD PTR [edx]
	fld	DWORD PTR [edx+24]
	sub	esp, 52					; 00000034H
	push	ebx
	movzx	ebx, WORD PTR [esi+8]
	push	edi
	mov	edi, DWORD PTR [ecx]
	push	ebx
	lea	ebx, DWORD PTR [edx+12]
	push	ebx
	push	edx
	push	ecx
	lea	edx, DWORD PTR _br$[esp+76]
	add	eax, 80					; 00000050H
	fstp	DWORD PTR [esp]
	push	edx
	push	eax
	mov	eax, DWORD PTR [edi+8]
	call	eax

; 41   : 			if(res)

	test	al, al
	pop	edi
	pop	ebx
	je	SHORT $LN1@operator@21

; 42   : 			{
; 43   : 				r.range = br.dist;

	movss	xmm0, DWORD PTR _br$[esp+64]
	movss	DWORD PTR [esi+4], xmm0
$LN1@operator@21:

; 44   : 			}
; 45   : #if	0
; 46   : 			if(res)
; 47   : 			{
; 48   : 				ph_debug_render->open_cashed_draw();
; 49   : 				ph_debug_render->draw_tri(  br.tri[0], br.tri[1], br.tri[2], D3DCOLOR_ARGB(50, 255,0,0), 0 );
; 50   : 				ph_debug_render->close_cashed_draw(50000);
; 51   : 			}
; 52   : #endif
; 53   : 			return !res;

	xor	ecx, ecx
	test	al, al
	sete	cl
	mov	al, cl

; 54   : 		}

	add	esp, 52					; 00000034H
	ret	0
??Rspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@4@@Z@QAE_NAAUrq_result@4@@Z ENDP ; `CCF_DynamicMesh::_RayQuery'::`8'::spick::operator()
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrcdb\xr_collide_defs.h
_TEXT	ENDS
;	COMDAT ?r_count@rq_results@collide@@QAEHXZ
_TEXT	SEGMENT
?r_count@rq_results@collide@@QAEHXZ PROC		; collide::rq_results::r_count, COMDAT
; _this$ = eax

; 131  : 		IC int			r_count			()	{ return results.size();	}

	mov	ecx, DWORD PTR [eax+4]
	sub	ecx, DWORD PTR [eax]
	mov	eax, 715827883				; 2aaaaaabH
	imul	ecx
	sar	edx, 1
	mov	eax, edx
	shr	eax, 31					; 0000001fH
	add	eax, edx
	ret	0
?r_count@rq_results@collide@@QAEHXZ ENDP		; collide::rq_results::r_count
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\sdk\include\stlport\stl\_algobase.c
;	COMDAT ??$__find_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@priv@stlp_std@@YAPAUrq_result@collide@@PAU23@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@3@AAVrq_results@3@@Z@ABUrandom_access_iterator_tag@1@@Z
_TEXT	SEGMENT
___trip_count$ = -56					; size = 4
_br$304109 = -52					; size = 52
_br$304088 = -52					; size = 52
_br$304067 = -52					; size = 52
_br$304046 = -52					; size = 52
_br$304025 = -52					; size = 52
_br$304004 = -52					; size = 52
_br$303989 = -52					; size = 52
___last$ = 8						; size = 4
___pred$ = 12						; size = 12
??$__find_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@priv@stlp_std@@YAPAUrq_result@collide@@PAU23@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@3@AAVrq_results@3@@Z@ABUrandom_access_iterator_tag@1@@Z PROC ; stlp_std::priv::__find_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>, COMDAT
; ___first$ = eax

; 154  :                                               const random_access_iterator_tag &) {

	sub	esp, 56					; 00000038H

; 155  :   _STLP_DIFFERENCE_TYPE(_RandomAccessIter) __trip_count = (__last - __first) >> 2;

	mov	ecx, DWORD PTR ___last$[esp+52]
	push	ebx

; 156  : 
; 157  :   for ( ; __trip_count > 0 ; --__trip_count) {

	mov	ebx, DWORD PTR ___pred$[esp+64]
	push	ebp
	push	esi
	mov	esi, eax
	sub	ecx, esi
	mov	eax, 715827883				; 2aaaaaabH
	imul	ecx
	sar	edx, 1
	mov	eax, edx
	shr	eax, 31					; 0000001fH
	add	eax, edx
	sar	eax, 2
	test	eax, eax
	push	edi
	mov	edi, DWORD PTR ___pred$[esp+68]
	mov	DWORD PTR ___trip_count$[esp+72], eax
	jle	$LN14@find_if@4
	lea	ebp, DWORD PTR [edi+12]
	npad	7
$LL105@find_if@4:

; 158  :     if (__pred(*__first)) return __first;

	movzx	eax, WORD PTR [esi+8]
	fld	DWORD PTR [edi+24]
	mov	edx, DWORD PTR [ebx]
	mov	edx, DWORD PTR [edx+8]
	push	eax
	mov	eax, DWORD PTR ___pred$[esp+76]
	push	ebp
	push	edi
	push	ecx
	lea	ecx, DWORD PTR _br$303989[esp+88]
	add	eax, 80					; 00000050H
	fstp	DWORD PTR [esp]
	push	ecx
	push	eax
	mov	ecx, ebx
	call	edx
	test	al, al
	je	$LN97@find_if@4
	movss	xmm0, DWORD PTR _br$303989[esp+84]
	movss	DWORD PTR [esi+4], xmm0

; 159  :     ++__first;
; 160  : 
; 161  :     if (__pred(*__first)) return __first;

	fld	DWORD PTR [edi+24]
	movzx	eax, WORD PTR [esi+20]
	mov	edx, DWORD PTR [ebx]
	mov	edx, DWORD PTR [edx+8]
	push	eax
	mov	eax, DWORD PTR ___pred$[esp+76]
	add	esi, 12					; 0000000cH
	push	ebp
	push	edi
	push	ecx
	lea	ecx, DWORD PTR _br$304004[esp+88]
	add	eax, 80					; 00000050H
	fstp	DWORD PTR [esp]
	push	ecx
	push	eax
	mov	ecx, ebx
	call	edx
	test	al, al
	je	$LN97@find_if@4
	movss	xmm0, DWORD PTR _br$304004[esp+84]
	movss	DWORD PTR [esi+4], xmm0

; 162  :     ++__first;
; 163  : 
; 164  :     if (__pred(*__first)) return __first;

	fld	DWORD PTR [edi+24]
	movzx	eax, WORD PTR [esi+20]
	mov	edx, DWORD PTR [ebx]
	mov	edx, DWORD PTR [edx+8]
	push	eax
	mov	eax, DWORD PTR ___pred$[esp+76]
	add	esi, 12					; 0000000cH
	push	ebp
	push	edi
	push	ecx
	lea	ecx, DWORD PTR _br$304025[esp+88]
	add	eax, 80					; 00000050H
	fstp	DWORD PTR [esp]
	push	ecx
	push	eax
	mov	ecx, ebx
	call	edx
	test	al, al
	je	$LN97@find_if@4
	movss	xmm0, DWORD PTR _br$304025[esp+84]
	movss	DWORD PTR [esi+4], xmm0

; 165  :     ++__first;
; 166  : 
; 167  :     if (__pred(*__first)) return __first;

	fld	DWORD PTR [edi+24]
	movzx	eax, WORD PTR [esi+20]
	mov	edx, DWORD PTR [ebx]
	mov	edx, DWORD PTR [edx+8]
	push	eax
	mov	eax, DWORD PTR ___pred$[esp+76]
	add	esi, 12					; 0000000cH
	push	ebp
	push	edi
	push	ecx
	lea	ecx, DWORD PTR _br$304046[esp+88]
	add	eax, 80					; 00000050H
	fstp	DWORD PTR [esp]
	push	ecx
	push	eax
	mov	ecx, ebx
	call	edx
	test	al, al
	je	$LN97@find_if@4
	mov	eax, DWORD PTR ___trip_count$[esp+72]
	movss	xmm0, DWORD PTR _br$304046[esp+84]
	sub	eax, 1
	movss	DWORD PTR [esi+4], xmm0

; 168  :     ++__first;

	add	esi, 12					; 0000000cH
	test	eax, eax
	mov	DWORD PTR ___trip_count$[esp+72], eax
	jg	$LL105@find_if@4
$LN14@find_if@4:

; 169  :   }
; 170  : 
; 171  :   switch(__last - __first) {

	mov	ecx, DWORD PTR ___last$[esp+68]
	sub	ecx, esi
	mov	eax, 715827883				; 2aaaaaabH
	imul	ecx
	sar	edx, 1
	mov	eax, edx
	shr	eax, 31					; 0000001fH
	add	eax, edx
	sub	eax, 1
	je	$LN88@find_if@4
	sub	eax, 1
	je	SHORT $LN77@find_if@4
	sub	eax, 1
	jne	$LN2@find_if@4

; 172  :   case 3:
; 173  :     if (__pred(*__first)) return __first;

	movzx	ecx, WORD PTR [esi+8]
	fld	DWORD PTR [edi+24]
	mov	eax, DWORD PTR [ebx]
	mov	eax, DWORD PTR [eax+8]
	push	ecx
	lea	edx, DWORD PTR [edi+12]
	push	edx
	mov	edx, DWORD PTR ___pred$[esp+80]
	push	edi
	push	ecx
	lea	ecx, DWORD PTR _br$304067[esp+88]
	add	edx, 80					; 00000050H
	fstp	DWORD PTR [esp]
	push	ecx
	push	edx
	mov	ecx, ebx
	call	eax
	test	al, al
	je	$LN97@find_if@4
	movss	xmm0, DWORD PTR _br$304067[esp+84]
	movss	DWORD PTR [esi+4], xmm0

; 174  :     ++__first;

	add	esi, 12					; 0000000cH

; 175  :   case 2:
; 176  :     if (__pred(*__first)) return __first;

$LN77@find_if@4:
	movzx	eax, WORD PTR [esi+8]
	fld	DWORD PTR [edi+24]
	mov	edx, DWORD PTR [ebx]
	mov	edx, DWORD PTR [edx+8]
	push	eax
	lea	ecx, DWORD PTR [edi+12]
	push	ecx
	push	edi
	push	ecx
	mov	ecx, DWORD PTR ___pred$[esp+88]
	lea	eax, DWORD PTR _br$304088[esp+88]
	fstp	DWORD PTR [esp]
	add	ecx, 80					; 00000050H
	push	eax
	push	ecx
	mov	ecx, ebx
	call	edx
	test	al, al
	je	SHORT $LN97@find_if@4
	movss	xmm0, DWORD PTR _br$304088[esp+84]
	movss	DWORD PTR [esi+4], xmm0

; 177  :     ++__first;

	add	esi, 12					; 0000000cH

; 178  :   case 1:
; 179  :     if (__pred(*__first)) return __first;

$LN88@find_if@4:
	movzx	ecx, WORD PTR [esi+8]
	fld	DWORD PTR [edi+24]
	mov	eax, DWORD PTR [ebx]
	mov	eax, DWORD PTR [eax+8]
	push	ecx
	lea	edx, DWORD PTR [edi+12]
	push	edx
	mov	edx, DWORD PTR ___pred$[esp+80]
	push	edi
	push	ecx
	lea	ecx, DWORD PTR _br$304109[esp+88]
	add	edx, 80					; 00000050H
	fstp	DWORD PTR [esp]
	push	ecx
	push	edx
	mov	ecx, ebx
	call	eax
	test	al, al
	je	SHORT $LN97@find_if@4
	movss	xmm0, DWORD PTR _br$304109[esp+84]
	movss	DWORD PTR [esi+4], xmm0
$LN2@find_if@4:

; 180  :       //++__first;
; 181  :   case 0:
; 182  :   default:
; 183  :     return __last;

	mov	eax, DWORD PTR ___last$[esp+68]
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx

; 184  :   }
; 185  : }

	add	esp, 56					; 00000038H
	ret	0
$LN97@find_if@4:
	pop	edi

; 158  :     if (__pred(*__first)) return __first;

	mov	eax, esi
	pop	esi
	pop	ebp
	pop	ebx

; 184  :   }
; 185  : }

	add	esp, 56					; 00000038H
	ret	0
??$__find_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@priv@stlp_std@@YAPAUrq_result@collide@@PAU23@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@3@AAVrq_results@3@@Z@ABUrandom_access_iterator_tag@1@@Z ENDP ; stlp_std::priv::__find_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\sdk\include\stlport\stl\_algo.h
_TEXT	ENDS
;	COMDAT ??$remove_copy_if@PAUrq_result@collide@@PAU12@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@00Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z
_TEXT	SEGMENT
_br$304151 = -52					; size = 52
___last$ = 8						; size = 4
tv168 = 12						; size = 4
___result$ = 12						; size = 4
___pred$ = 16						; size = 12
??$remove_copy_if@PAUrq_result@collide@@PAU12@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@00Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z PROC ; stlp_std::remove_copy_if<collide::rq_result *,collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>, COMDAT
; ___first$ = eax

; 253  : remove_copy_if(_InputIter __first, _InputIter __last, _OutputIter __result, _Predicate __pred) {

	sub	esp, 52					; 00000034H
	push	ebp
	mov	ebp, DWORD PTR ___result$[esp+52]
	push	esi
	mov	esi, eax

; 254  :   _STLP_DEBUG_CHECK(_STLP_PRIV __check_range(__first, __last))
; 255  :   for ( ; __first != __last; ++__first) {

	cmp	esi, DWORD PTR ___last$[esp+56]
	je	SHORT $LN20@remove_cop@3
	mov	eax, DWORD PTR ___pred$[esp+60]
	push	ebx
	mov	ebx, DWORD PTR ___pred$[esp+68]

; 256  :     if (!__pred(*__first)) {

	add	eax, 80					; 00000050H
	push	edi
	mov	edi, DWORD PTR ___pred$[esp+64]
	mov	DWORD PTR tv168[esp+64], eax
$LL21@remove_cop@3:
	movzx	eax, WORD PTR [esi+8]
	fld	DWORD PTR [edi+24]
	mov	edx, DWORD PTR [ebx]
	mov	edx, DWORD PTR [edx+8]
	push	eax
	lea	eax, DWORD PTR [edi+12]
	push	eax
	mov	eax, DWORD PTR tv168[esp+72]
	push	edi
	push	ecx
	lea	ecx, DWORD PTR _br$304151[esp+84]
	fstp	DWORD PTR [esp]
	push	ecx
	push	eax
	mov	ecx, ebx
	call	edx
	test	al, al
	je	SHORT $LN3@remove_cop@3
	movss	xmm0, DWORD PTR _br$304151[esp+80]
	movss	DWORD PTR [esi+4], xmm0

; 257  :       *__result = *__first;

	mov	eax, DWORD PTR [esi]
	mov	DWORD PTR [ebp], eax
	mov	ecx, DWORD PTR [esi+4]
	mov	DWORD PTR [ebp+4], ecx
	mov	edx, DWORD PTR [esi+8]
	mov	DWORD PTR [ebp+8], edx

; 258  :       ++__result;

	add	ebp, 12					; 0000000cH
$LN3@remove_cop@3:
	add	esi, 12					; 0000000cH
	cmp	esi, DWORD PTR ___last$[esp+64]
	jne	SHORT $LL21@remove_cop@3
	pop	edi
	pop	ebx
$LN20@remove_cop@3:
	pop	esi

; 259  :     }
; 260  :   }
; 261  :   return __result;

	mov	eax, ebp
	pop	ebp

; 262  : }

	add	esp, 52					; 00000034H
	ret	0
??$remove_copy_if@PAUrq_result@collide@@PAU12@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@00Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z ENDP ; stlp_std::remove_copy_if<collide::rq_result *,collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\sdk\include\stlport\stl\_algobase.c
_TEXT	ENDS
;	COMDAT ??$find_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z
_TEXT	SEGMENT
___first$ = 8						; size = 4
___last$ = 12						; size = 4
___pred$ = 16						; size = 12
??$find_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z PROC ; stlp_std::find_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>, COMDAT

; 209  :   _STLP_DEBUG_CHECK(_STLP_PRIV __check_range(__first, __last))
; 210  :   return _STLP_PRIV __find_if(__first, __last, __pred, _STLP_ITERATOR_CATEGORY(__first, _InputIter));

	mov	ecx, DWORD PTR ___pred$[esp-4]
	mov	edx, DWORD PTR ___pred$[esp]
	sub	esp, 12					; 0000000cH
	mov	eax, esp
	mov	DWORD PTR [eax], ecx
	mov	ecx, DWORD PTR ___pred$[esp+16]
	mov	DWORD PTR [eax+4], edx
	mov	edx, DWORD PTR ___last$[esp+8]
	mov	DWORD PTR [eax+8], ecx
	mov	eax, DWORD PTR ___first$[esp+8]
	push	edx
	call	??$__find_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@priv@stlp_std@@YAPAUrq_result@collide@@PAU23@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@3@AAVrq_results@3@@Z@ABUrandom_access_iterator_tag@1@@Z ; stlp_std::priv::__find_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
	add	esp, 16					; 00000010H

; 211  : }

	ret	0
??$find_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z ENDP ; stlp_std::find_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\sdk\include\stlport\stl\_algo.h
_TEXT	ENDS
;	COMDAT ??$remove_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z
_TEXT	SEGMENT
___first$ = 8						; size = 4
___pred$ = 12						; size = 12
??$remove_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z PROC ; stlp_std::remove_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>, COMDAT
; ___last$ = esi

; 279  : remove_if(_ForwardIter __first, _ForwardIter __last, _Predicate __pred) {

	push	ebx

; 280  :   _STLP_DEBUG_CHECK(_STLP_PRIV __check_range(__first, __last))
; 281  :   __first = find_if(__first, __last, __pred);

	mov	ebx, DWORD PTR ___pred$[esp+4]
	push	ebp
	mov	ebp, DWORD PTR ___pred$[esp+12]
	push	edi
	mov	edi, DWORD PTR ___pred$[esp+8]
	sub	esp, 12					; 0000000cH
	mov	eax, esp
	mov	DWORD PTR [eax], edi
	mov	DWORD PTR [eax+4], ebx
	mov	DWORD PTR [eax+8], ebp
	mov	eax, DWORD PTR ___first$[esp+20]
	push	esi
	call	??$__find_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@priv@stlp_std@@YAPAUrq_result@collide@@PAU23@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@3@AAVrq_results@3@@Z@ABUrandom_access_iterator_tag@1@@Z ; stlp_std::priv::__find_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
	add	esp, 16					; 00000010H

; 282  :   if ( __first == __last )

	cmp	eax, esi

; 283  :     return __first;

	je	SHORT $LN1@remove_if@2

; 284  :   else {
; 285  :     _ForwardIter __next = __first;
; 286  :     return remove_copy_if(++__next, __last, __first, __pred);

	sub	esp, 12					; 0000000cH
	mov	ecx, esp
	push	eax
	mov	DWORD PTR [ecx], edi
	mov	DWORD PTR [ecx+4], ebx
	push	esi
	add	eax, 12					; 0000000cH
	mov	DWORD PTR [ecx+8], ebp
	call	??$remove_copy_if@PAUrq_result@collide@@PAU12@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@00Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z ; stlp_std::remove_copy_if<collide::rq_result *,collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
	add	esp, 20					; 00000014H
$LN1@remove_if@2:

; 287  :   }
; 288  : }

	pop	edi
	pop	ebp
	pop	ebx
	ret	0
??$remove_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z ENDP ; stlp_std::remove_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
_TEXT	ENDS
PUBLIC	??0CCF_DynamicMesh@@QAE@ABV0@@Z			; CCF_DynamicMesh::CCF_DynamicMesh
; Function compile flags: /Ogtpy
;	COMDAT ??0CCF_DynamicMesh@@QAE@ABV0@@Z
_TEXT	SEGMENT
___that$ = 8						; size = 4
??0CCF_DynamicMesh@@QAE@ABV0@@Z PROC			; CCF_DynamicMesh::CCF_DynamicMesh, COMDAT
; _this$ = ecx
	mov	eax, DWORD PTR ___that$[esp-4]
	push	esi
	push	eax
	mov	esi, ecx
	call	??0CCF_Skeleton@@QAE@ABV0@@Z
	mov	DWORD PTR [esi], OFFSET ??_7CCF_DynamicMesh@@6B@
	mov	eax, esi
	pop	esi
	ret	4
??0CCF_DynamicMesh@@QAE@ABV0@@Z ENDP			; CCF_DynamicMesh::CCF_DynamicMesh
_TEXT	ENDS
PUBLIC	??1CCF_DynamicMesh@@UAE@XZ			; CCF_DynamicMesh::~CCF_DynamicMesh
; Function compile flags: /Ogtpy
;	COMDAT ??1CCF_DynamicMesh@@UAE@XZ
_TEXT	SEGMENT
??1CCF_DynamicMesh@@UAE@XZ PROC				; CCF_DynamicMesh::~CCF_DynamicMesh, COMDAT
; _this$ = ecx
	push	esi
	mov	esi, ecx
	mov	eax, DWORD PTR [esi+64]
	test	eax, eax
	je	SHORT $LN47@CCF_Dynami
	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	push	eax
	call	DWORD PTR __imp_?mem_free@xrMemory@@QAEXPAX@Z
$LN47@CCF_Dynami:
	mov	DWORD PTR [esi], OFFSET ??_7ICollisionForm@@6B@
	pop	esi
	ret	0
??1CCF_DynamicMesh@@UAE@XZ ENDP				; CCF_DynamicMesh::~CCF_DynamicMesh
_TEXT	ENDS
PUBLIC	??0CCF_DynamicMesh@@QAE@PAVCObject@@@Z		; CCF_DynamicMesh::CCF_DynamicMesh
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrengine\cf_dynamic_mesh.h
;	COMDAT ??0CCF_DynamicMesh@@QAE@PAVCObject@@@Z
_TEXT	SEGMENT
__owner$ = 8						; size = 4
??0CCF_DynamicMesh@@QAE@PAVCObject@@@Z PROC		; CCF_DynamicMesh::CCF_DynamicMesh, COMDAT
; _this$ = ecx

; 11   : 						CCF_DynamicMesh	( CObject* _owner ) :CCF_Skeleton(_owner){};

	mov	eax, DWORD PTR __owner$[esp-4]
	push	esi
	push	eax
	mov	esi, ecx
	call	??0CCF_Skeleton@@QAE@PAVCObject@@@Z	; CCF_Skeleton::CCF_Skeleton
	mov	DWORD PTR [esi], OFFSET ??_7CCF_DynamicMesh@@6B@
	mov	eax, esi
	pop	esi
	ret	4
??0CCF_DynamicMesh@@QAE@PAVCObject@@@Z ENDP		; CCF_DynamicMesh::CCF_DynamicMesh
; Function compile flags: /Ogtpy
;	COMDAT ??_ECCF_DynamicMesh@@UAEPAXI@Z
_TEXT	SEGMENT
___flags$ = 8						; size = 4
??_ECCF_DynamicMesh@@UAEPAXI@Z PROC			; CCF_DynamicMesh::`vector deleting destructor', COMDAT
; _this$ = ecx
	test	BYTE PTR ___flags$[esp-4], 2
	push	ebx
	push	ebp
	push	esi
	push	edi
	mov	edi, ecx
	je	SHORT $LN3@vector@71
	mov	ebp, DWORD PTR [edi-4]
	lea	ebx, DWORD PTR [edi-4]
	mov	esi, ebp
	imul	esi, 88					; 00000058H
	add	esi, edi
	sub	ebp, 1
	mov	edi, DWORD PTR __imp_?mem_free@xrMemory@@QAEXPAX@Z
	js	SHORT $LN6@vector@71
$LL7@vector@71:
	mov	eax, DWORD PTR [esi-24]
	sub	esi, 88					; 00000058H
	test	eax, eax
	je	SHORT $LN51@vector@71
	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	push	eax
	call	edi
$LN51@vector@71:
	sub	ebp, 1
	mov	DWORD PTR [esi], OFFSET ??_7ICollisionForm@@6B@
	jns	SHORT $LL7@vector@71
$LN6@vector@71:
	test	BYTE PTR ___flags$[esp+12], 1
	je	SHORT $LN58@vector@71
	test	ebx, ebx
	je	SHORT $LN58@vector@71
	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	push	ebx
	call	edi
$LN58@vector@71:
	mov	eax, ebx
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	ret	4
$LN3@vector@71:
	mov	eax, DWORD PTR [edi+64]
	test	eax, eax
	mov	esi, DWORD PTR __imp_?mem_free@xrMemory@@QAEXPAX@Z
	je	SHORT $LN102@vector@71
	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	push	eax
	call	esi
$LN102@vector@71:
	test	BYTE PTR ___flags$[esp+12], 1
	mov	DWORD PTR [edi], OFFSET ??_7ICollisionForm@@6B@
	je	SHORT $LN109@vector@71
	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	push	edi
	call	esi
$LN109@vector@71:
	mov	eax, edi
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	ret	4
??_ECCF_DynamicMesh@@UAEPAXI@Z ENDP			; CCF_DynamicMesh::`vector deleting destructor'
_TEXT	ENDS
PUBLIC	??_GCCF_DynamicMesh@@UAEPAXI@Z			; CCF_DynamicMesh::`scalar deleting destructor'
; Function compile flags: /Ogtpy
;	COMDAT ??_GCCF_DynamicMesh@@UAEPAXI@Z
_TEXT	SEGMENT
___flags$ = 8						; size = 4
??_GCCF_DynamicMesh@@UAEPAXI@Z PROC			; CCF_DynamicMesh::`scalar deleting destructor', COMDAT
; _this$ = ecx
	push	esi
	mov	esi, ecx
	mov	eax, DWORD PTR [esi+64]
	test	eax, eax
	push	edi
	mov	edi, DWORD PTR __imp_?mem_free@xrMemory@@QAEXPAX@Z
	je	SHORT $LN45@scalar@45
	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	push	eax
	call	edi
$LN45@scalar@45:
	test	BYTE PTR ___flags$[esp+4], 1
	mov	DWORD PTR [esi], OFFSET ??_7ICollisionForm@@6B@
	je	SHORT $LN55@scalar@45
	mov	ecx, DWORD PTR __imp_?Memory@@3VxrMemory@@A
	push	esi
	call	edi
$LN55@scalar@45:
	pop	edi
	mov	eax, esi
	pop	esi
	ret	4
??_GCCF_DynamicMesh@@UAEPAXI@Z ENDP			; CCF_DynamicMesh::`scalar deleting destructor'
_TEXT	ENDS
PUBLIC	??4CCF_DynamicMesh@@QAEAAV0@ABV0@@Z		; CCF_DynamicMesh::operator=
; Function compile flags: /Ogtpy
;	COMDAT ??4CCF_DynamicMesh@@QAEAAV0@ABV0@@Z
_TEXT	SEGMENT
___that$ = 8						; size = 4
??4CCF_DynamicMesh@@QAEAAV0@ABV0@@Z PROC		; CCF_DynamicMesh::operator=, COMDAT
; _this$ = ecx
	mov	eax, DWORD PTR ___that$[esp-4]
	push	esi
	push	eax
	mov	esi, ecx
	call	??4CCF_Skeleton@@QAEAAV0@ABV0@@Z
	mov	eax, esi
	pop	esi
	ret	4
??4CCF_DynamicMesh@@QAEAAV0@ABV0@@Z ENDP		; CCF_DynamicMesh::operator=
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrengine\cf_dynamic_mesh.cpp
;	COMDAT ?_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@3@@Z
_TEXT	SEGMENT
_pick$ = -12						; size = 12
_Q$ = 8							; size = 4
_R$ = 12						; size = 4
?_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@3@@Z PROC ; CCF_DynamicMesh::_RayQuery, COMDAT
; _this$ = ecx

; 15   : {

	sub	esp, 12					; 0000000cH
	push	ebx
	push	ebp
	push	esi

; 16   : 	int s_count = R.r_count();

	mov	esi, DWORD PTR _R$[esp+20]
	push	edi
	mov	edi, ecx
	mov	ecx, DWORD PTR [esi+4]
	sub	ecx, DWORD PTR [esi]
	mov	eax, 715827883				; 2aaaaaabH
	imul	ecx

; 17   : 	BOOL res = inherited::_RayQuery( Q, R );

	mov	eax, DWORD PTR _Q$[esp+24]
	sar	edx, 1
	mov	ebx, edx
	push	esi
	shr	ebx, 31					; 0000001fH
	push	eax
	mov	ecx, edi
	add	ebx, edx
	call	?_RayQuery@CCF_Skeleton@@UAEHABUray_defs@collide@@AAVrq_results@3@@Z ; CCF_Skeleton::_RayQuery

; 18   : 	if( !res )

	test	eax, eax
	jne	SHORT $LN9@RayQuery@3

; 70   : }

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	add	esp, 12					; 0000000cH
	ret	8
$LN9@RayQuery@3:

; 19   : 		return FALSE;
; 20   : 
; 21   : 	VERIFY( owner );
; 22   : 	VERIFY( owner->Visual() );
; 23   : 	IKinematics *K = owner->Visual()->dcast_PKinematics();

	mov	ecx, DWORD PTR [edi+4]
	mov	eax, DWORD PTR [ecx+144]
	mov	edx, DWORD PTR [eax]
	push	eax
	mov	eax, DWORD PTR [edx+12]
	call	eax

; 24   : 	
; 25   : 	struct spick
; 26   : 	{
; 27   : 		 const collide::ray_defs& Q;
; 28   : 		 const  CObject			& obj;
; 29   : 		 IKinematics			& K;
; 30   : 		 
; 31   : 		 spick(  const collide::ray_defs& Q_, const CObject &obj_, IKinematics &K_ ): Q( Q_ ), obj( obj_ ), K( K_ )
; 32   : 		 {
; 33   : 
; 34   : 		 }
; 35   : 		 
; 36   : 		 bool operator() ( collide::rq_result &r )
; 37   : 		{
; 38   : 			IKinematics::pick_result br;
; 39   : 			VERIFY( r.O == &obj );
; 40   : 			bool  res = K.PickBone( obj.XFORM(), br, Q.range, Q.start, Q.dir, (u16) r.element ) ;
; 41   : 			if(res)
; 42   : 			{
; 43   : 				r.range = br.dist;
; 44   : 			}
; 45   : #if	0
; 46   : 			if(res)
; 47   : 			{
; 48   : 				ph_debug_render->open_cashed_draw();
; 49   : 				ph_debug_render->draw_tri(  br.tri[0], br.tri[1], br.tri[2], D3DCOLOR_ARGB(50, 255,0,0), 0 );
; 50   : 				ph_debug_render->close_cashed_draw(50000);
; 51   : 			}
; 52   : #endif
; 53   : 			return !res;
; 54   : 		}
; 55   : 		private:
; 56   : 			spick& operator = (spick& ){ NODEFAULT;  return *this; }
; 57   : 	} pick( (collide::ray_defs&) (Q), (const CObject &)(*owner),(IKinematics&) (*K) );

	mov	ecx, DWORD PTR [edi+4]

; 58   : 	
; 59   : 	R.r_results().erase( std::remove_if( R.r_results().begin() + s_count, R.r_results().end() , pick), R.r_results().end() );

	mov	edi, DWORD PTR [esi+4]
	mov	ebp, eax
	mov	eax, DWORD PTR [esi]
	lea	edx, DWORD PTR [ebx+ebx*2]
	mov	DWORD PTR _pick$[esp+32], ecx
	lea	eax, DWORD PTR [eax+edx*4]
	mov	edx, DWORD PTR _Q$[esp+24]
	sub	esp, 12					; 0000000cH
	mov	ecx, esp
	mov	DWORD PTR [ecx], edx
	mov	edx, DWORD PTR _pick$[esp+44]
	mov	DWORD PTR [ecx+4], edx
	push	edi
	mov	DWORD PTR [ecx+8], ebp
	call	??$__find_if@PAUrq_result@collide@@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@priv@stlp_std@@YAPAUrq_result@collide@@PAU23@0Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@3@AAVrq_results@3@@Z@ABUrandom_access_iterator_tag@1@@Z ; stlp_std::priv::__find_if<collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
	add	esp, 16					; 00000010H
	cmp	eax, edi
	je	SHORT $LN3@RayQuery@3
	mov	edx, DWORD PTR _Q$[esp+24]
	sub	esp, 12					; 0000000cH
	mov	ecx, esp
	mov	DWORD PTR [ecx], edx
	mov	edx, DWORD PTR _pick$[esp+44]
	push	eax
	mov	DWORD PTR [ecx+4], edx
	push	edi
	add	eax, 12					; 0000000cH
	mov	DWORD PTR [ecx+8], ebp
	call	??$remove_copy_if@PAUrq_result@collide@@PAU12@Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@stlp_std@@YAPAUrq_result@collide@@PAU12@00Uspick@?7??_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@2@AAVrq_results@2@@Z@@Z ; stlp_std::remove_copy_if<collide::rq_result *,collide::rq_result *,`CCF_DynamicMesh::_RayQuery'::`8'::spick>
	add	esp, 20					; 00000014H
	cmp	eax, edi
	je	SHORT $LN3@RayQuery@3
	mov	ebp, DWORD PTR [esi+4]
	sub	ebp, edi
	je	SHORT $LN72@RayQuery@3
	push	ebp
	push	edi
	push	eax
	call	DWORD PTR __imp__memmove
	add	esp, 12					; 0000000cH
	add	eax, ebp
$LN72@RayQuery@3:
	mov	DWORD PTR [esi+4], eax
$LN3@RayQuery@3:

; 60   : 	/*
; 61   : 	for( collide::rq_result* i = R.r_begin() + s_count; i < R.r_end(); ++i )
; 62   : 	{
; 63   : 		IKinematics::pick_result r;
; 64   : 		if( K->PickBone( owner->XFORM(), r, Q.range, Q.start, Q.dir, (u16) i->element ) )
; 65   : 			return TRUE;
; 66   : 	}
; 67   : 	*/
; 68   : 	VERIFY( R.r_count() >= s_count );
; 69   : 	return R.r_count() > s_count;

	mov	ecx, DWORD PTR [esi+4]
	sub	ecx, DWORD PTR [esi]
	mov	eax, 715827883				; 2aaaaaabH
	imul	ecx
	sar	edx, 1
	mov	eax, edx
	shr	eax, 31					; 0000001fH
	add	eax, edx
	xor	ecx, ecx
	cmp	eax, ebx
	setg	cl

; 70   : }

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	mov	eax, ecx
	add	esp, 12					; 0000000cH
	ret	8
?_RayQuery@CCF_DynamicMesh@@UAEHABUray_defs@collide@@AAVrq_results@3@@Z ENDP ; CCF_DynamicMesh::_RayQuery
END
