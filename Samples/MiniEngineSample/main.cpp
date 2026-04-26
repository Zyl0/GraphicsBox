
#include "App.h"
#include "Core/Spec.h"
#include "Modules/Window/Module.h"

int main(int argc, char* argv[])
{
    Engine::Spec Specification;
    Specification.Register<Window::Module>();
    
    Engine::App App(std::move(Specification));
    
    App.Run();
}
