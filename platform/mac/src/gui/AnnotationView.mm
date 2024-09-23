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

#include "AnnotationView.h"

@interface AnnotationView (Local)

- (void)initializeStyle;
- (void)initializeView;
- (NSAttributedString*)newHeader:(NSString*)string;
- (void)setDefinitiveAnnotation:(NSString*)string;
- (void)setOptionalAnnotation:(NSString*)string;
- (void)scrollToTop;
- (NSString*)normalizeString:(NSString*)string;
- (void)appendNewLine;
- (NSAttributedString*)getParagraph:(NSString*)string withStyle:(NSParagraphStyle*)style;

@end

@implementation AnnotationView

- (id)init {
    self = [super initWithFrame:NSMakeRect(0, 0, 256, 128)];

    if(self) {
        [self initializeStyle];
        [self initializeView];

        strokeColor_ = [[NSColor windowFrameColor] retain];

        definitiveHeader_ = [self newHeader:@"意味・語源"];
        annotationHeader_ = [self newHeader:@"SKK 辞書の註釈"];
    }

    return self;
}

- (void)dealloc {
    [textView_ release];
    [blockStyle_ release];
    [listStyle_ release];
    [strokeColor_ release];
    [definitiveHeader_ release];
    [annotationHeader_ release];

    [super dealloc];
}

- (void)setAnnotation:(NSString*)definition optional:(NSString*)annotation {
    [textView_ setString:@""];
    NSTextStorage* storage = [textView_ textStorage];

    [storage beginEditing];

    if(definition) {
        [storage appendAttributedString:definitiveHeader_];
        [self setDefinitiveAnnotation:definition];
    }

    if(annotation) {
        if(definition) {
            [self appendNewLine];
        }

        [storage appendAttributedString:annotationHeader_];
        [self setOptionalAnnotation:annotation];
    }

    [storage endEditing];

    [self scrollToTop];
}

- (BOOL)hasAnnotation {
    return [[textView_ string] length] ? YES : NO;
}

- (void)drawRect:(NSRect)rect {
    [strokeColor_ set];
    NSFrameRect([self frame]);
}

@end

@implementation AnnotationView (Local)

- (void)initializeStyle {
    float leftMargin = [NSFont systemFontSize];
    float tabStop = leftMargin * 2.5;

    blockStyle_ = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    [blockStyle_ setFirstLineHeadIndent:leftMargin];
    [blockStyle_ setHeadIndent:leftMargin];

    listStyle_ = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    [listStyle_ setFirstLineHeadIndent:leftMargin];
    [listStyle_ setHeadIndent:tabStop];
    [listStyle_ setTabStops:[NSArray array]];
    [listStyle_ setDefaultTabInterval:tabStop];
}

- (void)initializeView {
    NSRect frame = [self frame];
    frame.origin.x = 1;
    frame.origin.y = 1;
    frame.size.width -= 2;
    frame.size.height -= 2;

    NSScrollView* scrollView = [[NSScrollView alloc] initWithFrame:frame];
    [scrollView setHasVerticalScroller:YES];
    [[scrollView verticalScroller] setControlSize:NSControlSizeSmall];

    textView_ = [[NSTextView alloc] initWithFrame:[[scrollView contentView] frame]];
    [textView_ setEditable:NO];
    [textView_ setBackgroundColor:[NSColor colorWithDeviceRed:1.0 green:1.0 blue:0.94 alpha:1.0]];
    [textView_ setTextContainerInset:NSMakeSize(0, 2)];

    [scrollView setDocumentView:textView_];

    [self addSubview:scrollView];

    [scrollView release];
}

- (NSAttributedString*)newHeader:(NSString*)string {
    NSMutableAttributedString* header = [[NSMutableAttributedString alloc] initWithString:string];
    NSRange range = NSMakeRange(0, [string length]);

    [header addAttribute:NSFontAttributeName
                   value:[NSFont boldSystemFontOfSize:[NSFont labelFontSize]] range:range];

    [header addAttribute:NSForegroundColorAttributeName
                   value:[NSColor grayColor] range:range];

    NSMutableParagraphStyle* style = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    [style setLineSpacing:4.0];

    [header addAttribute:NSParagraphStyleAttributeName
                   value:style range:range];

    [style release];

    return header;
}

- (void)setDefinitiveAnnotation:(NSString*)string {
    NSTextStorage* storage = [textView_ textStorage];
    NSArray* array = [string componentsSeparatedByString:@"\n"];

    for(int i = 0; array && i < [array count]; ++ i) {
        NSString* line = [array objectAtIndex:i];

        if([line length] == 0) continue;

        [self appendNewLine];

        if([line length] == 1 && i + 1 < [array count]) {
            NSString* item = [NSString stringWithFormat:@"%@\t%@",
                                       [array objectAtIndex:i],
                                       [array objectAtIndex:i + 1]];

            NSAttributedString* attr = [self getParagraph:[self normalizeString:item]
                                             withStyle:listStyle_];
            [storage appendAttributedString:attr];
            ++ i;
        } else {
            NSAttributedString* attr = [self getParagraph:[self normalizeString:line]
                                             withStyle:blockStyle_];
            [storage appendAttributedString:attr];
        }
    }
}

- (void)setOptionalAnnotation:(NSString*)string {
    NSTextStorage* storage = [textView_ textStorage];

    [self appendNewLine];
    [storage appendAttributedString:[self getParagraph:string withStyle:blockStyle_]];
}

- (void)scrollToTop {
    NSPoint top;

    if([textView_ isFlipped]) {
        top = NSMakePoint(0.0, 0.0);
    } else {
        top = NSMakePoint(0.0, NSMaxY([textView_ frame]) - NSHeight([textView_  bounds]));
    }

    [textView_ scrollPoint:top];
}

- (NSString*)normalizeString:(NSString*)string {
    const static struct {
        unsigned short unichar;
        NSString* string;
    } table[] = {
        { 0xe021, @"[ー]" },
        { 0xe022, @"[二]" },
        { 0xe023, @"[三]" },
        { 0xe024, @"[四]" },
        { 0xe025, @"[五]" },
        { 0xe026, @"[六]" },
        { 0xe027, @"[文]" },
        { 0,      0x00 }
    };

    NSString* tmp = string;
    for(int i = 0; table[i].string != 0x00; ++ i) {
        NSString* from = [NSString stringWithFormat:@"%C", table[i].unichar];
        NSString* to = table[i].string;
        tmp = [tmp stringByReplacingOccurrencesOfString:from withString:to];
    }

    return tmp;
}

- (void)appendNewLine {
    [[[textView_ textStorage] mutableString] appendString:@"\n"];
}

- (NSAttributedString*)getParagraph:(NSString*)string withStyle:(NSParagraphStyle*)style {
    NSMutableAttributedString* attr = [[NSMutableAttributedString alloc] initWithString:string];
    NSRange range = NSMakeRange(0, [attr length]);

    [attr addAttribute:NSParagraphStyleAttributeName value:style range:range];

    return [attr autorelease];
}

@end
