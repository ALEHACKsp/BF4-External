#pragma once
#include <Windows.h>
#include "../Overlay/Overlay.hpp"
#include <vector>
#include <map>
#include "../Game.hpp"
#include <unordered_map>

namespace Renderer
{
	inline D3DXVECTOR2 radar_size = { 30, 170 };
	inline float radar_distance = 0.6;
	inline Direct2DOverlay* overlay;
	inline bool in_server = true;
	inline std::unordered_map<uintptr_t, player_t> player_list;
	inline std::vector<uintptr_t> dead_list;
	inline game_t G;

	extern void RenderBones(uintptr_t soldier, D2D1::ColorF colour);
	extern D3DXVECTOR2 ConvertToRadar(player_t player);
	extern void DrawPlayer(player_t player);
	extern void DrawLocalPlayer();
	extern void DrawLocalRadar();
	extern void Update();
	//extern bool BitBlt();
	extern void RenderLoop(Direct2DOverlay* overlay);
}
