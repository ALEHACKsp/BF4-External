#pragma once
#include <Windows.h>
#include "../Overlay/Overlay.hpp"
#include <vector>
#include <map>
#include "../Game.hpp"
#include <unordered_map>

namespace Renderer
{
	inline Direct2DOverlay* overlay;
	inline bool in_server = false;
	inline std::map<uintptr_t, player_t> player_list;
	inline std::vector<uintptr_t> dead_list;
	inline game_t G;
	extern bool ConnectedToServer();
	extern void Discovery();
	extern void RenderBones(uintptr_t soldier, D2D1::ColorF colour);
	void Update();
	extern void RenderLoop(Direct2DOverlay* overlay);
}
