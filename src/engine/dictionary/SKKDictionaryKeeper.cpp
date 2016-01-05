/*

  MacOS X implementation of the SKK input method.

  Copyright (C) 2008,2010 Tomotaka SUWA <tomotaka.suwa@gmail.com>

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

#include "SKKDictionaryKeeper.h"
#include "SKKCandidateParser.h"
#include "jconv.h"
#include "utf8util.h"

namespace {
    // 見出し語補完用比較ファンクタ
    class CompareFunctor {
        int length_;

        bool compare(const std::string& lhs, const std::string& rhs) const {
            return 0 < rhs.compare(0, length_, lhs, 0, length_);
        }

    public:
        CompareFunctor(int length) : length_(length) {}

        bool operator()(const SKKDictionaryEntry& lhs, const SKKDictionaryEntry& rhs) const {
            return compare(lhs.first, rhs.first);
        }

        bool operator()(const SKKDictionaryEntry& lhs, const std::string& rhs) const {
            return compare(lhs.first, rhs);
        }

        bool operator()(const std::string& lhs, const SKKDictionaryEntry& rhs) const {
            return compare(lhs, rhs.first);
        }
    };
}

SKKDictionaryKeeper::SKKDictionaryKeeper(Encoding encoding)
    : timer_(0), loaded_(false), needs_conversion_(encoding == EUC_JP) {}

void SKKDictionaryKeeper::Initialize(SKKDictionaryLoader* loader) {
    if(timer_.get()) return;

    loader->Connect(this);

    timeout_ = loader->Timeout();
    timer_ = std::auto_ptr<pthread::timer>(new pthread::timer(loader, loader->Interval()));
}

std::string SKKDictionaryKeeper::FindOkuriAri(const std::string& query) {
    return fetch(query, file_.OkuriAri());
}

std::string SKKDictionaryKeeper::FindOkuriNasi(const std::string& query) {
    return fetch(query, file_.OkuriNasi());
}

std::string SKKDictionaryKeeper::ReverseLookup(const std::string& candidate) {
    pthread::lock scope(condition_);

    if(!ready()) return "";

    SKKDictionaryEntryContainer& container = file_.OkuriNasi();
    SKKDictionaryEntryContainer entries;
    SKKCandidateParser parser;

    std::remove_copy_if(container.begin(), container.end(),
                        std::back_inserter(entries), NotIncludeDicionaryEntry("/" + eucj_from_utf8(candidate)));

    for(unsigned i = 0; i < entries.size(); ++ i) {
        parser.Parse(utf8_from_eucj(entries[i].second));
        const SKKCandidateContainer& suite = parser.Candidates();

        if(std::find(suite.begin(), suite.end(), candidate) != suite.end()) {
            return utf8_from_eucj(entries[i].first);
        }
    }

    return "";
}

void SKKDictionaryKeeper::Complete(SKKCompletionHelper& helper) {
    pthread::lock scope(condition_);

    if(!ready()) return;

    typedef std::pair<SKKDictionaryEntryIterator, SKKDictionaryEntryIterator> EntryRange;

    SKKDictionaryEntryContainer& container = file_.OkuriNasi();
    std::string query = eucj_from_utf8(helper.Entry());
    EntryRange range = std::equal_range(container.begin(), container.end(),
                                        query, CompareFunctor(query.size()));

    for(SKKDictionaryEntryIterator iter = range.first; iter != range.second; ++ iter) {
        std::string completion = utf8_from_eucj(iter->first);

        helper.Add(completion);

        if(!helper.CanContinue()) return;
    }
}

// ------------------------------------------------------------

void SKKDictionaryKeeper::SKKDictionaryLoaderUpdate(const SKKDictionaryFile& file) {
    pthread::lock scope(condition_);

    file_ = file;

    loaded_ = true;

    condition_.signal();
}

std::string SKKDictionaryKeeper::fetch(const std::string& query, SKKDictionaryEntryContainer& container) {
    pthread::lock scope(condition_);

    if(!ready()) return "";

    std::string index = eucj_from_utf8(query);

    if(!std::binary_search(container.begin(), container.end(), index, SKKDictionaryEntryCompare())) {
	return "";
    }

    SKKDictionaryEntryIterator iter = std::lower_bound(container.begin(), container.end(),
						       index, SKKDictionaryEntryCompare());

    return utf8_from_eucj(iter->second);
}

bool SKKDictionaryKeeper::ready() {
    // 辞書のロードが完了するまで待つ
    if(!loaded_) {
        if(!condition_.wait(timeout_)) return false;
    }

    return true;
}

std::string SKKDictionaryKeeper::eucj_from_utf8(const std::string& src) {
    if(needs_conversion_) {
        return jconv::eucj_from_utf8(src);
    }

    return src;
}

std::string SKKDictionaryKeeper::utf8_from_eucj(const std::string& src) {
    if(needs_conversion_) {
        return jconv::utf8_from_eucj(src);
    }

    return src;
}
