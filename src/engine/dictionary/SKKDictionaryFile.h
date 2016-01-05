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

#ifndef SKKDictionaryFile_h
#define SKKDictionaryFile_h

#include <string>
#include <deque>
#include <iosfwd>
#include <utility>
#include "SKKDictionaryEntry.h"

// SKK 辞書ファイル
class SKKDictionaryFile {
    SKKDictionaryEntryContainer okuriAri_;
    SKKDictionaryEntryContainer okuriNasi_;

    bool exist(const std::string& path);
    bool fetch(std::istream& is, SKKDictionaryEntry& entry);
    bool store(std::ostream& os, const SKKDictionaryEntryContainer& container);
    void sort(SKKDictionaryEntryContainer& container);

public:
    bool Load(const std::string& path);
    bool Save(const std::string& path);
    bool IsEmpty() const;

    void Sort();

    SKKDictionaryEntryContainer& OkuriAri();
    SKKDictionaryEntryContainer& OkuriNasi();
};

// SKKDictionaryEntry を比較するファンクタ(Effective STL, p.99)
class SKKDictionaryEntryCompare {
    bool keyLess(const SKKDictionaryEntry::first_type& key1,
		 const SKKDictionaryEntry::first_type& key2) const {
	return key1 < key2;
    }

public:
    // ソート用比較関数
    bool operator()(const SKKDictionaryEntry& lhs,
		    const SKKDictionaryEntry& rhs) const {
	return keyLess(lhs.first, rhs.first);
    }

    // 探索用比較関数(その1)
    bool operator()(const SKKDictionaryEntry& lhs,
		    const SKKDictionaryEntry::first_type& key) const {
	return keyLess(lhs.first, key);
    }

    // 探索用比較関数(その2)
    bool operator()(const SKKDictionaryEntry::first_type& key,
		    const SKKDictionaryEntry& rhs) const {
	return keyLess(key, rhs.first);
    }
};

#endif
