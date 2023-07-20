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

#include "SKKCompleter.h"
#include "SKKBackEnd.h"

SKKCompleter::SKKCompleter(SKKCompleterBuddy* buddy) : buddy_(buddy) {}

bool SKKCompleter::Execute(int limit) {
    if(complete(limit)) {
	notify();
    }

    return !completions_.empty();
}

bool SKKCompleter::Remove() {
    if(completions_.empty()) return false;

    SKKBackEnd& backend = SKKBackEnd::theInstance();

    backend.Remove(completions_[pos_], SKKCandidate());

    SKKCandidateSuite tmp;

    return !backend.Find(completions_[pos_], tmp);
}

void SKKCompleter::Next() {
    if(completions_.empty()) return;

    if(maxPosition() < ++ pos_) {
	pos_ = minPosition();
    }

    notify();
}

void SKKCompleter::Prev() {
    if(completions_.empty()) return;

    if(-- pos_ < minPosition()) {
	pos_ = maxPosition();
    }

    notify();
}

bool SKKCompleter::complete(int limit) {
    std::string query(buddy_->SKKCompleterQueryString());

    pos_ = 0;
    completions_.clear();

    return SKKBackEnd::theInstance().Complete(query, completions_, limit);
}

int SKKCompleter::minPosition() const {
    return 0;
}

int SKKCompleter::maxPosition() const {
    return (int)(completions_.size() - 1);
}

void SKKCompleter::notify() {
    buddy_->SKKCompleterUpdate(completions_[pos_]);
}
