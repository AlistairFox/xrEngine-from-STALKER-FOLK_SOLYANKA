#pragma once
#include "..\xrServerEntities\PHNetState.h"

 
class CPhysicStorage
{

	struct SyncPhysic
	{
		// ONLY IF NEED
		Fvector linear_vel;
		Fvector angular_vel;

		// ALWAYS WRITE
		bool enabled;
		Fvector force;
		Fvector torque;
		Fvector position;
		Fquaternion quaternion;

		// FOR SETUP ONLY
		Fquaternion previous_quaternion;
		Fvector previous_position;


		// NO WRITE
		Fvector accel;
		float max_velocity;

		// TODO FLAGS
		flags8 enabled_forces;
		enum FLAGS_TYPE
		{
			eEnable = 1 << 0,
			eLinear = 1 << 1,
			eAngular = 1 << 2
		};
	public:

		void WritePacket(NET_Packet& Packet);

		void ReadPacket(NET_Packet& Packet);

		void GetFromPhysicState(SPHNetState& state)
		{
			// NOT USED
			accel = state.accel;
			max_velocity = state.max_velocity;

			// Satates
			enabled = state.enabled;
			force = state.force;
			torque = state.torque;

			position = state.position;
			quaternion = state.quaternion;

			linear_vel = state.linear_vel;
			angular_vel = state.angular_vel;
		}

		SPHNetState GetPhysicState()
		{
			SPHNetState state;
			// NOT USED
			state.accel = accel;
			state.max_velocity = max_velocity;

			// Satates
			state.enabled = enabled;
			state.force = force;
			state.torque = torque;

			// POS, DIR
			state.position = position;
			state.quaternion = quaternion;

			// VELOCITY
			state.linear_vel = linear_vel;
			state.angular_vel = angular_vel;

			// OLD STATE
			state.previous_position = previous_position;
			state.previous_quaternion = previous_quaternion;

			return state;
		}
	};
	
	//virtual void						make_Interpolation	(); // interpolation from last visible to corrected position/rotation
	struct net_update_PItem
	{
		u32 dwTimeStamp;
		SyncPhysic State;
		u8 count;
	};

	float interpolate_states(net_update_PItem const& first, net_update_PItem const& last, SyncPhysic& current);


	// USED FOR INTERPOLATION
	xr_deque<net_update_PItem> NET_IItem;

	net_update_PItem current_state;

	u32 m_dwIStartTime;
	u32 m_dwIEndTime;

	//	void* object;

public:
	// DOOR
	bool DoorState = false;
	bool freezed   = false;
 
	NET_Packet door_packet;

	Fvector door_axis;
	float high;
	float low;
	float force;
	float velocity;

	// LAST
	u8 SyncActive = 0;


	void WriteData(NET_Packet& P); // moved
	void ReadData(NET_Packet& P); // moved

	virtual void Interpolate(void* ptr);
	void CalculateInterpolationParams(void* ptr);


	void GetStateClient(void* ptr);
	void SetStateClient(void* ptr);

	void GetStateServer(void* ptr);
	void SetStateServer(void* ptr);
}; 
