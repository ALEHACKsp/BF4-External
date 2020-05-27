#include "Renderer.hpp"
#include "../Memory/Memory.hpp"
#include "../Math/Math.hpp"
#include <d3dx9math.h>
#include "../Misc/Logger.hpp"
#include <unordered_map>
#include <mutex>
#include "../Memory/Shellcode.hpp"

#define M_PI 3.14159265358979323846

std::mutex render_mutex;

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

D3DXVECTOR2 RotatePoint(D3DXVECTOR2 pointToRotate, D3DXVECTOR2 centerPoint, float angle, bool angleInRadians = true)
{
	if (!angleInRadians)
		angle = (float)(angle * (M_PI / 180.0f));

	float cosTheta = (float)std::cos(angle);
	float sinTheta = (float)std::sin(angle);

	D3DXVECTOR2 returnVec = D3DXVECTOR2(
		cosTheta * (pointToRotate.x - centerPoint.x) - sinTheta * (pointToRotate.y - centerPoint.y),
		sinTheta * (pointToRotate.x - centerPoint.x) + cosTheta * (pointToRotate.y - centerPoint.y)
	);

	returnVec += centerPoint;
	return returnVec;
}

float DegToRad(float deg) { return (float)(deg * (M_PI / 180.0f)); }
float RadToDeg(float deg) { return (float)(deg * (180.0f / M_PI)); }

D3DXVECTOR2 Renderer::ConvertToRadar(player_t player)
{
	D3DXVECTOR2 center = {(radar_size.x + radar_size.y) / 2, (radar_size.x + radar_size.y) / 2 };

	D3DXVECTOR2 enemy_pos_2d = { player.origin.x, player.origin.z };
	D3DXVECTOR2 local_pos_2d = { G.local_player.origin.x, G.local_player.origin.z };


	D3DXVECTOR2 screen_pos = local_pos_2d - enemy_pos_2d;

	float distance = D3DXVec2Length(&screen_pos) * radar_distance;

	D3DXVECTOR2 normalized_screen_pos;
	D3DXVec2Normalize(&normalized_screen_pos, &screen_pos);


	normalized_screen_pos *= distance;

	normalized_screen_pos += center;



	return RotatePoint(normalized_screen_pos, center, G.local_player.view_angles.y + 90, false);
}

void Renderer::DrawPlayer(player_t player)
{
	int FOV = 105;
	D3DXVECTOR2 screen_pos = ConvertToRadar(player);

	if (G.local_player.health <= 0.01f)
		return;

	//overlay->DrawLine({ (radar_size.x + radar_size.y) / 2, (radar_size.x + radar_size.y) / 2 }, {  }, 1, { 0, 255, 0, 255 });
	overlay->DrawCircle({ screen_pos.x, screen_pos.y }, 2, 2, { 255, 0, 0, 255 }, 1);

}

void Renderer::DrawLocalPlayer()
{

	overlay->DrawCircle({ (radar_size.x + radar_size.y) / 2, (radar_size.x + radar_size.y) / 2 }, 1.5f, 1, { 0, 255, 0, 255 }, true);

}

void Renderer::DrawLocalRadar()
{
	overlay->DrawBox({ radar_size.x, radar_size.x }, { radar_size.y, radar_size.y }, 1, { 0, 0, 0, 120 }, true);
	overlay->DrawBox({ radar_size.x - 1, radar_size.x + 1 }, { radar_size.y - 1, radar_size.y + 1 }, 1, { 255, 0, 0, 255 }, false);

}

