#pragma once
#include "proc_utils.h"
#include "Structs.h"

enum  _IN_CROSS_OBJECT : unsigned char
{
	INC_CROSS_CLEAR,
	INC_FRIEND,
	INC_ENEMY,
	INC_HOSTAGE
};

class Game
{
	private:
		unsigned int in_cross_addr = 0;
		unsigned int hl_pid = 0;
		unsigned int base_addr = 0;
		unsigned int is_user_in_spect_addr = 0;
		unsigned int userPV_addr = 0;
		HANDLE hlprc = 0;
		_IN_CROSS_OBJECT in_cross = _IN_CROSS_OBJECT::INC_CROSS_CLEAR;
		unsigned char is_user_in_spect = 0;
		SIZE_T io;
		PlayerView pv;

	public:
		bool FindGameProccess();
		bool OpenGameProcess();
		bool UpdateGameData();
		_IN_CROSS_OBJECT GetInCrossObject();
		bool IsUserInSpects();
		void SendFire();
		bool IsGameFocused();
		void CloseHandles();
		PlayerView GetPV();
}; 

