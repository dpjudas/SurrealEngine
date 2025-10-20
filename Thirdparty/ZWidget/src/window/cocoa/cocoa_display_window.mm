#include "cocoa_display_window.h"
#include <stdio.h>
#include <vector> // Required for std::vector
#include <map>
#include <dlfcn.h>
#include <zwidget/core/image.h>



#import "AppKitWrapper.h"
#import <Cocoa/Cocoa.h>

#ifdef HAVE_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#endif

#ifdef HAVE_OPENGL
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#endif

// Forward declarations
@class ZWidgetView;
@class ZWidgetWindowDelegate;

InputKey keycode_to_inputkey(unsigned short keycode)
{
    static const std::map<unsigned short, InputKey> keymap =
    {
        {0, InputKey::A},
        {1, InputKey::S},
        {2, InputKey::D},
        {3, InputKey::F},
        {4, InputKey::H},
        {5, InputKey::G},
        {6, InputKey::Z},
        {7, InputKey::X},
        {8, InputKey::C},
        {9, InputKey::V},
        {11, InputKey::B},
        {12, InputKey::Q},
        {13, InputKey::W},
        {14, InputKey::E},
        {15, InputKey::R},
        {16, InputKey::Y},
        {17, InputKey::T},
        {36, InputKey::Enter},
        {48, InputKey::Tab},
        {49, InputKey::Space},
        {51, InputKey::Backspace},
        {53, InputKey::Escape},
        {55, InputKey::LShift}, // Command
        {56, InputKey::LShift},
        {57, InputKey::CapsLock},
        {58, InputKey::Alt},
        {59, InputKey::LControl},
        {60, InputKey::RShift},
        {61, InputKey::Alt}, // Right Alt
        {62, InputKey::RControl},
        {115, InputKey::Home},
        {116, InputKey::PageUp},
        {117, InputKey::Delete},
        {119, InputKey::End},
        {121, InputKey::PageDown},
        {122, InputKey::F1},
        {120, InputKey::F2},
        {99, InputKey::F3},
        {118, InputKey::F4},
        {96, InputKey::F5},
        {97, InputKey::F6},
        {98, InputKey::F7},
        {100, InputKey::F8},
        {101, InputKey::F9},
        {109, InputKey::F10},
        {103, InputKey::F11},
        {111, InputKey::F12},
        {123, InputKey::Left},
        {124, InputKey::Right},
        {125, InputKey::Down},
        {126, InputKey::Up}
    };

    auto it = keymap.find(keycode);
    if (it != keymap.end())
    {
        return it->second;
    }
    return InputKey::None;
}

struct CocoaDisplayWindowImpl
{
public:
    DisplayWindowHost* windowHost = nullptr;
    NSWindow* window = nil;
    ZWidgetWindowDelegate* delegate = nil;
    NSBitmapImageRep* bitmapRep = nil;
    std::map<InputKey, bool> keyState;
    bool mouseCaptured = false;
    RenderAPI renderAPI = RenderAPI::Unspecified;

#ifdef HAVE_METAL
    id<MTLDevice> metalDevice = nil;
    id<MTLCommandQueue> commandQueue = nil;
    CAMetalLayer* metalLayer = nil;
#endif

#ifdef HAVE_OPENGL
    NSOpenGLContext* openglContext = nil;
#endif

    // Declare methods, but implement them outside the struct
    void initMetal(ZWidgetView* view);
    void initOpenGL(ZWidgetView* view);
};

@interface ZWidgetView : NSView
{
    CocoaDisplayWindowImpl* impl;
}
- (id)initWithImpl:(CocoaDisplayWindowImpl*)impl;
@end

@implementation ZWidgetView

#ifdef HAVE_METAL
+ (Class)layerClass
{
    return [CAMetalLayer class];
}
#endif

- (id)initWithImpl:(CocoaDisplayWindowImpl*)d
{
    self = [super init];
    if (self)
    {
        impl = d;
        [self addTrackingArea:[[NSTrackingArea alloc] initWithRect:self.bounds options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow owner:self userInfo:nil]];
    }
    return self;
}
- (BOOL)isOpaque
{
    return YES;
}
- (BOOL)canBecomeKeyView
{
    return YES;
}
- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect
{
    if (impl && impl->bitmapRep)
    {
        [impl->bitmapRep drawInRect:self.bounds];
    }
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    // Not used yet
}

- (void)mouseExited:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
        impl->windowHost->OnWindowMouseLeave();
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
    {
        if (impl->mouseCaptured)
        {
            impl->windowHost->OnWindowRawMouseMove([theEvent deltaX], [theEvent deltaY]);
        }
        else
        {
            NSPoint p = [theEvent locationInWindow];
            impl->windowHost->OnWindowMouseMove(Point(p.x, [self frame].size.height - p.y));
        }
    }
}

