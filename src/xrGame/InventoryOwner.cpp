#include "pch_script.h"
#include "InventoryOwner.h"
#include "entity_alive.h"
#include "pda.h"
#include "actor.h"
#include "trade.h"
#include "inventory.h"
#include "xrserver_objects_alife_items.h"
#include "character_info.h"
#include "script_game_object.h"
#include "script_engine.h"
#include "AI_PhraseDialogManager.h"
#include "level.h"
#include "game_base_space.h"
#include "PhraseDialog.h"
#include "xrserver.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "alife_registry_wrappers.h"
#include "relation_registry.h"
#include "ai_object_location.h"
#include "script_callback_ex.h"
#include "game_object_space.h"
#include "AI/Monsters/BaseMonster/base_monster.h"
#include "trade_parameters.h"
#include "purchase_list.h"
#include "alife_object_registry.h"
#include "CustomOutfit.h"
#include "Bolt.h"
#include "actor_mp_server.h"
#include "./ui/UIActorMenu.h"
#include "UIGameCustom.h"
#include "./ui/UITalkWnd.h"

CInventoryOwner::CInventoryOwner			()
{
	m_pTrade					= NULL;
	m_trade_parameters			= 0;

	m_inventory					= xr_new<CInventory>();
	m_pCharacterInfo			= xr_new<CCharacterInfo>();
	
	EnableTalk();
	EnableTrade();
	bDisableBreakDialog			= false;

	m_known_info_registry		= xr_new<CInfoPortionWrapper>();
	m_tmp_active_slot_num		= NO_ACTIVE_SLOT;
	m_need_osoznanie_mode		= FALSE;

	m_deadbody_can_take				= true;
	m_deadbody_closed				= false;
	m_play_show_hide_reload_sounds	= true;

	m_known_info_client			= xr_new<vec_INFO>();
}

DLL_Pure *CInventoryOwner::_construct		()
{
	m_trade_parameters			= 0;
	m_purchase_list				= 0;

	return						(smart_cast<DLL_Pure*>(this));
}

CInventoryOwner::~CInventoryOwner			() 
{
	xr_delete					(m_inventory);
	xr_delete					(m_pTrade);
	xr_delete					(m_pCharacterInfo);
	xr_delete					(m_known_info_registry);
	xr_delete					(m_trade_parameters);
	xr_delete					(m_purchase_list);
	xr_delete					(m_known_info_client);
}

void CInventoryOwner::Load					(LPCSTR section)
{
	if(pSettings->line_exist(section, "inv_max_weight"))
		m_inventory->SetMaxWeight( pSettings->r_float(section,"inv_max_weight") );

	if(pSettings->line_exist(section, "need_osoznanie_mode"))
	{
		m_need_osoznanie_mode=pSettings->r_bool(section,"need_osoznanie_mode");
	}
	else
	{
		m_need_osoznanie_mode=FALSE;
	}
}

void CInventoryOwner::reload				(LPCSTR section)
{
	inventory().Clear			();
	inventory().m_pOwner		= this;
	inventory().SetSlotsUseful (true);

	m_money						= 0;
	m_bTrading					= false;
	m_bTalking					= false;
	m_pTalkPartner				= NULL;

	CAttachmentOwner::reload	(section);
}

void CInventoryOwner::reinit				()
{
	CAttachmentOwner::reinit	();
	m_item_to_spawn				= shared_str();
	m_ammo_in_box_to_spawn		= 0;
}

