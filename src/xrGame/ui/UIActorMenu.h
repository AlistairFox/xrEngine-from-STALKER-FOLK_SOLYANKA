#pragma once

#include "UIDialogWnd.h"
#include "UIWndCallback.h"
#include "../../xrServerEntities/inventory_space.h"
#include "UIHint.h"
#include "../UIArtefactUpgradeWnd.h"

class CUICharacterInfo;
class CUIDragDropListEx;
class CUIDragDropReferenceList;
class CUICellItem;
class CUIDragItem;
class ui_actor_state_wnd;
class CUIItemInfo;
class CUIFrameLineWnd;
class CUIStatic;
class CUITextWnd;
class CUI3tButton;
class CInventoryOwner;
class CInventoryBox;
class CUIInventoryUpgradeWnd;
class UIInvUpgradeInfo;
class CUIMessageBoxEx;
class CUIPropertiesBox;
class CTrade;
class CUIProgressBar;

namespace inventory { namespace upgrade {
	class Upgrade;
} } // namespace upgrade, inventory

enum EDDListType{
		iInvalid,
		iActorSlot,
		iActorBag,
		iActorBelt,

		iActorTrade,
		iPartnerTradeBag,
		iPartnerTrade,
		iDeadBodyBag,
		iQuickSlot,
		iTrashSlot,
		iListTypeMax
};

enum EMenuMode{
		mmUndefined,
		mmInventory,
		mmTrade,
		mmUpgrade,
		mmDeadBodySearch,
		mmArtUpgrade,
};

