
#include "RenderEngine/RenderEngine.h"
#include "logging.h"

class App {
public:
    void run() {
        RenderEngine::init();
        RenderEngine::loop();

        exit();
    }

private:
    void loop() {
        while (!RenderEngine::window_is_closed()) {
            RenderEngine::frame();
            RenderEngine::window_update();
        }
    }

    void exit() {
        RenderEngine::exit();
    }
};

int main() {
    App app {};

    try {
        app.run();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        spdlog::error("\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}