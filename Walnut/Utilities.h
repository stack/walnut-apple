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

#endif

#ifndef __OBJC__

void AddViewToWindow(void* _Nonnull view, void* _Nonnull window);

#endif
