#ifndef xrLevelH
#define xrLevelH

// Se7kills AI FIX
#pragma once



struct xrGUID {
	u64	g[2];

	ICF	bool operator==	(const xrGUID& o) const
	{
		return	((g[0] == o.g[0]) && (g[1] == o.g[1]));
	}

	ICF	bool operator!=	(const xrGUID& o) const
	{
		return	!(*this == o);
	}
	ICF void LoadLTX(CInifile& ini, LPCSTR section, LPCSTR name)
	{
		string128				buff;

		g[0] = ini.r_u64(section, strconcat(sizeof(buff), buff, name, "_g0"));
		g[1] = ini.r_u64(section, strconcat(sizeof(buff), buff, name, "_g1"));
	}

	ICF void SaveLTX(CInifile& ini, LPCSTR section, LPCSTR name)
	{
		string128				buff;

		ini.w_u64(section, strconcat(sizeof(buff), buff, name, "_g0"), g[0]);
		ini.w_u64(section, strconcat(sizeof(buff), buff, name, "_g1"), g[1]);
	}
};

enum fsL_Chunks {
	fsL_HEADER = 1,		//*
	fsL_SHADERS = 2,		//*
	fsL_VISUALS = 3,		//*
	fsL_PORTALS = 4,		//*		- Portal polygons
	fsL_LIGHT_DYNAMIC = 6,		//*
	fsL_GLOWS = 7,		//*		- All glows inside level
	fsL_SECTORS = 8,		//*		- All sectors on level
	fsL_VB = 9,		//*		- Static geometry
	fsL_IB = 10,	//*
	fsL_SWIS = 11,	//*		- collapse info, usually for trees
	fsL_forcedword = 0xFFFFFFFF
};
enum fsESectorChunks {
	fsP_Portals = 1,	// - portal polygons
	fsP_Root,			// - geometry root
	fsP_forcedword = u32(-1)
};
enum fsSLS_Chunks {
	fsSLS_Description = 1,	// Name of level
	fsSLS_ServerState,
	fsSLS_forcedword = u32(-1)
};

enum EBuildQuality {
	ebqDraft = 0,
	ebqHigh,
	ebqCustom,
	ebq_force_u16 = u16(-1)
};

#pragma pack(push,8)
struct hdrLEVEL
{
	u16		XRLC_version;
	u16		XRLC_quality;
};

struct hdrCFORM
{
	u32		version;
	u32		vertcount;
	u32		facecount;
	Fbox	aabb;
};

struct	hdrNODES
{
	u32		version;
	u32		count;
	float	size;
	float	size_y;
	Fbox	aabb;
	xrGUID	guid;
};
#pragma pack(pop)

#pragma pack(push,1)
#pragma pack(1)

#define _USE_NODE_POSITION_11

#ifndef _EDITOR


#ifndef _USE_NODE_POSITION_11
class NodePosition
{
	u8	data[5];
public:
	ICF	void xz(u32 value) { CopyMemory(data, &value, 3); }
	ICF	void y(u16 value) { CopyMemory(data + 3, &value, 2); }

	ICF	u32	xz() const {
		return			((*((u32*)data)) & 0x00ffffff);
	}
	ICF	u32	x(u32 row) const {
		return			(xz() / row);
	}
	ICF	u32	z(u32 row) const {
		return			(xz() % row);
	}
	ICF	u32	y() const {
		return			(*((u16*)(data + 3)));
	}

	friend class	CLevelGraph;
	friend struct	CNodePositionCompressor;
	friend struct	CNodePositionConverter;
};
#else 
class NodePosition
{
	u8	data[6];
public:
	ICF	void xz(u32 value) { CopyMemory(data, &value, 4); }
	ICF	void y(u16 value) { CopyMemory(data + 4, &value, 2); }

