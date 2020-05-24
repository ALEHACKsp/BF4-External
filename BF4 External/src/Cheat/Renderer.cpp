#include "Renderer.hpp"
#include "../Memory/Memory.hpp"
#include "../Math/Math.hpp"
#include <d3dx9math.h>
#include "../Misc/Logger.hpp"
#include <unordered_map>
#include <mutex>

std::mutex render_mutex;

bool Renderer::ConnectedToServer()
{
	G.client_game_context = M.Read<uintptr_t>(Offsets::CLIENTGAMECONTEXT);

	if (!G.client_game_context)
		return false;

	G.player_manager = M.Read<uintptr_t>(G.client_game_context + Offsets::PLAYER_MANAGER);

	if (!G.player_manager)
		return false;

	G.local_player = M.Read<uintptr_t>(G.player_manager + Offsets::LOCAL_PLAYER_ARRAY);

	if (!G.local_player)
		return false;

	in_server = true;
	return true;
}

void Renderer::RenderBones(uintptr_t soldier, D2D1::ColorF colour)
{
	for (int i = 0; i < 12; ++i)
		{
			D3DXVECTOR3 first_bone_element = Math::GetBone(soldier, PlayerStuff::bone_list[i][0]);
			D3DXVECTOR3 second_bone_element = Math::GetBone(soldier, PlayerStuff::bone_list[i][1]);

			D3DXVECTOR3 first, second;

			if (Math::WorldToScreenNew(&first_bone_element, &first) && Math::WorldToScreenNew(&second_bone_element, &second))
				overlay->DrawLine({ first.x, first.y }, { second.x, second.y }, 1, colour);
		}
}

void Renderer::Update()
{
	uintptr_t players = M.Read<uintptr_t>(G.player_manager + Offsets::PUBLIC_PLAYER_ARRAY);

	render_mutex.lock();

	for (unsigned int idx = 0; idx < 64; ++idx)
	{
		uintptr_t current_player = M.Read<uintptr_t>(players + (idx * 0x8));

		if (!current_player || current_player == G.local_player)
			continue;

		uint64_t local_team_id = M.Read<uint64_t>(G.local_player + Offsets::TEAM_ID);
		uint64_t player_team_id = M.Read<uint64_t>(current_player + Offsets::TEAM_ID);

		bool is_team = local_team_id == player_team_id;

		uintptr_t current_soldier = M.Read<uintptr_t>(current_player + Offsets::SOILDER);

		D3DXVECTOR3 player_origin = M.Read<D3DXVECTOR3>(M.Read<uintptr_t>(current_soldier + 0x490) + 0x30);

		D3DXVECTOR3 player_head = Math::GetBone(current_soldier, HEAD);

		if (!player_origin || !player_head)
			continue;

		uintptr_t comp = M.Read<uintptr_t>(current_soldier + 0x0140);

		float health = M.Read<float>(comp + 0x0020);

		bool occluded = M.Read<bool>(current_soldier + Offsets::OCCLUDED);

		bool is_visible = !occluded;

		if (local_team_id != player_team_id)
		{
			player_t pinsert { current_player, current_soldier, player_origin, player_head, is_visible, is_team, health, PlayerStuff::GetPlayersName(current_player), PlayerStuff::GetSoldiersWeapon(current_soldier) };
			player_list.insert_or_assign(current_player, pinsert);
		}

		continue;
	}

	render_mutex.unlock();

}

