#include "SDL2Window.h"

#include <stdexcept>

#include <zvulkan/vulkansurface.h>
#include <zvulkan/vulkancompatibledevice.h>
#include <zvulkan/vulkanbuilders.h>

#include "RenderDevice/RenderDevice.h"

std::map<int, SDL2Window*> SDL2Window::windows;

SDL2Window::SDL2Window(DisplayWindowHost *windowHost) : windowHost(windowHost)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        SDLWindowError("Unable to initialize SDL: " + std::string(SDL_GetError()));
    }
    // Width and height won't matter much as the window will be resized based on the values in [GameExecutableName].ini anyways
    m_SDLWindow = SDL_CreateWindow("Surreal Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_VULKAN);
    if (!m_SDLWindow) {
        SDLWindowError("Unable to create SDL Window: " + std::string(SDL_GetError()));
    }

    // Generate a required extensions list
    unsigned int extCount;
    SDL_Vulkan_GetInstanceExtensions(m_SDLWindow, &extCount, nullptr);
    const char** extNames = new const char*[extCount];
    SDL_Vulkan_GetInstanceExtensions(m_SDLWindow, &extCount, extNames);

    // Create the instance
    auto instanceBuilder = VulkanInstanceBuilder();
    for (int i = 0 ; i < extCount ; i++)
    {
        instanceBuilder.RequireExtension(std::string(extNames[i]));
    }
    instanceBuilder.OptionalExtension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)
                   .DebugLayer(false);
    auto instance = instanceBuilder.Create();
    delete[] extNames;

    VkSurfaceKHR surfaceHandle;
    SDL_Vulkan_CreateSurface(m_SDLWindow, instance->Instance, &surfaceHandle);

    auto surface = std::make_shared<VulkanSurface>(instance, surfaceHandle);

    rendDevice = RenderDevice::Create(this, surface);

    windows[SDL_GetWindowID(m_SDLWindow)] = this;
}

SDL2Window::~SDL2Window()
{
    rendDevice.reset();
    if (m_SDLWindow) {
        windows.erase(windows.find(SDL_GetWindowID(m_SDLWindow)));
        SDL_DestroyWindow(m_SDLWindow);
    }
    if (windows.empty())
        SDL_Quit();
}

void SDL2Window::ProcessEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        int windowID;
        switch (event.type) {
            case SDL_WINDOWEVENT:
                windowID = event.window.windowID; break;
            case SDL_TEXTINPUT:
                windowID = event.text.windowID; break;
            case SDL_KEYUP:
            case SDL_KEYDOWN:
                windowID = event.key.windowID; break;
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN:
                windowID = event.button.windowID; break;
            case SDL_MOUSEWHEEL:
                windowID = event.wheel.windowID; break;
            case SDL_MOUSEMOTION:
                windowID = event.motion.windowID; break;

            default: continue;
        }
        auto it = windows.find(windowID);
        if (it != windows.end()) {
            it->second->OnSDLEvent(event);
        }
    }
}

void SDL2Window::RunLoop()
{
}

void SDL2Window::ExitLoop()
{
}

void SDL2Window::SDLWindowError(const std::string&& message) const
{
    throw std::runtime_error(message.c_str());
}

void SDL2Window::OnSDLEvent(SDL_Event& event)
{
    switch (event.type) {
    case SDL_QUIT:
        windowHost->OnWindowClose();
        break;

    case SDL_WINDOWEVENT:
        // ???
        windowHost->OnWindowGeometryChanged();
        break;

    case SDL_TEXTINPUT:
        OnKeyboardTextInput(event.text);
        break;
    case SDL_KEYUP:
    case SDL_KEYDOWN:
        OnKeyboardInput(event.key);
        break;

    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
        OnMouseInput(event.button);
        break;

    case SDL_MOUSEWHEEL:
        OnMouseWheel(event.wheel);
        break;

    case SDL_MOUSEMOTION:
        OnMouseMove(event.motion);
        break;
    }
}

void SDL2Window::SetWindowTitle(const std::string& text)
{
    SDL_SetWindowTitle(m_SDLWindow, text.c_str());
}

