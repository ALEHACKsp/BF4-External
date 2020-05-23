/* STL */
#include <Windows.h>
#include <thread>

/* Other */
#include "Memory/Memory.hpp"
#include "Misc/Logger.hpp"
#include "Cheat/Cheat.hpp"
#include "Overlay/Overlay.hpp"
#include "Cheat/Renderer.hpp"


HINSTANCE hinstance = GetModuleHandle(NULL);

void RenderUpdate()
{
	Direct2DOverlay* over = new Direct2DOverlay(hinstance, Renderer::RenderLoop);
	while ( 1 )
	{
		over->Initialize(M.process_id, "External");
		over->StartLoop();
		Sleep(1);
	}
}

void Update()
{
	while (1)
	{
		Renderer::Update();
		Sleep(1);
	}
}

void Initialize()
{
	M.Attach();
	Renderer::ConnectedToServer();
	Direct2DOverlay* over = new Direct2DOverlay(hinstance, Renderer::RenderLoop);

	std::thread update_thread(Update);
	over->Initialize(M.process_id, "External");
	over->StartLoop();
	
	update_thread.join();
}

int main()
{
	Initialize();
}  