#pragma once
#include <Windows.h>
#include <cmath>
#include <d3dx9math.h>
#include "../Memory/Memory.hpp"
#include "../Game.hpp"

namespace Math
{
	inline bool WorldToScreenNew(D3DXVECTOR3* vPos, D3DXVECTOR3* vOut)
	{
		if (!vPos || !vOut)
			return false;

		uintptr_t GameRenderer = M.Read<uintptr_t>(Offsets::GAMERENDERER);
		if (!GameRenderer)
			return false;

		uintptr_t pRenderView = M.Read<uintptr_t>(GameRenderer + 0x60);
		if (!pRenderView)
			return false;

		auto DxRenderer = M.Read<uintptr_t>(Offsets::DXRENDERER);
		if (!DxRenderer)
			return false;

		uintptr_t m_pScreen = M.Read<uintptr_t>(DxRenderer + 0x38);

		if (!m_pScreen)
			return false;

		int ww = M.Read<int>(m_pScreen + 0x58);
		int hh = M.Read<int>(m_pScreen + 0x5C);

		float mX = static_cast<float>(ww * 0.5f);
		float mY = static_cast<float>(hh * 0.5f);

		D3DXMATRIX screenTransform = M.Read<D3DXMATRIX>(pRenderView + 0x420);
		D3DXVECTOR3 origin = (*vPos);

		float w = screenTransform(0, 3) * origin.x + screenTransform(1, 3) * origin.y + screenTransform(2, 3) * origin.z + screenTransform(3, 3);

		if (w < 0.0001f)
		{
			vOut->z = w;

			return false;
		}

		float x = screenTransform(0, 0) * origin.x + screenTransform(1, 0) * origin.y + screenTransform(2, 0) * origin.z + screenTransform(3, 0);
		float y = screenTransform(0, 1) * origin.x + screenTransform(1, 1) * origin.y + screenTransform(2, 1) * origin.z + screenTransform(3, 1);

		vOut->x = mX + mX * x / w;
		vOut->y = mY - mY * y / w;
		vOut->z = w;

		return true;
	}

	inline bool WorldToScreen(const D3DXVECTOR3& world_point, D3DXVECTOR3& out)
	{
		uintptr_t GameRenderer = M.Read<uintptr_t>(Offsets::GAMERENDERER);
		if (!GameRenderer)
			return false;

		uintptr_t pRenderView = M.Read<uintptr_t>(GameRenderer + 0x60);
		if (!pRenderView)
			return false;

		D3DXMATRIXA16 view_x_projection = M.Read<D3DXMATRIXA16>(pRenderView + 0x420);

		auto DxRenderer = M.Read<uintptr_t>(Offsets::DXRENDERER);
		if (!DxRenderer)
			return false;

		uintptr_t m_pScreen = M.Read<uintptr_t>(DxRenderer + 0x38);

		if (!m_pScreen)
			return false;

		int ScreenWidth = M.Read<int>(m_pScreen + 0x58);
		int ScreenHeight = M.Read<int>(m_pScreen + 0x5C);
		//printf("%d %d ", ScreenWidth, ScreenHeight);

		float cX = ScreenWidth * 0.5f;
		float cY = ScreenHeight * 0.5f;

		auto x = view_x_projection(0, 0) * world_point.x + view_x_projection(1, 0) * world_point.y + view_x_projection(2, 0) * world_point.z + view_x_projection(3, 0);
		auto y = view_x_projection(0, 1) * world_point.x + view_x_projection(1, 1) * world_point.y + view_x_projection(2, 1) * world_point.z + view_x_projection(3, 1);
		auto z = view_x_projection(0, 2) * world_point.x + view_x_projection(1, 2) * world_point.y + view_x_projection(2, 2) * world_point.z + view_x_projection(3, 2);
		auto w = view_x_projection(0, 3) * world_point.x + view_x_projection(1, 3) * world_point.y + view_x_projection(2, 3) * world_point.z + view_x_projection(3, 3);

		if (w < 0.65f)
		{
			out.z = w;
			return false;
		}

		out.x = cX * (1 + x / w);
		out.y = cY * (1 - y / w);
		out.z = w;

		return true;
	}

	inline D3DXVECTOR3 GetBone(uintptr_t pSoldier, int bone_id)
	{
		D3DXVECTOR3 tmp, out;
		uintptr_t ragdoll_component = M.Read<uintptr_t>(pSoldier + 0x0580);

		if (!ragdoll_component)
			return D3DXVECTOR3(0, 0, 0);

		uintptr_t quat = M.Read<uintptr_t>(ragdoll_component + 0xB0); //0xB0
		if (!quat)
			return D3DXVECTOR3(0, 0, 0);

		tmp = M.Read<D3DXVECTOR3>(quat + bone_id * 0x20);

		out.x = tmp.x;
		out.y = tmp.y;
		out.z = tmp.z;

		return out;
	}
}