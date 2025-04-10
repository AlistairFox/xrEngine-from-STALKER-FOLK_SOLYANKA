#pragma once
#define XRAPI_API 

class IRender_interface;
extern XRAPI_API IRender_interface* Render;

class dxRenderFactory;
extern XRAPI_API dxRenderFactory* RenderFactory;


class CDUInterface;
extern XRAPI_API CDUInterface* DU;

struct xr_token;
extern XRAPI_API xr_token* vid_mode_token;

class IUIRender;
extern XRAPI_API IUIRender* UIRender;


class CGameMtlLibrary;
extern XRAPI_API CGameMtlLibrary* PGMLib;

#ifdef DEBUG
class IDebugRender;
extern XRAPI_API IDebugRender* DRender;
#endif // DEBUG
