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

#ifndef MacCloudStrategy_h
#define MacCloudStrategy_h

#include <string>
#include <vector>
#include "pthreadutil.h"
#include "SKKSyncStrategy.h"
#import "CloudKitDatabase.h"

class MacCloudStrategy : public SKKSyncStrategy, public pthread::task {
    typedef std::pair<SKKEntry, SKKCandidate> DictionaryEntry;
    typedef std::vector<DictionaryEntry> DictionaryContainer;

    CloudKitDatabase* database_;
    SKKDictionaryFile* file_;
    std::auto_ptr<pthread::timer> timer_;
    DictionaryContainer restored_entries_;
    DictionaryContainer removed_entries_;

    // iCloudのデータでローカルの辞書を更新する
    int update_local_dictionary(NSArray* okuri_ari, NSArray* okuri_nasi, NSArray* removed_entries);

    // ローカルの辞書でiCloudのデータを更新する
    int update_remote_dictionary(NSArray* okuri_ari, NSArray* okuri_nasi, NSArray* removed_entries);

    int update_entries(SKKDictionaryEntryContainer& container, NSArray* records);
    int remove_entries(NSArray* records);
    int remove_entries(DictionaryContainer& entries);
    int restore_entries(DictionaryContainer& entries);

    int send_entries(NSString* record_type, SKKDictionaryEntryContainer& container, NSArray* records);
    int remove_remote_entries(SKKDictionaryEntryContainer& container, NSArray* records);

    int send_removed_entries();
    int send_restored_entries(NSArray* records);

    void cleanup(NSArray* records);

    void notify(int received_count, int sent_count);

public:
    virtual ~MacCloudStrategy();
    virtual void Initialize(SKKDictionaryFile& file);
    virtual void Save();
    virtual void Register(const SKKEntry& entry, const SKKCandidate& candidate);
    virtual void Remove(const SKKEntry& entry, const SKKCandidate& candidate);

    virtual bool run();
};

#endif 