//call this after CGameObject::net_Spawn
BOOL CInventoryOwner::net_Spawn		(CSE_Abstract* DC)
{
	if (!m_pTrade)
		m_pTrade				= xr_new<CTrade>(this);

	if (m_trade_parameters)
		xr_delete				(m_trade_parameters);

	m_trade_parameters			= xr_new<CTradeParameters>(trade_section());

	//�������� ��������� �� ������, InventoryOwner
	//m_inventory->setSlotsBlocked(false);
	CGameObject			*pThis = smart_cast<CGameObject*>(this);
	if(!pThis) return FALSE;
	CSE_Abstract* E	= (CSE_Abstract*)(DC);

	if (IsGameTypeSingle() || !smart_cast<CSE_ActorMP*>(E))
	{
		CSE_ALifeTraderAbstract* pTrader = NULL;
		if(E) pTrader = smart_cast<CSE_ALifeTraderAbstract*>(E);
		if(!pTrader) return FALSE;

		R_ASSERT( pTrader->character_profile().size() );

		//�������������� ��������� ��������� � ��������� ��������
		CharacterInfo().Init(pTrader);

		//-------------------------------------
		m_known_info_registry->registry().init(E->ID);
		//-------------------------------------
 
		CAI_PhraseDialogManager* dialog_manager = smart_cast<CAI_PhraseDialogManager*>(this);
		if( dialog_manager && !dialog_manager->GetStartDialog().size() )
		{
			dialog_manager->SetStartDialog(CharacterInfo().StartDialog());
			dialog_manager->SetDefaultStartDialog(CharacterInfo().StartDialog());
		}
		m_game_name			= pTrader->m_character_name;
		
		m_deadbody_can_take = pTrader->m_deadbody_can_take;
		m_deadbody_closed   = pTrader->m_deadbody_closed;
	}
	else
	{
		CharacterInfo().m_SpecificCharacter.Load					("mp_actor");
		CharacterInfo().InitSpecificCharacter						("mp_actor");
		CharacterInfo().m_SpecificCharacter.data()->m_sGameName = (E->name_replace()[0]) ? E->name_replace() : *pThis->cName();
		m_game_name												= (E->name_replace()[0]) ? E->name_replace() : *pThis->cName();
	}
	

	if(!pThis->Local())  return TRUE;


	return TRUE;
}

void CInventoryOwner::net_Destroy()
{
	OPTICK_EVENT("CInventoryOwner::net_Destroy");

	if (!g_dedicated_server)
	{
		if (CurrentGameUI())
		{
			if (CurrentGameUI()->ActorMenu().GetPartner() == this)
			{
				StopTrading();
			}
			if (Actor() && Actor()->GetTalkPartner() == this)
			{
				CurrentGameUI()->TalkMenu->HideDialog();
			}
		}
	}

	CAttachmentOwner::net_Destroy();
	
	inventory().Clear();
	inventory().SetActiveSlot(NO_ACTIVE_SLOT);
}


void	CInventoryOwner::save	(NET_Packet &output_packet)
{
	if(inventory().GetActiveSlot() == NO_ACTIVE_SLOT)
		output_packet.w_u8((u8)NO_ACTIVE_SLOT);
	else
		output_packet.w_u8((u8)inventory().GetActiveSlot());

	CharacterInfo().save(output_packet);
	save_data	(m_game_name, output_packet);
	save_data	(m_money,	output_packet);
}
void	CInventoryOwner::load	(IReader &input_packet)
{
	u8 active_slot = input_packet.r_u8();
	if(active_slot == NO_ACTIVE_SLOT)
		inventory().SetActiveSlot(NO_ACTIVE_SLOT);
	//else
		//inventory().Activate_deffered(active_slot, Device.dwFrame);

	m_tmp_active_slot_num		 = active_slot;

	CharacterInfo().load(input_packet);
	load_data		(m_game_name, input_packet);
	load_data		(m_money,	input_packet);
}


void CInventoryOwner::UpdateInventoryOwner(u32 deltaT)
{
	inventory().Update();

	if ( m_pTrade )
	{
		m_pTrade->UpdateTrade();
	}
	if ( IsTrading() )
	{
		//���� �� ������, �� ��� "trade"
		if ( !is_alive() )
		{
			StopTrading();
		}
	}

	if ( IsTalking() )
	{
		//���� ��� ���������� �������� �������� � ����,
		//�� � ��� ������ �����.
		if ( !m_pTalkPartner->IsTalking() )
		{
			StopTalk();
		}

		//���� �� ������, �� ���� �� ��������
		if ( !is_alive() )
		{
			StopTalk();
		}
	}
}


