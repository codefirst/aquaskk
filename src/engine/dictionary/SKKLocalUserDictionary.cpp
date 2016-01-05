/*

  MacOS X implementation of the SKK input method.

  Copyright (C) 2010 Tomotaka SUWA <tomotaka.suwa@gmail.com>

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

#include "SKKLocalUserDictionary.h"
#include "SKKCandidateSuite.h"
#include "utf8util.h"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <ctime>

namespace {
    static const int MAX_IDLE_COUNT = 20;
    static const int MAX_SAVE_INTERVAL = 60 * 5;
}

SKKLocalUserDictionary::SKKLocalUserDictionary() : privateMode_(false) {}

SKKLocalUserDictionary::~SKKLocalUserDictionary() {
    // 強制保存
    if(!path_.empty()) save(true);
}

void SKKLocalUserDictionary::Initialize(const std::string& path) {
    if(!path_.empty()) {
        if(path_ == path) {
            return;
        }

        save(true);
    }

    path_ = path;
    idle_count_ = 0;
    lastupdate_ = std::time(0);

    if(!file_.Load(path)) {
	std::cerr << "SKKLocalUserDictionary: can't load file: " << path << std::endl;
    }

    fix();
}

void SKKLocalUserDictionary::Find(const SKKEntry& entry, SKKCandidateSuite& result) {
    SKKCandidateSuite suite;

    if(entry.IsOkuriAri()) {
        suite.Parse(fetch(entry, file_.OkuriAri()));

        SKKCandidateSuite strict;

        if(suite.FindOkuriStrictly(entry.OkuriString(), strict)) {
            strict.Add(suite.Hints());
            suite = strict;
        }
    } else {
        suite.Parse(fetch(entry, file_.OkuriNasi()));
        
        SKKCandidateContainer& candidates = suite.Candidates();

        std::for_each(candidates.begin(), candidates.end(),
                      std::mem_fun_ref(&SKKCandidate::Decode));
    }

    result.Add(suite);
}

std::string SKKLocalUserDictionary::ReverseLookup(const std::string& candidate) {
    SKKDictionaryEntryContainer& container = file_.OkuriNasi();
    SKKDictionaryEntryContainer entries;
    SKKCandidateParser parser;

    std::remove_copy_if(container.begin(), container.end(),
                        std::back_inserter(entries), NotIncludeDicionaryEntry("/" + candidate));

    for(unsigned i = 0; i < entries.size(); ++ i) {
        parser.Parse(entries[i].second);
        const SKKCandidateContainer& suite = parser.Candidates();

        if(std::find(suite.begin(), suite.end(), candidate) != suite.end()) {
            return entries[i].first;
        }
    }

    return "";
}

void SKKLocalUserDictionary::Complete(SKKCompletionHelper& helper) {
    const std::string& entry = helper.Entry();
    SKKDictionaryEntryContainer& container = file_.OkuriNasi();

    for(SKKDictionaryEntryIterator iter = container.begin(); iter != container.end(); ++ iter) {
        if(iter->first.compare(0, entry.length(), entry) != 0) continue;

        helper.Add(iter->first);

        if(!helper.CanContinue()) break;
    }
}

void SKKLocalUserDictionary::Register(const SKKEntry& entry, const SKKCandidate& candidate) {
    if(entry.IsOkuriAri()) {
        SKKOkuriHint hint;

        hint.first = entry.OkuriString();
        hint.second.push_back(candidate.ToString());

        update_entry(entry.EntryString(), hint, file_.OkuriAri());
    } else {
        SKKCandidate tmp(candidate);

        tmp.Encode();

        update_entry(entry.EntryString(), tmp, file_.OkuriNasi());
    }

    save();
}

void SKKLocalUserDictionary::Remove(const SKKEntry& entry, const SKKCandidate& candidate) {
    if(entry.IsOkuriAri()) {
        remove(entry, candidate.ToString(), file_.OkuriAri());
    } else {
        SKKCandidate tmp(candidate);

        tmp.Encode();
    
        remove(entry, tmp.ToString(), file_.OkuriNasi());
    }
        
    save();
}

void SKKLocalUserDictionary::SetPrivateMode(bool flag) {
    if(privateMode_ != flag) {
        if(flag) {
            save(true);
        } else {
            file_.Load(path_);
        }

        privateMode_ = flag;
    }
}

SKKDictionaryFile& SKKLocalUserDictionary::GetDicionaryFile() {
    return file_;
}

// ======================================================================
// private method
// ======================================================================

std::string SKKLocalUserDictionary::fetch(const SKKEntry& entry, SKKDictionaryEntryContainer& container) {
    SKKDictionaryEntryIterator iter = find_entry(container, entry.EntryString());

    if(iter == container.end()) {
	return std::string();
    }

    return iter->second;
}

void SKKLocalUserDictionary::remove(const SKKEntry& entry, const std::string& kanji,
			       SKKDictionaryEntryContainer& container) {
    remove_entry(entry, kanji, container);
}

void SKKLocalUserDictionary::save(bool force) {
    if(privateMode_) return;

    if(!force && ++ idle_count_ < MAX_IDLE_COUNT && std::time(0) - lastupdate_ < MAX_SAVE_INTERVAL) {
	return;
    }

    idle_count_ = 0;
    lastupdate_ = std::time(0);

    std::string tmp_path = path_ + ".tmp";
    if(!file_.Save(tmp_path)) {
	std::cout << "SKKLocalUserDictionary: can't save: " << tmp_path << std::endl;
	return;
    }

    if(rename(tmp_path.c_str(), path_.c_str()) < 0) {
	std::cout << "SKKLocalUserDictionary: rename() failed[" << std::strerror(errno) << "]" << std::endl;
    } else {
	std::cout << "SKKLocalUserDictionary: saved" << std::endl;
    }
}

void SKKLocalUserDictionary::fix() {
    SKKDictionaryEntryContainer& container = file_.OkuriNasi();
    SKKDictionaryEntryIterator iter = find_entry(container, "#");

    // ユーザー辞書の "#" は無意味なのでまるごと削除する 
    if(iter != container.end()) {
        container.erase(iter);
    }
}
