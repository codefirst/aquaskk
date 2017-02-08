/* -*- C++ -*-

   MacOS X implementation of the SKK input method.

   Copyright (C) 2007-2008 Tomotaka SUWA <t.suwa@mac.com>

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

#ifndef SKKEvent_h
#define SKKEvent_h

#include <sstream>
#include "GenericStateMachine.h"

// キー入力イベント
enum {
    SKK_NULL = statemachinecxx_sourceforge_jp::USER_EVENT, // 無効なイベント
    SKK_JMODE,                  // Ctrl-J
    SKK_ENTER,                  // Ctrl-M
    SKK_CANCEL,                 // Ctrl-G
    SKK_BACKSPACE,              // Ctrl-H
    SKK_DELETE,                 // Ctrl-D
    SKK_TAB,                    // Ctrl-I
    SKK_PASTE,                  // Ctrl-Y
    SKK_LEFT,                   // ←
    SKK_RIGHT,                  // →
    SKK_UP,                     // ↑
    SKK_DOWN,                   // ↓
    SKK_CHAR,                   // その他全てのキー入力
    SKK_PING,                   // CTRL-L(内部状態問い合わせ)
    SKK_UNDO,                   // CTRL-/
    SKK_ASCII_MODE,             // ASCII モード
    SKK_HIRAKANA_MODE,          // ひらかなモード
    SKK_KATAKANA_MODE,          // カタカナモード
    SKK_JISX0201KANA_MODE,      // 半角カナモード
    SKK_JISX0208LATIN_MODE,     // 全角英数モード
    SKK_YES,                    // 仮想イベント
    SKK_NO,                     // 仮想イベント
    SKK_ON,                     // 仮想イベント
    SKK_OFF                     // 仮想イベント
};

// SKK_CHAR 属性
enum {
    None,
    Direct                      = (1 << 0),
    UpperCases                  = (1 << 1),
    ToggleKana                  = (1 << 2),
    ToggleJisx0201Kana          = (1 << 3),
    SwitchToAscii               = (1 << 4),
    SwitchToJisx0208Latin       = (1 << 5),
    EnterJapanese               = (1 << 6),
    EnterAbbrev                 = (1 << 7),
    NextCompletion              = (1 << 8),
    PrevCompletion              = (1 << 9),
    NextCandidate               = (1 << 10),
    PrevCandidate               = (1 << 11),
    RemoveTrigger               = (1 << 12),
    InputChars			= (1 << 13),
    CompConversion		= (1 << 14),
    PrefixSuffixConversion      = (1 << 15), // >による接頭語、接尾語変換
};

// 処理オプション
enum {
    Default,
    AlwaysHandled,              // 強制的に「処理済み」にする
    PseudoHandled,              // 処理は行うが「未処理」とする
    CapsLock                    // CapsLock
};

// イベントパラメータ
class SKKEvent {
public:
    int id;                     // イベント(冗長だが仕方がない)
    unsigned char code;         // 文字そのもの
    int attribute;              // SKK_CHAR 属性
    int option;                 // 処理オプション

    SKKEvent() : id(0), code(0), attribute(0), option(0) {}
    SKKEvent(int e, unsigned char c, int a = None) : id(e), code(c), attribute(a), option(0) {}

    // SKK_CHAR 属性問い合わせ
    bool IsDirect() const                       { return attribute & Direct; }
    bool IsUpperCases() const                   { return attribute & UpperCases; }
    bool IsToggleKana() const                   { return attribute & ToggleKana; }
    bool IsToggleJisx0201Kana() const           { return attribute & ToggleJisx0201Kana; }
    bool IsSwitchToAscii() const                { return attribute & SwitchToAscii; }
    bool IsSwitchToJisx0208Latin() const        { return attribute & SwitchToJisx0208Latin; }
    bool IsEnterJapanese() const                { return attribute & EnterJapanese; }
    bool IsEnterAbbrev() const                  { return attribute & EnterAbbrev; }
    bool IsNextCompletion() const               { return attribute & NextCompletion; }
    bool IsPrevCompletion() const               { return attribute & PrevCompletion; }
    bool IsNextCandidate() const                { return attribute & NextCandidate; }
    bool IsPrevCandidate() const                { return attribute & PrevCandidate; }
    bool IsRemoveTrigger() const                { return attribute & RemoveTrigger; }
    bool IsInputChars() const	                { return attribute & InputChars; }
    bool IsCompConversion() const               { return attribute & CompConversion; }
    bool IsPrefixSuffixConversion() const       { return attribute & PrefixSuffixConversion; }

    const static SKKEvent& Null() {
        static SKKEvent obj(SKK_NULL, 0, 0);
        return obj;
    }

    bool operator==(const SKKEvent& rhs) const {
        return (id == rhs.id && code == rhs.code && attribute == rhs.attribute);
    }

    std::string attr() const {
        std::string result;

#define TEST_attribute(attr)	if(Is ## attr()) result += "," #attr

        TEST_attribute(Direct);
        TEST_attribute(UpperCases);
        TEST_attribute(ToggleKana);
        TEST_attribute(ToggleJisx0201Kana);
        TEST_attribute(SwitchToAscii);
        TEST_attribute(SwitchToJisx0208Latin);
        TEST_attribute(EnterJapanese);
        TEST_attribute(EnterAbbrev);
        TEST_attribute(NextCompletion);
        TEST_attribute(PrevCompletion);
        TEST_attribute(NextCandidate);
        TEST_attribute(PrevCandidate);
        TEST_attribute(RemoveTrigger);
        TEST_attribute(InputChars);
        TEST_attribute(CompConversion);
        TEST_attribute(PrefixSuffixConversion);

#undef TEST_attribute

        if(result.empty()) {
            result = "attr=none";
        } else {
            result = "attr=" + result.substr(1);
        }

        return result;
    }

    std::string dump() const {
        const char* eventName[] = {
            "SKK_NULL",
            "SKK_JMODE",
            "SKK_ENTER",
            "SKK_CANCEL",
            "SKK_BACKSPACE",
            "SKK_DELETE",
            "SKK_TAB",
            "SKK_PASTE",
            "SKK_LEFT",
            "SKK_RIGHT",
            "SKK_UP",
            "SKK_DOWN",
            "SKK_CHAR",
            "SKK_PING",
            "SKK_UNDO",
            "SKK_ASCII_MODE",
            "SKK_HIRAKANA_MODE",
            "SKK_KATAKANA_MODE",
            "SKK_JISX0201KANA_MODE",
            "SKK_JISX0208LATIN_MODE",
            "SKK_YES",
            "SKK_NO",
            "SKK_ON",
            "SKK_OFF"
        };

        std::ostringstream buf;
        if(0 <= id - SKK_NULL) {
            buf << "event=" << eventName[id - SKK_NULL] << ", ";
        } else {
            buf << "event=" << id << "(UNKNOWN), ";
        }

        buf << "code=0x" << std::hex << (unsigned)code << ", " << attr();

        return buf.str();
    }
};

#endif