- (void)mouseDown:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
    {
        NSPoint p = [theEvent locationInWindow];
        impl->windowHost->OnWindowMouseDown(Point(p.x, [self frame].size.height - p.y), InputKey::LeftMouse);
    }
}

- (void)mouseUp:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
    {
        NSPoint p = [theEvent locationInWindow];
        impl->windowHost->OnWindowMouseUp(Point(p.x, [self frame].size.height - p.y), InputKey::LeftMouse);
    }
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
    {
        NSPoint p = [theEvent locationInWindow];
        impl->windowHost->OnWindowMouseDown(Point(p.x, [self frame].size.height - p.y), InputKey::RightMouse);
    }
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
    {
        NSPoint p = [theEvent locationInWindow];
        impl->windowHost->OnWindowMouseUp(Point(p.x, [self frame].size.height - p.y), InputKey::RightMouse);
    }
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
    {
        NSPoint p = [theEvent locationInWindow];
        impl->windowHost->OnWindowMouseMove(Point(p.x, [self frame].size.height - p.y));
    }
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
    {
        NSPoint p = [theEvent locationInWindow];
        impl->windowHost->OnWindowMouseMove(Point(p.x, [self frame].size.height - p.y));
    }
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
    {
        NSPoint p = [theEvent locationInWindow];
        if ([theEvent deltaY] > 0)
            impl->windowHost->OnWindowMouseWheel(Point(p.x, [self frame].size.height - p.y), InputKey::MouseWheelUp);
        else if ([theEvent deltaY] < 0)
            impl->windowHost->OnWindowMouseWheel(Point(p.x, [self frame].size.height - p.y), InputKey::MouseWheelDown);
    }
}

- (void)keyDown:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
    {
        InputKey key = keycode_to_inputkey([theEvent keyCode]);
        impl->keyState[key] = true;
        impl->windowHost->OnWindowKeyDown(key); // Removed isARepeat as it's not in the ZWidget API
        
        NSString* characters = [theEvent characters];
        if ([characters length] > 0)
        {
            impl->windowHost->OnWindowKeyChar([characters UTF8String]);
        }
    }
}

- (void)keyUp:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
    {
        InputKey key = keycode_to_inputkey([theEvent keyCode]);
        impl->keyState[key] = false;
        impl->windowHost->OnWindowKeyUp(key);
    }
}

- (void)flagsChanged:(NSEvent *)theEvent
{
    if (impl && impl->windowHost)
    {
        bool shiftPressed = ([theEvent modifierFlags] & NSEventModifierFlagShift) != 0;
        bool ctrlPressed = ([theEvent modifierFlags] & NSEventModifierFlagControl) != 0;
        bool altPressed = ([theEvent modifierFlags] & NSEventModifierFlagOption) != 0;

        if (impl->keyState[InputKey::Shift] != shiftPressed)
        {
            impl->keyState[InputKey::Shift] = shiftPressed;
            if (shiftPressed)
                impl->windowHost->OnWindowKeyDown(InputKey::Shift);
            else
                impl->windowHost->OnWindowKeyUp(InputKey::Shift);
        }

        if (impl->keyState[InputKey::Ctrl] != ctrlPressed)
        {
            impl->keyState[InputKey::Ctrl] = ctrlPressed;
            if (ctrlPressed)
                impl->windowHost->OnWindowKeyDown(InputKey::Ctrl);
            else
                impl->windowHost->OnWindowKeyUp(InputKey::Ctrl);
        }

        if (impl->keyState[InputKey::Alt] != altPressed)
        {
            impl->keyState[InputKey::Alt] = altPressed;
            if (altPressed)
                impl->windowHost->OnWindowKeyDown(InputKey::Alt);
            else
                impl->windowHost->OnWindowKeyUp(InputKey::Alt);
        }
    }
}

@end

// ZWidgetWindowDelegate interface and implementation
@interface ZWidgetWindowDelegate : NSObject <NSWindowDelegate>

{

    CocoaDisplayWindowImpl* impl;

}

- (id)initWithImpl:(CocoaDisplayWindowImpl*)impl;

@end



@implementation ZWidgetWindowDelegate



- (id)initWithImpl:(CocoaDisplayWindowImpl*)d

{

    self = [super init];

    if (self)

    {

        impl = d;

    }

    return self;

}



// Implement NSWindowDelegate methods here if needed

// For example:

// - (BOOL)windowShouldClose:(NSWindow *)sender

// {

//     // Handle window close event

//     return YES;

// }



@end

