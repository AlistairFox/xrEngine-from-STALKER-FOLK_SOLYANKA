#include "stdafx.h"
#include "xrSheduler.h"
#include "xr_object.h"

//#define DEBUG_SCHEDULER

float			psShedulerCurrent		= 10.f	;
float			psShedulerTarget		= 10.f	;
const	float	psShedulerReaction		= 0.1f	;
BOOL			g_bSheduleInProgress	= FALSE	;

//-------------------------------------------------------------------------------------
void CSheduler::Initialize		()
{
	m_current_step_obj	= NULL;
	m_processing_now	= false;
}

void CSheduler::Destroy			()
{
	internal_Registration		();

	for (u32 it=0; it<Items.size(); it++)
	{
		if (0==Items[it].Object)	
		{
			Items.erase(Items.begin()+it);
			it	--;
		}
	}

	ItemsRT.clear		();
	Items.clear			();
	ItemsProcessed.clear();
	Registration.clear	();
}

void	CSheduler::internal_Registration()
{
	for (u32 it=0; it<Registration.size(); it++)
	{
		ItemReg&	R	= Registration	[it];
		if (R.OP)	
		{
			// register
			// search for paired "unregister"
			BOOL	bFoundAndErased		= FALSE;
			for (u32 pair=it+1; pair<Registration.size(); pair++)
			{
				ItemReg&	R_pair	= Registration	[pair];
				if	((!R_pair.OP)&&(R_pair.Object == R.Object))	{
					bFoundAndErased		= TRUE;
					Registration.erase	(Registration.begin()+pair	);
					break				;
				}
			}

			// register if non-paired
			if (!bFoundAndErased)	
			{
				internal_Register		(R.Object,R.RT);
			}
		}
		else		{
			// unregister
			internal_Unregister			(R.Object,R.RT);
		}
	}
	Registration.clear	();
}

void CSheduler::internal_Register	(ISheduled* O, BOOL RT)
{
	VERIFY	(!O->shedule.b_locked)	;
	if (RT)
	{
		// Fill item structure
		Item						TNext;
		TNext.dwTimeForExecute		= Device.dwTimeGlobal;
		TNext.dwTimeOfLastExecute	= Device.dwTimeGlobal;
		TNext.Object				= O;
		TNext.scheduled_name		= O->shedule_Name();
		O->shedule.b_RT				= TRUE;

		ItemsRT.push_back			(TNext);
	} else {
		// Fill item structure
		Item						TNext;
		TNext.dwTimeForExecute		= Device.dwTimeGlobal;
		TNext.dwTimeOfLastExecute	= Device.dwTimeGlobal;
		TNext.Object				= O;
		TNext.scheduled_name		= O->shedule_Name();
		O->shedule.b_RT				= FALSE;

		// Insert into priority Queue
		Push						(TNext);
	}
}

bool CSheduler::internal_Unregister	(ISheduled* O, BOOL RT, bool warn_on_not_found)
{
	//the object may be already dead
 	if (RT)
	{
		for (u32 i=0; i<ItemsRT.size(); i++)
		{
			if (ItemsRT[i].Object==O)
			{
				ItemsRT.erase(ItemsRT.begin()+i);
				return				(true);
			}
		}
	}
	else 
	{
		for (u32 i=0; i<Items.size(); i++)
		{
			if (Items[i].Object==O)
			{
				Items[i].Object	= NULL;
				return				(true);
			}
		}
	}
	if (m_current_step_obj == O)
	{
		m_current_step_obj = NULL;
		return true;
	}
	return							(false);
}

void	CSheduler::Register		(ISheduled* A, BOOL RT				)
{
	VERIFY		(!Registered(A));

	ItemReg		R;
	R.OP		= TRUE				;
	R.RT		= RT				;
	R.Object	= A					;
	R.Object->shedule.b_RT	= RT	;
	Registration.push_back	(R);
}

void	CSheduler::Unregister	(ISheduled* A						)
{
	if (m_processing_now) 
	{
		if (internal_Unregister(A,A->shedule.b_RT,false))
			return;
	}

	ItemReg		R;
	R.OP		= FALSE				;
	R.RT		= A->shedule.b_RT	;
	R.Object	= A					;

	Registration.push_back			(R);
}

void CSheduler::EnsureOrder		(ISheduled* Before, ISheduled* After)
{
	for (u32 i=0; i<ItemsRT.size(); i++)
	{
		if (ItemsRT[i].Object==After) 
		{
			Item	A			= ItemsRT[i];
			ItemsRT.erase		(ItemsRT.begin()+i);
			ItemsRT.push_back	(A);
			return;
		}
	}
}