//������� PDA �� ������������ ����� ���������
CPda* CInventoryOwner::GetPDA() const
{
	return (CPda*)(m_inventory->ItemFromSlot(PDA_SLOT));
}

CTrade* CInventoryOwner::GetTrade() 
{
	R_ASSERT2(m_pTrade, "trade for object does not init yet");
	return m_pTrade;
}


//��������� �������

//��� ���������� ����������,
//��������� ���� ��������� 
//� ���� �� ���� �������� ��������
bool CInventoryOwner::OfferTalk(CInventoryOwner* talk_partner)
{
	if(!IsTalkEnabled()) return false;

	//��������� ��������� � �����������
	CEntityAlive* pPartnerEntityAlive = smart_cast<CEntityAlive*>(talk_partner);
	R_ASSERT(pPartnerEntityAlive);
	
//	ALife::ERelationType relation = RELATION_REGISTRY().GetRelationType(this, talk_partner);
//	if(relation == ALife::eRelationTypeEnemy) return false;

	if(!is_alive() || !pPartnerEntityAlive->g_Alive()) return false;

	StartTalk(talk_partner);

	return true;
}


void CInventoryOwner::StartTalk(CInventoryOwner* talk_partner, bool start_trade)
{
	m_bTalking = true;
	m_pTalkPartner = talk_partner;

}
#include "UIGameSP.h"
#include "ui\UITalkWnd.h"

void CInventoryOwner::StopTalk()
{
	m_pTalkPartner			= NULL;
	m_bTalking				= false;

	CUIGameSP* ui_sp = smart_cast<CUIGameSP*>(CurrentGameUI());
	if(ui_sp && ui_sp->TalkMenu->IsShown())
		ui_sp->TalkMenu->Stop();
}

bool CInventoryOwner::IsTalking()
{
	return m_bTalking;
}

void CInventoryOwner::StartTrading()
{
	m_bTrading = true;
}

void CInventoryOwner::StopTrading()
{
	m_bTrading = false;


	if (CurrentGameUI())
	{
		CurrentGameUI()->HideActorMenu();
	}
}

bool CInventoryOwner::IsTrading()
{
	return m_bTrading;
}

#include "Weapon.h"

//==============
void CInventoryOwner::renderable_Render		()
{
	//Se7Kills (Only Render Active ITEM)

	if (inventory().ActiveItem())
	{
		CWeapon* wpn = smart_cast<CWeapon*>(inventory().ActiveItem());
		if (wpn)
			wpn->strapped_mode(false);
		inventory().ActiveItem()->renderable_Render();
	}
	
	if (smart_cast<CActor*>(this))
	{
		CInventoryItem* item_2 = inventory().ItemFromSlot(INV_SLOT_2);
		CInventoryItem* item_3 = inventory().ItemFromSlot(INV_SLOT_3);

		CWeapon* wpn1 = smart_cast<CWeapon*>(item_2);
		CWeapon* wpn2 = smart_cast<CWeapon*>(item_3);

		if (wpn1)
		{
			wpn1->strapped_mode(wpn1 != inventory().ActiveItem());
			wpn1->renderable_Render();
		}

		if (wpn2)
		{
			wpn2->strapped_mode(wpn2 != inventory().ActiveItem());
			wpn2->renderable_Render();
		}
	}	 

	CAttachmentOwner::renderable_Render();
}

