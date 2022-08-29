//
//  Utilities.cpp
//  Walnut
//
//  Created by Stephen Gerstacker on 2022-08-29.
//

#import <AppKit/AppKit.h>

static void AddViewToWindowEx(NSView* view, NSWindow* window);

void AddViewToWindow(void* _Nonnull view, void* _Nonnull window) {
    NSView *nativeView = (__bridge NSView *)view;
    NSWindow *nativeWindow = (__bridge NSWindow *)window;
    
    AddViewToWindowEx(nativeView, nativeWindow);
}

void AddViewToWindowEx(NSView* view, NSWindow* window) {
    [window.contentView addSubview:view];
}
