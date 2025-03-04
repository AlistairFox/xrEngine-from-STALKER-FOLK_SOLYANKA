#include "stdafx.h"
#include "igame_level.h"
#include "igame_persistent.h"

#include "xrSheduler.h"
#include "xr_object_list.h"
#include "std_classes.h"

#include "xr_object.h"
#include "../xrCore/net_utils.h"

#include "CustomHUD.h"

class fClassEQ {
	CLASS_ID cls;
public:
	fClassEQ(CLASS_ID C) : cls(C) {};
	IC bool operator() (CObject* O) { return cls==O->CLS_ID; }
};

CObjectList::CObjectList	( ) : m_owner_thread_id		(GetCurrentThreadId())
{
	ZeroMemory				(map_NETID, 0xffff*sizeof(CObject*));
}

CObjectList::~CObjectList	( )
{
	R_ASSERT				( objects_active.empty()	);
	R_ASSERT				( objects_sleeping.empty()	);
	R_ASSERT				( destroy_queue.empty()		);
}

CObject*	CObjectList::FindObjectByName	( shared_str name )
{
	for (Objects::iterator I=objects_active.begin(); I!=objects_active.end(); I++)
		if ((*I)->cName().equal(name))	return (*I);
	for (Objects::iterator I=objects_sleeping.begin(); I!=objects_sleeping.end(); I++)
		if ((*I)->cName().equal(name))	return (*I);
	return	NULL;
}

CObject*	CObjectList::FindObjectByName	( LPCSTR name )
{
	return	FindObjectByName				(shared_str(name));
}

CObject*	CObjectList::FindObjectByCLS_ID	( CLASS_ID cls )
{
 	auto itActive = std::find_if(objects_active.begin(),objects_active.end(),fClassEQ(cls));
	if (itActive != objects_active.end())
		return *itActive;
 
	auto itSleep = std::find_if(objects_sleeping.begin(),objects_sleeping.end(),fClassEQ(cls));
	if (itSleep != objects_sleeping.end())
		return *itSleep;
	  
	return	NULL;
}


void	CObjectList::o_remove		( Objects&	v,  CObject* O)
{
	auto iterator	= std::find(v.begin(),v.end(),O);
	if (iterator != v.end())
		v.erase					(iterator);
}

void	CObjectList::o_activate		( CObject*		O		)
{
	VERIFY						(O && O->processing_enabled());
	o_remove					(objects_sleeping,O);

	objects_active.push_back	(O);
}

void	CObjectList::o_sleep		( CObject*		O		)
{
	VERIFY	(O && !O->processing_enabled());
	
	o_remove(objects_active,O);
	objects_sleeping.push_back	(O);
}

#include "../xrServerEntities/clsid_game.h"
extern int MT_Work = 0;
extern int WorkDistance = 0;
void CObjectList::UpdateProcessing(CObject* O)
{
	if (!g_dedicated_server && WorkDistance)
		O->EnabledMP_Processing = O->Position().distance_to(Device.vCameraPosition) < 200;
	else
		O->EnabledMP_Processing = true;
}

void	CObjectList::SingleUpdate(CObject* O)
{ 
	if (!O->processing_enabled() || Device.dwFrame == O->dwFrame_UpdateCL)
		return;
  	if (O->H_Parent())
		SingleUpdate(O->H_Parent());
	O->UpdateCL();
 	Device.Statistic->UpdateClient_updated++;
 	O->dwFrame_UpdateCL = Device.dwFrame;
} 
 
