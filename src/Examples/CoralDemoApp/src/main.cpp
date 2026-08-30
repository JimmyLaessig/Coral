#include "CoralDemoApp.hpp"

int main()
{
    CoralDemoApp app;
    if (!app.initialize())
    {
        return EXIT_FAILURE;
    }

    return app.run();
}
