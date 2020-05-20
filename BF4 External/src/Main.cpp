/* STL */
#include <Windows.h>
#include <thread>

/* Other */
#include "Memory/Memory.hpp"
#include "Misc/Logger.hpp"
#include "Cheat/Cheat.hpp"
#include "Overlay/Overlay.hpp"
#include "Cheat/Renderer.hpp"

Cheat C;
HINSTANCE hinstance = GetModuleHandle(NULL);

void RenderUpdate()
{
	while (true)
	{
		Direct2DOverlay* over = new Direct2DOverlay(hinstance, Renderer::RenderLoop);
		over->Initialize(M.process_id, "External");
		over->StartLoop();
		Sleep(1);
	}
}

void ESPUpdate()
{
	while (1)
	{
		C.Init();
		Sleep(1);
	}
}
void Initialize()
{
	M.Attach();

	std::thread render_thread(RenderUpdate);
	//std::thread esp_thread(ESPUpdate);


	render_thread.join();
	//esp_thread.join();

}

int main()
{
	Initialize();
	
}  