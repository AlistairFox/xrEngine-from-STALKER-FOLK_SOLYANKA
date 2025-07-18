#ifndef	dx10StateUtils_included
#define	dx10StateUtils_included
#pragma once

namespace dx10StateUtils
{
	D3D_BLEND					ConvertBlendArg(D3D11_BLEND Arg);
	D3D_BLEND_OP				ConvertBlendOp(D3DBLENDOP Op);
	D3D_TEXTURE_ADDRESS_MODE	ConvertTextureAddressMode(D3DTEXTUREADDRESS Mode);

	//	Set description to default values
	void	ResetDescription( D3D_RASTERIZER_DESC &desc );
	void	ResetDescription( D3D_DEPTH_STENCIL_DESC &desc );
	void	ResetDescription( D3D_BLEND_DESC &desc );
	void	ResetDescription( D3D_SAMPLER_DESC &desc );

	//	State comparison (memcmp doesn't work due to padding bytes in structure)
	bool	operator==(const D3D_RASTERIZER_DESC &desc1, const D3D_RASTERIZER_DESC &desc2);
	bool	operator==(const D3D_DEPTH_STENCIL_DESC &desc1, const D3D_DEPTH_STENCIL_DESC &desc2);
	bool	operator==(const D3D_BLEND_DESC &desc1, const D3D_BLEND_DESC &desc2);
	bool	operator==(const D3D_SAMPLER_DESC &desc1, const D3D_SAMPLER_DESC &desc2);

	//	Calculate hash values
	u32		GetHash( const D3D_RASTERIZER_DESC &desc );
	u32		GetHash( const D3D_DEPTH_STENCIL_DESC &desc );
	u32		GetHash( const D3D_BLEND_DESC &desc );
	u32		GetHash( const D3D_SAMPLER_DESC &desc );

	//	Modify state to meet DX10 automatic modifications
	void	ValidateState(D3D_RASTERIZER_DESC &desc);
	void	ValidateState(D3D_DEPTH_STENCIL_DESC &desc);
	void	ValidateState(D3D_BLEND_DESC &desc);
	void	ValidateState(D3D_SAMPLER_DESC &desc);
};

#endif	//	dx10StateUtils_included