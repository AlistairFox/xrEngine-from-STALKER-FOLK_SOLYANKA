//////////////////////////////////////////////////////////////////////
// inventory_owner_info.h:	для работы с сюжетной информацией
//
//////////////////////////////////////////////////////////////////////
 
#include "pch_script.h"
#include "InventoryOwner.h"
#include "GameObject.h"
#include "xrMessages.h"
#include "ai_space.h"
#include "ai_debug.h"
#include "alife_simulator.h"
#include "alife_registry_container.h"
#include "script_game_object.h"
#include "level.h"
#include "infoportion.h"
#include "alife_registry_wrappers.h"
#include "script_callback_ex.h"
#include "game_object_space.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "game_sv_freemp.h"
 
void  CInventoryOwner::OnEvent (NET_Packet& P, u16 type)
{
	switch (type)
	{
	case GE_MONEY:
		{
			
			u32 amount = P.r_u32();
			u32 senderID = P.r_u32();

			if (InfinitiveMoney())
				m_money = _max(m_money, amount);
			else
				m_money = amount;

			
			if (OnServer())
			{
				CSE_Abstract* e_dest = Level().Server->ID_to_entity(this->object_id());
				CSE_ALifeTraderAbstract* pTa = smart_cast<CSE_ALifeTraderAbstract*>(e_dest);
				  
				if (pTa)
					pTa->m_dwMoney = m_money;
				
				Msg("Money_pre_actor %d, id %d", m_money, this->object_id());
				ClientID id;
				id.set(senderID);

				xrClientData* data = Level().Server->ID_to_client(id);
					
				if (!data)
					break;

				Msg("data_actor %d, id %d", m_money, this->object_id());

				if (!data->ps)
					break;

				Msg("ps_actor %d, id %d", m_money, this->object_id());

				data->ps->money_for_round = m_money;
					
				game_sv_freemp* freemp = smart_cast<game_sv_freemp*>(Level().Server->game);
					
				if (freemp)
					freemp->signal_Syncronize();

				Msg("money_set == %d", m_money);
					
				 
			}
		}
		break;
	case GE_INFO_TRANSFER:
		{
			u16				id;
			shared_str		info_id;
			u8				add_info;

			P.r_u16			(id);				//отправитель
			P.r_stringZ		(info_id);		//номер полученной информации
			P.r_u8			(add_info);			//добавление или убирание информации

			Msg("GE_INFO_TRANSFER");

			if(add_info)
				OnReceiveInfo	(info_id);
			else
				OnDisableInfo	(info_id);
		}
		break;
	}
}



bool CInventoryOwner::OnReceiveInfo(shared_str info_id) const
{
	VERIFY( info_id.size() );
	//добавить запись в реестр
	//if (OnClient())
	{
		for (auto info : *m_known_info_client)
		{
			if (xr_strcmp(info_id.c_str(), info) == 0)
				return false;
		}

		u16 ID = this->object_id();
	 
		(*m_known_info_client).push_back(info_id.c_str());
		Msg("[%s] Received Info [%s]", Name(), *info_id);

		return true;
	}
	
	/*
	KNOWN_INFO_VECTOR& known_info = m_known_info_registry->registry().objects();
	KNOWN_INFO_VECTOR_IT it = std::find_if(known_info.begin(), known_info.end(), CFindByIDPred(info_id));
	if( known_info.end() == it)
		known_info.push_back(info_id);
	else
		return false;

#ifdef DEBUG
	if(psAI_Flags.test(aiInfoPortion))
		Msg("[%s] Received Info [%s]", Name(), *info_id);
#endif
	*/

	return true;
}
 
void CInventoryOwner::DumpInfo() const
{
	Msg("------------------------------------------");	
	Msg("Start KnownInfo dump for [%s]", Name());	
	
	/*if (OnServer())
	{
		KNOWN_INFO_VECTOR& known_info = m_known_info_registry->registry().objects();
		KNOWN_INFO_VECTOR_IT it = known_info.begin();

		for (int i = 0; it != known_info.end(); ++it, ++i)
		{
			Msg("known info[%d]:%s", i, (*it).c_str());
		}
	}
	else
	{  */
		for (auto info : *m_known_info_client)
		{ 
			Msg("known [%d] info: %s", object_id(), info);
		}
	//}


	Msg("------------------------------------------");	

}		  

void CInventoryOwner::OnDisableInfo(shared_str info_id) const
{
	VERIFY( info_id.size() );
	//удалить запись из реестра
	
#ifdef DEBUG
	if(psAI_Flags.test(aiInfoPortion))
		Msg("[%s] Disabled Info [%s]", Name(), info_id.c_str());
#endif

	KNOWN_INFO_VECTOR& known_info = m_known_info_registry->registry().objects();

	KNOWN_INFO_VECTOR_IT it = std::find_if(known_info.begin(), known_info.end(), CFindByIDPred(info_id));
	if( known_info.end() == it)	return;
	known_info.erase(it);
}

void CInventoryOwner::TransferInfo(shared_str info_id, bool add_info) const
{
	VERIFY( info_id.size() );
	const CObject* pThisObject = smart_cast<const CObject*>(this); VERIFY(pThisObject);

	//отправляем от нашему PDA пакет информации с номером
	NET_Packet		P;
	CGameObject::u_EventGen(P, GE_INFO_TRANSFER, pThisObject->ID());
	P.w_u16			(pThisObject->ID());					//отправитель
	P.w_stringZ		(info_id);							//сообщение
	P.w_u8			(add_info?1:0);							//добавить/удалить информацию
	CGameObject::u_EventSend(P);

	CInfoPortion info_portion;
	info_portion.Load(info_id);
	{
		if(add_info)
			OnReceiveInfo	(info_id);
		else
			OnDisableInfo	(info_id);
	}
}

bool CInventoryOwner::HasInfo(shared_str info_id) const
{
	VERIFY(info_id.size());
	
	/*
	if (OnServer())
	{
		const KNOWN_INFO_VECTOR* known_info = m_known_info_registry->registry().objects_ptr();
		if (!known_info) return false;

		if (std::find_if(known_info->begin(), known_info->end(), CFindByIDPred(info_id)) == known_info->end())
			return false;
	}
	else
	{
		for (auto info : m_known_info_client)
		{
			if (this->object_id() == info.first)
			for (auto info2 : info.second)
			{
				if (xr_strcmp(info_id.c_str(), info2) == 0)
					return true;
			}
		}

		return false;
	}
	*/

	for (auto info : *m_known_info_client)
	{
		if (xr_strcmp(info_id.c_str(), info) == 0)
			return true;
	}

	return false;
}


/*
bool CInventoryOwner::GetInfo	(shared_str info_id, INFO_DATA& info_data) const
{
	VERIFY( info_id.size() );
	const KNOWN_INFO_VECTOR* known_info = m_known_info_registry->registry().objects_ptr ();
	if(!known_info) return false;

	KNOWN_INFO_VECTOR::const_iterator it = std::find_if(known_info->begin(), known_info->end(), CFindByIDPred(info_id));
	if(known_info->end() == it)
		return false;

	info_data = *it;
	return true;
}
*/