class CUIActorMenu :	public CUIDialogWnd, 
						public CUIWndCallback
{
	typedef CUIDialogWnd		inherited;
	typedef inventory::upgrade::Upgrade 	Upgrade_type;

public:
	enum eActorMenuSndAction{	eSndOpen	=0,
								eSndClose,
								eItemToSlot,
								eItemToBelt,
								eItemToRuck,
								eProperties,
								eDropItem,
								eAttachAddon,
								eDetachAddon,
								eItemUse,
								eSquadInvite,
								eSquadAction,
								eSndMax};



	EMenuMode					m_currMenuMode;
	ref_sound					sounds					[eSndMax];
	void						PlaySnd					(eActorMenuSndAction a);


protected:

	UIHint*						m_hint_wnd;
	CUIItemInfo*				m_ItemInfo;
	CUICellItem*				m_InfoCellItem;
	u32							m_InfoCellItem_timer;
	CUICellItem*				m_pCurrentCellItem;
	CUICellItem*				m_upgrade_selected;
	CUIPropertiesBox*			m_UIPropertiesBox;

	ui_actor_state_wnd*			m_ActorStateInfo;
	CUICharacterInfo*			m_ActorCharacterInfo;
	CUICharacterInfo*			m_PartnerCharacterInfo;

	CUIDragDropListEx*			m_pInventoryBeltList;
	CUIDragDropListEx*			m_pInventoryPistolList;
	CUIDragDropListEx*			m_pInventoryAutomaticList;
	CUIDragDropListEx*			m_pInventoryOutfitList;
	CUIDragDropListEx*			m_pInventoryHelmetList;
	CUIDragDropListEx*			m_pInventoryDetectorList;
	CUIDragDropListEx*			m_pInventoryBagList;


	CUIDragDropListEx*			m_pTradeActorBagList;
	CUIDragDropListEx*			m_pTradeActorList;
	CUIDragDropListEx*			m_pTradePartnerBagList;
	CUIDragDropListEx*			m_pTradePartnerList;
	CUIDragDropListEx*			m_pDeadBodyBagList;
	CUIDragDropListEx*			m_pTrashList;

	enum						{e_af_count = 5};
	CUIStatic*					m_belt_list_over[e_af_count];
	CUIStatic*					m_HelmetOver;

	CUIStatic*					m_InvSlot2Highlight;
	CUIStatic*					m_InvSlot3Highlight;
	CUIStatic*					m_HelmetSlotHighlight;
	CUIStatic*					m_OutfitSlotHighlight;
	CUIStatic*					m_DetectorSlotHighlight;


	CUIStatic*					m_QuickSlotsHighlight[4];
	CUIStatic*					m_ArtefactSlotsHighlight[e_af_count];

	//PDA SLOT
	CUIStatic*					m_PdaSlotHighlight;
	CUIDragDropListEx*			m_pInventoryPdaList;
	//	CUIDragDropListEx*		m_pInventoryKnifeList;
	//	CUIStatic*				m_KnifeSlotHighlight;

	//END

	CUIInventoryUpgradeWnd*		m_pUpgradeWnd;
	CUIArtefactUpgradeWnd*		m_artUpgradeWnd;
	
	CUIStatic*					m_LeftBackground;

	UIInvUpgradeInfo*			m_upgrade_info;
	CUIMessageBoxEx*			m_message_box_yes_no;
	CUIMessageBoxEx*			m_message_box_ok;

	CInventoryOwner*			m_pActorInvOwner;
	CInventoryOwner*			m_pPartnerInvOwner;
	CInventoryBox*				m_pInvBox;

	CUITextWnd*					m_ActorMoney;
	CUITextWnd*					m_PartnerMoney;
	CUITextWnd*					m_QuickSlot1;
	CUITextWnd*					m_QuickSlot2;
	CUITextWnd*					m_QuickSlot3;
	CUITextWnd*					m_QuickSlot4;
	
	CUIProgressBar*				m_WeaponSlot1_progress;
	CUIProgressBar*				m_WeaponSlot2_progress;
	CUIProgressBar*				m_Helmet_progress;
	CUIProgressBar*				m_Outfit_progress;
	// bottom ---------------------------------
	CUIStatic*					m_ActorBottomInfo;
	CUITextWnd*					m_ActorWeight;
	CUITextWnd*					m_ActorWeightMax;
	
	CUIStatic*					m_PartnerBottomInfo;
	CUITextWnd*					m_PartnerWeight;
	float						m_PartnerWeight_end_x;
//*	CUIStatic*					m_PartnerWeightMax;

	// delimiter ------------------------------
	CUIStatic*					m_LeftDelimiter;
//	CUITextWnd*					m_PartnerTradeCaption;
	CUITextWnd*					m_PartnerTradePrice;
	CUITextWnd*					m_PartnerTradeWeightMax;

	CUIStatic*					m_RightDelimiter;
//	CUITextWnd*					m_ActorTradeCaption;
	CUITextWnd*					m_ActorTradePrice;
	CUITextWnd*					m_ActorTradeWeightMax;

	CTrade*						m_actor_trade;
	CTrade*						m_partner_trade;

	CUI3tButton*				m_trade_buy_button;
	CUI3tButton*				m_trade_sell_button;
	CUI3tButton*				m_takeall_button;
	CUI3tButton*				m_exit_button;
//	CUIStatic*					m_clock_value;

	u32							m_last_time;
	bool						m_repair_mode;
	bool						m_item_info_view;
	bool						m_highlight_clear;
	u32							m_trade_partner_inventory_state;
public:
	CUIDragDropReferenceList*	m_pQuickSlot;

public:
	void						SetMenuMode					(EMenuMode mode);
	EMenuMode					GetMenuMode					() {return m_currMenuMode;};
	void						SetActor					(CInventoryOwner* io);
	void						SetPartner					(CInventoryOwner* io);
	CInventoryOwner*			GetPartner					() {return m_pPartnerInvOwner;};
	void						SetInvBox					(CInventoryBox* box);
	CInventoryBox*				GetInvBox					() {return m_pInvBox;};
private:
	void						PropertiesBoxForSlots		(PIItem item, bool& b_show);
	void						PropertiesBoxForWeapon		(CUICellItem* cell_item, PIItem item, bool& b_show);
	void						PropertiesBoxForAddon		(PIItem item, bool& b_show);
	void						PropertiesBoxForUsing		(PIItem item, bool& b_show);
	void						PropertiesBoxForPlaying		(PIItem item, bool& b_show);
	void						PropertiesBoxForDrop		(CUICellItem* cell_item, PIItem item, bool& b_show);
	void						PropertiesBoxForRepair		(PIItem item, bool& b_show);
	void						PropertiesBoxForGift		(PIItem item, bool& b_show);
	void						PropertiesBoxForInject		(PIItem item, bool& b_show);
private:
	void						clear_highlight_lists		();
	void						set_highlight_item			(CUICellItem* cell_item);
	void						highlight_item_slot			(CUICellItem* cell_item);
	void						highlight_armament			(PIItem item, CUIDragDropListEx* ddlist);
	void						highlight_ammo_for_weapon	(PIItem weapon_item, CUIDragDropListEx* ddlist);
	void						highlight_weapons_for_ammo	(PIItem ammo_item, CUIDragDropListEx* ddlist);
	bool						highlight_addons_for_weapon	(PIItem weapon_item, CUICellItem* ci);
	void						highlight_weapons_for_addon	(PIItem addon_item, CUIDragDropListEx* ddlist);

protected:			
	void						Construct					();
	void						InitCallbacks				();

	void						InitCellForSlot				(u16 slot_idx);
	void						InitInventoryContents		(CUIDragDropListEx* pBagList);
	void						ClearAllLists				();
	void						BindDragDropListEvents		(CUIDragDropListEx* lst);
	
	EDDListType					GetListType					(CUIDragDropListEx* l);
	CUIDragDropListEx*			GetListByType				(EDDListType t);
	CUIDragDropListEx*			GetSlotList					(u16 slot_idx);
	bool						CanSetItemToList			(PIItem item, CUIDragDropListEx* l, u16& ret_slot);
	
	xr_vector<EDDListType>		m_allowed_drops				[iListTypeMax];
	bool						AllowItemDrops				(EDDListType from, EDDListType to);

	bool		xr_stdcall		OnItemDrop					(CUICellItem* itm);
	bool		xr_stdcall		OnItemStartDrag				(CUICellItem* itm);
	bool		xr_stdcall		OnItemDbClick				(CUICellItem* itm);
	bool		xr_stdcall		OnItemSelected				(CUICellItem* itm);
	bool		xr_stdcall		OnItemRButtonClick			(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusReceive			(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusLost				(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusedUpdate			(CUICellItem* itm);
	void		xr_stdcall		OnDragItemOnTrash			(CUIDragItem* item, bool b_receive);
	bool						OnItemDropped				(PIItem itm, CUIDragDropListEx* new_owner, CUIDragDropListEx* old_owner);

	void						ResetMode					();

	void						InitInventoryMode			();
	void						DeInitInventoryMode			();
//Trader
	void						InitTradeMode				();
	void						DeInitTradeMode				();
//Mechanic
	void						InitUpgradeMode				();
	void						DeInitUpgradeMode			();
//DeadBody
	void						InitDeadBodySearchMode		();
	void						DeInitDeadBodySearchMode	();
//Artefact Upgrade	
	void						DeInitArtUpgradeMode		();
	void						InitArtUpgradeMode();


	void						CurModeToScript				();
	void						RepairEffect_CurItem		();

	void						SetCurrentItem				(CUICellItem* itm);
	CUICellItem*				CurrentItem					();
	PIItem						CurrentIItem				();

	void						InfoCurItem					(CUICellItem* cell_item); //on update item

	void						ActivatePropertiesBox		();
	void						TryHidePropertiesBox		();
	void		xr_stdcall		ProcessPropertiesBoxClicked	(CUIWindow* w, void* d);
	
	void						CheckDistance				();
	void						UpdateItemsPlace			();

	void						SetupUpgradeItem			();
	void						UpdateUpgradeItem			();
	void						TrySetCurUpgrade			();
	void						UpdateButtonsLayout			();

	// inventory
	bool						ToSlot						(CUICellItem* itm, bool force_place, u16 slot_id);
	bool						ToBag						(CUICellItem* itm, bool b_use_cursor_pos);
	bool						ToBelt						(CUICellItem* itm, bool b_use_cursor_pos);
	bool						TryUseItem					(CUICellItem* cell_itm);
	bool						ToQuickSlot					(CUICellItem* itm);
	bool						TryGiftItem					(CUICellItem* cell_itm);
	bool						TryInjectItem				(CUICellItem* cell_itm);
	void						SetActorInfoMP				();
	void						UpdateActorMoneyMP();
	void						UpdateOutfit				();
	void						MoveArtefactsToBag			();
	bool						TryActiveSlot				(CUICellItem* itm);
	void		xr_stdcall		TryRepairItem				(CUIWindow* w, void* d);
	bool						CanUpgradeItem				(PIItem item);

	bool						ToActorTrade				(CUICellItem* itm, bool b_use_cursor_pos);
	bool						ToPartnerTrade				(CUICellItem* itm, bool b_use_cursor_pos);
	bool						ToPartnerTradeBag			(CUICellItem* itm, bool b_use_cursor_pos);
	bool						ToDeadBodyBag				(CUICellItem* itm, bool b_use_cursor_pos);

	void						AttachAddon					(PIItem item_to_upgrade);
	void						DetachAddon					(LPCSTR addon_name, PIItem itm = NULL);

	void						SendEvent_Item2Slot			(PIItem	pItem, u16 parent, u16 slot_id);
	void						SendEvent_Item2Belt			(PIItem	pItem, u16 parent);
	void						SendEvent_Item2Ruck			(PIItem	pItem, u16 parent);
	void						SendEvent_Item_Drop			(PIItem	pItem, u16 parent);
	void						SendEvent_Item_Eat			(PIItem	pItem, u16 parent);
	void						SendEvent_ActivateSlot		(u16 slot, u16 recipient);
	void						DropAllCurrentItem			();
	void						OnPressUserKey				();

	// trade
	void						InitPartnerInventoryContents();
	void						ColorizeItem				(CUICellItem* itm, bool colorize);
	float						CalcItemsWeight				(CUIDragDropListEx* pList);
	u32							CalcItemsPrice				(CUIDragDropListEx* pList, CTrade* pTrade, bool bBuying);
	void						UpdatePrices				();
	bool						CanMoveToPartner			(PIItem pItem);
	void						TransferItems				(CUIDragDropListEx* pSellList, CUIDragDropListEx* pBuyList, CTrade* pTrade, bool bBuying);
	void						TransferItemsMp				(CUIDragDropListEx* pSellList, CUIDragDropListEx* pBuyList, CTrade* pTrade, bool bBuying);

public:
	virtual void				ReloadActorMenu();


								CUIActorMenu				();
	virtual						~CUIActorMenu				();

	virtual bool				StopAnyMove					();
	virtual void				SendMessage					(CUIWindow* pWnd, s16 msg, void* pData = NULL);
	virtual void				Draw						();
	virtual void				Update						();
	virtual void				Show						(bool status);

	virtual bool				OnKeyboardAction					(int dik, EUIMessages keyboard_action);
	virtual bool				OnMouseAction						(float x, float y, EUIMessages mouse_action);

	void						CallMessageBoxYesNo			(LPCSTR text);
	void						CallMessageBoxOK			(LPCSTR text);
	void		xr_stdcall		OnMesBoxYes					(CUIWindow*, void*);
	void		xr_stdcall		OnMesBoxNo					(CUIWindow*, void*);

	void						OnInventoryAction			(PIItem pItem, u16 action_type);
	void						ShowRepairButton			(bool status);
	bool						SetInfoCurUpgrade			(Upgrade_type* upgrade_type, CInventoryItem* inv_item );
	void						SeparateUpgradeItem			();
	PIItem						get_upgrade_item			();
	bool						DropAllItemsFromRuck		(bool quest_force = false); //debug func

	void						UpdateActor					();
	void						UpdatePartnerBag			();
	void						UpdateDeadBodyBag			();

	void		xr_stdcall		OnBtnPerformTradeBuy		(CUIWindow* w, void* d);
	void		xr_stdcall		OnBtnPerformTradeSell		(CUIWindow* w, void* d);
	void		xr_stdcall		OnBtnExitClicked			(CUIWindow* w, void* d);
	void		xr_stdcall		TakeAllFromPartner			(CUIWindow* w, void* d);
	void						TakeAllFromInventoryBox		();
	void						UpdateConditionProgressBars	();

	IC	UIHint*					get_hint_wnd				() { return m_hint_wnd; }

private: //Craft

	struct RecipeSection
	{
		float craft_chance;
		shared_str out_section;
		xr_vector<shared_str> ing_1_sections;
		xr_vector<shared_str> ing_2_sections;
	};

	xr_vector<RecipeSection> recipes;

	void						InitCraft					();
	void						CraftDestroyItem			(PIItem itm);
	void						TryCraftItem				(PIItem first, PIItem second, RecipeSection section_to_craft);
	
	bool						VerifyCraftRecipe			(shared_str sect1, shared_str sect2, RecipeSection& recipe);
}; // class CUIActorMenu
