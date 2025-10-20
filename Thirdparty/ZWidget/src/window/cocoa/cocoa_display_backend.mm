#include "cocoa_display_backend.h"
#include "cocoa_display_window.h"
#include "cocoa_open_file_dialog.h"
#include "cocoa_save_file_dialog.h"
#include "cocoa_open_folder_dialog.h"

#include <zwidget/core/timer.h>

#include "AppKitWrapper.h"

@interface ZWidgetTimerTarget : NSObject
{
    Timer* timer;
}
- (id)initWithTimer:(Timer*)t;
- (void)timerFired:(NSTimer*)nstimer;
@end

@implementation ZWidgetTimerTarget
- (id)initWithTimer:(Timer*)t;
{
    self = [super init];
    if (self)
    {
        timer = (Timer*)t;
    }
    return self;
}

- (void)timerFired:(NSTimer*)nstimer
{
    if (timer->FuncExpired)
        timer->FuncExpired();
}
@end

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateCocoa()
{
    return std::make_unique<CocoaDisplayBackend>();
}


CocoaDisplayBackend::CocoaDisplayBackend()
{
}

CocoaDisplayBackend::~CocoaDisplayBackend()
{
}

std::unique_ptr<DisplayWindow> CocoaDisplayBackend::Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner, RenderAPI renderAPI)
{
    return std::make_unique<CocoaDisplayWindow>(windowHost, popupWindow, owner, renderAPI);
}

void CocoaDisplayBackend::ProcessEvents()
{
    NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
    if (event)
    {
        [NSApp sendEvent:event];
    }
}

void CocoaDisplayBackend::RunLoop()
{
    [NSApp run];
}

void CocoaDisplayBackend::ExitLoop()
{
    [NSApp stop:nil];
}

void* CocoaDisplayBackend::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
    Timer* timer = new Timer(nullptr);
    timer->FuncExpired = onTimer;
    ZWidgetTimerTarget* target = [[ZWidgetTimerTarget alloc] initWithTimer:timer];
    NSTimer* nstimer = [NSTimer scheduledTimerWithTimeInterval:timeoutMilliseconds / 1000.0 target:target selector:@selector(timerFired:) userInfo:nil repeats:YES];
    timer->SetTimerId((__bridge void*)nstimer); // No retain needed with ARC
    // [target release]; // No release needed with ARC
    return timer;
}

void CocoaDisplayBackend::StopTimer(void* timerID)
{
    Timer* timer = static_cast<Timer*>(timerID);
    NSTimer* nstimer = (__bridge NSTimer*)timer->GetTimerId();
    [nstimer invalidate];
    // [nstimer release]; // No release needed with ARC
    delete timer;
}

Size CocoaDisplayBackend::GetScreenSize()
{
    NSRect screenFrame = [[NSScreen mainScreen] frame];
    return Size(screenFrame.size.width, screenFrame.size.height);
}

std::unique_ptr<OpenFileDialog> CocoaDisplayBackend::CreateOpenFileDialog(DisplayWindow* owner)
{
    return std::make_unique<CocoaOpenFileDialog>(owner);
}

std::unique_ptr<SaveFileDialog> CocoaDisplayBackend::CreateSaveFileDialog(DisplayWindow* owner)
{
    return std::make_unique<CocoaSaveFileDialog>(owner);
}

std::unique_ptr<OpenFolderDialog> CocoaDisplayBackend::CreateOpenFolderDialog(DisplayWindow* owner)
{
    return std::make_unique<CocoaOpenFolderDialog>(owner);
}