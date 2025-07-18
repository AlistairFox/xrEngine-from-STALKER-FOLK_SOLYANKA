////////////////////////////////////////////////////////////////////////////
//	Module 		: profiler.cpp
//	Created 	: 23.07.2004
//  Modified 	: 23.07.2004
//	Author		: Dmitriy Iassenev
//	Description : Profiler
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "profiler.h"
#include "../xrEngine/gamefont.h"

#ifdef PROFILE_CRITICAL_SECTIONS
static volatile LONG					critical_section_counter = 0;

void add_profile_portion				(LPCSTR id, const u64 &time)
{
	if (!*id)
		return;

	if (!psAI_Flags.test(aiStats))
		return;
	
	if (!psDeviceFlags.test(rsStatistic))
		return;

	CProfileResultPortion			temp;
	temp.m_timer_id					= id;
	temp.m_time						= time;
	
	profiler().add_profile_portion	(temp);
}
#endif // PROFILE_CRITICAL_SECTIONS

CProfiler	*g_profiler			= 0;
LPCSTR		indent				= "  ";
char		white_character		= '.';

struct CProfilePortionPredicate {
	IC		bool operator()			(const CProfileResultPortion &_1, const CProfileResultPortion &_2) const
	{
		return					(xr_strcmp(_1.m_timer_id,_2.m_timer_id) < 0);
	}
};

CProfiler::CProfiler				()
#ifdef PROFILE_CRITICAL_SECTIONS
	:m_section("")
#endif // PROFILE_CRITICAL_SECTIONS
{
	m_actual							= true;
}

CProfiler::~CProfiler				()
{
#ifdef PROFILE_CRITICAL_SECTIONS
	set_add_profile_portion		(0);
#endif // PROFILE_CRITICAL_SECTIONS
}

IC	u32 compute_string_length		(LPCSTR str)
{
	LPCSTR						i, j = str;
	u32							count = 0;
	while ((i = strchr(j,'/')) != 0)
	{
		j = i					= i + 1;
		++count;
	}
	return						(count*xr_strlen(indent) + xr_strlen(j));
}

IC	void CProfiler::convert_string	(LPCSTR str, shared_str &out, u32 max_string_size)
{
	string256					m_temp;
	LPCSTR						i, j = str;
	u32							count = 0;
	while ((i = strchr(j,'/')) != 0)
	{
		j = i					= i + 1;
		++count;
	}
	xr_strcpy						(m_temp,"");
	for (u32 k = 0; k<count; ++k)
		xr_strcat					(m_temp,indent);
	xr_strcat						(m_temp,j);
	count						= xr_strlen(m_temp);
	
	//for ( ; count < max_string_size; ++count)
	//	m_temp[count]			= white_character;

	m_temp[max_string_size]		= 0;
	out							= m_temp;
}

void CProfiler::setup_timer			(LPCSTR timer_id, const u64 &timer_time, const u32 &call_count)
{
	string256					m_temp;
	float						_time = float(timer_time)*1000.f/CPU::qpc_freq;
	TIMERS::iterator			i = m_timers.find(timer_id);
	if (i == m_timers.end()) {
		xr_strcpy					(m_temp,timer_id);
		LPSTR					j,k = m_temp;
		while ((j = strchr(k,'/')) != 0) {
			*j					= 0;
			TIMERS::iterator	m = m_timers.find(m_temp);
			if (m == m_timers.end())
				m_timers.insert	(std::make_pair(shared_str(m_temp),CProfileStats()));
			*j					= '/';
			k					= j + 1;
		}
		i						= m_timers.insert(std::make_pair(shared_str(timer_id),CProfileStats())).first;

		CProfileStats			&current = (*i).second;
		current.m_min_time		= _time;
		current.m_max_time		= _time;
		current.m_total_time	= _time;
		current.m_count			= 1;
		current.m_call_count	= call_count;
		m_actual				= false;
	}
	else 
	{
		CProfileStats			&current = (*i).second;
		current.m_min_time		= _min(current.m_min_time,_time);
		current.m_max_time		= _max(current.m_max_time,_time);
		current.m_total_time	+= _time;
		++current.m_count;
		current.m_call_count	+= call_count;
	}

	if (_time > (*i).second.m_time)
		(*i).second.m_time		= _time;
	else
		(*i).second.m_time		= .01f*_time + .99f*(*i).second.m_time;

	(*i).second.m_update_time	= Device.dwTimeGlobal;
}

void CProfiler::clear				()
{
#ifdef PROFILE_CRITICAL_SECTIONS
	while (InterlockedExchange(&critical_section_counter,1))
		Sleep					(0);
#endif // PROFILE_CRITICAL_SECTIONS

	m_section.Enter				();
	m_portions.clear			();
	m_timers.clear				();
	m_section.Leave				();

	m_call_count				= 0;

#ifdef PROFILE_CRITICAL_SECTIONS
	InterlockedExchange			(&critical_section_counter,0);
#endif // PROFILE_CRITICAL_SECTIONS
}

