/* -*- C++ -*-

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

#include "SKKWindowSelector.h"
#include "SKKCandidateWindow.h"
#include <cassert>

SKKWindowSelector::SKKWindowSelector(SKKCandidateWindow* window) : window_(window) {}

void SKKWindowSelector::Initialize(SKKCandidateContainer& container, unsigned inlineCount) {
    range_.set(container, inlineCount);
    window_->Setup(range_.begin(), range_.end(), pages_);

    page_pos_ = 0;
    cursor_pos_ = 0;
    offset_ = 0;

    refresh();
}

bool SKKWindowSelector::Next() {
    if(page_pos_ == maxPage()) {
	return false;
    }

    offset_ += pages_[page_pos_];
    ++ page_pos_;
    cursor_pos_ = 0;

    refresh();
    Show();

    return true;
}

bool SKKWindowSelector::Prev() {
    if(page_pos_ == minPage()) {
	return false;
    }

    -- page_pos_;
    offset_ -= pages_[page_pos_];
    cursor_pos_ = 0;

    refresh();
    Show();

    return true;
}

const SKKCandidate& SKKWindowSelector::Current() const {
    assert(!view_.empty());

    return view_[cursor_pos_];
}

bool SKKWindowSelector::IsEmpty() const {
    return range_.empty();
}

void SKKWindowSelector::CursorLeft() {
    if(cursor_pos_ != minPosition()) {
	-- cursor_pos_;
	Show();
    }
}

void SKKWindowSelector::CursorRight() {
    if(cursor_pos_ != maxPosition()) {
	++ cursor_pos_;
	Show();
    }
}

void SKKWindowSelector::CursorUp() {
    cursor_pos_ = minPosition();
    Show();
}

void SKKWindowSelector::CursorDown() {
    cursor_pos_ = maxPosition();
    Show();
}

bool SKKWindowSelector::Select(char label) {
    int index = window_->LabelIndex(label);

    if(-1 < index && (unsigned)index < view_.size()) {
	cursor_pos_ = index;
	Show();
	return true;
    }

    return false;
}

void SKKWindowSelector::Show() {
    window_->Update(view_.begin(), view_.end(), cursor_pos_, page_pos_ + 1, (int)pages_.size());
    window_->Show();
}

void SKKWindowSelector::Hide() {
    window_->Hide();
}

void SKKWindowSelector::refresh() {
    if(!pages_.empty()) {
	view_ = PageRange(range_, offset_, pages_[page_pos_]);
    }
}

int SKKWindowSelector::minPage() const {
    return 0;
}

int SKKWindowSelector::maxPage() const {
    return (int)(pages_.size() - 1);
}

int SKKWindowSelector::minPosition() const {
    return 0;
}

int SKKWindowSelector::maxPosition() const {
    return view_.size() - 1;
}