void SDL2Window::SetWindowFrame(const Rect& box)
{
    if (isFullscreen)
    {
        // Fullscreen windows are handled with SDL_DisplayMode
        SDL_DisplayMode mode;
        SDL_GetWindowDisplayMode(m_SDLWindow, &mode);
        mode.w = box.width;
        mode.h = box.height;
        mode.refresh_rate = 0;
        int set_result = SDL_SetWindowDisplayMode(m_SDLWindow, &mode);
        if (set_result < 0)
            SDLWindowError("Error on SDL_SetWindowDisplayMode(): " + std::string(SDL_GetError()));
    }
    else
    {
        SDL_SetWindowPosition(m_SDLWindow, box.x, box.y);
        SDL_SetWindowSize(m_SDLWindow, box.width, box.height);
    }
}

void SDL2Window::SetClientFrame(const Rect& box)
{
    SetWindowFrame(box);
}

void SDL2Window::Show()
{
    SDL_ShowWindow(m_SDLWindow);
}

void SDL2Window::ShowFullscreen()
{
    SDL_SetWindowFullscreen(m_SDLWindow, SDL_WINDOW_FULLSCREEN);
    isFullscreen = true;
}

void SDL2Window::ShowMaximized()
{
    SDL_MaximizeWindow(m_SDLWindow);
}

void SDL2Window::ShowMinimized()
{
    SDL_MinimizeWindow(m_SDLWindow);
}

void SDL2Window::ShowNormal()
{
    // Clear the Fullscreen flag
    SDL_SetWindowFullscreen(m_SDLWindow, 0);
    isFullscreen = false;
}

void SDL2Window::Hide()
{
    SDL_HideWindow(m_SDLWindow);
}

void SDL2Window::Activate()
{
}

void SDL2Window::ShowCursor(bool enable)
{
    SDL_ShowCursor(enable);
}

