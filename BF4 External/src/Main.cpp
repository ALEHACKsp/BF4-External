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

byte bforce_false[3]
{
	0x31, 0xC0,           //xor eax, eax
	0xC3                  //ret
};

void ForceFalse()
{
	while ( 1 )
	{
		Sleep(2500);
		M.Write(M.gdi32_address + 0x2EB0, bforce_false);
		Sleep(2500);
	}
}

int main()
{
	M.Attach();
	Direct2DOverlay* over = new Direct2DOverlay(hinstance, Renderer::RenderLoop);

	std::thread update_thread(Update);
	std::thread force_false(ForceFalse);
	over->Initialize(M.process_id, "Microsoft Visualizer");
	over->StartLoop();

	update_thread.join();
	force_false.join();
}  