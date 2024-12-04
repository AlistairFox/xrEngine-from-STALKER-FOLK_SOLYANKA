#include "stdafx.h"
#include "VoiceChat.h"
#include "Level.h"
#include "../xrSound/SoundVoiceChat.h"
#include "game_cl_mp.h"
#include "Actor.h"
#include "Inventory.h"

CVoiceChat::CVoiceChat()
{
	m_pSoundVoiceChat = ::Sound->GetSoundVoiceChat();
	CreateRecorder();
}

CVoiceChat::~CVoiceChat()
{
	xr_delete(m_pSender);

	for (auto I = m_soundPlayersMap.begin(); I != m_soundPlayersMap.end(); ++I)
	{
		m_pSoundVoiceChat->DestroySoundPlayer(I->second);
	}
	m_soundPlayersMap.clear();
}


bool CVoiceChat::CreateRecorder()
{
	if (m_pSender != nullptr)
	{
		xr_delete(m_pSender);
		m_pSender = nullptr;
	}

	m_pSender = xr_new<CVoiceSender>();
	m_pSender->SetDistance(10); // default

	m_pRecorder = m_pSoundVoiceChat->CreateRecorder((IVoicePacketSender*)m_pSender);

	return m_pRecorder != nullptr;
}

void CVoiceChat::Start()
{
	m_pRecorder->Start();
}

void CVoiceChat::Stop()
{
	m_pRecorder->Stop();
}

bool CVoiceChat::IsStarted()
{
	return m_pRecorder->IsStarted();
}

u8 CVoiceChat::GetDistance() const
{
	return m_pSender->GetDistance();
}
	
/*
u8 CVoiceChat::SwitchDistance()
{
	R_ASSERT(m_pSender != nullptr);

	switch (m_pSender->GetDistance())
	{
	case 5:
		m_pSender->SetDistance(10);
		return 10;
	case 10:
		m_pSender->SetDistance(30);
		return 30;
	default:
		m_pSender->SetDistance(5);
		return 5;
	}
} 
*/

void CVoiceChat::Update()
{
	CheckAndClearPlayers(m_soundPlayersMap);
}

void CVoiceChat::OnRender()
{
	if (psActorFlags.test(AF_DISPLAY_VOICE_ICON))
	{
		constexpr Fvector pos = Fvector{ 0.0, 0.5, 0.0 };

		game_PlayerState* local_player = Game().local_player;
		auto& players = Game().players;

		for (auto it = players.begin(); it != players.end(); ++it)
		{
			game_PlayerState* ps = it->second;
			u16 id = ps->GameID;

			if (ps == local_player || ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD) || ps->testFlag(GAME_PLAYER_MP_INVIS))
				continue;

			auto voiceTimeIt = m_voiceTimeMap.find(id);
			if (voiceTimeIt == m_voiceTimeMap.end())
				continue;

			auto& voiceIconInfo = voiceTimeIt->second;

			if (voiceIconInfo.time + 200 < GetTickCount())
				continue;

			CObject* pObject = Level().Objects.net_Find(id);
			if (!pObject) continue;

			CActor* pActor = smart_cast<CActor*>(pObject);
			if (!pActor) continue;

			bool isCorrectSquad = false;
			for (auto PL : players_in_squad)
			{
				if (PL.PlayerState->MPSquadID == ps->MPSquadID)
					isCorrectSquad = true;
			}
			
			if (isCorrectSquad)
				pActor->RenderIndicator(pos, 0.2, 0.2, GetVoiceIndicatorShader());
		}
	}

	if (m_last_distance != psSoundDistance)
	{
		m_last_distance = (int) psSoundDistance * 30;
		if (m_last_distance < 5)
			m_pSender->SetDistance(5);
		else 
			m_pSender->SetDistance(psSoundDistance);
	}
}

void CVoiceChat::ReceiveMessage(NET_Packet* P)
{
	game_PlayerState* local_player = Game().local_player;
	if (!local_player || local_player->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD) || !Actor())
 		return;

	u8 voiceDistance = P->r_u8();

	// Кто говорит ID
	u16 clientId = P->r_u16();
	CObject* obj = Level().Objects.net_Find(clientId);
	if (!obj)
 		return;
 
	bool isValidDistance = true;

	// Se7kills
	//const bool isValidDistance = (Actor()->Position().distance_to(obj->Position()) <= voiceDistance);
	//
	//if (isValidDistance == false)
	//	return;

	// Дистанция до игрока
	float distance = Actor()->Position().distance_to(obj->Position());

	bool isCorrectSquad = false;
	for (auto PL : players_in_squad)
	{
		if (PL.PlayerState->MPSquadID == local_player->MPSquadID)
			isCorrectSquad = true;
	}

	if (!isCorrectSquad && distance > 30)
	{
		Msg("Squad Is Not Correct");
		return;
	}

 	IStreamPlayer* player = GetStreamPlayer(clientId);
	player->SetPosition(obj->Position());
	player->SetDistance(voiceDistance); // voiceDistance
 	player->SetSquad(isCorrectSquad);
 
	u8 packetsCount = P->r_u8();

	for (u32 i = 0; i < packetsCount; ++i)
	{
		u32 length = P->r_u32();
		P->r(m_buffer, length);
		player->PushToPlay(m_buffer, length);
		Msg("Squad Is Correct PushToPlay to OpenAL (если вывело значит проблемма в OPENAL) ");
	}

	if (isValidDistance)
	{
		m_voiceTimeMap[clientId] = SVoiceIconInfo(GetTickCount());
	}
}


const ui_shader& CVoiceChat::GetVoiceIndicatorShader()
{
	if (m_voiceIndicatorShader->inited()) return m_voiceIndicatorShader;

	m_voiceIndicatorShader->create("friendly_indicator", "ui\\ui_voice");
	return m_voiceIndicatorShader;
}

IStreamPlayer* CVoiceChat::GetStreamPlayer(u16 clientId)
{
	IStreamPlayer* player = m_soundPlayersMap[clientId];
	if (!player)
	{
		player = m_pSoundVoiceChat->CreateStreamPlayer();
		m_soundPlayersMap[clientId] = player;
	}
	return player;
}

void CVoiceChat::CheckAndClearPlayers(SOUND_PLAYERS& players)
{
	auto I = players.begin();
	auto E = players.end();
	decltype(I) J;

	for (; I != E;)
	{
		CObject* obj = Level().Objects.net_Find(I->first);
		if (!obj)
		{
			J = I;
			++I;

			auto voiceTimeIt = m_voiceTimeMap.find(J->first);
			if (voiceTimeIt != m_voiceTimeMap.end())
			{
				m_voiceTimeMap.erase(voiceTimeIt);
			}

			m_pSoundVoiceChat->DestroySoundPlayer(J->second);
			players.erase(J);
		}
		else {
			++I;
		}
	}
}