void Renderer::RenderLoop(Direct2DOverlay * o)
{
	overlay = o;

	overlay->BeginDraw();
	overlay->ClearScreen();

	const RECT overlayer_rect = overlay->GetOverlayRect();

	overlay->DrawString(L"aura#0240", 11, { 13.0, 4.0 }, ESP_FONT, FONT_RIGHT, { 255, 255, 255, 255 });


	if (in_server)
	{
		uintptr_t players = M.Read<uintptr_t>(G.player_manager + Offsets::PUBLIC_PLAYER_ARRAY);

		render_mutex.lock();
		for (auto [ptr, player] : player_list)
		{
			if (player.current_player == G.local_player || !player.current_player || !player.current_soldier)
				continue;

			if (player.health <= 0.1f || player.is_team)
				continue;

			D3DXVECTOR3 origin_screen, head_screen;

			if (!Math::WorldToScreenNew(&player.player_origin, &origin_screen) && !Math::WorldToScreenNew(&Math::GetBone(player.current_soldier, HEAD), &head_screen))
				continue;

			float green = player.health * 2.90f;
			float red = 255 - green;

			D2D1::ColorF render_colour = player.is_visible ? D2D1::ColorF(255, 0, 0, 255) : D2D1::ColorF(255, 255, 255, 255);
			
			/* Soldier Info */
			std::stringstream ss;
			ss << player.name << " [" << player.held_weapon << "]";
			overlay->DrawBoxWithString(M.StringToWString(ss.str()), 0, { origin_screen.x , origin_screen.y + 8 }, 7, { 255, 255, 255,255 }, ESP_FONT, { 12, 12, 12, 255 });
			overlay->DrawBoxWithString(std::to_wstring(static_cast<int>(player.health)), 0, { origin_screen.x + 2, origin_screen.y + 19 }, 6, { red, green, 0, 255 }, ESP_FONT, { 12, 12, 12, 255 });

			RenderBones(player.current_soldier, render_colour);
		}

	}
	render_mutex.unlock();
	
	//
	//if (Math::WorldToScreenNew(&player_origin, &origin_screen) && Math::WorldToScreenNew(&Math::GetBone(soldier, HEAD), &head_screen))
	//{
	//	float bh = std::abs(origin_screen.y - head_screen.y);
	//	float bw = bh * 0.5f;

	//	float green = HP * 2.70f;
	//	float red = 255 - green;
	//	float health_bar = (bw * HP) / 100.0f / 0.5;

	//	if (!is_team && soldier && HP > 0.1f)
	//	{
	//		if (bw >= 100.0f || bw <= 0.10f)
	//			continue;

	//		D3DXVECTOR3 origin_screen, head_screen, leftfoot_screen, rightfoot_screen;
	//		//overlay->DrawLine({ origin_screen.x, origin_screen.y, static_cast<float>(overlayer_rect.right - overlayer_rect.left), static_cast<float>(overlayer_rect.bottom - overlayer_rect.top) }, 1, render_colour);
	//		overlay->DrawCircle({ head_screen.x, head_screen.y }, 1.5f, 1, render_colour, false);

	//		/* HP Bar & Outline */
	//		overlay->DrawBox({ origin_screen.x + bw , origin_screen.y - 1 }, { origin_screen.x - bw, origin_screen.y + 3 }, 1, { 0, 0, 0, 255 }, false);
	//		overlay->DrawBox({ origin_screen.x + health_bar - bw, origin_screen.y }, { origin_screen.x - bw, origin_screen.y + 2 }, 1, { red, green, 0, 255 }, true);

	//		/* Health */
	//		//bool hp_check = HP > 10.0f;
	//		overlay->DrawString(std::to_wstring(static_cast<int>(HP)), 7, { origin_screen.x + health_bar - bw, origin_screen.y + 5.5f }, ESP_FONT, FONT_CENTER, { 255, 255, 255, 255 });

	//		std::stringstream ss;
	//		ss << Functions::GetPlayersName(current_player) << " [" << Functions::GetSoldiersWeapon(soldier) << "]";
	//		overlay->DrawBoxWithString(M.StringToWString(ss.str()), 0, { origin_screen.x , origin_screen.y + 16 }, 9, { 255, 255, 255,255 }, ESP_FONT, { 12, 12, 12, 255 });
	//	}

	//}

	//
	overlay->EndDraw();
	Sleep(1);
}



//uintptr_t game_render = M.Read<uintptr_t>(Offsets::GAMERENDERER);
//uintptr_t render_view = M.Read<uintptr_t>(game_render + 0x60);

//float xfov = M.Read<float>(render_view + 0x0250);
//float yfov = M.Read<float>(render_view + 0x00B4);







//overlay = o;

//overlay->BeginDraw();
//overlay->ClearScreen();

//const RECT overlayer_rect = overlay->GetOverlayRect();