void CProfiler::show_stats			(CGameFont *game_font, bool show)
{
 	if (!show)
	{
#ifdef PROFILE_CRITICAL_SECTIONS
		set_add_profile_portion	(0);
#endif // PROFILE_CRITICAL_SECTIONS
		clear					();
		return;
	}
 
	
#ifdef PROFILE_CRITICAL_SECTIONS
	set_add_profile_portion		(&::add_profile_portion);
#endif // PROFILE_CRITICAL_SECTIONS

	++m_call_count;
		
#ifdef PROFILE_CRITICAL_SECTIONS
	while (InterlockedExchange(&critical_section_counter,1))
		Sleep					(0);
#endif // PROFILE_CRITICAL_SECTIONS

	m_section.Enter				();
  
	if (!m_portions.empty()) 
	{
		std::sort				(m_portions.begin(),m_portions.end(),CProfilePortionPredicate());
		u64						timer_time = 0;
		u32						call_count = 0;

		PORTIONS::const_iterator	I = m_portions.begin(), J = I;
		PORTIONS::const_iterator	E = m_portions.end();
		for ( ; I != E; ++I) {
			if (xr_strcmp((*I).m_timer_id,(*J).m_timer_id)) {
				setup_timer		((*J).m_timer_id,timer_time,call_count);
				timer_time		= 0;
				call_count		= 0;
				J				= I;
			}

			++call_count;
			timer_time			+= (*I).m_time;
		}
		setup_timer				((*J).m_timer_id,timer_time,call_count);

		m_portions.clear		();

		m_section.Leave			();

		if (!m_actual) 
		{
			u32					max_string_size = 0;
			TIMERS::iterator	I = m_timers.begin();
			TIMERS::iterator	E = m_timers.end();
			for ( ; I != E; ++I)
				max_string_size	= _max(max_string_size,compute_string_length(*(*I).first));

			I					= m_timers.begin();

			for ( ; I != E; ++I)
				convert_string	(*(*I).first,(*I).second.m_name,max_string_size);

			m_actual			= true;
		}
	}
	else
		m_section.Leave			();

#ifdef PROFILE_CRITICAL_SECTIONS
	InterlockedExchange			(&critical_section_counter,0);
#endif // PROFILE_CRITICAL_SECTIONS

	TIMERS::iterator			I = m_timers.begin();
	TIMERS::iterator			E = m_timers.end();

	for ( ; I != E; ++I)
	{
		if ((*I).second.m_update_time != Device.dwTimeGlobal)
			(*I).second.m_time	*= .99f;

		float					average = (*I).second.m_count ? (*I).second.m_total_time/float((*I).second.m_count) : 0.f;
		
		if (!g_dedicated_server)
		{
			if (average >= (*I).second.m_time)
				game_font->SetColor(color_xrgb(127, 127, 127));
			else
				game_font->SetColor(color_xrgb(255, 255, 255));


			game_font->OutNext(
				//			"%s.. %8.3f %8.3f %8.3f %8.3f %8.3f %8d %12.3f",
				"%s%c%c %8.3f %8.3f %8.3f %6.1f %8d %12.3f",
				*(*I).second.m_name,
				white_character,
				white_character,
				(*I).second.m_time,
				average,
				(*I).second.m_max_time,
				float((*I).second.m_call_count) / m_call_count,//float((*I).second.m_count),
	//			(*I).second.m_min_time,
				(*I).second.m_call_count,
				(*I).second.m_total_time
			);
		}
		else
		{
			/*
			Msg(
 				"%s%c%c %8.3f %8.3f %8.3f %6.1f %8d %12.3f",
				*(*I).second.m_name,
				white_character,
				white_character,
				(*I).second.m_time,
				average,
				(*I).second.m_max_time,
				float((*I).second.m_call_count) / m_call_count,
				//float((*I).second.m_count),
				//(*I).second.m_min_time,
				(*I).second.m_call_count,
				(*I).second.m_total_time
			);
			*/
		}
	}

	if (!g_dedicated_server)
		game_font->SetColor			(color_xrgb(255,255,255));
}

void CProfiler::GetServerInfo(CServerInfo* si)
{
	TIMERS::iterator			I = m_timers.begin();
	TIMERS::iterator			E = m_timers.end();

	int i = 0;
	for (; I != E; ++I)
	{
		i += 1;

		if ((*I).second.m_update_time != Device.dwTimeGlobal)
			(*I).second.m_time *= .99f;
 
		shared_str name = *(*I).second.m_name;
 		float TIME = (*I).second.m_time;
		float average = (*I).second.m_count ? (*I).second.m_total_time / float((*I).second.m_count) : 0.f;
 		float max_TIME = (*I).second.m_max_time;
		float m_calls  = float((*I).second.m_call_count) / m_call_count;
		u32 call_count = (*I).second.m_call_count;
		float time_total = (*I).second.m_total_time;
		
		string256 tmp;


		sprintf(tmp, "time[%.3f] max[%.3f] total[%.0f] name[%s]", //calls[%6.1f] c_count[%8u] 
			TIME,
			//average,
			max_TIME,
			//m_calls,
			//call_count,
			time_total,
			name.c_str()
		);	

		string32 temp;
		si->AddItem(itoa(i, temp, 10), tmp, RGB(128, 255, 255));
		
	
		Msg(tmp);
 
	}

}

void CProfiler::add_profile_portion	(const CProfileResultPortion &profile_portion)
{
#ifdef PROFILE_CRITICAL_SECTIONS
	if (InterlockedExchange(&critical_section_counter,1))
		return;

	do {
		Sleep					(0);
	}
	while (!InterlockedExchange(&critical_section_counter,1));
#endif // PROFILE_CRITICAL_SECTIONS

	m_section.Enter				();
	m_portions.push_back		(profile_portion);
	m_section.Leave				();

#ifdef PROFILE_CRITICAL_SECTIONS
	InterlockedExchange			(&critical_section_counter,0);
#endif // PROFILE_CRITICAL_SECTIONS
}
