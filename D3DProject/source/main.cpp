#include <common/glfwAutoInit.h>
#include <window/Window.h>
#include <renderer/Renderer.h>
#include <renderer/RenderManager.h>

#include <iostream>

namespace object {
	ObjectData triangleData {
		std::vector<Vertex> {
			{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ {  0.0f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		},

		std::vector<uint32_t> {
		    0, 1, 2
		}
	};
}

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

	auto state = render::State::Create();
	auto renderer = std::make_shared<render::Renderer>(wnd, state);

	auto manager = render::RenderManager(renderer, state);
	size_t id = manager.AddObject(object::triangleData);

	while (wnd.IsVisible()) {
		wnd.Run();
		renderer->Clear({ 126, 154, 56, 255 });
		manager.Draw(id);
		renderer->Present();
	}

	return 0;
}