void CInventoryOwner::OnItemTake			(CInventoryItem *inventory_item)
{
	CGameObject	*object = smart_cast<CGameObject*>(this);
	VERIFY		(object);
	object->callback(GameObject::eOnItemTake)(inventory_item->object().lua_game_object());

	attach		(inventory_item);

	if(m_tmp_active_slot_num!=NO_ACTIVE_SLOT					&& 
		inventory_item->CurrPlace()==eItemPlaceSlot	&&
		inventory_item->CurrSlot()==m_tmp_active_slot_num)
	{
		if(inventory().ItemFromSlot(m_tmp_active_slot_num))
		{
			inventory().Activate(m_tmp_active_slot_num);
			m_tmp_active_slot_num	= NO_ACTIVE_SLOT;
		}
	}
}

//���������� ������� ������� �������� � ������ �������� (� ��������)
float CInventoryOwner::GetWeaponAccuracy	() const
{
	return 0.f;
}

//������������ ���������� ���
float  CInventoryOwner::MaxCarryWeight () const
{
	float ret =  inventory().GetMaxWeight();

	const CCustomOutfit* outfit	= GetOutfit();
	if(outfit)
		ret += outfit->m_additional_weight2;

	return ret;
}

void CInventoryOwner::spawn_supplies		()
{
	CGameObject								*game_object = smart_cast<CGameObject*>(this);
	VERIFY									(game_object);
	if (smart_cast<CBaseMonster*>(this))	return;


	if (use_bolts())
		Level().spawn_item					("bolt",game_object->Position(),game_object->ai_location().level_vertex_id(),game_object->ID());

	if (!ai().get_alife() && IsGameTypeSingle() ) 
	{
		CSE_Abstract						*abstract = Level().spawn_item("device_pda",game_object->Position(),game_object->ai_location().level_vertex_id(),game_object->ID(),true);
		CSE_ALifeItemPDA					*pda = smart_cast<CSE_ALifeItemPDA*>(abstract);
		R_ASSERT							(pda);
		pda->m_original_owner				= (u16)game_object->ID();
		NET_Packet							P;
		abstract->Spawn_Write				(P,TRUE);
		Level().Send						(P,net_flags(TRUE));
		F_entity_Destroy					(abstract);
	}
}

//������� ��� 
LPCSTR	CInventoryOwner::Name () const
{
//	return CharacterInfo().Name();
	return m_game_name.c_str();
}

void CInventoryOwner::SetName(LPCSTR name)
{
	m_game_name = name;
}

LPCSTR	CInventoryOwner::IconName () const
{
	return CharacterInfo().IconName().c_str();
}


void CInventoryOwner::NewPdaContact		(CInventoryOwner* pInvOwner)
{
}
void CInventoryOwner::LostPdaContact	(CInventoryOwner* pInvOwner)
{
}

//////////////////////////////////////////////////////////////////////////
//��� ������ � relation system
u16 CInventoryOwner::object_id	()  const
{
	return smart_cast<const CGameObject*>(this)->ID();
}


//////////////////////////////////////////////////////////////////////////
//��������� ����������� �� ���������� � ��������� ������

void CInventoryOwner::SetCommunity	(CHARACTER_COMMUNITY_INDEX new_community)
{
	CEntityAlive* EA					= smart_cast<CEntityAlive*>(this); VERIFY(EA);

	CharacterInfo().SetCommunity( new_community );
	if( EA->g_Alive() )
	{
		EA->ChangeTeam(CharacterInfo().Community().team(), EA->g_Squad(), EA->g_Group());
	}

	CSE_Abstract* e_entity;
	if (IsGameTypeSingle())
		e_entity = ai().alife().objects().object(EA->ID(), false);
	else
		e_entity = smart_cast<CSE_Abstract*>(Level().Objects.net_Find(EA->ID()));

	if (!e_entity) return;

	CSE_ALifeTraderAbstract* trader		= smart_cast<CSE_ALifeTraderAbstract*>(e_entity);
	if(!trader) return;
//	EA->id_Team = CharacterInfo().Community().team();
	trader->m_community_index  = new_community;
}

void CInventoryOwner::SetRankClient(CHARACTER_RANK_VALUE rank)
{
	CharacterInfo().m_CurrentRank.set(rank);
}

