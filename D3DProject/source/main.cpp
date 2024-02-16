#include <common/glfwAutoInit.h>
#include <window/Window.h>
#include <renderer/Renderer.h>

#include <iostream>

int main() {
	glfwAutoInit init;
	if (!init.Initialized()) {
		std::cout << "Failed to initialize glfw." << std::endl;
		return EXIT_FAILURE;
	}

	Window wnd({ 1280, 720 }, "DirectX Proj");
	if (!wnd.Initialize()) {
		std::cout << "Failed to create window." << std::endl;
		return EXIT_FAILURE;
	}

	Renderer renderer(wnd);

	while (wnd.IsVisible()) {
		wnd.Run();
		renderer.Clear({ 126, 154, 56, 255 });
		renderer.Draw(3);
		renderer.Present();
	}

	return 0;
}
