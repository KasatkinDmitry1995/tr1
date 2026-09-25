#pragma once
#include "proc_utils.h"

enum  _IN_CROSS_OBJECT : unsigned char
{
	INC_CROSS_CLEAR,
	INC_FRIEND,
	INC_ENEMY,
	INC_HOSTAGE
};

class Game
{
	public:
		bool FindGameProccess();
		bool OpenGameProcess();
		bool UpdateGameData();
		_IN_CROSS_OBJECT GetInCrossObject();
		bool IsUserInSpects();
		void SendFire();
		bool IsGameFocused();
		void CloseHandles();
}; 

