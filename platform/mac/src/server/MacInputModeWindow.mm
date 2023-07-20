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

#include "MacInputModeWindow.h"
#include "SKKFrontEnd.h"
#include "SKKConstVars.h"
#include "SKKLayoutManager.h"
#include "InputModeWindow.h"
#include <iostream>
#include <vector>

// MacInputModeWindow::Activate() から呼ばれるユーティリティ群
namespace {
    // 左下原点を左上原点に変換する
    CGPoint FlipPoint(int x, int y) {
        NSRect screen = [[NSScreen mainScreen] frame];

        return CGPointMake(x, NSHeight(screen) - y);
    }

    int ActiveProcessID() {
        return [[NSWorkspace sharedWorkspace] frontmostApplication].processIdentifier;
    }

    typedef std::vector<CGRect> CGRectContainer;

    // プロセス ID に関連したウィンドウ矩形群の取得
    CGRectContainer CreateWindowBoundsListOf(int pid) {
        CGRectContainer result;
        NSArray* array = (NSArray*)CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly,
                                                              kCGNullWindowID);
        NSEnumerator* enumerator = [array objectEnumerator];

        while(NSDictionary* window = [enumerator nextObject]) {
            // 引数のプロセス ID でフィルタ
            NSNumber* owner = [window objectForKey:(NSString*)kCGWindowOwnerPID];
            if([owner intValue] != pid) continue;

            // デスクトップ全面を覆う Finder のウィンドウは除外
            NSNumber* level = [window objectForKey:(NSString*)kCGWindowLayer];
            if([level intValue] == kCGMinimumWindowLevel) continue;

            CGRect rect;
            NSDictionary* bounds = [window objectForKey:(NSString*)kCGWindowBounds];
            if(CGRectMakeWithDictionaryRepresentation((CFDictionaryRef)bounds, &rect)) {
                result.push_back(rect);
            }
        }

        [array release];

        return result;
    }
}

// ----------------------------------------------------------------------
// SKKModeTips -- 遅延表示用の緩衝クラス
// ----------------------------------------------------------------------

@interface SKKModeTips : NSObject {
    InputModeWindow* window_;
    SKKLayoutManager* layout_;
}

- (id)initWithLayoutManager:(SKKLayoutManager*)layout;
- (void)changeMode:(SKKInputMode)mode;
- (void)show;
- (void)hide;
@end

@implementation SKKModeTips

- (void)activate:(id)sender {
    NSPoint pt = layout_->InputOrigin();
    CGPoint cursor = FlipPoint(pt.x, pt.y);
    CGRectContainer list = CreateWindowBoundsListOf(ActiveProcessID());

    // カーソル位置がウィンドウ矩形に含まれていなければ無視する
    long count = std::count_if(list.begin(), list.end(),
                              std::bind2nd(std::ptr_fun(CGRectContainsPoint), cursor));
    if(!count) {
        return;
    }

    [window_ showAt:pt level:layout_->WindowLevel()];
}

- (void)cancel {
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}

- (id)initWithLayoutManager:(SKKLayoutManager*)layout {
    self = [super init];
    if(self) {
        window_ = [InputModeWindow sharedWindow];
        layout_ = layout;
        [self changeMode:HirakanaInputMode];
    }

    return self;
}

- (void)dealloc {
    [self cancel];
    [super dealloc];
}

- (void)changeMode:(SKKInputMode)mode {
    [window_ changeMode:mode];
}

- (void)show {
    [self cancel];
    [self performSelector:@selector(activate:) withObject:self afterDelay:0.1];
}

- (void)hide {
    [self cancel];
    [window_ hide];
}

@end

// ----------------------------------------------------------------------
// MacInputModeWindow
// ----------------------------------------------------------------------

MacInputModeWindow::MacInputModeWindow(SKKLayoutManager* layout) {
    tips_ = [[SKKModeTips alloc] initWithLayoutManager:layout];
}

MacInputModeWindow::~MacInputModeWindow() {
    [tips_ release];
}

void MacInputModeWindow::SelectInputMode(SKKInputMode mode) {
    [tips_ changeMode:mode];
}

// ----------------------------------------------------------------------

bool MacInputModeWindow::enabled() const {
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    return [defaults boolForKey:SKKUserDefaultKeys::show_input_mode_icon] == YES;
}

void MacInputModeWindow::SKKWidgetShow() {
    if(!enabled()) return;

    [tips_ show];
}

void MacInputModeWindow::SKKWidgetHide() {
    [tips_ hide];
}
