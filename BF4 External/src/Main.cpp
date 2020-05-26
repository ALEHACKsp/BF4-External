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

void Update()
{
	while (1)
	{
		Renderer::Update();
		Sleep(1);
	}
}


void ForceFalse()
{
	while (1)
	{
		if (Renderer::BitBlt());
		printf("1 \n");
		Sleep(150);
	}
}

int main()
{
	M.Attach();
	Direct2DOverlay* over = new Direct2DOverlay(hinstance, Renderer::RenderLoop);

	std::thread update_thread(Update);
	std::thread force_false(ForceFalse);

	over->Initialize(M.process_id, "External");
	over->StartLoop();

	update_thread.join();
	force_false.join();
}  