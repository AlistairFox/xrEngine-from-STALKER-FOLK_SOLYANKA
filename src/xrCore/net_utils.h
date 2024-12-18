#pragma once

#include "client_id.h"

#pragma pack(push,1)

const	u32			NET_PacketSizeLimit = (48 * 1024) - 64;

struct XRCORE_API IIniFileStream
{
	virtual void	 __stdcall 	move_begin() = 0;

	virtual void	 __stdcall 	w_float(float a) = 0;
	virtual void 	 __stdcall 	w_vec3(const Fvector& a) = 0;
	virtual void 	 __stdcall 	w_vec4(const Fvector4& a) = 0;
	virtual void 	 __stdcall 	w_u64(u64 a) = 0;
	virtual void 	 __stdcall 	w_s64(s64 a) = 0;
	virtual void 	 __stdcall 	w_u32(u32 a) = 0;
	virtual void 	 __stdcall 	w_s32(s32 a) = 0;
	virtual void 	 __stdcall 	w_u16(u16 a) = 0;
	virtual void 	__stdcall	w_s16(s16 a) = 0;
	virtual void	__stdcall	w_u8(u8 a) = 0;
	virtual void	__stdcall	w_s8(s8 a) = 0;
	virtual void	__stdcall	w_stringZ(LPCSTR S) = 0;

	virtual void	__stdcall	r_vec3(Fvector&) = 0;
	virtual void	__stdcall	r_vec4(Fvector4&) = 0;
	virtual void	__stdcall	r_float(float&) = 0;
	virtual void	__stdcall	r_u8(u8&) = 0;
	virtual void	__stdcall	r_u16(u16&) = 0;
	virtual void	__stdcall	r_u32(u32&) = 0;
	virtual void	__stdcall	r_u64(u64&) = 0;
	virtual void	__stdcall	r_s8(s8&) = 0;
	virtual void	__stdcall	r_s16(s16&) = 0;
	virtual void	__stdcall	r_s32(s32&) = 0;
	virtual void	__stdcall	r_s64(s64&) = 0;

	virtual void	__stdcall	r_string(LPSTR dest, u32 dest_size) = 0;
	//	virtual void	__stdcall	r_tell			()							= 0;
	//	virtual void	__stdcall	r_seek			(u32 pos)					= 0;
	virtual void	__stdcall	skip_stringZ() = 0;
};


#define INI_W(what_to_do)\
if(inistream)\
{\
	inistream->what_to_do;\
}

