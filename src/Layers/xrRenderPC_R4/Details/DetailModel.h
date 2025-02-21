#pragma once

#include "..\xrRender\IRenderDetailModel.h"
#include <memory>

class ECORE_API CDetail : public IRender_DetailModel
{
public:
	struct	SlotItem
	{								// один кустик
		float						scale;

		float						RenderScale;
		Fvector						Position;
		Fvector						Direction;

		u8							vis_ID;				// индекс в visibility списке он же тип [не качается, качается1, качается2]
		float						c_hemi;
		float						c_sun;
	};

	xr_vector<std::shared_ptr<SlotItem>> m_items[3][2];
 
	void			Load(IReader* S);
	void			Optimize();
	virtual void	Unload();

	virtual void	transfer(Fmatrix& mXform, fvfVertexOut* vDest, u32 C, u16* iDest, u32 iOffset);
	virtual void	transfer(Fmatrix& mXform, fvfVertexOut* vDest, u32 C, u16* iDest, u32 iOffset, float du, float dv);
	virtual			~CDetail();
};