	ICF	u32	xz() const {
		return			((*((u32*)data)) & 0xffffffff);
	}
	ICF	u32	x(u32 row) const {
		return			(xz() / row);
	}
	ICF	u32	z(u32 row) const {
		return			(xz() % row);
	}
	ICF	u32	y() const {
		return			(*((u16*)(data + 4)));
	}

	friend class	CLevelGraph;
	friend struct	CNodePositionCompressor;
	friend struct	CNodePositionConverter;
};
#endif

struct NodeCompressed
{
public:
#ifndef _USE_NODE_POSITION_11
	u8			data[12];
#else 
	u32				data[4];
	u8 data_light;
#endif

private:

	ICF	void link(u8 link_index, u32 value)
	{
#ifndef _USE_NODE_POSITION_11	 
		value &= 0x007fffff;
		switch (link_index)
		{
		case 0:
		{
			value |= (*(u32*)data) & 0xff800000;
			CopyMemory(data, &value, sizeof(u32));
			break;
		}
		case 1: {
			value <<= 7;
			value |= (*(u32*)(data + 2)) & 0xc000007f;
			CopyMemory(data + 2, &value, sizeof(u32));
			break;
		}
		case 2: {
			value <<= 6;
			value |= (*(u32*)(data + 5)) & 0xe000003f;
			CopyMemory(data + 5, &value, sizeof(u32));
			break;
		}
		case 3: {
			value <<= 5;
			value |= (*(u32*)(data + 8)) & 0xf000001f;
			CopyMemory(data + 8, &value, sizeof(u32));
			break;
		}
		}
#else 
		data[link_index] = value;
#endif 

	}

	ICF	void light(u8 value)
	{
#ifdef _USE_NODE_POSITION_11
		data_light = value;
#else 
		data[10] |= value << 4;
#endif

		//
	};

public:
	struct SCover
	{
		u16			cover0 : 4;
		u16			cover1 : 4;
		u16			cover2 : 4;
		u16			cover3 : 4;

		ICF	u16	cover(u8 index) const
		{
			switch (index) {
			case 0: return(cover0);
			case 1: return(cover1);
			case 2: return(cover2);
			case 3: return(cover3);
			default: NODEFAULT;
			}
#ifdef DEBUG
			return				(u8(-1));
#endif
		}
	};

	SCover			high;
	SCover			low;
	u16				plane;
	NodePosition	p;

	// 32 + 16 + 40 + 92 = 180 bits = 24.5 bytes => 25 bytes

	ICF	u32	link(u8 index) const
	{
#ifndef _USE_NODE_POSITION_11

		switch (index)
		{
		case 0:
			return	((*(u32*)data) & 0x007fffff);
		case 1:
			return	(((*(u32*)(data + 2)) >> 7) & 0x007fffff);
		case 2:
			return	(((*(u32*)(data + 5)) >> 6) & 0x007fffff);
		case 3:
			return	(((*(u32*)(data + 8)) >> 5) & 0x007fffff);

		default:
			NODEFAULT;
		}

#else 
		return data[index];
#endif


#ifdef DEBUG
		return			(0);
#endif
	}

	friend class	CLevelGraph;
	friend struct	CNodeCompressed;
	friend class	CNodeRenumberer;
	friend class	CRenumbererConverter;
};

#endif

struct SNodePositionOld
{
	s16				x;
	u16				y;
	s16				z;
};
#pragma pack	(pop)

#ifdef _EDITOR
typedef	SNodePositionOld NodePosition;
#endif

const u32 XRCL_CURRENT_VERSION = 18; //17;	// input
const u32 XRCL_PRODUCTION_VERSION = 14;	// output 
const u32 CFORM_CURRENT_VERSION = 4;

#ifdef _USE_NODE_POSITION_11
const u32 XRAI_CURRENT_VERSION = 11;
const u32 MAX_NODE_BIT_COUNT = 31;
#else 
const u32 XRAI_CURRENT_VERSION = 10;
const u32 MAX_NODE_BIT_COUNT = 24;
#endif
#endif // xrLevelH