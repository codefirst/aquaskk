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

#include "CandidateWindow.h"
#include "CandidateView.h"

@implementation CandidateWindow

+ (CandidateWindow*)sharedWindow {
    static CandidateWindow* obj = [[CandidateWindow alloc] init];

    return obj;
}

- (id)init {
    self = [super init];
    if(self) {
        view_ = [[CandidateView alloc] initWithFrame:NSZeroRect];
        window_ = [[NSWindow alloc] initWithContentRect:NSZeroRect
                                              styleMask:NSWindowStyleMaskBorderless
                                    backing:NSBackingStoreBuffered
                                    defer:YES];
        [window_ setIgnoresMouseEvents:YES];
        [window_ setContentView:view_];
        labels_ = @"";
    }
    return self;
}

- (void)dealloc {
    [labels_ release];
    [window_ release];
    [view_ release];

    [super dealloc];
}

- (NSWindow*)window {
    return window_;
}

- (void)prepareWithFont:(NSFont*)newFont labels:(NSString*)newLabels {
    [labels_ release];
    labels_ = [newLabels retain];

    [view_ prepareWithFont:newFont labels:labels_];
    [window_ setContentSize:[view_ contentSize]];
}

- (void)setCandidates:(NSArray*)candidates selectedIndex:(int)cursor {
    [view_ setCandidates:candidates selectedIndex:cursor];
}

- (void)setPage:(NSRange)page {
    [view_ setPage:page];
}

- (void)showAt:(NSPoint)origin level:(int)level {
    [window_ setFrameOrigin:origin];
    [window_ setLevel:level];
    [window_ orderFront:nil];
}

- (void)hide {
    [window_ orderOut:nil];
}

- (NSUInteger)indexOfLabel:(char)label {
    NSString* target = [NSString stringWithFormat:@"%c", label];
    NSRange result = [labels_ rangeOfString:target options:NSCaseInsensitiveSearch];

    if(result.location == NSNotFound) {
	return -1;
    } else {
	return result.location;
    }
}

- (id)newCandidateCell {
    return [view_ newCandidateCell];
}

@end