#define INI_ASSERT(what_to_do)\
{\
if(inistream)\
	R_ASSERT3(0,#what_to_do,"not implemented");\
}

struct NET_Buffer
{
	BYTE	data[NET_PacketSizeLimit];
	u32		count;
	u32		max_buffer_size = NET_PacketSizeLimit;
};

struct NET_BufferSteam
{
	BYTE* data;	// 1MB
	u32		count;
	u32		max_buffer_size;
};

template<typename Buffer>
class NET_PacketBase
{

private:
	Qvector3 vectorQuantize_netutils(Fvector3 pos)
	{
		float normalizedX = (pos.x + 4096.0f) / 8192.0f;
		float normalizedY = (pos.y + 4096.0f) / 8192.0f;
		float normalizedZ = (pos.z + 4096.0f) / 8192.0f;

		u16 quantizedX = static_cast<u16>(normalizedX * 65535.0f);
		u16 quantizedY = static_cast<u16>(normalizedY * 65535.0f);
		u16 quantizedZ = static_cast<u16>(normalizedZ * 65535.0f);

		return Qvector3().set(quantizedX, quantizedY, quantizedZ);
	}

	Fvector3 vectorDeQuantize_netutils(Qvector3 pos)
	{
		float restoredX = (static_cast<float>(pos.x) / 65535.0f) * 8192.0f - 4096.0f;	// 255
		float restoredY = (static_cast<float>(pos.y) / 65535.0f) * 8192.0f - 4096.0f;	// 255
		float restoredZ = (static_cast<float>(pos.z) / 65535.0f) * 8192.0f - 4096.0f;	// 255

		return Fvector3().set(restoredX, restoredY, restoredZ);
	}


public:
	IIniFileStream* inistream;

	void            construct(const void* data, unsigned size)
	{
		memcpy(B.data, data, size);
		B.count = size;
	}

	Buffer			B;
	u32				r_pos;
	u32				timeReceive;
	bool			w_allow;


	NET_PacketBase() :inistream(NULL), w_allow(true) {}

	// BASE CALLS
	void	w(const void* p, u32 count)
	{
		R_ASSERT(B.count + count < B.max_buffer_size);
		CopyMemory(&B.data[B.count], p, count);
		B.count += count;
	}

	void		r(void* p, u32 count)
	{
		R_ASSERT(r_pos + count < B.max_buffer_size);
		CopyMemory(p, &B.data[r_pos], count);
		r_pos += count;
	}


	// writing - main
	void write_start() { B.count = 0;				INI_W(move_begin()); }
	void	w_begin(u16 type) { B.count = 0;	w_u16(type); }

	struct W_guard
	{
		bool* guarded;
		W_guard(bool* b) :guarded(b) { *b = true; }
		~W_guard() { *guarded = false; }
	};

	u32	w_tell() { return B.count; }

	// writing - utilities
	void	w_float(float a) { W_guard g(&w_allow); w(&a, 4);				INI_W(w_float(a)); }			// float
	void w_vec3(const Fvector& a) { W_guard g(&w_allow);  w(&a, 3 * sizeof(float)); INI_W(w_vec3(a)); }			// vec3
	void w_vec4(const Fvector4& a) { W_guard g(&w_allow);  w(&a, 4 * sizeof(float)); INI_W(w_vec4(a)); }			// vec4
	void w_u64(u64 a) { W_guard g(&w_allow);  w(&a, 8);				INI_W(w_u64(a)); }			// qword (8b)
	void w_s64(s64 a) { W_guard g(&w_allow);  w(&a, 8);				INI_W(w_s64(a)); }			// qword (8b)
	void w_u32(u32 a) { W_guard g(&w_allow);  w(&a, 4);				INI_W(w_u32(a)); }			// dword (4b)
	void w_s32(s32 a) { W_guard g(&w_allow);  w(&a, 4);				INI_W(w_s32(a)); }			// dword (4b)
	void w_u16(u16 a) { W_guard g(&w_allow);  w(&a, 2);				INI_W(w_u16(a)); }			// word (2b)
	void w_s16(s16 a) { W_guard g(&w_allow);  w(&a, 2);				INI_W(w_s16(a)); }			// word (2b)
	void	w_u8(u8 a) { W_guard g(&w_allow);  w(&a, 1);				INI_W(w_u8(a)); }			// byte (1b)
	void	w_s8(s8 a) { W_guard g(&w_allow);  w(&a, 1);				INI_W(w_s8(a)); }			// byte (1b)

	void w_float_q16(float a, float min, float max)
	{
		VERIFY(a >= min && a <= max);
		float q = (a - min) / (max - min);
		w_u16(u16(iFloor(q * 65535.f + 0.5f)));
	}
	void w_float_q8(float a, float min, float max)
	{
		VERIFY(a >= min && a <= max);
		float q = (a - min) / (max - min);
		w_u8(u8(iFloor(q * 255.f + 0.5f)));
	}
	void w_angle16(float a) { w_float_q16(angle_normalize(a), 0, PI_MUL_2); }
	void w_angle8(float a) { w_float_q8(angle_normalize(a), 0, PI_MUL_2); }
	void w_dir(const Fvector& D) { w_u16(pvCompress(D)); }
	void w_sdir(const Fvector& D) {
		Fvector C;
		float mag = D.magnitude();
		if (mag > EPS_S) {
			C.div(D, mag);
		}
		else {
			C.set(0, 0, 1);
			mag = 0;
		}
		w_dir(C);
		w_float(mag);
	}

	void w_stringZ(LPCSTR S) { W_guard g(&w_allow); w(S, (u32)xr_strlen(S) + 1);	INI_W(w_stringZ(S)); }
	void w_stringZ(const shared_str& p)
	{
		W_guard g(&w_allow);
		if (*p)
			w(*p, p.size() + 1);
		else {
			IIniFileStream* tmp = inistream;
			inistream = NULL;
			w_u8(0);
			inistream = tmp; //hack -(
		}

		INI_W(w_stringZ(p.c_str()));
	}
	void w_matrix(Fmatrix& M)
	{
		w_vec3(M.i);
		w_vec3(M.j);
		w_vec3(M.k);
		w_vec3(M.c);
	}

	void w_clientID(ClientID& C) { w_u32(C.value()); }

	void	w_chunk_open8(u32& position)
	{
		position = w_tell();
		w_u8(0);
		INI_ASSERT(w_chunk_open8)
	}

	void w_chunk_close8(u32 position)
	{
		u32 size = u32(w_tell() - position) - sizeof(u8);
		VERIFY(size < 256);
		u8			_size = (u8)size;
		w_seek(position, &_size, sizeof(_size));
		INI_ASSERT(w_chunk_close8)
	}

	void	w_chunk_open16(u32& position)
	{
		position = w_tell();
		w_u16(0);
		INI_ASSERT(w_chunk_open16)
	}

	void w_chunk_close16(u32 position)
	{
		u32 size = u32(w_tell() - position) - sizeof(u16);
		VERIFY(size < 65536);
		u16			_size = (u16)size;
		w_seek(position, &_size, sizeof(_size));
		INI_ASSERT(w_chunk_close16)
	}

	// reading
	void read_start() { r_pos = 0; INI_W(move_begin()); }

	u32 r_begin(u16& type)	// returns time of receiving
	{
		r_pos = 0;
		if (!inistream)
			r_u16(type);
		else
			inistream->r_u16(type);

		return		timeReceive;
	}

	void w_seek(u32 pos, const void* p, u32 count)
	{
		CopyMemory(&B.data[pos], p, count);
	}

	void r_seek(u32 pos)
	{
		r_pos = pos;
	}

	u32 r_tell()
	{
		return			r_pos;
	}

	BOOL r_eof()
	{
		return			(r_pos >= B.count);
	}

	u32 r_elapsed()
	{
		return			(B.count - r_pos);
	}

	void r_advance(u32 size)
	{
		r_pos += size;
		VERIFY(r_pos <= B.count);
	}

	// reading - utilities
	void r_vec3(Fvector& A)
	{
		if (!inistream)
			r(&A, sizeof(Fvector));
		else
			inistream->r_vec3(A);
	} // vec3

	void r_vec4(Fvector4& A)
	{
		if (!inistream)
			r(&A, sizeof(Fvector4));
		else
			inistream->r_vec4(A);
	} // vec4

	void r_float(float& A)
	{
		if (!inistream)
			r(&A, sizeof(float));
		else
			inistream->r_float(A);
	} // float

	void r_u64(u64& A)
	{
		if (!inistream)
			r(&A, sizeof(u64));
		else
			inistream->r_u64(A);
	} // qword (8b)

	void r_s64(s64& A)
	{
		if (!inistream)
			r(&A, sizeof(s64));
		else
			inistream->r_s64(A);
	} // qword (8b)

	void r_u32(u32& A)
	{
		if (!inistream)
			r(&A, sizeof(u32));
		else
			inistream->r_u32(A);
	} // dword (4b)

	void r_s32(s32& A)
	{
		if (!inistream)
			r(&A, sizeof(s32));
		else
			inistream->r_s32(A);
	} // dword (4b)

	void r_u16(u16& A)
	{
		if (!inistream)
			r(&A, sizeof(u16));
		else
			inistream->r_u16(A);
	} // word (2b)

	void r_s16(s16& A)
	{
		if (!inistream)
			r(&A, sizeof(s16));
		else
			inistream->r_s16(A);
	} // word (2b)

	void r_u8(u8& A)
	{
		if (!inistream)
			r(&A, sizeof(u8));
		else
			inistream->r_u8(A);
	} // byte (1b)

	void r_s8(s8& A)
	{
		if (!inistream)
			r(&A, sizeof(s8));
		else
			inistream->r_s8(A);
	} // byte (1b)

	// IReader compatibility
	Fvector r_vec3()
	{
		Fvector		A;
		r_vec3(A);
		return		(A);
	}

	Fvector4 r_vec4()
	{
		Fvector4	A;
		r_vec4(A);
		return		(A);
	}

	float r_float_q8(float min, float max)
	{
		float		A;
		r_float_q8(A, min, max);
		return		A;
	}

	float r_float_q16(float min, float max)
	{
		float		A;
		r_float_q16(A, min, max);
		return		A;
	}

	float r_float()
	{
		float		A;
		r_float(A);
		return		(A);
	}

	u64 r_u64()
	{
		u64 		A;
		r_u64(A);
		return		(A);
	}

	s64 r_s64()
	{
		s64 		A;
		r_s64(A);
		return		(A);
	} // qword (8b)

	u32 r_u32()
	{
		u32 		A;
		r_u32(A);
		return		(A);
	} // dword (4b)

	s32 r_s32()
	{
		s32			A;
		r_s32(A);
		return		(A);
	} // dword (4b)

	u16 r_u16()
	{
		u16			A;
		r_u16(A);
		return		(A);
	} // word (2b)

	s16 r_s16()
	{
		s16			A;
		r_s16(A);
		return		(A);
	} // word (2b)

	u8 r_u8()
	{
		u8			A;
		r_u8(A);
		return		(A);
	} // byte (1b)

	s8 r_s8()
	{
		s8			A;
		r_s8(A);
		return		(A);
	}

	void r_float_q16(float& A, float min, float max)
	{
		u16			val;
		r_u16(val);
		A = (float(val) * (max - min)) / 65535.f + min;		// floating-point-error possible
		VERIFY((A >= min - EPS_S) && (A <= max + EPS_S));
	}

	void r_float_q8(float& A, float min, float max)
	{
		u8			val;
		r_u8(val);
		A = (float(val) / 255.0001f) * (max - min) + min;	// floating-point-error possible
		VERIFY((A >= min) && (A <= max));
	}

	void r_angle16(float& A)
	{
		r_float_q16(A, 0, PI_MUL_2);
	}

	void r_angle8(float& A)
	{
		r_float_q8(A, 0, PI_MUL_2);
	}

	void r_dir(Fvector& A)
	{
		u16			t;
		r_u16(t);
		pvDecompress(A, t);
	}

	void r_sdir(Fvector& A)
	{
		u16				t;
		float			s;
		r_u16(t);
		r_float(s);
		pvDecompress(A, t);
		A.mul(s);
	}

	void r_stringZ(LPSTR S)
	{
		if (!inistream)
		{
			LPCSTR	data = LPCSTR(&B.data[r_pos]);
			size_t	len = xr_strlen(data);
			r(S, (u32)len + 1);
		}
		else {
			inistream->r_string(S, 4096);//???
		}
	}

	void r_stringZ(xr_string& dest)
	{
		if (!inistream)
		{
			dest = LPCSTR(&B.data[r_pos]);
			r_advance(u32(dest.size() + 1));
		}
		else {
			string4096		buff;
			inistream->r_string(buff, sizeof(buff));
			dest = buff;
		}
	}

	void r_stringZ(shared_str& dest)
	{
		if (!inistream)
		{
			dest = LPCSTR(&B.data[r_pos]);
			r_advance(dest.size() + 1);
		}
		else {
			string4096		buff;
			inistream->r_string(buff, sizeof(buff));
			dest = buff;
		}
	}

	void skip_stringZ()
	{
		if (!inistream)
		{
			LPCSTR	data = LPCSTR(&B.data[r_pos]);
			u32	len = xr_strlen(data);
			r_advance(len + 1);
		}
		else {
			inistream->skip_stringZ();
		}
	}

	void r_matrix(Fmatrix& M)
	{
		r_vec3(M.i);	M._14_ = 0;
		r_vec3(M.j);	M._24_ = 0;
		r_vec3(M.k);	M._34_ = 0;
		r_vec3(M.c);	M._44_ = 1;
	}

	void r_clientID(ClientID& C)
	{
		u32				tmp;
		r_u32(tmp);
		C.set(tmp);
	}

	void r_stringZ_s(LPSTR string, u32 const size)
	{
		if (inistream)
		{
			inistream->r_string(string, size);
			return;
		}

		LPCSTR data = LPCSTR(B.data + r_pos);
		u32 length = xr_strlen(data);
		R_ASSERT2((length + 1) <= size, "buffer overrun");
		r(string, length + 1);
	}

	template <u32 size>
	void	r_stringZ_s(char(&string)[size])
	{
		r_stringZ_s(string, size);
	}


	// VECTOR 3 Qvector
	Qvector		r_vec3Q() { Qvector A; r(&A, sizeof(A));	return A; };
	void		w_vec3Q(Qvector& A) { w(&A, sizeof(A)); };

	void		w_quantize3(Fvector3& A)
	{
		w_vec3Q(vectorQuantize_netutils(A));
	};

	void		r_quantize3(Fvector3& A)
	{
		A = vectorDeQuantize_netutils(r_vec3Q());
		//	return A;
	};

};

class XRCORE_API NET_Packet : public NET_PacketBase<NET_Buffer>
{
	//	~NET_Packet() { delete[](B.data); }
};


class XRCORE_API NET_PacketSteam : public NET_PacketBase<NET_BufferSteam>
{
	//	~NET_PacketSteam() { delete[](B.data); }
public:
	NET_PacketSteam(u32 max_packet)
	{
		B.data = xr_alloc<BYTE>(max_packet);
		B.count = 0;
		B.max_buffer_size = max_packet;

		w_allow = true;
	}

	~NET_PacketSteam()
	{
		xr_delete(B.data);
	}
};

#pragma pack(pop)
