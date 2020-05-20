#pragma once
#include <Windows.h>
#include "../Overlay/Overlay.hpp"
#include <vector>
#include "../Game.hpp"

namespace Renderer
{
	inline std::vector<player_t> player_list;
	extern void RenderLoop(Direct2DOverlay* overlay);
}