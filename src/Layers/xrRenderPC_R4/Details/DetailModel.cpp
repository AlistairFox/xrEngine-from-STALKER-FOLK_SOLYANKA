#include "stdafx.h"
#include "DetailModel.h"

CDetail::~CDetail()
{
	for (u32 i = 0; i < 3; ++i)
		for (u32 j = 0; j < 2; ++j)
			m_items[i][j].clear();
}

void CDetail::Unload()
{
	if (vertices) { xr_free(vertices);	vertices = 0; }
	if (indices) { xr_free(indices);		indices = 0; }
	shader.destroy();
}

void CDetail::transfer(Fmatrix& mXform, fvfVertexOut* vDest, u32 C, u16* iDest, u32 iOffset)
{
	// Transfer vertices
	{
		CDetail::fvfVertexIn* srcIt = vertices, * srcEnd = vertices + number_vertices;
		CDetail::fvfVertexOut* dstIt = vDest;
		for (; srcIt != srcEnd; srcIt++, dstIt++)
		{
			mXform.transform_tiny(dstIt->P, srcIt->P);
			dstIt->C = C;
			dstIt->u = srcIt->u;
			dstIt->v = srcIt->v;
		}
	}

	// Transfer indices (in 32bit lines)
	VERIFY(iOffset < 65535);
	{
		u32	item = (iOffset << 16) | iOffset;
		u32	count = number_indices / 2;
		LPDWORD	sit = LPDWORD(indices);
		LPDWORD	send = sit + count;
		LPDWORD	dit = LPDWORD(iDest);
		for (; sit != send; dit++, sit++)	*dit = *sit + item;
		if (number_indices & 1)
			iDest[number_indices - 1] = u16(indices[number_indices - 1] + u16(iOffset));
	}
}

void CDetail::transfer(Fmatrix& mXform, fvfVertexOut* vDest, u32 C, u16* iDest, u32 iOffset, float du, float dv)
{
	// Transfer vertices
	{
		CDetail::fvfVertexIn* srcIt = vertices, * srcEnd = vertices + number_vertices;
		CDetail::fvfVertexOut* dstIt = vDest;
		for (; srcIt != srcEnd; srcIt++, dstIt++)
		{
			mXform.transform_tiny(dstIt->P, srcIt->P);
			dstIt->C = C;
			dstIt->u = srcIt->u + du;
			dstIt->v = srcIt->v + dv;
		}
	}

	// Transfer indices (in 32bit lines)
	VERIFY(iOffset < 65535);
	{
		u32	item = (iOffset << 16) | iOffset;
		u32	count = number_indices / 2;
		LPDWORD	sit = LPDWORD(indices);
		LPDWORD	send = sit + count;
		LPDWORD	dit = LPDWORD(iDest);
		for (; sit != send; dit++, sit++)	*dit = *sit + item;
		if (number_indices & 1)
			iDest[number_indices - 1] = u16(indices[number_indices - 1] + u16(iOffset));
	}
}

// se7kills Instances
void CDetail::Load(IReader* S)
{
	// Shader
	string256		fnT, fnS;
	S->r_stringZ(fnS, sizeof(fnS));
	S->r_stringZ(fnT, sizeof(fnT));
	shader.create(fnS, fnT);

	Msg("CDetail Slot Shader[%s] Texture[%s]", fnS, fnT);

	// Params
	m_Flags.assign(S->r_u32());
	m_fMinScale = S->r_float();
	m_fMaxScale = S->r_float();
	number_vertices = S->r_u32();
	number_indices = S->r_u32();
	R_ASSERT(0 == (number_indices % 3));

	// Vertices                             
	u32				size_vertices = number_vertices * sizeof(fvfVertexIn);
	vertices = xr_alloc<CDetail::fvfVertexIn>(number_vertices);
	S->r(vertices, size_vertices);


	// Indices
	u32				size_indices = number_indices * sizeof(u16);
	indices = xr_alloc<u16>(number_indices);
	S->r(indices, size_indices);

	Msg("CDetail Slot SizeVertex[%u], SizeIndexes[%u]", size_vertices, size_indices);
	 
	// Calc BB & SphereRadius
	bv_bb.invalidate();
	for (u32 i = 0; i < number_vertices; i++)
		bv_bb.modify(vertices[i].P);
	bv_bb.getsphere(bv_sphere.P, bv_sphere.R);
 
	Optimize();
}
 
#include "../xrRender/xrStripify.h"

void CDetail::Optimize()
{
	xr_vector<u16>		vec_indices, vec_permute;
	const int			cache = 24;

	// Stripify
	vec_indices.assign(indices, indices + number_indices);
	vec_permute.resize(number_vertices);
	int vt_old = xrSimulate(vec_indices, cache);
	xrStripify(vec_indices, vec_permute, cache, 0);

	int vt_new = xrSimulate(vec_indices, cache);
	if (vt_new < vt_old)
	{
		// Copy faces
		CopyMemory(indices, &*vec_indices.begin(), vec_indices.size() * sizeof(u16));

		// Permute vertices
		xr_vector<fvfVertexIn>	verts;
		verts.assign(vertices, vertices + number_vertices);
		for (u32 i = 0; i < verts.size(); i++)
			vertices[i] = verts[vec_permute[i]];
	}
}