void CInventoryOwner::SetRank			(CHARACTER_RANK_VALUE rank)
{
	if (!OnServer())
		return;

	CEntityAlive* EA					= smart_cast<CEntityAlive*>(this); VERIFY(EA);

	CharacterInfo().m_CurrentRank.set(rank);


	//������� ������ �������� ai().alife().objects().object(); (������ ��� � ai().alife().objects() ) 
	//����� �� ����� ����������� �� ������ � objects();

	NET_Packet packet;
	packet.w_begin(M_GAMEMESSAGE);
	packet.w_u32(M_INVENTORY_OWNER_RANK);  //������� ����� ����� � xrMessages.h
	packet.w_u32(this->object_id());
	packet.w_u32(rank);
	Level().Server->SendBroadcast(Level().Server->GetServerClient()->ID, packet, net_flags(true, true));

	

	CSE_Abstract* e_entity = ai().alife().objects().object(EA->ID(), false);
 	if(!e_entity) return;
	CSE_ALifeTraderAbstract* trader		= smart_cast<CSE_ALifeTraderAbstract*>(e_entity);
	if(!trader) return;
 
	trader->m_rank  = rank;
}

void CInventoryOwner::ChangeRank			(CHARACTER_RANK_VALUE delta)
{
	SetRank(Rank()+delta);
}

void CInventoryOwner::SetReputation		(CHARACTER_REPUTATION_VALUE reputation)
{
	CEntityAlive* EA					= smart_cast<CEntityAlive*>(this); VERIFY(EA);

	CharacterInfo().m_CurrentReputation.set(reputation);

	CSE_Abstract* e_entity				= ai().alife().objects().object(EA->ID(), false);
	if(!e_entity) return;

	CSE_ALifeTraderAbstract* trader		= smart_cast<CSE_ALifeTraderAbstract*>(e_entity);
	if(!trader) return;


	trader->m_reputation  = reputation;
}

void CInventoryOwner::ChangeReputation	(CHARACTER_REPUTATION_VALUE delta)
{
	SetReputation(Reputation() + delta);
}


void CInventoryOwner::OnItemDrop(CInventoryItem *inventory_item, bool just_before_destroy)
{
	CGameObject	*object = smart_cast<CGameObject*>(this);
	VERIFY		(object);
	object->callback(GameObject::eOnItemDrop)(inventory_item->object().lua_game_object());

	detach		(inventory_item);
}

void CInventoryOwner::OnItemDropUpdate ()
{
}

void CInventoryOwner::OnItemBelt	(CInventoryItem *inventory_item, const SInvItemPlace& previous_place)
{
}

void CInventoryOwner::OnItemRuck	(CInventoryItem *inventory_item, const SInvItemPlace& previous_place)
{
	detach		(inventory_item);
}
void CInventoryOwner::OnItemSlot	(CInventoryItem *inventory_item, const SInvItemPlace& previous_place)
{
	attach		(inventory_item);
}

CCustomOutfit* CInventoryOwner::GetOutfit() const
{
    return smart_cast<CCustomOutfit*>(inventory().ItemFromSlot(OUTFIT_SLOT));
}
#include "ActorHelmet.h"
CInventoryItem* CInventoryOwner::GetOutfitOrHelmet()
{
	CHelmet* helm = smart_cast<CHelmet*>(inventory().ItemFromSlot(HELMET_SLOT));
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(inventory().ItemFromSlot(OUTFIT_SLOT));
	if (helm)
	{
		return helm;
	}

	if (outfit)
	{
		return outfit;
	}


	return nullptr;
}



void CInventoryOwner::on_weapon_shot_start		(CWeapon *weapon)
{
}

void CInventoryOwner::on_weapon_shot_update		()
{
}

void CInventoryOwner::on_weapon_shot_stop		()
{
}

void CInventoryOwner::on_weapon_shot_remove		(CWeapon *weapon)
{
}

