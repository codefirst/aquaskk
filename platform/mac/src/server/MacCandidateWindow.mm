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

#include "MacCandidateWindow.h"
#include "CandidateWindow.h"
#include "CandidateView.h"
#include "CandidateCell.h"
#include "SKKConstVars.h"
#include "utf8util.h"

#include <InputMethodKit/InputMethodKit.h>

MacCandidateWindow::MacCandidateWindow(SKKLayoutManager* layout)
    : layout_(layout) {
    window_ = [CandidateWindow sharedWindow];
    candidates_ = [[NSMutableArray alloc] initWithCapacity:0];
    reloadUserDefaults();
}

MacCandidateWindow::~MacCandidateWindow() {
    [candidates_ release];
}

void MacCandidateWindow::Setup(SKKCandidateIterator begin, SKKCandidateIterator end, std::vector<int>& pages) {
    reloadUserDefaults();

    std::vector<int> cell_width;

    CandidateCell* cell = [window_ newCandidateCell];
    int width;

    // 全ての cell の幅を求める
    while(begin != end) {
        std::string candidate(begin->Variant());

        // UTF-8 で二文字以下ならデフォルトサイズを使う(最適化)
        if(utf8::length(candidate) < 3) {
            width = [cell defaultSize].width;
        } else {
            NSString* string = [NSString stringWithUTF8String:candidate.c_str()];

            [cell setString:string withLabel:'A'];

            width = [cell size].width;
        }

        cell_width.push_back(width + [CandidateView cellSpacing]);

        ++ begin;
    }

    unsigned limit = ([cell defaultSize].width + [CandidateView cellSpacing]) * cellCount_;
    int offset = 0;

    // 候補ウィンドウに表示可能な cell の数を求める
    pages.clear();
    do {
        unsigned size = 0;
        int count = 0;
        while(offset < cell_width.size()) {
            if(limit < size + cell_width[offset]) {
                if(size == 0) {
                    ++ offset;
                    count = 1;
                }
                break;
            }
            size += cell_width[offset];
            ++ offset;
            ++ count;
        }

        pages.push_back(count);
    } while(offset < cell_width.size());

    [cell release];
}

void MacCandidateWindow::Update(SKKCandidateIterator begin, SKKCandidateIterator end,
                                int cursor, int page_pos, int page_max) {
    [candidates_ removeAllObjects];

    for(SKKCandidateIterator curr = begin; curr != end; ++ curr) {
        std::string candidate(curr->Variant());
	[candidates_ addObject:[NSString stringWithUTF8String:candidate.c_str()]];
    }

    page_ = NSMakeRange(page_pos, page_max);
    cursor_ = cursor;
}

int MacCandidateWindow::LabelIndex(char label) {
    return (int)[window_ indexOfLabel:label];
}

// ------------------------------------------------------------

void MacCandidateWindow::reloadUserDefaults() {
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    NSString* fontName = [defaults stringForKey:SKKUserDefaultKeys::candidate_window_font_name];
    float fontSize = [defaults floatForKey:SKKUserDefaultKeys::candidate_window_font_size];

    NSFont* font = [NSFont fontWithName:fontName size:fontSize] ?: [NSFont labelFontOfSize:fontSize];

    NSString* labels = [defaults stringForKey:SKKUserDefaultKeys::candidate_window_labels];
    cellCount_ = [labels length];

    putUpward_ = [defaults boolForKey:SKKUserDefaultKeys::put_candidate_window_upward] == YES;

    [window_ prepareWithFont:font labels:labels];
}

void MacCandidateWindow::SKKWidgetShow() {
    [window_ setCandidates:candidates_ selectedIndex:cursor_];
    [window_ setPage:page_];
    [window_ showAt:layout_->CandidateWindowOrigin()
             level:layout_->WindowLevel()];
}

void MacCandidateWindow::SKKWidgetHide() {
    [window_ hide];
}