// Implement CocoaDisplayWindowImpl methods here
void CocoaDisplayWindowImpl::initMetal(ZWidgetView* view)
{
#ifdef HAVE_METAL
    metalDevice = MTLCreateSystemDefaultDevice();
    if (metalDevice)
    {
        commandQueue = [metalDevice newCommandQueue];
        metalLayer = (CAMetalLayer*)[view layer];
        metalLayer.device = metalDevice;
        metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        metalLayer.framebufferOnly = YES;
        renderAPI = RenderAPI::Metal;
    }
    else
    {
        renderAPI = RenderAPI::Bitmap; // Fallback if Metal not available
    }
#else
    renderAPI = RenderAPI::Bitmap; // Fallback if Metal not available
#endif
}

void CocoaDisplayWindowImpl::initOpenGL(ZWidgetView* view)
{
#ifdef HAVE_OPENGL
    NSOpenGLPixelFormatAttribute attrs[] = { NSOpenGLPFAAccelerated, NSOpenGLPFANoRecovery, NSOpenGLPFADoubleBuffer, NSOpenGLPFAColorSize, 24, NSOpenGLPFAAlphaSize, 8, NSOpenGLPFADepthSize, 24, 0 };
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    openglContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    if (openglContext)
    {
        renderAPI = RenderAPI::OpenGL;
        [openglContext setView:(NSView*)view];
    }
    else
    {
        renderAPI = RenderAPI::Bitmap; // Fallback if OpenGL not available
    }
#else
    renderAPI = RenderAPI::Bitmap; // Fallback if OpenGL not available
#endif
}


CocoaDisplayWindow::CocoaDisplayWindow(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner, RenderAPI renderAPI) : impl(std::make_unique<CocoaDisplayWindowImpl>()) {}

CocoaDisplayWindow::~CocoaDisplayWindow() = default;

void CocoaDisplayWindow::SetWindowTitle(const std::string& text) {}
void CocoaDisplayWindow::SetWindowIcon(const std::vector<std::shared_ptr<Image>>& images) {}
void CocoaDisplayWindow::SetWindowFrame(const Rect& box) {}
void CocoaDisplayWindow::SetClientFrame(const Rect& box) {}

void CocoaDisplayWindow::Show() {}
void CocoaDisplayWindow::ShowFullscreen() {}
void CocoaDisplayWindow::ShowMaximized() {}
void CocoaDisplayWindow::ShowMinimized() {}
void CocoaDisplayWindow::ShowNormal() {}
bool CocoaDisplayWindow::IsWindowFullscreen() { return false; }
void CocoaDisplayWindow::Hide() {}
void CocoaDisplayWindow::Activate() {}

void CocoaDisplayWindow::ShowCursor(bool enable) {}
void CocoaDisplayWindow::LockKeyboard() {}
void CocoaDisplayWindow::UnlockKeyboard() {}
void CocoaDisplayWindow::LockCursor() {}
void CocoaDisplayWindow::UnlockCursor() {}
void CocoaDisplayWindow::CaptureMouse() {}
void CocoaDisplayWindow::ReleaseMouseCapture() {}

void CocoaDisplayWindow::Update() {}

bool CocoaDisplayWindow::GetKeyState(InputKey key) { return false; }

void CocoaDisplayWindow::SetCursor(StandardCursor cursor, std::shared_ptr<CustomCursor> custom) {}

Rect CocoaDisplayWindow::GetWindowFrame() const { return {}; }
Size CocoaDisplayWindow::GetClientSize() const { return {}; }
int CocoaDisplayWindow::GetPixelWidth() const { return 0; }
int CocoaDisplayWindow::GetPixelHeight() const { return 0; }
double CocoaDisplayWindow::GetDpiScale() const { return 1.0; }

Point CocoaDisplayWindow::MapFromGlobal(const Point& pos) const { return {}; }
Point CocoaDisplayWindow::MapToGlobal(const Point& pos) const { return {}; }

void CocoaDisplayWindow::SetBorderColor(uint32_t bgra8) {}
void CocoaDisplayWindow::SetCaptionColor(uint32_t bgra8) {}
void CocoaDisplayWindow::SetCaptionTextColor(uint32_t bgra8) {}

void CocoaDisplayWindow::PresentBitmap(int width, int height, const uint32_t* pixels) {}

std::string CocoaDisplayWindow::GetClipboardText() { return {}; }
void CocoaDisplayWindow::SetClipboardText(const std::string& text) {}

void* CocoaDisplayWindow::GetNativeHandle() { return nullptr; }

std::vector<std::string> CocoaDisplayWindow::GetVulkanInstanceExtensions() { return {}; }
VkSurfaceKHR CocoaDisplayWindow::CreateVulkanSurface(VkInstance instance) { return nullptr; }
