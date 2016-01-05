/* -*- C++ -*-

 MacOS X implementation of the SKK input method.

 Copyright (C) 2015 mzp <mzpppp@gmail.com>

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

#ifndef SKKDictionaryEntry_h
#define SKKDictionaryEntry_h

#include "SKKCandidateSuite.h"
#include "SKKEntry.h"


// 「見出し語」と「変換候補」のペア(変換候補は分解する前の状態)
typedef std::pair<std::string, std::string> SKKDictionaryEntry;

// エントリのコンテナ
typedef std::deque<SKKDictionaryEntry> SKKDictionaryEntryContainer;
typedef SKKDictionaryEntryContainer::iterator SKKDictionaryEntryIterator;


// SKKDictionaryEntry と文字列を比較するファンクタ
class CompareUserDictionaryEntry: public std::unary_function<SKKDictionaryEntry, bool> {
    const std::string str_;

public:
    CompareUserDictionaryEntry(const std::string& str);
    bool operator()(const SKKDictionaryEntry& entry) const;
};

// 逆引き用ファンクタ
class NotIncludeDicionaryEntry {
    std::string candidate_;

public:
    NotIncludeDicionaryEntry(const std::string& candidate);

    bool operator()(const SKKDictionaryEntry& entry) const;
};

SKKDictionaryEntryIterator find_entry(SKKDictionaryEntryContainer& container, const std::string& query);

template <typename T>
void update_entry(const SKKEntry& entry, const T& obj, SKKDictionaryEntryContainer& container) {
    SKKCandidateSuite suite;
    const std::string& index = entry.EntryString();
    SKKDictionaryEntryIterator iter = find_entry(container, index);

    if(iter != container.end()) {
        suite.Parse(iter->second);
        container.erase(iter);
    }

    suite.Update(obj);

    container.push_front(SKKDictionaryEntry(index, suite.ToString()));
}

bool add_entry(const SKKEntry& entry, SKKCandidateSuite& suite, SKKDictionaryEntryContainer& container);
bool remove_entry(const SKKEntry& entry, const std::string& candidate, SKKDictionaryEntryContainer& container);


#endif