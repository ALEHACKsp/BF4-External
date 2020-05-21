#include "Renderer.hpp"
#include "../Memory/Memory.hpp"
#include "../Math/Math.hpp"
#include <d3dx9math.h>
#include "../Misc/Logger.hpp"

void Renderer::RenderLoop(Direct2DOverlay* overlay)
{
	overlay->BeginDraw();
	overlay->ClearScreen();

	const RECT overlayer_rect = overlay->GetOverlayRect();

	overlay->DrawString(L"aura#0240", 11, { 13.0, 4.0 }, ESP_FONT, FONT_RIGHT, { 255, 255, 255, 255 });
	uintptr_t client_game_context = M.Read<uintptr_t>(Offsets::CLIENTGAMECONTEXT);

	if (!client_game_context)
		return;

	uintptr_t player_manager = M.Read<uintptr_t>(client_game_context + Offsets::PLAYER_MANAGER);

	if (!player_manager)
		return;

	uintptr_t local_player = M.Read<uintptr_t>(player_manager + Offsets::LOCAL_PLAYER_ARRAY);

	if (!local_player)
		return;

	uintptr_t players = M.Read<uintptr_t>(player_manager + Offsets::PUBLIC_PLAYER_ARRAY);

	if (!players)
		return;

	for (unsigned int idx = 0; idx < 64; idx++)
	{
		uintptr_t current_player = M.Read<uintptr_t>(players + (idx * 0x8));

		if (!current_player || current_player == local_player)
			continue;

		uint64_t local_team_id = M.Read<uint64_t>(local_player + Offsets::TEAM_ID);
		uint64_t player_team_id = M.Read<uint64_t>(current_player + Offsets::TEAM_ID);

		bool is_team = local_team_id == player_team_id;

		uintptr_t soldier = M.Read<uintptr_t>(current_player + Offsets::SOILDER);

		if (!soldier)
			continue;

		D3DXVECTOR3 player_origin = M.Read<D3DXVECTOR3>(M.Read<uint64_t>(soldier + 0x490) + 0x30);

		D3DXVECTOR3 origin_screen, head_screen, leftfoot_screen, rightfoot_screen;
		float HP = M.ReadChain<float>(soldier, { 0x140, 0x20 });


		bool occluded = M.Read<bool>(soldier + Offsets::OCCLUDED);

		bool visible = !occluded;

		D2D1::ColorF render_colour = visible ? D2D1::ColorF(255, 0, 0, 255) : D2D1::ColorF(255, 255, 255, 255);
		if (Math::WorldToScreenNew(&player_origin, &origin_screen) && Math::WorldToScreenNew(&Math::GetBone(soldier, HEAD), &head_screen))
		{
			float bh = std::abs(origin_screen.y - head_screen.y);
			float bw = bh * 0.5f;

			float green = HP * 2.70f;
			float red = 255 - green;
			float health_bar = (bw * HP) / 100.0f / 0.5;
			
			if (!is_team && soldier && HP > 0.1f)
			{
				if (bw >= 80.0f || bw <= 0.0)
					continue;

				//overlay->DrawLine({ origin_screen.x, origin_screen.y, static_cast<float>(overlayer_rect.right - overlayer_rect.left), static_cast<float>(overlayer_rect.bottom - overlayer_rect.top) }, 1, render_colour);
				overlay->DrawCircle({ head_screen.x, head_screen.y }, 1.5f, 1, render_colour, false);
				overlay->DrawBox({ origin_screen.x + bw , origin_screen.y - 1 }, { origin_screen.x - bw, origin_screen.y + 3 }, 1, { 0, 0, 0, 255 }, false);
				overlay->DrawBox({ origin_screen.x + health_bar - bw, origin_screen.y}, { origin_screen.x - bw, origin_screen.y + 2 }, 1, { red, green, 0, 255 }, true);
				overlay->DrawString(std::to_wstring(static_cast<int>(HP)), 9, { origin_screen.x + health_bar - bw, origin_screen.y + 7 }, ESP_FONT, FONT_CENTER, { 255, 255, 255, 255 });
				//overlay->DrawString(M.StringToWString(Functions::GetSoldiersWeapon(soldier)), 10, { origin_screen.x , origin_screen.y + 12 }, ESP_FONT, FONT_CENTER, { 0,0,255,255 });
				overlay->DrawBoxWithString(M.StringToWString(Functions::GetPlayersName(current_player)), 0, { origin_screen.x , origin_screen.y + 14 }, 10, { 255,255,255,255 }, ESP_FONT, { 10,10,10,255 });
				//overlay->DrawString(M.StringToWString(Functions::GetPlayersName(current_player)) /*+ M.StringToWString(Functions::GetSoldiersWeapon(soldier))*/, 10, { origin_screen.x , origin_screen.y + 13 }, ESP_FONT, FONT_CENTER, { 255, 255, 255,255 });
			}

		}

		//std::vector<Bones> bone_list = { HEAD, NECK, SPINE2, SPINE1, SPINE, LEFTSHOULDER, RIGHTSHOULDER, LEFTELBOWROLL,RIGHTELBOWROLL, LEFTHAND,RIGHTHAND, LEFTKNEEROLL, RIGHTKNEEROLL, LEFTFOOT, RIGHTFOOT };
		
		int aSkeleton[][2] =
		{
		{ 104, 142 }, { 142, 9 },{ 9, 11 },{ 11, 15 },
		{ 142,109},{109,111 },{111, 115},{ 142, 5 },
		{ 5,  188},{ 5, 197},{ 188, 184},{ 197, 198},
		};

		for (int i = 0; i < 12; ++i)
		{
			D3DXVECTOR3 Bone1 = Math::GetBone(soldier, aSkeleton[i][0]);
			D3DXVECTOR3 Bone2 = Math::GetBone(soldier, aSkeleton[i][1]);
			D3DXVECTOR3 Out1, Out2, Out3;

			if (Math::WorldToScreenNew(&Bone1, &Out1) && Math::WorldToScreenNew(&Bone2, &Out2))
			{
				if (!is_team && soldier && HP > 0.1f)
					overlay->DrawLine({ Out1.x, Out1.y }, { Out2.x, Out2.y }, 1, render_colour);
			}
		}
		//player_t player{ position_foot, visible, is_team };
		//Renderer::player_list.push_back(player);
		//Logger::Print("[%d] VEC3(%f, %f, %f)", idx, output.x, output.y, output.z);

		
	}
		

	overlay->EndDraw();
	Sleep(1);

}
