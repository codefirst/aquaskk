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

#include <stdio.h>
#include "SKKDictionaryEntry.h"

CompareUserDictionaryEntry::CompareUserDictionaryEntry(const std::string& str) : str_(str) {
}

bool CompareUserDictionaryEntry::operator()(const SKKDictionaryEntry& entry) const {
    return entry.first == str_;
}

NotIncludeDicionaryEntry::NotIncludeDicionaryEntry(const std::string& candidate) : candidate_(candidate) {
}

bool NotIncludeDicionaryEntry::operator()(const SKKDictionaryEntry& entry) const {
    return entry.second.find(candidate_) == std::string::npos;
}

SKKDictionaryEntryIterator find_entry(SKKDictionaryEntryContainer& container, const std::string& query) {
    return std::find_if(container.begin(), container.end(),
                        CompareUserDictionaryEntry(query));
}

static std::string toString(SKKCandidateSuite& suite) {
    std::string str = suite.ToString();

    if(str.empty()) {
        return "//";
    } else {
        return str;
    }
}

bool add_entry(const SKKEntry& entry, SKKCandidateSuite& candidates, SKKDictionaryEntryContainer& container) {
    const std::string& index = entry.EntryString();
    SKKDictionaryEntryIterator iter = find_entry(container, index);
    SKKCandidateSuite suite;

    if(iter != container.end()) {
        if(iter->second == toString(candidates)) {
            // skip
            return false;
        }
        suite.Parse(iter->second);
        container.erase(iter);
    }

    suite.Add(candidates);

    container.push_front(SKKDictionaryEntry(index, toString(suite)));
    return true;
}

bool remove_entry(const SKKEntry& entry, const std::string& candidate, SKKDictionaryEntryContainer& container) {
    SKKDictionaryEntryIterator iter = find_entry(container, entry.EntryString());

    if(iter == container.end()) {
        return false;
    }

    SKKCandidateSuite suite;
    suite.Parse(iter->second);
    suite.Remove(candidate);

    if(suite.IsEmpty()) {
        container.erase(iter);
    } else {
        iter->second = suite.ToString();
    }
    return true;
}