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

#include "MessengerView.h"

@interface MessengerView (Local)

- (NSRect)messageRect;

@end

@implementation MessengerView

- (id)init {
    self = [super initWithFrame:NSMakeRect(0, 0, 0, 0)];
    if(self) {
        message_ = nil;
        icon_ = [[NSImage imageNamed:NSImageNameInfo] retain];
        [icon_ setSize:NSMakeSize(16, 16)];
        attributes_ = [NSDictionary dictionaryWithObject:[NSFont systemFontOfSize:0.0]
                                    forKey:NSFontAttributeName];
        [attributes_ retain];
    }
    return self;
}

- (void)dealloc {
    if(message_) {
        [message_ release];
    }

    [icon_ release];
    [attributes_ release];

    [super dealloc];
}

- (void)setMessage:(NSString*)message {
    if(message_) {
        [message_ release];
    }

    message_ = [message retain];

    [self setFrame:[self messageRect]];
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect {
    NSRect frame = [self messageRect];

    [[NSColor controlColor] setFill];
    [[NSColor controlShadowColor] setStroke];

    NSRectFill(frame);
    [NSBezierPath strokeRect:frame];

    NSPoint pt;
    pt.x = 3;
    pt.y = (NSHeight(frame) - [icon_ size].height) / 2.0;

    [icon_ drawAtPoint:pt fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:1.0];
    [message_ drawAtPoint:NSMakePoint(pt.x + [icon_ size].width + 2, 4) withAttributes:attributes_];
}

@end

@implementation MessengerView (Local)

- (NSRect)messageRect {
    NSRect rect;

    rect.origin = NSMakePoint(0, 0);
    rect.size = [message_ sizeWithAttributes:attributes_];
    rect.size.height += 8;
    rect.size.width += [icon_ size].width + 8;

    return rect;
}

@end