void Renderer::Update()
{
	G.client_game_context = M.Read<uintptr_t>(Offsets::CLIENTGAMECONTEXT);

	if (!G.client_game_context)
		return;

	G.player_manager = M.Read<uintptr_t>(G.client_game_context + Offsets::PLAYER_MANAGER);

	if (!G.player_manager)
		return;
	/*uintptr_t game_renderer = M.Read<uintptr_t>(0x142672378);
	uintptr_t render_view = M.Read<uintptr_t>(game_renderer + 0x60);

	D3DXMATRIX matrix_inverse = M.Read<D3DXMATRIX>(render_view + 0x02E0);*/

	uintptr_t local_player_ptr = M.Read<uintptr_t>(G.player_manager + Offsets::LOCAL_PLAYER_ARRAY);

	if (!local_player_ptr)
		return;

	uintptr_t players = M.Read<uintptr_t>(G.player_manager + Offsets::PUBLIC_PLAYER_ARRAY);

	render_mutex.lock();

	for (unsigned int idx = 0; idx <= 70; ++idx)
	{
		uintptr_t current_player = M.Read<uintptr_t>(players + (idx * 0x8));

		if (!current_player)
			continue;

		uintptr_t local_soldier = M.Read<uintptr_t>(local_player_ptr + Offsets::SOILDER);

		float yaw = M.Read<float>(local_soldier + 0x04D8);
		float pitch = M.Read<float>(local_soldier + 0x04DC);

		uintptr_t comp_local = M.Read<uintptr_t>(local_soldier + 0x0140);

		float health_local = M.Read<float>(comp_local + 0x0020);

		uint64_t local_team_id = M.Read<uint64_t>(local_player_ptr + Offsets::TEAM_ID);
		uint64_t player_team_id = M.Read<uint64_t>(current_player + Offsets::TEAM_ID);

		bool is_team = local_team_id == player_team_id;

		uintptr_t current_soldier = M.Read<uintptr_t>(current_player + Offsets::SOILDER);

		D3DXVECTOR3 player_origin = M.Read<D3DXVECTOR3>(M.Read<uintptr_t>(current_soldier + 0x490) + 0x30);

		D3DXVECTOR3 local_player_origin = M.Read<D3DXVECTOR3>(M.Read<uintptr_t>(local_soldier + 0x490) + 0x30);

		D3DXVECTOR3 player_head = Math::GetBone(current_soldier, HEAD);

		if (!player_origin || !player_head)
			continue;

		uintptr_t comp = M.Read<uintptr_t>(current_soldier + 0x0140);

		float health = M.Read<float>(comp + 0x0020);

		bool occluded = M.Read<bool>(current_soldier + Offsets::OCCLUDED);

		bool is_visible = !occluded;


		player_t pinsert { current_player, current_soldier, player_origin, player_head, is_visible, is_team, health, PlayerStuff::GetPlayersName(current_player), PlayerStuff::GetSoldiersWeapon(current_soldier) };
		player_list.insert_or_assign(current_player, pinsert);

		G.local_player.health = health_local;
		G.local_player.origin = local_player_origin;
		G.local_player.view_angles = D3DXVECTOR2(pitch, yaw);
		G.local_player.player = local_player_ptr;

		continue;
	}

	render_mutex.unlock();

}

float CalcDistance(D3DXVECTOR3 enemy_pos, D3DXVECTOR3 localplayer_pos)
{
	float x = enemy_pos.x - localplayer_pos.x;
	float z = enemy_pos.z - localplayer_pos.z;

	return sqrt((x * x) + (z * z));
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
		/* Draw Radar */
		//DrawLocalRadar();

		/* draw local*/
		//DrawLocalPlayer();

		render_mutex.lock();
		for (auto [ptr, player] : player_list)
		{

			if (player.player == G.local_player.player || !player.player || !player.soldier || !ptr || player.held_weapon == "" || player.name == "")
				continue;

			if (player.health <= 0.01f || player.is_team || G.local_player.health <= 0.1f)
				continue;

			/* draw players*/
			//DrawPlayer(player);

			D3DXVECTOR3 origin_screen, head_screen;

			if (!Math::WorldToScreenNew(&player.origin, &origin_screen) && !Math::WorldToScreenNew(&Math::GetBone(player.soldier, HEAD), &head_screen))
				continue;

			//float red = 255 - (player.health * 2.55f);
			//float green = (player.health * 2.55f);

			D2D1::ColorF render_colour = player.is_visible ? D2D1::ColorF(255, 0, 0, 255) : D2D1::ColorF(255, 255, 255, 255);
			
			float distance = CalcDistance(player.origin, G.local_player.origin);
			std::string distance_str = std::to_string(static_cast<int>(distance)) + "m";
			overlay->DrawBoxWithString(M.StringToWString(distance_str), false, { origin_screen.x, origin_screen.y + 8 }, 7, { 255,255,255,255 }, ESP_FONT, { 20, 20, 20, 255 });
			
			/* Soldier Info */
			//std::stringstream ss;
			//ss << player.name << " [" << player.held_weapon << "]";
			//overlay->DrawBoxWithString(M.StringToWString(ss.str()), 0, { origin_screen.x , origin_screen.y + 8 }, 7, { 255, 255, 255,255 }, ESP_FONT, { 12, 12, 12, 255 });
			//overlay->DrawBoxWithString(std::to_wstring(static_cast<int>(player.health)), 0, { origin_screen.x + 2, origin_screen.y + 19 }, 6, { red, green, 0, 255 }, ESP_FONT, { 12, 12, 12, 255 });
			
			RenderBones(player.soldier, render_colour);
		}

	}
	render_mutex.unlock();

	overlay->EndDraw();
	Sleep(1);
}
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