/* -*- C++ -*-

   MacOS X implementation of the SKK input method.

   Copyright (C) 2007 Tomotaka SUWA <t.suwa@mac.com>

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

#include <iostream>
#include "SKKEvent.h"
#include "SKKKeyState.h"
#include "SKKKeymapEntry.h"

// エントリーのタイプ
enum KeymapEntryType {
    TYPE_EVENT,                 // SKK_* イベント
    TYPE_ATTRIBUTE,             // SKK_CHAR の属性
    TYPE_HANDLE_OPTION          // 処理オプション
};

// ======================================================================
// キーマップシンボルテーブル
// ======================================================================
static const struct {
    const char* name;
    int symbol;
    int type;
} KeymapTable[] = {
    { "SKK_JMODE",              SKK_JMODE,              TYPE_EVENT },
    { "SKK_ENTER",              SKK_ENTER,              TYPE_EVENT },
    { "SKK_CANCEL",             SKK_CANCEL,             TYPE_EVENT },
    { "SKK_BACKSPACE",          SKK_BACKSPACE,          TYPE_EVENT },
    { "SKK_DELETE",             SKK_DELETE,             TYPE_EVENT },
    { "SKK_TAB",                SKK_TAB,                TYPE_EVENT },
    { "SKK_PASTE",              SKK_PASTE,              TYPE_EVENT },
    { "SKK_LEFT",               SKK_LEFT,               TYPE_EVENT },
    { "SKK_RIGHT",              SKK_RIGHT,              TYPE_EVENT },
    { "SKK_UP",                 SKK_UP,                 TYPE_EVENT },
    { "SKK_DOWN",               SKK_DOWN,               TYPE_EVENT },
    { "SKK_CHAR",               SKK_CHAR,               TYPE_EVENT },
    { "SKK_PING",               SKK_PING,               TYPE_EVENT },
    { "SKK_YES",                SKK_YES,                TYPE_EVENT },
    { "SKK_NO",                 SKK_NO,                 TYPE_EVENT },
    { "SKK_UNDO",               SKK_UNDO,               TYPE_EVENT },

    { "Direct",                 Direct,                 TYPE_ATTRIBUTE },
    { "UpperCases",             UpperCases,             TYPE_ATTRIBUTE },
    { "ToggleKana",             ToggleKana,             TYPE_ATTRIBUTE },
    { "ToggleJisx0201Kana",     ToggleJisx0201Kana,     TYPE_ATTRIBUTE },
    { "SwitchToAscii",          SwitchToAscii,          TYPE_ATTRIBUTE },
    { "SwitchToJisx0208Latin",  SwitchToJisx0208Latin,  TYPE_ATTRIBUTE },
    { "EnterJapanese",          EnterJapanese,          TYPE_ATTRIBUTE },
    { "EnterAbbrev",            EnterAbbrev,            TYPE_ATTRIBUTE },
    { "NextCompletion",         NextCompletion,         TYPE_ATTRIBUTE },
    { "PrevCompletion",         PrevCompletion,         TYPE_ATTRIBUTE },
    { "NextCandidate",          NextCandidate,          TYPE_ATTRIBUTE },
    { "PrevCandidate",          PrevCandidate,          TYPE_ATTRIBUTE },
    { "RemoveTrigger",          RemoveTrigger,          TYPE_ATTRIBUTE },
    { "InputChars",             InputChars,             TYPE_ATTRIBUTE },
    { "CompConversion",         CompConversion,         TYPE_ATTRIBUTE },
    { "PrefixSuffixConversion", PrefixSuffixConversion, TYPE_ATTRIBUTE },

    { "AlwaysHandled",          AlwaysHandled,          TYPE_HANDLE_OPTION },
    { "PseudoHandled",          PseudoHandled,          TYPE_HANDLE_OPTION },

    { 0x00,                     SKK_NULL,               -1 }
};

// 検索
int fetchKeymapIndex(const std::string& key) {
    for(int i = 0; KeymapTable[i].name != 0x00; ++ i) {
	if(key == KeymapTable[i].name) {
	    return i;
	}
    }

    return -1;
}

// 文字列一括置換ユーティリティ
void translate(std::string& src, const std::string& from, const std::string& to) {
    std::string::size_type pos = 0;

    while((pos = src.find(from, pos)) != std::string::npos) {
	src.replace(pos, from.size(), to);
    }
}

// ======================================================================
// SKKKeymapEntry インタフェース
// ======================================================================
SKKKeymapEntry::SKKKeymapEntry() : pos_(0) {}

SKKKeymapEntry::SKKKeymapEntry(const std::string& configKey, const std::string& configValue) : pos_(0), not_(false) {
    std::string key(configKey);

    if(configKey.find("Not") == 0) {
        not_ = true;
        key = configKey.substr(3);
    }

    int index = fetchKeymapIndex(key);
    if(index < 0) {
	std::cout << "SKKKeymapEntry::SKKKeymapEntry(): invalid key name[" << configKey << "]" << std::endl;
	return;
    }

    type_= KeymapTable[index].type;
    symbol_ = KeymapTable[index].symbol;

    // 全ての "||" を空白に置換
    std::string tmp(configValue);
    translate(tmp, "||", " ");

    // 各エントリをパース
    std::istringstream buf(tmp);
    while(buf >> tmp) {
	tmp = setup(tmp);

	if(label_ & LABEL_GROUP) {
	    parseGroup(tmp);
	} else {
	    parseEntry(tmp);
	}
    }
}

// キーの読み出し
bool SKKKeymapEntry::operator>>(int& state) {
    if(!(pos_ < keys_.size())) return false;

    if(keys_[pos_].first <= keys_[pos_].second) {
	state = keys_[pos_].first ++;
	return true;
    } else {
	++ pos_;
	return *this >> state; // 再帰
    }
}

bool SKKKeymapEntry::IsEvent() const {
    return type_ == TYPE_EVENT;
}

bool SKKKeymapEntry::IsAttribute() const {
    return type_ == TYPE_ATTRIBUTE;
}

bool SKKKeymapEntry::IsNot() const {
    return not_;
}

int SKKKeymapEntry::Symbol() const {
    return symbol_;
}

// ======================================================================
// private method
// ======================================================================
std::string SKKKeymapEntry::setup(const std::string& str) {
    label_ = mods_ = 0;

    // 全ての "::" を空白に置換
    std::string tmp(str);
    translate(tmp, "::", " ");

    // 各ラベルを解析する
    std::istringstream buf(tmp);
    while(buf >> tmp) {
	if(tmp == "group")	label_ |= LABEL_GROUP;
	if(tmp == "hex")	label_ |= LABEL_HEX;
	if(tmp == "keycode")	label_ |= LABEL_KEYCODE;

	if(tmp == "shift")	mods_ |= SKKKeyState::SHIFT;
	if(tmp == "ctrl")	mods_ |= SKKKeyState::CTRL;
	if(tmp == "alt")	mods_ |= SKKKeyState::ALT;
	if(tmp == "meta")	mods_ |= SKKKeyState::META;
    }

    // ラベルを取り除いたキー情報を返す
    return tmp;
}

// グループエントリの解析
void SKKKeymapEntry::parseGroup(const std::string& str) {
    // 全ての ',' を空白に置換
    std::string tmp(str);
    std::replace(tmp.begin(), tmp.end(), ',', ' ');

    // 各エントリを解析
    std::istringstream buf(tmp);
    while(buf >> tmp) {
	std::string::size_type pos = tmp.find_first_of('-');

	// 範囲コードか？
	if(pos != std::string::npos) {
	    tmp[pos] = ' ';
	    parseRange(tmp);
	} else {
	    parseEntry(tmp);
	}
    }
}

// 範囲コードの解析
void SKKKeymapEntry::parseRange(const std::string& str) {
    std::string tmp(str);
    std::istringstream buf(tmp);
    KeyRange range;

    if(buf >> tmp) {
	range.first = makeKey(tmp);

	if(buf >> tmp) {
	    range.second = makeKey(tmp);
	    keys_.push_back(range);
	}
    }
}

// 単一エントリの解析
void SKKKeymapEntry::parseEntry(const std::string& str) {
    KeyRange range;

    range.first = range.second = makeKey(str);

    keys_.push_back(range);
}

// キーの生成
int SKKKeymapEntry::makeKey(const std::string& str) {
    int key;

    // 16 進数表記？
    if(label_ & (LABEL_HEX | LABEL_KEYCODE)) {
	std::istringstream buf(str);
	buf >> std::hex >> key;
    } else {
	key = str[0];
    }

    if(label_ & LABEL_KEYCODE) {
	return SKKKeyState::KeyCode(key, mods_);
    } else {
	return SKKKeyState::CharCode(key, mods_);
    }
}
