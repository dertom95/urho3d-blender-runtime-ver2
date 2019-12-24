#include "thegame.h"

#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Engine/EngineDefs.h>

URHO3D_DEFINE_APPLICATION_MAIN(TheGameMain)

TheGameMain::TheGameMain(Context* ctx) : Application(ctx)
{
    setbuf(stdout, NULL);
    SubscribeToEvents();
}

void TheGameMain::Setup()
{
    // register game
    game_ = new GameLogic(context_);
    context_->RegisterSubsystem(game_);
    // setup game
    game_->Setup(engineParameters_);
}

void TheGameMain::Start()
{
    game_->LoadScene("Scene.xml");
}

void TheGameMain::Stop()
{

}

void TheGameMain::SubscribeToEvents()
{
    // Called after engine initialization. Setup application & subscribe to events here
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(TheGameMain, HandleKeyDown));
}

void TheGameMain::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
    int key = eventData[P_KEY].GetInt();
    if (key == KEY_ESCAPE)
        engine_->Exit();
}
