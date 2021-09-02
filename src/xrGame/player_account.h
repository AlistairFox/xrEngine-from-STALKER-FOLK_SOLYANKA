#ifndef PLAYER_ACCOUNT_H
#define PLAYER_ACCOUNT_H

#include "profile_data_types.h"

class player_account
{
public:
							player_account	();
							~player_account	();
	
	shared_str		const &	name			() const { return m_player_name; };
	//new Accaunt System
	shared_str      const & name_save		() const { return m_player_save_name; };
	shared_str      const & name_login		() const { return m_player_login; };
	shared_str		const & password		() const { return m_player_password; };
	//new Accaunt System

	shared_str		const &	clan_name		() const { return m_clan_name; };
	u32				const	profile_id		() const { return m_profile_id; };		
	bool			const	is_clan_leader	() const { return m_clan_leader; };
	
	void					net_Import		(NET_Packet & P);
	void					net_Export		(NET_Packet & P);
	static			void	skip_Import		(NET_Packet & P);
	void					load_account	();
	bool					is_online		() const { return m_online_account; };

	gamespy_profile::all_awards_t const &	get_awards() const { return m_awards; };
	void					set_player_name	(char const * new_name);
	void					set_player_save (char const * new_save_name);
	void                    set_player_login(char const * new_login );
	void					set_player_password(char const * new_pass);

protected:
	shared_str						m_player_name;
	shared_str						m_clan_name;
	u32								m_profile_id;
	bool							m_clan_leader;
	bool							m_online_account;
	//new Accaunt System
	shared_str						m_player_save_name;
	shared_str						m_player_login;
	shared_str                      m_player_password;
	//new Accaunt System

	gamespy_profile::all_awards_t	m_awards;
}; //class player_account


#endif //#ifndef PLAYER_ACCOUNT_H