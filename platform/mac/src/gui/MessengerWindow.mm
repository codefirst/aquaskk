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

#include "MessengerWindow.h"
#include "MessengerView.h"

@implementation MessengerWindow

+ (MessengerWindow*)sharedWindow {
    static MessengerWindow* obj =  [[MessengerWindow alloc] init];

    return obj;
}

- (id)init {
    self = [super init];
    if(self) {
        view_ = [[MessengerView alloc] init];
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

- (void)showMessage:(NSString*)msg at:(NSPoint)topleft level:(int)level {
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    [view_ setMessage:msg];
    
    [window_ setFrame:[view_ frame] display:NO];
    [window_ setFrameTopLeftPoint:topleft];
    [window_ setLevel:level];
    [window_ setAlphaValue:1.0];
    [window_ orderFront:nil];

    [self performSelector:@selector(hideNotify:) withObject:self afterDelay:1.5];
}

- (void)hide {
    NSMutableDictionary* dictionary = [NSMutableDictionary dictionaryWithCapacity:2];

    [dictionary setObject:window_ forKey:NSViewAnimationTargetKey];
    [dictionary setObject:NSViewAnimationFadeOutEffect forKey:NSViewAnimationEffectKey];
    
    NSViewAnimation* animation = [[NSViewAnimation alloc]
                                     initWithViewAnimations:[NSArray arrayWithObjects:dictionary, nil]];

    [animation setDuration:0.5];
    [animation startAnimation];
    [animation release];
}

- (void)hideNotify:(id)sender {
    [self hide];
}

@end