void CSheduler::Push				(Item& I)
{
	Items.push_back	(I);
	std::push_heap	(Items.begin(), Items.end());
}

void CSheduler::Pop					()
{
	std::pop_heap	(Items.begin(), Items.end());
	Items.pop_back	();
}


#include "../xrServerEntities/clsid_game.h"
#include "IGame_Persistent.h"


u32 oldTimeUpdate;

struct time_data
{
	shared_str object_name;
	u64 ticks;
};

typedef time_data TDATA;

IC bool time_sort(TDATA E1, TDATA E2)
{
	return E1.ticks > E2.ticks;
};


IC bool time_find(TDATA E1, LPCSTR name)
{
	return E1.object_name == name;
};
 
void CSheduler::ProcessStep			()
{
	// Normal priority
	u32		dwTime					= Device.dwTimeGlobal;
	
	for (int i=0; !Items.empty() && Top().dwTimeForExecute < dwTime; ++i)
	{
		// Update
		Item	T					= Top	();	Pop();
		u32		Elapsed				= dwTime-T.dwTimeOfLastExecute;
		bool	condition			= (NULL==T.Object || !T.Object->shedule_Needed());
		if (condition)
			continue;
		m_current_step_obj = T.Object;
		if (!m_current_step_obj)
			continue;

		// Calc next update interval
		u32		dwMin				= _max(u32(30),T.Object->shedule.t_min);
		u32		dwMax				= (1000+T.Object->shedule.t_max)/2;
		float	scale				= T.Object->shedule_Scale();
		u32		dwUpdate			= dwMin+iFloor(float(dwMax-dwMin)*scale);
 		clamp(dwUpdate, (u32)0, (u32)5000);

		u32 delta = clampr(Elapsed, u32(1), u32( _max(u32(T.Object->shedule.t_max), u32(1000)) ));
 		m_current_step_obj->shedule_Update(delta);
 		m_current_step_obj = NULL;

		// Fill item structure
		Item						TNext;
		TNext.dwTimeForExecute		= dwTime+dwUpdate;
		TNext.dwTimeOfLastExecute	= dwTime;
		TNext.Object				= T.Object;
		TNext.scheduled_name		= T.Object->shedule_Name();

		ItemsProcessed.push_back	(TNext);

		if (Device.dwPrecacheFrame==0 && CPU::QPC() > cycles_limit)		
		{
			// we have maxed out the load - increase heap
			psShedulerTarget		+= (psShedulerReaction * 3);
			break;
		}
	}
 
	// Push "processed" back
	while (ItemsProcessed.size())
	{
		Push	(ItemsProcessed.back())	;
		ItemsProcessed.pop_back		()	;
	}

	// always try to decrease target
	psShedulerTarget	-= psShedulerReaction;
}

void CSheduler::Update				()
{			 
	R_ASSERT						(Device.Statistic);
	// Initialize
	
	cycles_start					= CPU::QPC			();
	cycles_limit					= CPU::qpc_freq * u64 (iCeil(psShedulerCurrent)) / 1000i64 + cycles_start;
	internal_Registration			();
	g_bSheduleInProgress			= TRUE;
 
	// Realtime priority
	m_processing_now				= true;
	u32	dwTime						= Device.dwTimeGlobal;
 
	Device.Statistic->ShedulerLow.Begin();
	for (u32 it=0; it<ItemsRT.size(); it++)
	{
		Item&	T					= ItemsRT[it];
		R_ASSERT					(T.Object);

		if(!T.Object->shedule_Needed())
		{
			T.dwTimeOfLastExecute	= dwTime;
			continue;
		}

		u32	Elapsed					= dwTime-T.dwTimeOfLastExecute;
		T.Object->shedule_Update	(Elapsed);
		T.dwTimeOfLastExecute		= dwTime;
	}
	Device.Statistic->ShedulerLow.End();

	
	// Normal (sheduled)
	Device.Statistic->Sheduler.Begin();
 	ProcessStep						();
	Device.Statistic->Sheduler.End();
	 
	m_processing_now				= false;

	clamp							(psShedulerTarget,3.f,66.f);
	psShedulerCurrent				= 0.9f*psShedulerCurrent + 0.1f*psShedulerTarget;
	Device.Statistic->fShedulerLoad	= psShedulerCurrent;

	// Finalize
	g_bSheduleInProgress			= FALSE;
	internal_Registration			();
}