//overlay->DrawString(L"aura#0240", 11, { 13.0, 4.0 }, ESP_FONT, FONT_RIGHT, { 255, 255, 255, 255 });


//if (in_server)
//{
//	uintptr_t players = M.Read<uintptr_t>(G.player_manager + Offsets::PUBLIC_PLAYER_ARRAY);

//	if (!players)
//		return;

//	for (unsigned int idx = 0; idx < 64; idx++)
//	{
//		uintptr_t current_player = M.Read<uintptr_t>(players + (idx * 0x8));

//		if (!current_player || current_player == G.local_player)
//			continue;

//		uint64_t local_team_id = M.Read<uint64_t>(G.local_player + Offsets::TEAM_ID);
//		uint64_t player_team_id = M.Read<uint64_t>(current_player + Offsets::TEAM_ID);

//		bool is_team = local_team_id == player_team_id;

//		uintptr_t current_soldier = M.Read<uintptr_t>(current_player + Offsets::SOILDER);

//		if (!current_soldier)
//			continue;

//		D3DXVECTOR3 player_origin = M.Read<D3DXVECTOR3>(M.Read<uintptr_t>(current_soldier + 0x490) + 0x30);
//		D3DXVECTOR3 player_head = Math::GetBone(current_soldier, HEAD);

//		float health = M.ReadChain<float>(current_soldier, { 0x0140, 0x0020 });

//		bool occluded = M.Read<bool>(current_soldier + Offsets::OCCLUDED);

//		bool is_visible = !occluded;

//		D3DXVECTOR3 origin_screen, head_screen;
//		if (!Math::WorldToScreenNew(&player_origin, &origin_screen) && !Math::WorldToScreenNew(&Math::GetBone(current_soldier, HEAD), &head_screen))
//			continue;

//		float bh = std::abs(player_origin.y - player_head.y);
//		float bw = bh * 0.5f;

//		float green = health * 2.70f;
//		float red = 255 - green;
//		float health_bar = (bw * health) / 100.0f / 0.5;

//		if (bw >= 150.0f || bw <= 0.00f)
//			continue;

//		D2D1::ColorF render_colour = is_visible ? D2D1::ColorF(255, 0, 0, 255) : D2D1::ColorF(255, 255, 255, 255);


//		if (is_team || health <= 0.1)
//			continue;

//		/* HP BAR */
//		overlay->DrawBox({ origin_screen.x + bw , origin_screen.y - 1 }, { origin_screen.x - bw, origin_screen.y + 3 }, 1, { 0, 0, 0, 255 }, false);
//		overlay->DrawBox({ origin_screen.x + health_bar - bw, origin_screen.y }, { origin_screen.x - bw, origin_screen.y + 2 }, 1, { red, green, 0, 255 }, true);
//		overlay->DrawString(std::to_wstring(static_cast<int>(health)), 7, { origin_screen.x + health_bar - bw, origin_screen.y + 5.5f }, ESP_FONT, FONT_CENTER, { 255, 255, 255, 255 });

//		/* Soldier Info */
//		std::stringstream ss;
//		ss << PlayerStuff::GetPlayersName(current_player) << " [" << PlayerStuff::GetSoldiersWeapon(current_soldier) << "]";
//		overlay->DrawBoxWithString(M.StringToWString(ss.str()), 0, { origin_screen.x , origin_screen.y + 16 }, 9, { 255, 255, 255,255 }, ESP_FONT, { 12, 12, 12, 255 });

//		/* Bones */
//		RenderBones(current_soldier, render_colour);
//	}
//}


/* HP BAR */
			//overlay->DrawBox({ origin_screen.x + bw , origin_screen.y - 1 }, { origin_screen.x - bw, origin_screen.y + 3 }, 1, { 0, 0, 0, 255 }, false);
			//overlay->DrawBox({ origin_screen.x + health_bar - bw, origin_screen.y }, { origin_screen.x - bw, origin_screen.y + 2 }, 1, { red, green, 0, 255 }, true);
			//overlay->DrawString(std::to_wstring(static_cast<int>(player.health)), 7, { origin_screen.x + health_bar - bw, origin_screen.y + 5.5f }, ESP_FONT, FONT_CENTER, { 255, 255, 255, 255 });