#pragma once
#include <Windows.h>
#include <d3dx9math.h>

struct player_t
{
	D3DXVECTOR3 player_position;
	bool is_visible;
	bool is_team;
	//float health;
	//std::string name;
	//std::vector<D3DXVECTOR3> bones;

};

namespace Offsets
{
	inline uintptr_t DXRENDERER = 0x142738080; //0x142572fa0;
	inline uintptr_t CLIENTGAMECONTEXT = 0x142670d80; //0x1424abd20;
	inline uintptr_t GAMERENDERER = 0x142672378; //0x1424ad330;
	inline uintptr_t ANGLES = 0x1423b2ec0; //0x1421caee0;        
	inline uintptr_t CURRENT_WEAPONFIRING = ANGLES + 0x8;
	inline uintptr_t SETTINGS = 0x1423717C0; //0x1421AC5A0;
	inline uintptr_t SHOTSTATS = 0x142737A40; //0x142572950;
	inline uintptr_t PLAYER_MANAGER = 0x60;
	inline uintptr_t LOCAL_PLAYER_ARRAY = 0x540;
	inline uintptr_t PUBLIC_PLAYER_ARRAY = 0x548;
	inline uintptr_t TEAM_ID = 0x13CC;
	inline uintptr_t SOILDER = 0x14D0;
	inline uintptr_t OCCLUDED = 0x5B1;

}

enum Bones
{
	HEAD = 104,
	NECK = 142,
	SPINE2 = 7,
	SPINE1 = 6,
	SPINE = 5,
	LEFTSHOULDER = 9,
	RIGHTSHOULDER = 109,
	LEFTELBOWROLL = 11,
	RIGHTELBOWROLL = 111,
	LEFTHAND = 15,
	RIGHTHAND = 115,
	LEFTKNEEROLL = 188,
	RIGHTKNEEROLL = 197,
	LEFTFOOT = 184,
	RIGHTFOOT = 198
};