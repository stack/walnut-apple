//
//  Utilities.cpp
//  Walnut
//
//  Created by Stephen Gerstacker on 2022-08-29.
//

#import <AppKit/AppKit.h>

#include <string>

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

NSString * _Nonnull GetIniPathEx(NSString * _Nonnull applicationName) {
    NSError *error = nil;
    NSURL *rootURL = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:&error];
    
    NSURL *directoryURL = [rootURL URLByAppendingPathComponent:applicationName];
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:directoryURL.path]) {
        [[NSFileManager defaultManager] createDirectoryAtURL:directoryURL withIntermediateDirectories:YES attributes:nil error:&error];
    }
    
    NSURL *iniURL = [directoryURL URLByAppendingPathComponent:@"imgui.ini"];
    
    return iniURL.path;
}

std::string GetIniPath(const std::string& applicationName) {
    NSString *nsApplicationName = [NSString stringWithUTF8String:applicationName.c_str()];
    NSString *nsIniPath = GetIniPathEx(nsApplicationName);
    
    std::string iniPath(nsIniPath.UTF8String);
    
    return iniPath;
}