void SDL2Window::LockCursor()
{
    SDL_SetWindowGrab(m_SDLWindow, SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void SDL2Window::UnlockCursor()
{
    SDL_SetWindowGrab(m_SDLWindow, SDL_FALSE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
}

void SDL2Window::Update()
{
}

bool SDL2Window::GetKeyState(EInputKey key)
{
    if (key == IK_None)
        return false;

    int numkeys;
    auto keys = SDL_GetKeyboardState(&numkeys);

    if (numkeys <= 0)
        return false;

    auto sdl_key = InputKeyToSDLScancode(key);
    return keys[sdl_key];
}

void SDL2Window::OnKeyboardInput(SDL_KeyboardEvent& event)
{
    if (event.state == SDL_PRESSED)
        windowHost->OnWindowKeyDown(SDLScancodeToInputKey(event.keysym.scancode));
    else if (event.state == SDL_RELEASED)
        windowHost->OnWindowKeyUp(SDLScancodeToInputKey(event.keysym.scancode));
}

void SDL2Window::OnKeyboardTextInput(SDL_TextInputEvent& event)
{
    windowHost->OnWindowKeyChar(std::string(event.text));
}

void SDL2Window::OnMouseInput(SDL_MouseButtonEvent& event)
{
    EInputKey id = IK_None;

    switch (event.button) {
        case SDL_BUTTON_LEFT:       id = IK_LeftMouse; break;
        case SDL_BUTTON_MIDDLE:     id = IK_MiddleMouse; break;
        case SDL_BUTTON_RIGHT:      id = IK_RightMouse; break;
        // case SDL_BUTTON_X1:         id = IK_XButton1; break;
        // case SDL_BUTTON_X2:         id = IK_XButton2; break;
        default:                    return;
    }

    Point mousePos(event.x, event.y);

    if (event.state == SDL_PRESSED)
    {
        windowHost->OnWindowMouseDown(mousePos, id);
    }
    else if (event.state == SDL_RELEASED) {
        windowHost->OnWindowMouseUp(mousePos, id);
        if (event.clicks >= 2)
            windowHost->OnWindowMouseDoubleclick(mousePos, id);
    }
}

void SDL2Window::OnMouseWheel(SDL_MouseWheelEvent& event)
{
    EInputKey id = IK_None;

    if (event.y > 0) // Scroll up
        id = IK_MouseWheelUp;
    else if (event.y < 0) // scroll down
        id = IK_MouseWheelDown;

    Point mousePos(event.x, event.y);
    windowHost->OnWindowMouseWheel(mousePos, id);
}

void SDL2Window::OnMouseMove(SDL_MouseMotionEvent& event)
{
    if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
        windowHost->OnWindowRawMouseMove(event.xrel, event.yrel);
        int w, h;
        SDL_GetWindowSize(m_SDLWindow, &w, &h);
        SDL_WarpMouseInWindow(m_SDLWindow, w / 2, h / 2);
    }
    else {
        windowHost->OnWindowMouseMove(Point(event.x, event.y));
    }
}

Rect SDL2Window::GetWindowFrame() const
{
    int x, y, w, h;
    SDL_GetWindowPosition(m_SDLWindow, &x, &y);
    SDL_GetWindowSize(m_SDLWindow, &w, &h);

    return Rect::xywh(x, y, w, h); // DPI-Aware?
}

Size SDL2Window::GetClientSize() const
{
    int width, height;
    SDL_Vulkan_GetDrawableSize(m_SDLWindow, &width, &height);

    return Size((double)width, (double)height);
}

int SDL2Window::GetPixelWidth() const
{
    int width;
    SDL_GetWindowSizeInPixels(m_SDLWindow, &width, nullptr);

    return width;
}

int SDL2Window::GetPixelHeight() const
{
    int height;
    SDL_GetWindowSizeInPixels(m_SDLWindow, nullptr, &height);

    return height;
}

double SDL2Window::GetDpiScale() const
{
    /*
     * From the SDL2 documentation:
     * It is almost always better to use SDL_GetWindowSize() to find the window size, which might be in logical points instead of pixels,
     * and then SDL_GL_GetDrawableSize(), SDL_Vulkan_GetDrawableSize(), SDL_Metal_GetDrawableSize(), or SDL_GetRendererOutputSize(),
     * and compare the two values to get an actual scaling value between the two.
     */
    int drawable_width, window_width;
    SDL_GetWindowSize(m_SDLWindow, &window_width, nullptr);
    SDL_Vulkan_GetDrawableSize(m_SDLWindow, &drawable_width, nullptr);

    return (double) drawable_width / (double) window_width;
}

std::vector<Size> SDL2Window::QueryAvailableResolutions() const
{
    std::vector<Size> result{};

    // First, obtain the display the window is on
    int displayIndex = SDL_GetWindowDisplayIndex(m_SDLWindow);
    if (displayIndex < 0)
        SDLWindowError("Error on SDL_GetWindowDisplayIndex(): " + std::string(SDL_GetError()));

    // Then obtain the amount of available resolutions
    int numDisplayModes = SDL_GetNumDisplayModes(displayIndex);
    if (numDisplayModes < 0)
        SDLWindowError("Error on SDL_GetNumDisplayModes(): " + std::string(SDL_GetError()));

    for (int i = 0; i < numDisplayModes; i++)
    {
        SDL_DisplayMode displayMode;

        SDL_GetDisplayMode(displayIndex, i, &displayMode);

        // The data also includes refresh rate but we ignore it (for now?)
        Size resolution(displayMode.w, displayMode.h);

        // Skip over the current resolution if it is already inserted
        // (in case of multiple refresh rates being available for the display)
        bool resolutionAlreadyAdded = false;
        for (auto res : availableResolutions)
        {
            if (resolution == res)
            {
                resolutionAlreadyAdded = true;
                break;
            }
        }
        if (resolutionAlreadyAdded)
            continue;

        // Add the resolution, as it is not added before
        result.push_back(resolution);
    }

    return result;
}

std::string SDL2Window::GetAvailableResolutions() const
{
    std::string result = "";

    auto resolutions = QueryAvailableResolutions();

    // "Flatten" the resolutions list into a single string
    for (int i = 0; i < resolutions.size(); i++)
    {
        auto& res = resolutions[i];
        std::string resString = std::to_string(int(res.width)) + "x" + std::to_string(int(res.height));

        result += resString;
        if (i < resolutions.size() - 1)
            result += " ";
    }

    return result;
}

EInputKey SDL2Window::SDLScancodeToInputKey(SDL_Scancode keycode)
{
    switch (keycode) {
        case SDL_SCANCODE_BACKSPACE: return IK_Backspace;
        case SDL_SCANCODE_TAB: return IK_Tab;
        case SDL_SCANCODE_CLEAR: return IK_OEMClear;
        case SDL_SCANCODE_RETURN: return IK_Enter;
        case SDL_SCANCODE_MENU: return IK_Alt;
        case SDL_SCANCODE_PAUSE: return IK_Pause;
        case SDL_SCANCODE_ESCAPE: return IK_Escape;
        case SDL_SCANCODE_SPACE: return IK_Space;
        case SDL_SCANCODE_END: return IK_End;
        case SDL_SCANCODE_HOME: return IK_Home;
        case SDL_SCANCODE_LEFT: return IK_Left;
        case SDL_SCANCODE_UP: return IK_Up;
        case SDL_SCANCODE_RIGHT: return IK_Right;
        case SDL_SCANCODE_DOWN: return IK_Down;
        case SDL_SCANCODE_SELECT: return IK_Select;
        case SDL_SCANCODE_PRINTSCREEN: return IK_Print;
        case SDL_SCANCODE_EXECUTE: return IK_Execute;
        case SDL_SCANCODE_INSERT: return IK_Insert;
        case SDL_SCANCODE_DELETE: return IK_Delete;
        case SDL_SCANCODE_HELP: return IK_Help;
        case SDL_SCANCODE_0: return IK_0;
        case SDL_SCANCODE_1: return IK_1;
        case SDL_SCANCODE_2: return IK_2;
        case SDL_SCANCODE_3: return IK_3;
        case SDL_SCANCODE_4: return IK_4;
        case SDL_SCANCODE_5: return IK_5;
        case SDL_SCANCODE_6: return IK_6;
        case SDL_SCANCODE_7: return IK_7;
        case SDL_SCANCODE_8: return IK_8;
        case SDL_SCANCODE_9: return IK_9;
        case SDL_SCANCODE_A: return IK_A;
        case SDL_SCANCODE_B: return IK_B;
        case SDL_SCANCODE_C: return IK_C;
        case SDL_SCANCODE_D: return IK_D;
        case SDL_SCANCODE_E: return IK_E;
        case SDL_SCANCODE_F: return IK_F;
        case SDL_SCANCODE_G: return IK_G;
        case SDL_SCANCODE_H: return IK_H;
        case SDL_SCANCODE_I: return IK_I;
        case SDL_SCANCODE_J: return IK_J;
        case SDL_SCANCODE_K: return IK_K;
        case SDL_SCANCODE_L: return IK_L;
        case SDL_SCANCODE_M: return IK_M;
        case SDL_SCANCODE_N: return IK_N;
        case SDL_SCANCODE_O: return IK_O;
        case SDL_SCANCODE_P: return IK_P;
        case SDL_SCANCODE_Q: return IK_Q;
        case SDL_SCANCODE_R: return IK_R;
        case SDL_SCANCODE_S: return IK_S;
        case SDL_SCANCODE_T: return IK_T;
        case SDL_SCANCODE_U: return IK_U;
        case SDL_SCANCODE_V: return IK_V;
        case SDL_SCANCODE_W: return IK_W;
        case SDL_SCANCODE_X: return IK_X;
        case SDL_SCANCODE_Y: return IK_Y;
        case SDL_SCANCODE_Z: return IK_Z;
        case SDL_SCANCODE_KP_0: return IK_NumPad0;
        case SDL_SCANCODE_KP_1: return IK_NumPad1;
        case SDL_SCANCODE_KP_2: return IK_NumPad2;
        case SDL_SCANCODE_KP_3: return IK_NumPad3;
        case SDL_SCANCODE_KP_4: return IK_NumPad4;
        case SDL_SCANCODE_KP_5: return IK_NumPad5;
        case SDL_SCANCODE_KP_6: return IK_NumPad6;
        case SDL_SCANCODE_KP_7: return IK_NumPad7;
        case SDL_SCANCODE_KP_8: return IK_NumPad8;
        case SDL_SCANCODE_KP_9: return IK_NumPad9;
        // case SDL_SCANCODE_KP_ENTER: return IK_NumPadEnter;
        // case SDL_SCANCODE_KP_MULTIPLY: return IK_Multiply;
        // case SDL_SCANCODE_KP_PLUS: return IK_Add;
        case SDL_SCANCODE_SEPARATOR: return IK_Separator;
        // case SDL_SCANCODE_KP_MINUS: return IK_Subtract;
        case SDL_SCANCODE_KP_PERIOD: return IK_NumPadPeriod;
        // case SDL_SCANCODE_KP_DIVIDE: return IK_Divide;
        case SDL_SCANCODE_F1: return IK_F1;
        case SDL_SCANCODE_F2: return IK_F2;
        case SDL_SCANCODE_F3: return IK_F3;
        case SDL_SCANCODE_F4: return IK_F4;
        case SDL_SCANCODE_F5: return IK_F5;
        case SDL_SCANCODE_F6: return IK_F6;
        case SDL_SCANCODE_F7: return IK_F7;
        case SDL_SCANCODE_F8: return IK_F8;
        case SDL_SCANCODE_F9: return IK_F9;
        case SDL_SCANCODE_F10: return IK_F10;
        case SDL_SCANCODE_F11: return IK_F11;
        case SDL_SCANCODE_F12: return IK_F12;
        case SDL_SCANCODE_F13: return IK_F13;
        case SDL_SCANCODE_F14: return IK_F14;
        case SDL_SCANCODE_F15: return IK_F15;
        case SDL_SCANCODE_F16: return IK_F16;
        case SDL_SCANCODE_F17: return IK_F17;
        case SDL_SCANCODE_F18: return IK_F18;
        case SDL_SCANCODE_F19: return IK_F19;
        case SDL_SCANCODE_F20: return IK_F20;
        case SDL_SCANCODE_F21: return IK_F21;
        case SDL_SCANCODE_F22: return IK_F22;
        case SDL_SCANCODE_F23: return IK_F23;
        case SDL_SCANCODE_F24: return IK_F24;
        case SDL_SCANCODE_NUMLOCKCLEAR: return IK_NumLock;
        case SDL_SCANCODE_SCROLLLOCK: return IK_ScrollLock;
        case SDL_SCANCODE_LSHIFT: return IK_LShift;
        case SDL_SCANCODE_RSHIFT: return IK_RShift;
        case SDL_SCANCODE_LCTRL: return IK_LControl;
        case SDL_SCANCODE_RCTRL: return IK_RControl;
        case SDL_SCANCODE_GRAVE: return IK_Tilde;
        default: return IK_None;
    }
}

SDL_Scancode SDL2Window::InputKeyToSDLScancode(EInputKey inputkey)
{
    switch (inputkey) {
        case IK_Backspace: return SDL_SCANCODE_BACKSPACE;
        case IK_Tab: return SDL_SCANCODE_TAB;
        case IK_OEMClear: return SDL_SCANCODE_CLEAR;
        case IK_Enter: return SDL_SCANCODE_RETURN;
        case IK_Alt: return SDL_SCANCODE_MENU;
        case IK_Pause: return SDL_SCANCODE_PAUSE;
        case IK_Escape: return SDL_SCANCODE_ESCAPE;
        case IK_Space: return SDL_SCANCODE_SPACE;
        case IK_End: return SDL_SCANCODE_END;
        case IK_Home: return SDL_SCANCODE_HOME;
        case IK_Left: return SDL_SCANCODE_LEFT;
        case IK_Up: return SDL_SCANCODE_UP;
        case IK_Right: return SDL_SCANCODE_RIGHT;
        case IK_Down: return SDL_SCANCODE_DOWN;
        case IK_Select: return SDL_SCANCODE_SELECT;
        case IK_Print: return SDL_SCANCODE_PRINTSCREEN;
        case IK_Execute: return SDL_SCANCODE_EXECUTE;
        case IK_Insert: return SDL_SCANCODE_INSERT;
        case IK_Delete: return SDL_SCANCODE_DELETE;
        case IK_Help: return SDL_SCANCODE_HELP;
        case IK_0: return SDL_SCANCODE_0;
        case IK_1: return SDL_SCANCODE_1;
        case IK_2: return SDL_SCANCODE_2;
        case IK_3: return SDL_SCANCODE_3;
        case IK_4: return SDL_SCANCODE_4;
        case IK_5: return SDL_SCANCODE_5;
        case IK_6: return SDL_SCANCODE_6;
        case IK_7: return SDL_SCANCODE_7;
        case IK_8: return SDL_SCANCODE_8;
        case IK_9: return SDL_SCANCODE_9;
        case IK_A: return SDL_SCANCODE_A;
        case IK_B: return SDL_SCANCODE_B;
        case IK_C: return SDL_SCANCODE_C;
        case IK_D: return SDL_SCANCODE_D;
        case IK_E: return SDL_SCANCODE_E;
        case IK_F: return SDL_SCANCODE_F;
        case IK_G: return SDL_SCANCODE_G;
        case IK_H: return SDL_SCANCODE_H;
        case IK_I: return SDL_SCANCODE_I;
        case IK_J: return SDL_SCANCODE_J;
        case IK_K: return SDL_SCANCODE_K;
        case IK_L: return SDL_SCANCODE_L;
        case IK_M: return SDL_SCANCODE_M;
        case IK_N: return SDL_SCANCODE_N;
        case IK_O: return SDL_SCANCODE_O;
        case IK_P: return SDL_SCANCODE_P;
        case IK_Q: return SDL_SCANCODE_Q;
        case IK_R: return SDL_SCANCODE_R;
        case IK_S: return SDL_SCANCODE_S;
        case IK_T: return SDL_SCANCODE_T;
        case IK_U: return SDL_SCANCODE_U;
        case IK_V: return SDL_SCANCODE_V;
        case IK_W: return SDL_SCANCODE_W;
        case IK_X: return SDL_SCANCODE_X;
        case IK_Y: return SDL_SCANCODE_Y;
        case IK_Z: return SDL_SCANCODE_Z;
        case IK_NumPad0: return SDL_SCANCODE_KP_0;
        case IK_NumPad1: return SDL_SCANCODE_KP_1;
        case IK_NumPad2: return SDL_SCANCODE_KP_2;
        case IK_NumPad3: return SDL_SCANCODE_KP_3;
        case IK_NumPad4: return SDL_SCANCODE_KP_4;
        case IK_NumPad5: return SDL_SCANCODE_KP_5;
        case IK_NumPad6: return SDL_SCANCODE_KP_6;
        case IK_NumPad7: return SDL_SCANCODE_KP_7;
        case IK_NumPad8: return SDL_SCANCODE_KP_8;
        case IK_NumPad9: return SDL_SCANCODE_KP_9;
        // case IK_NumPadEnter: return SDL_SCANCODE_KP_ENTER;
        // case IK_Multiply return SDL_SCANCODE_KP_MULTIPLY:;
        // case IK_Add: return SDL_SCANCODE_KP_PLUS;
        case IK_Separator: return SDL_SCANCODE_SEPARATOR;
        // case IK_Subtract: return SDL_SCANCODE_KP_MINUS;
        case IK_NumPadPeriod: return SDL_SCANCODE_KP_PERIOD;
        // case IK_Divide: return SDL_SCANCODE_KP_DIVIDE;
        case IK_F1: return SDL_SCANCODE_F1;
        case IK_F2: return SDL_SCANCODE_F2;
        case IK_F3: return SDL_SCANCODE_F3;
        case IK_F4: return SDL_SCANCODE_F4;
        case IK_F5: return SDL_SCANCODE_F5;
        case IK_F6: return SDL_SCANCODE_F6;
        case IK_F7: return SDL_SCANCODE_F7;
        case IK_F8: return SDL_SCANCODE_F8;
        case IK_F9: return SDL_SCANCODE_F9;
        case IK_F10: return SDL_SCANCODE_F10;
        case IK_F11: return SDL_SCANCODE_F11;
        case IK_F12: return SDL_SCANCODE_F12;
        case IK_F13: return SDL_SCANCODE_F13;
        case IK_F14: return SDL_SCANCODE_F14;
        case IK_F15: return SDL_SCANCODE_F15;
        case IK_F16: return SDL_SCANCODE_F16;
        case IK_F17: return SDL_SCANCODE_F17;
        case IK_F18: return SDL_SCANCODE_F18;
        case IK_F19: return SDL_SCANCODE_F19;
        case IK_F20: return SDL_SCANCODE_F20;
        case IK_F21: return SDL_SCANCODE_F21;
        case IK_F22: return SDL_SCANCODE_F22;
        case IK_F23: return SDL_SCANCODE_F23;
        case IK_F24: return SDL_SCANCODE_F24;
        case IK_NumLock: return SDL_SCANCODE_NUMLOCKCLEAR;
        case IK_ScrollLock: return SDL_SCANCODE_SCROLLLOCK;
        case IK_LShift: return SDL_SCANCODE_LSHIFT;
        case IK_RShift: return SDL_SCANCODE_RSHIFT;
        case IK_LControl: return SDL_SCANCODE_LCTRL;
        case IK_RControl: return SDL_SCANCODE_RCTRL;
        case IK_Tilde: return SDL_SCANCODE_GRAVE;
        default: return (SDL_Scancode)0;
    }
}