void CObjectList::Update		(bool bForce)
{
	if ( !Device.Paused() || bForce )
	{
		OPTICK_EVENT("UpdateCL");
		// Clients
		if (Device.fTimeDelta>EPS_S || bForce)			
		{
			// Select Crow-Mode
			Device.Statistic->UpdateClient_updated	= 0;
 			Device.Statistic->UpdateClient_active = objects_active.size();

			Device.Statistic->UpdateClient.Begin();

			if (LastUpdateTime < Device.dwTimeGlobal)
			{
  				Objects objects_processed;
 				for (auto& O : objects_active)
				{
					UpdateProcessing(O);
 					objects_processed.push_back(O);
				}
 				
				std::sort(objects_processed.begin(), objects_processed.end(), [&](CObject* O, CObject* O2)
				{
					return O->dwFrame_UpdateCL < O2->dwFrame_UpdateCL && !O->EnabledMP_Processing;
				});
			
				u32 CurrentObjects = 0;
				for (auto& O : objects_processed)
				{	 
					if (O->EnabledMP_Processing || CurrentObjects > 100)
						continue; 	
   					SingleUpdate(O);
					CurrentObjects++;
  				}
			
				LastUpdateTime = Device.dwTimeGlobal + 100;
			}

			
			for (auto& O : objects_active)
			{
				if (!O->EnabledMP_Processing)
				 	continue;
 				SingleUpdate(O);
			}
			Device.Statistic->UpdateClient.End();
		}
	}

	// Destroy
	if (!destroy_queue.empty()) 
	{
		{
			OPTICK_EVENT("Destroy_queue (active)");
			// Info
			for (auto& O : objects_active)
			for (int it = destroy_queue.size() - 1; it >= 0; it--)
				O->net_Relcase(destroy_queue[it]);
		}

 		for (auto& O : objects_sleeping)
		for (int it = destroy_queue.size()-1; it>=0; it--)
			O->net_Relcase	(destroy_queue[it]);

		{
			OPTICK_EVENT("Destroy_queue (sounds)");
			for (int it = destroy_queue.size() - 1; it >= 0; it--)
				Sound->object_relcase(destroy_queue[it]);
		}
		 
		{
			OPTICK_EVENT("Destroy_queue (relcase)");
			for (auto& O : m_relcase_callbacks)
			{
				for (auto& D : destroy_queue)
				{
					O.m_Callback(D);
					g_hud->net_Relcase(D);
				}
			}
		}

		{
			OPTICK_EVENT("Destroy_from_list");
			// Destroy
			for (int it = destroy_queue.size() - 1; it >= 0; it--)
			{
				CObject* O = destroy_queue[it];
 				O->net_Destroy();
				Destroy(O);
 			}
 		}
		
		destroy_queue.clear	();
	}
}

void CObjectList::net_Register		(CObject* O)
{
	R_ASSERT		(O);
	R_ASSERT		(O->ID() < 0xffff);
 	map_NETID[O->ID()] = O;
}

void CObjectList::net_Unregister	(CObject* O)
{
	if (O->ID() < 0xffff)
		map_NETID[O->ID()] = NULL;
}

int	g_Dump_Export_Obj = 0;

u32	CObjectList::net_Export			(NET_Packet* _Packet,	u32 start, u32 max_object_size	)
{
	if (g_Dump_Export_Obj)
		Msg("---- net_export --- ");

	NET_Packet& Packet	= *_Packet;
	u32			position;
	for (; start<objects_active.size() + objects_sleeping.size(); start++)			{
		CObject* P = (start<objects_active.size()) ? objects_active[start] : objects_sleeping[start-objects_active.size()];
		if (P->net_Relevant() && !P->getDestroy())	{			
			Packet.w_u16			(u16(P->ID())	);
			Packet.w_chunk_open8	(position);
			P->net_Export			(Packet);
 
			if (g_Dump_Export_Obj)
			{
				u32 size				= u32		(Packet.w_tell()-position)-sizeof(u8);
				Msg("* %s : %d", *(P->cNameSect()), size);
			}
			Packet.w_chunk_close8	(position);
 
			if (max_object_size >= (NET_PacketSizeLimit - Packet.w_tell()))
				break;
		}
	}
	if (g_Dump_Export_Obj)
		Msg("------------------- ");
	return	start+1;
}

int	g_Dump_Import_Obj = 0;

void CObjectList::net_Import		(NET_Packet* Packet)
{
	if (g_Dump_Import_Obj) 
		Msg("---- net_import --- ");

	while (!Packet->r_eof())
	{
		u16 ID;		Packet->r_u16	(ID);
		Packet->r_u32   (m_update_time);

		u8  size;	Packet->r_u8	(size);
		CObject* P  = net_Find		(ID);
		if (P)		
		{

			u32 rsize = Packet->r_tell();			
			
			P->net_Import	(*Packet);

			if (g_Dump_Import_Obj) Msg("* %s : %d - %d", *(P->cNameSect()), size, Packet->r_tell() - rsize);

		}
		else		Packet->r_advance(size);
	}

	if (g_Dump_Import_Obj) 
		Msg("------------------- ");
}

u32 CObjectList::net_Import_Time()
{
	return m_update_time;
}
 
