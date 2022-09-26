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
    view.translatesAutoresizingMaskIntoConstraints = false;
    
    [window.contentView addSubview:view];
    
    [view.leadingAnchor constraintEqualToAnchor:window.contentView.leadingAnchor].active = YES;
    [view.trailingAnchor constraintEqualToAnchor:window.contentView.trailingAnchor].active = YES;
    [view.topAnchor constraintEqualToAnchor:window.contentView.topAnchor].active = YES;
    [view.bottomAnchor constraintEqualToAnchor:window.contentView.bottomAnchor].active = YES;
}
