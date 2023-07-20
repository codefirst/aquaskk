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

#include "AnnotationWindow.h"
#include "AnnotationView.h"

@implementation AnnotationWindow

+ (AnnotationWindow*)sharedWindow {
    static AnnotationWindow* obj =  [[AnnotationWindow alloc] init];
    return obj;
}

- (id)init {
    self = [super init];
    if(self) {
        view_ = [[AnnotationView alloc] init];
        window_ = [[NSWindow alloc] initWithContentRect:[view_ frame]
                                              styleMask:NSWindowStyleMaskBorderless
                                    backing:NSBackingStoreBuffered
                                    defer:YES];
        [window_ setContentView:view_];
    }
    return self;
}

- (void)dealloc {
    [window_ release];
    [view_ release];

    [super dealloc];
}

- (NSWindow*)window {
    return window_;
}

- (void)setAnnotation:(NSString*)definition optional:(NSString*)annotation {
    [view_ setAnnotation:definition optional:annotation];
}

- (void)activate:(id)sender {
    [window_ orderFront:nil];

    [view_ setNeedsDisplay:YES];
}

- (void)showAt:(NSPoint)origin level:(int)level {
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

    if(![view_ hasAnnotation]) {
        [self hide];
        return;
    }

    [window_ setFrameOrigin:origin];
    [window_ setLevel:level];

    [self performSelector:@selector(activate:) withObject:self afterDelay:1.0];
}

- (void)hide {
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    [window_ orderOut:nil];
}

@end