void CInventoryOwner::on_weapon_hide			(CWeapon *weapon)
{
}

LPCSTR CInventoryOwner::trade_section			() const
{
	const CGameObject			*game_object = smart_cast<const CGameObject*>(this);
	VERIFY						(game_object);
	return						(READ_IF_EXISTS(pSettings,r_string,game_object->cNameSect(),"trade_section","trade"));
}

float CInventoryOwner::deficit_factor			(const shared_str &section) const
{
	if (!m_purchase_list)
		return					(1.f);

	return						(m_purchase_list->deficit(section));
}

void CInventoryOwner::buy_supplies				(CInifile &ini_file, LPCSTR section)
{
	if (!m_purchase_list)
		m_purchase_list			= xr_new<CPurchaseList>();

	m_purchase_list->process	(ini_file,section,*this);
}

void CInventoryOwner::sell_useless_items		()
{
	CGameObject					*object = smart_cast<CGameObject*>(this);

	TIItemContainer::iterator	I = inventory().m_all.begin();
	TIItemContainer::iterator	E = inventory().m_all.end();
	for ( ; I != E; ++I) {
		if ( smart_cast<CBolt*>( *I ) )
		{
			continue;
		}
		CInventoryItem* item = smart_cast<CInventoryItem*>( *I );
		if (item->CurrSlot()&&item->cast_weapon())
			continue;

		CPda* pda = smart_cast<CPda*>( *I );
		if ( pda )
		{
			if (pda->GetOriginalOwnerID() == object->ID())
			{
				continue;
			}
		}
		(*I)->SetDropManual(FALSE);
		(*I)->object().DestroyObject();
	}
}

bool CInventoryOwner::AllowItemToTrade 			(CInventoryItem const * item, const SInvItemPlace& place) const
{
	return						(
		trade_parameters().enabled(
			CTradeParameters::action_sell(0),
			item->object().cNameSect()
		)
	);
}

void CInventoryOwner::set_money		(u32 amount, bool bSendEvent)
{

	if(InfinitiveMoney())
		m_money					= _max(m_money, amount);
	else
		m_money					= amount;

	if(bSendEvent)
	{
		CGameObject				*object = smart_cast<CGameObject*>(this);
		NET_Packet				packet;
		object->u_EventGen		(packet,GE_MONEY,object->ID());
		packet.w_u32			(m_money);
		object->u_EventSend		(packet);
	}
}

bool CInventoryOwner::use_default_throw_force	()
{
	return						(true);
}

float CInventoryOwner::missile_throw_force		() 
{
	NODEFAULT;
#ifdef DEBUG
	return						(0.f);
#endif
}

bool CInventoryOwner::use_throw_randomness		()
{
	return						(true);
}

bool CInventoryOwner::is_alive()
{
	CEntityAlive* pEntityAlive = smart_cast<CEntityAlive*>(this);
	R_ASSERT( pEntityAlive );
	return (!!pEntityAlive->g_Alive());
}

void CInventoryOwner::deadbody_can_take( bool status )
{
	if ( is_alive() )
	{
		return;
	}
	m_deadbody_can_take = status;

	NET_Packet P;
	CGameObject::u_EventGen( P, GE_INV_OWNER_STATUS, object_id() );
	P.w_u8( (m_deadbody_can_take)? 1 : 0 );
	P.w_u8( (m_deadbody_closed)? 1 : 0 );
	CGameObject::u_EventSend( P );
}

void CInventoryOwner::deadbody_closed( bool status )
{
	if ( is_alive() )
	{
		return;
	}
	m_deadbody_closed = status;

	NET_Packet P;
	CGameObject::u_EventGen( P, GE_INV_OWNER_STATUS, object_id() );
	P.w_u8( (m_deadbody_can_take)? 1 : 0 );
	P.w_u8( (m_deadbody_closed)? 1 : 0 );
	CGameObject::u_EventSend( P );
}