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

#include "CandidateCell.h"

@implementation CandidateCell

+ (int)margin {
    return 2;
}

+ (NSSize)focusSize:(NSSize)size {
    int margin = [CandidateCell margin] * 2;
    return NSMakeSize(size.width + margin, size.height + margin);
}

- (id)initWithFont:(NSFont*)font {
    if(self = [super init]) {
	entry_ = [[NSMutableAttributedString alloc] init];
	attributes_ = [[NSMutableDictionary dictionaryWithObjectsAndKeys:
                    font, NSFontAttributeName,
                    [NSColor labelColor], NSForegroundColorAttributeName,
                    nil]
                   retain];

	NSAttributedString* tmpstr = [[NSAttributedString alloc]
					 initWithString:[NSString stringWithUTF8String:" A  漢字 "]
					 attributes:attributes_];
	size_ = [CandidateCell focusSize:[tmpstr size]];
	[tmpstr release];
    }

    return self;
}

- (void)dealloc {
    [attributes_ release];
    [entry_ release];

    [super dealloc];
}

- (void)setString:(NSString*)string withLabel:(char)label {
    // 属性付き文字列
    NSAttributedString* tmpstr = [[NSAttributedString alloc]
				     initWithString:[NSString stringWithFormat:@" %c  %@", label, string]
				     attributes:attributes_];
    [entry_ setAttributedString:tmpstr];
    [tmpstr release];

    // ラベルの背景色
    if (@available(macOS 10_14, *)) {
      [entry_ addAttribute:NSBackgroundColorAttributeName
        value:[NSColor controlAccentColor] range:NSMakeRange(0, 3)];
    } else {
      [entry_ addAttribute:NSBackgroundColorAttributeName
        value:[NSColor selectedMenuItemColor] range:NSMakeRange(0, 3)];
    }

    // ラベルの文字色
    [entry_ addAttribute:NSForegroundColorAttributeName
	    value:[NSColor selectedMenuItemTextColor] range:NSMakeRange(0, 3)];

    // ラインブレイク属性
    NSMutableParagraphStyle* style = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    [style setLineBreakMode:NSLineBreakByTruncatingTail];
    [entry_ addAttribute:NSParagraphStyleAttributeName value:style range:NSMakeRange(0, [entry_ length])];
    [style release];
}

- (NSSize)size {
    NSSize current = [CandidateCell focusSize:[entry_ size]];

    if(current.width < [self defaultSize].width) {
	return [self defaultSize];
    } else {
	return current;
    }
}

- (NSSize)defaultSize {
    return size_;
}

- (void)drawFocusAtPoint:(NSPoint)pt withSize:(NSSize)size {
    NSRect focus;

    focus.origin = pt;
    focus.size = size;
    focus.size.height -= 1;

    [[NSGraphicsContext currentContext] setShouldAntialias:NO];

    if (@available(macOS 10_14, *)) {
        [[[NSColor windowBackgroundColor] colorWithSystemEffect: NSColorSystemEffectPressed] setFill];
    } else {
        [[[NSColor blackColor] colorWithAlphaComponent:0.1] setFill];
    }
    NSRectFillUsingOperation(focus, NSCompositingOperationSourceOver);

    [[NSColor windowFrameColor] setStroke];
    [NSBezierPath strokeRect:focus];

    [[NSGraphicsContext currentContext] setShouldAntialias:YES];
}

- (void)drawAtPoint:(NSPoint)pt withFocus:(BOOL)focus {
    int margin = [CandidateCell margin];

    if(focus) {
        [self drawFocusAtPoint:pt withSize:[self size]];
    }

    NSRect rect;
    rect.origin = NSMakePoint(pt.x + margin, pt.y + margin);
    rect.size = [entry_ size];

    [entry_ drawInRect:rect];
}

- (void)drawAtPoint:(NSPoint)pt withSize:(NSSize)size {
    int margin = [CandidateCell margin];

    [self drawFocusAtPoint:pt withSize:size];

    NSRect rect;
    rect.origin = NSMakePoint(pt.x + margin, pt.y + margin);
    rect.size = size;

    [entry_ drawInRect:rect];
}

@end