void CObjectList::Load		()
{
	R_ASSERT				(objects_active.empty() && destroy_queue.empty() && objects_sleeping.empty());
}

void CObjectList::Unload	( )
{
	if (objects_sleeping.size() || objects_active.size())
		Msg			("! objects-leaked: %d",objects_sleeping.size() + objects_active.size());

	// Destroy objects
	while (objects_sleeping.size())
	{
		CObject*	O	= objects_sleeping.back	();
		Msg				("! [%x] s[%4d]-[%s]-[%s]", O, O->ID(), *O->cNameSect(), *O->cName());
		O->setDestroy	( true );
		O->net_Destroy	(   );
		Destroy			( O );
	}

	while (objects_active.size())
	{
		CObject*	O	= objects_active.back	();
		Msg				("! [%x] a[%4d]-[%s]-[%s]", O, O->ID(), *O->cNameSect(), *O->cName());
		O->setDestroy	( true );
		O->net_Destroy	(   );
		Destroy			( O );
	}

	// objects_mt.clear();
	// objects_updates.clear();
}

CObject* CObjectList::Create				( LPCSTR	name	)
{
	CObject*	O				= g_pGamePersistent->ObjectPool.create(name);
 	objects_sleeping.push_back	(O);
	return						O;
}

void CObjectList::Destroy(CObject* O)
{
	if (0 == O)
		return;

	net_Unregister(O);

	// active/inactive
	bool isActive = false;
	{
		OPTICK_EVENT("Finding Active");
 		auto itActive	= std::find(objects_active.begin(), objects_active.end(), O);
		if (itActive	!= objects_active.end())
		{
			objects_active.erase(itActive); isActive = true;
		}
	}

	if (!isActive)
	{
		OPTICK_EVENT("Finding Sleeping");
		auto itSleep	= std::find(objects_sleeping.begin(),objects_sleeping.end(),O);
		if	(itSleep	!=objects_sleeping.end())
		{
			objects_sleeping.erase			(itSleep);
 		}	 
	}

	g_pGamePersistent->ObjectPool.destroy	(O);
}

void CObjectList::relcase_register		(RELCASE_CALLBACK cb, int *ID)
{
	*ID								= m_relcase_callbacks.size();
	m_relcase_callbacks.push_back	(SRelcasePair(ID,cb));
}

void CObjectList::relcase_unregister	(int* ID)
{
	VERIFY							(m_relcase_callbacks[*ID].m_ID==ID);
	m_relcase_callbacks[*ID]		= m_relcase_callbacks.back();
	*m_relcase_callbacks.back().m_ID= *ID;
	m_relcase_callbacks.pop_back	();
}

void CObjectList::dump_list(Objects& v, LPCSTR reason)
{
#ifdef DEBUG
	Objects::iterator it = v.begin();
	Objects::iterator it_e = v.end();
	Msg("----------------dump_list [%s]",reason);
	for(;it!=it_e;++it)
		Msg("%x - name [%s] ID[%d] parent[%s] getDestroy()=[%s]", 
			(*it),
			(*it)->cName().c_str(), 
			(*it)->ID(), 
			((*it)->H_Parent())?(*it)->H_Parent()->cName().c_str():"", 
			((*it)->getDestroy())?"yes":"no" );
#endif // #ifdef DEBUG
}

bool CObjectList::dump_all_objects()
{
	dump_list(destroy_queue,"destroy_queue");
	dump_list(objects_active,"objects_active");
	dump_list(objects_sleeping,"objects_sleeping");
 
	return false;
}

void CObjectList::register_object_to_destroy(CObject *object_to_destroy)
{
	VERIFY					(!registered_object_to_destroy(object_to_destroy));
 	destroy_queue.push_back	(object_to_destroy);
	for(auto& O : objects_active)
	{
 		if(!O->getDestroy() && O->H_Parent()==object_to_destroy)
		{
			Msg("setDestroy called, but not-destroyed child found parent[%d] child[%d]",object_to_destroy->ID(), O->ID(), Device.dwFrame);
			O->setDestroy(TRUE);
		}
	}
 
	for(auto& O : objects_sleeping)
	{
 		if(!O->getDestroy() && O->H_Parent()==object_to_destroy)
		{
			Msg("setDestroy called, but not-destroyed child found parent[%d] child[%d]",object_to_destroy->ID(), O->ID(), Device.dwFrame);
			O->setDestroy(TRUE);
		}
	}
}
 