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

#include "SKKEntry.h"
#include "jconv.h"

SKKEntry::SKKEntry() {}

SKKEntry::SKKEntry(const std::string& entry, const std::string& okuri)
    : normal_entry_(entry), kana_(okuri) {
    updateEntry();
}

void SKKEntry::SetEntry(const std::string& entry) {
    normal_entry_ = entry;

    if(!normal_entry_.empty()) {
        unsigned long last_index = normal_entry_.size() - 1;

        // 見出し語末尾の prefix を取り除く(ex. "かk" → "か")
        if(normal_entry_.find_last_of(prefix_) == last_index) {
            normal_entry_.erase(last_index);
        }
    }

    updateEntry();
}

void SKKEntry::AppendEntry(const std::string& str) {
    normal_entry_ += str;
}

void SKKEntry::SetOkuri(const std::string& prefix, const std::string& kana) {
    prefix_ = prefix;
    kana_ = kana;

    updateEntry();
}

const std::string& SKKEntry::EntryString() const {
    return IsOkuriAri() ? okuri_entry_ : normal_entry_;
}

const std::string& SKKEntry::OkuriString() const {
    return kana_;
}

const std::string& SKKEntry::PromptString() const {
    return prompt_;
}

std::string SKKEntry::ToggleKana(SKKInputMode mode) const {
    std::string result;

    switch(mode) {
    case HirakanaInputMode:
	jconv::hirakana_to_katakana(normal_entry_, result);
	break;

    case KatakanaInputMode:
	jconv::katakana_to_hirakana(normal_entry_, result);
	break;

    case Jisx0201KanaInputMode:
	jconv::jisx0201_kana_to_katakana(normal_entry_, result);
	break;

    default:
        break;
    }

    return result;
}

std::string SKKEntry::ToggleJisx0201Kana(SKKInputMode mode) const {
    std::string result;

    switch(mode) {
    case HirakanaInputMode:
	jconv::hirakana_to_jisx0201_kana(normal_entry_, result);
	break;
	
    case KatakanaInputMode:
	jconv::katakana_to_jisx0201_kana(normal_entry_, result);
	break;

    case Jisx0201KanaInputMode:
	jconv::jisx0201_kana_to_hirakana(normal_entry_, result);
	break;

    case AsciiInputMode:
	jconv::ascii_to_jisx0208_latin(normal_entry_, result);
	break;

    default:
        break;
    }

    return result;
}

SKKEntry SKKEntry::Normalize(SKKInputMode mode) const {
    SKKEntry entry(*this);
    std::string result = normal_entry_, roman;

    // 入力モードがカタカナ/半角カナなら、見出し語をひらかなに正規化する
    // ACT配列等では入力した文字がSKKの辞書とは一致しないので、カナから変換する。
    switch(mode) {
    case KatakanaInputMode:
        jconv::katakana_to_hirakana(normal_entry_, result);
        jconv::katakana_to_roman(kana_, roman);
        break;

    case Jisx0201KanaInputMode:
        jconv::jisx0201_kana_to_hirakana(normal_entry_, result);
        jconv::jisx0201_kana_to_roman(kana_, roman);
        break;

    default:
        jconv::hirakana_to_roman(kana_, roman);
        break;
    }

    entry.SetEntry(result);

    if(!roman.empty()) {
        entry.SetOkuri(roman.substr(0,1), kana_);
    }

    return entry;
}

bool SKKEntry::IsEmpty() const {
    return normal_entry_.empty();
}

bool SKKEntry::IsOkuriAri() const {
    return !kana_.empty();
}

bool operator==(const SKKEntry& left, const SKKEntry& right) {
    return left.normal_entry_ == right.normal_entry_
        && left.okuri_entry_ == right.okuri_entry_
        && left.prefix_ == right.prefix_
        && left.kana_ == right.kana_
        && left.prompt_ == right.prompt_;
}

// ----------------------------------------------------------------------

void SKKEntry::updateEntry() {
    okuri_entry_ = prompt_ = normal_entry_;
    okuri_entry_ += prefix_;

    if(IsOkuriAri()) {
        prompt_ += "*";
        prompt_ += kana_;
    }
}
