//
//  Utilities.hpp
//  Walnut
//
//  Created by Stephen Gerstacker on 2022-08-29.
//

#pragma once

#ifdef __OBJC__

@class NSWindow;
@class NSView;

void AddViewToWindowEx(NSView* _Nonnull view, NSWindow* _Nonnull window);
NSString * _Nonnull GetIniPathEx(NSString * _Nonnull applicationName);

#endif

#ifndef __OBJC__

#include <Foundation/Foundation.hpp>

void AddViewToWindow(void* _Nonnull view, void* _Nonnull window);

std::string GetIniPath(const std::string& applicationName);

#endif
