/* -*- ObjC -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2008 Tomotaka SUWA <t.suwa@mac.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "CompletionWindow.h"
#include "CompletionView.h"

@implementation CompletionWindow

+ (CompletionWindow*)sharedWindow {
    static CompletionWindow* obj =  [[CompletionWindow alloc] init];
    return obj;
}

- (id)init {
    self = [super init];
    if(self) {
        view_ = [[CompletionView alloc] init];
        window_ = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 0, 0)
                                              styleMask:NSWindowStyleMaskBorderless
                                    backing:NSBackingStoreBuffered
                                    defer:YES];
        [window_ setBackgroundColor:[NSColor clearColor]];
        [window_ setOpaque:NO];
        [window_ setIgnoresMouseEvents:YES];
        [window_ setContentView:view_];
    }
    return self;
}

- (void)dealloc {
    [window_ release];
    [view_ release];

    [super dealloc];
}

- (void)showCompletion:(NSAttributedString*)comp at:(NSPoint)topleft level:(int)level {
    [view_ setCompletion:comp];

    NSRect frame = [view_ frame];
    frame.origin = topleft;
    frame.origin.y -= frame.size.height;

    [window_ setFrame:frame display:NO];
    [window_ setLevel:level];
    [window_ orderFront:nil];
}

- (void)hide {
    [window_ orderOut:nil];
}

@end
