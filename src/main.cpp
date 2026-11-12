#include <iostream>
#include <iomanip>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_gfxPrimitives_font.h>

#define STICK_MAX_VAL (float(0x7fff))

static const float placement_factor = 6;
static const float size_factor = 10;

std::string cStringToStdString(const char* str, const std::string& or_="N/A")
{
    return str ? str : or_;
}

SDL_GameController* openControllerAndPrintInfo(int index)
{
    SDL_GameController* gcont = SDL_GameControllerOpen(index);
    std::cout << "\tName: " << cStringToStdString(SDL_GameControllerName(gcont)) << '\n';
    std::cout << "\tPath: " << cStringToStdString(SDL_GameControllerPath(gcont)) << '\n';
    std::cout << "\tVID:PID: " << std::hex << std::setfill('0')
        << std::setw(4) << SDL_GameControllerGetVendor(gcont) << ':'
        << std::setw(4) << SDL_GameControllerGetProduct(gcont) << std::dec << '\n';
    std::cout << "\tRumble support? " << (SDL_GameControllerHasRumble(gcont) ? "YES" : "NO") << '\n';
    return gcont;
}

struct StickState
{
    float x{};
    float y{};
};

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_DisplayMode current_mode;
    if (SDL_GetCurrentDisplayMode(0, &current_mode) == 0) {
        printf("Width: %d, Height: %d\n", current_mode.w, current_mode.h);
    } else {
        printf("Error getting display mode: %s\n", SDL_GetError());
    }
    SDL_Window* window = SDL_CreateWindow("Gamepad Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, current_mode.w, current_mode.h, 0);
    SDL_Renderer* rend = SDL_CreateRenderer(window, 0, 0);
    gfxPrimitivesSetFont(gfxPrimitivesFontdata, 8, 8);

    SDL_GameControllerAddMappingsFromFile("/storage/.config/SDL-GameControllerDB/gamecontrollerdb.txt");

    SDL_GameController* gcont = nullptr;
    StickState leftStickState{};
    StickState rightStickState{};
    bool buttonStates[SDL_CONTROLLER_BUTTON_MAX]{};
    bool f2State = false;
    bool f3State = false;
    uint8_t dpadState = 0;
    float lTriggerState = 0.0f;
    float rTriggerState = 0.0f;

    // Calculate some sizes and offsets
    int offsetX  = current_mode.w / placement_factor;
    int offsetY  = current_mode.h / placement_factor;
    int stickRadiusRange = std::min(current_mode.w, current_mode.h) / size_factor;
    int stickRadius = stickRadiusRange/2;
    int extraGap = offsetX/2 - 40;


    bool running = true;
    while (true)
    {
        SDL_Event event;
        //SDL_PumpEvents();
        while (running && SDL_PollEvent(&event))
        //while (running && SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_CONTROLLERDEVICEADDED:
                std::cout << "Controller added\n";
                gcont = openControllerAndPrintInfo(event.cdevice.which);
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                std::cout << "Controller removed\n";
                gcont = nullptr;
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                std::cout << "Controller button down: " << +event.cbutton.button  << '\n';
                //SDL_GameControllerRumble(gcont, 0, 0xffff/2, 200);
                buttonStates[event.cbutton.button] = true;
                break;

            case SDL_CONTROLLERBUTTONUP:
                std::cout << "Controller button up: " << +event.cbutton.button  << '\n';
                buttonStates[event.cbutton.button] = false;
                break;

            case SDL_CONTROLLERAXISMOTION:
                std::cout << "Controller axis movement\n";
                std::cout << "\tAxis: " << +event.caxis.axis << '\n';
                std::cout << "\tValue: " << event.caxis.value/STICK_MAX_VAL << '\n';
                switch (event.caxis.axis)
                {
                case SDL_CONTROLLER_AXIS_LEFTX:
                    leftStickState.x = event.caxis.value/STICK_MAX_VAL;
                    break;
                case SDL_CONTROLLER_AXIS_LEFTY:
                    leftStickState.y = event.caxis.value/STICK_MAX_VAL;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTX:
                    rightStickState.x = event.caxis.value/STICK_MAX_VAL;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTY:
                    rightStickState.y = event.caxis.value/STICK_MAX_VAL;
                    break;
                case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                    lTriggerState = event.caxis.value/STICK_MAX_VAL;
                    break;
                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                    rTriggerState = event.caxis.value/STICK_MAX_VAL;
                    break;
                }
                break;

            case SDL_JOYHATMOTION:
                std::cout << "Joyhat motion\n";
                std::cout << +event.jhat.value << '\n';
                dpadState = event.jhat.value;
                break;

            case SDL_KEYDOWN:
                if (!event.key.repeat)
                {
                    if (event.key.keysym.sym == SDLK_F1)
                    {
                        std::cout << "F1 down (Misc1)\n";
                        buttonStates[SDL_CONTROLLER_BUTTON_MISC1] = true;
                    }
                    else if (event.key.keysym.sym == SDLK_F2)
                    {
                        std::cout << "F2 down (Extra left)\n";
                        f2State = true;
                    }
                    else if (event.key.keysym.sym == SDLK_F3)
                    {
                        std::cout << "F3 down (Extra right)\n";
                        f3State = true;
                    }
                }
                break;

            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_F1)
                {
                    std::cout << "F1 up (Misc1)\n";
                    buttonStates[SDL_CONTROLLER_BUTTON_MISC1] = false;
                }
                else if (event.key.keysym.sym == SDLK_F2)
                {
                    std::cout << "F2 up (Extra left)\n";
                    f2State = false;
                }
                else if (event.key.keysym.sym == SDLK_F3)
                {
                    std::cout << "F3 up (Extra right)\n";
                    f3State = false;
                }
                break;

            default:
                std::cout << "Event: 0x" << std::hex << event.type << std::dec << std::endl;
                break;
            }
        }
        if (!running) break;


        SDL_SetRenderDrawColor(rend, 40, 40, 40, 255);
        SDL_RenderClear(rend);

        if (gcont)
        {
            stringColor(rend, 10, 10, cStringToStdString(SDL_GameControllerName(gcont), "Unknown").c_str(), 0xffffffff);

            // Left analog stick
            aacircleColor(rend, offsetX, offsetY*3, stickRadiusRange, 0xffffffff);
            aacircleColor(rend, offsetX+leftStickState.x*stickRadius, offsetY*3+leftStickState.y*stickRadius, stickRadius, 0xffffffff);
            filledCircleColor(rend, offsetX+leftStickState.x*stickRadius, offsetY*3+leftStickState.y*stickRadius, stickRadius,
                    (buttonStates[SDL_CONTROLLER_BUTTON_LEFTSTICK] ? 0xffffffff : 0));

            // Right analog stick
            aacircleColor(rend, current_mode.w-offsetX*2, current_mode.h-offsetY, stickRadiusRange, 0xffffffff);
            aacircleColor(rend, current_mode.w-offsetX*2+rightStickState.x*stickRadius, current_mode.h-offsetY+rightStickState.y*stickRadius, stickRadius, 0xffffffff);
            filledCircleColor(rend, current_mode.w-offsetX*2+rightStickState.x*stickRadius, current_mode.h-offsetY+rightStickState.y*stickRadius, stickRadius,
                    (buttonStates[SDL_CONTROLLER_BUTTON_RIGHTSTICK] ? 0xffffffff : 0));

            // Y button
            aacircleRGBA(rend, current_mode.w-offsetX, offsetY*3-40, 20, 255, 255, 0, 255);
            if (buttonStates[SDL_CONTROLLER_BUTTON_Y])
                filledCircleRGBA(rend, current_mode.w-offsetX, offsetY*3-40, 20, 255, 255, 0, 255);

            // X button
            aacircleRGBA(rend, current_mode.w-offsetX-40, offsetY*3+40-40, 20, 100, 100, 255, 255);
            if (buttonStates[SDL_CONTROLLER_BUTTON_X])
                filledCircleRGBA(rend, current_mode.w-offsetX-40, offsetY*3+40-40, 20, 100, 100, 255, 255);

            // B button
            aacircleRGBA(rend, current_mode.w-offsetX+40, offsetY*3+40-40, 20, 255, 0, 0, 255);
            if (buttonStates[SDL_CONTROLLER_BUTTON_B])
                filledCircleRGBA(rend, current_mode.w-offsetX+40, offsetY*3+40-40, 20, 255, 0, 0, 255);

            // A button
            aacircleRGBA(rend, current_mode.w-offsetX, offsetY*3+80-40, 20, 0, 255, 0, 255);
            if (buttonStates[SDL_CONTROLLER_BUTTON_A])
                filledCircleRGBA(rend, current_mode.w-offsetX, offsetY*3+80-40, 20, 0, 255, 0, 255);

            // D-pad up
            aatrigonRGBA(rend, offsetX*2, current_mode.h-offsetY-60, offsetX*2-20, current_mode.h-offsetY-20, offsetX*2+20, current_mode.h-offsetY-20, 255, 255, 255, 255);
            if ((dpadState & SDL_HAT_UP) || buttonStates[SDL_CONTROLLER_BUTTON_DPAD_UP])
                filledTrigonRGBA(rend, offsetX*2, current_mode.h-offsetY-60, offsetX*2-20, current_mode.h-offsetY-20, offsetX*2+20, current_mode.h-offsetY-20, 255, 255, 255, 255);

            // D-pad down
            aatrigonRGBA(rend, offsetX*2, current_mode.h-offsetY + 60, offsetX*2-20, current_mode.h-offsetY+20, offsetX*2+20, current_mode.h-offsetY+20, 255, 255, 255, 255);
            if ((dpadState & SDL_HAT_DOWN) || buttonStates[SDL_CONTROLLER_BUTTON_DPAD_DOWN])
                filledTrigonRGBA(rend, offsetX*2, current_mode.h-offsetY + 60, offsetX*2-20, current_mode.h-offsetY+20, offsetX*2+20, current_mode.h-offsetY+20, 255, 255, 255, 255);

            // D-pad left
            aatrigonRGBA(rend, offsetX*2-60, current_mode.h-offsetY, offsetX*2-20, current_mode.h-offsetY-20, offsetX*2-20, current_mode.h-offsetY+20, 255, 255, 255, 255);
            if ((dpadState & SDL_HAT_LEFT) || buttonStates[SDL_CONTROLLER_BUTTON_DPAD_LEFT])
                filledTrigonRGBA(rend, offsetX*2-60, current_mode.h-offsetY, offsetX*2-20, current_mode.h-offsetY-20, offsetX*2-20, current_mode.h-offsetY+20, 255, 255, 255, 255);

            // D-pad right
            aatrigonRGBA(rend, offsetX*2+60, current_mode.h-offsetY, offsetX*2+20, current_mode.h-offsetY-20, offsetX*2+20, current_mode.h-offsetY+20, 255, 255, 255, 255);
            if ((dpadState & SDL_HAT_RIGHT) || buttonStates[SDL_CONTROLLER_BUTTON_DPAD_RIGHT])
                filledTrigonRGBA(rend, offsetX*2+60, current_mode.h-offsetY, offsetX*2+20, current_mode.h-offsetY-20, offsetX*2+20, current_mode.h-offsetY+20, 255, 255, 255, 255);

            // Left bumper
            rectangleColor(rend, offsetX, offsetY-20, offsetX+100, offsetY+20, 0xffffffff);
            if (buttonStates[SDL_CONTROLLER_BUTTON_LEFTSHOULDER])
                boxColor(rend, offsetX, offsetY-20, offsetX+100, offsetY+20, 0xffffffff);

            // Right bumper
            rectangleColor(rend, current_mode.w-offsetX, offsetY-20, current_mode.w-offsetX-100, offsetY+20, 0xffffffff);
            if (buttonStates[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER])
                boxColor(rend, current_mode.w-offsetX, offsetY-20, current_mode.w-offsetX-100, offsetY+20, 0xffffffff);

            // M1 Button
            rectangleColor(rend, offsetX+100+extraGap, offsetY-20, offsetX+100+extraGap+50, offsetY+20, 0xffffffff);
            if (f2State)
                boxColor(rend, offsetX+100+extraGap, offsetY-20, offsetX+100+extraGap+50, offsetY+20, 0xffffffff);

            // M2 Button
            rectangleColor(rend, current_mode.w-offsetX-100-extraGap, offsetY-20, current_mode.w-offsetX-100-extraGap-50, offsetY+20, 0xffffffff);
            if (f3State)
              boxColor(rend, current_mode.w-offsetX-100-extraGap, offsetY-20, current_mode.w-offsetX-100-extraGap-50, offsetY+20, 0xffffffff);

            // Left trigger
            rectangleColor(rend, offsetX/2, offsetY-20, offsetX/2+40, offsetY+20+80, 0xffffffff);
            boxColor(rend, offsetX/2, offsetY-20, offsetX/2+40, offsetY-20+120*lTriggerState, 0xffffffff);

            // Right trigger
            rectangleColor(rend, current_mode.w-offsetX/2, offsetY-20, current_mode.w-offsetX/2-40, offsetY+20+80, 0xffffffff);
            boxColor(rend, current_mode.w-offsetX/2, offsetY-20, current_mode.w-offsetX/2- 40, offsetY-20+120*rTriggerState, 0xffffffff);

            // Back button
            aacircleColor(rend, current_mode.w/2-40, current_mode.h/2, 20, 0xffffffff);
            if (buttonStates[SDL_CONTROLLER_BUTTON_BACK])
                filledCircleColor(rend, current_mode.w/2-40, current_mode.h/2, 20, 0xffffffff);

            // Start button
            aacircleColor(rend, current_mode.w/2+40, current_mode.h/2, 20, 0xffffffff);
            if (buttonStates[SDL_CONTROLLER_BUTTON_START])
                filledCircleColor(rend, current_mode.w/2+40, current_mode.h/2, 20, 0xffffffff);

            // Misc1 button (Share)
            aacircleColor(rend, current_mode.w/2, current_mode.h/2 + 50, 18, 0xffffffff);
            if (buttonStates[SDL_CONTROLLER_BUTTON_MISC1])
                filledCircleColor(rend, current_mode.w/2, current_mode.h/2 + 50, 18, 0xffffffff);

            // Guide button
            aacircleColor(rend, current_mode.w/2, current_mode.h/2-60, 30, 0xffffffff);
            if (buttonStates[SDL_CONTROLLER_BUTTON_GUIDE])
                filledCircleColor(rend, current_mode.w/2, current_mode.h/2-60, 30, 0xffffffff);
        }
        else
        {
            stringColor(rend, current_mode.w/2-22*8/2, current_mode.h/2-8/2, "NO CONTROLLER DETECTED", 0xffffffff);
        }

        SDL_RenderPresent(rend);
        SDL_Delay(16);
    }

    SDL_GameControllerClose(gcont);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
