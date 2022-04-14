
#include "RenderEngine/RenderEngine.h"
#include "Logging.h"

class App {
public:
    RenderEngine renderEngine;
    
    void run() {
        renderEngine.init();
        loop();
        exit();
    }

private:
    void loop() {
        while (!renderEngine.window_is_closed()) {

            renderEngine.frame();

            renderEngine.window_update();
        }
    }

    void exit() {
        renderEngine.exit();
    }
};

int main() {
    App app{};

    try {
        app.run();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        spdlog::error("\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}