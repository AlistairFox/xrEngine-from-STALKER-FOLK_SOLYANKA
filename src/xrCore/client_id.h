#ifndef client_idH
#define client_idH
#pragma once

#pragma pack(push,1)
class ClientID 
{
	u32 id;
public:
			ClientID		():id(0)						{};
			ClientID		(u32 val):id(val)				{};
	
	u32		value			()const							{return id;};
	void	set				(u32 v)							{id=v;};
	bool	compare			(u32 v) const					{return id == v;};
	bool	operator ==		(const ClientID& other)const	{return value() == other.value();};
	bool	operator !=		(const ClientID& other)const	{return value() != other.value();};
	bool	operator <		(const ClientID& other)const	{return value() < other.value();};
};
#pragma pack(pop)

class MasterServerID
{
	u32 id;
public: 
	MasterServerID() :id(0) {};
	MasterServerID(u32 val) : id(val) {};

	u32 value() const { return id; };
	void set(u32 val) { id = val; };
	bool	operator ==		(const MasterServerID& other) const { return value() == other.value(); };
	bool	operator !=		(const MasterServerID& other)const { return value() != other.value(); };
	bool	operator <		(const MasterServerID& other)const { return value() < other.value(); };

};

#endif
