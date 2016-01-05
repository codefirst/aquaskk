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

#include "MacCloudStrategy.h"
#include <set>
#include "ObjCUtil.h"

MacCloudStrategy::~MacCloudStrategy() {
    [database_ release];
}

void MacCloudStrategy::Initialize(SKKDictionaryFile& file) {
    NSLog(@"MacCloudStrategy::Initialize");

    removed_entries_.clear();
    database_ = [[CloudKitDatabase alloc] init];
    file_ = &file;
    timer_ = std::auto_ptr<pthread::timer>(new pthread::timer(this, 300));
}

void MacCloudStrategy::Save() {
    NSLog(@"MacCloudStrategy::Save");
}

void MacCloudStrategy::Register(const SKKEntry& entry, const SKKCandidate& candidate) {
    NSLog(@"MacCloudStrategy::Register");
    restored_entries_.push_back(DictionaryEntry(entry, candidate));
}

void MacCloudStrategy::Remove(const SKKEntry& entry, const SKKCandidate& candidate) {
    NSLog(@"MacCloudStrategy::Remove");
    removed_entries_.push_back(DictionaryEntry(entry, candidate));
}

bool MacCloudStrategy::run() {
    [database_ fetch:@"OkuriAri" with: ^(NSArray* okuri_ari) {
        [database_ fetch:@"OkuriNasi" with: ^(NSArray* okuri_nasi) {
            [database_ fetch:@"DeletedDictionaryEntry" with:^(NSArray* removed) {
                NSLog(@"iCloud: OkuriAri = %lu, OkuriNasi = %lu, Removed = %lu",
                      (unsigned long)[okuri_ari count],
                      (unsigned long)[okuri_nasi count],
                      (unsigned long)[removed count]);

                int recv = update_local_dictionary(okuri_ari, okuri_nasi, removed);
                int sent = update_remote_dictionary(okuri_ari, okuri_nasi, removed);

                cleanup(okuri_ari);
                cleanup(okuri_nasi);

                notify(recv, sent);
            }];
        }];
    }];
    return true;
}

void MacCloudStrategy::notify(int received_count, int sent_count) {
    if(received_count == 0 && sent_count == 0) {
        return;
    }

    NSUserNotification *notification = [[NSUserNotification alloc] init];
    notification.title = @"AquaSKK同期";
    notification.subtitle = [NSString stringWithFormat: @"%d件を取得 / %d件を送信", received_count, sent_count];
    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
    [notification release];
}

// ----------------------------------------------------------------------
//  iCloud -> local
// ----------------------------------------------------------------------
int MacCloudStrategy::update_local_dictionary(NSArray* okuri_ari, NSArray* okuri_nasi, NSArray* removed_entries) {
    int count = 0;

    // リモートで追加された分を更新する
    count += update_entries(file_->OkuriAri(), okuri_ari);
    count += update_entries(file_->OkuriNasi(), okuri_nasi);

    // 削除されたエントリをローカルに反映する
    remove_entries(removed_entries_);
    count += remove_entries(removed_entries);

    // 削除されたエントリのうち、再登録された分を復活させる
    restore_entries(restored_entries_);

    return count;
}

int MacCloudStrategy::update_entries(SKKDictionaryEntryContainer& container, NSArray* records) {
    int count = 0;
    for (CKRecord* record in records) {
        SKKEntry entry([record[@"entry"] UTF8String]);

        // 候補が空の場合は適当に補完する
        NSString* candidates = record[@"candidates"];
        if([candidates isEqualToString:@""]) {
            candidates = @"//";
        }
        SKKCandidateSuite suite([candidates UTF8String]);

        bool result = add_entry(entry, suite, container);
        if(result) {
            NSLog(@"-> %@ %@", record[@"entry"], candidates);
            count ++;
        }
    }
    return count;
}

int MacCloudStrategy::remove_entries(NSArray* records) {
    int count = 0;

    for (CKRecord* record in records) {
        std::string entry = [record[@"entry"] UTF8String];
        std::string candidate = [record[@"candidate"] UTF8String];
        SKKDictionaryEntryContainer& container =
        [record[@"okuri"] isEqualToNumber: @1] ? file_->OkuriAri() : file_->OkuriNasi();

        bool result = remove_entry(entry, candidate, container);

        if(result) {
            count ++;
        }
    }
    return 0;
}

int MacCloudStrategy::remove_entries(DictionaryContainer& entries) {
    int count = 0;
    for(DictionaryContainer::iterator it = entries.begin(); it != entries.end(); ++it) {
        SKKDictionaryEntryContainer& container =
        it->first.IsOkuriAri() ? file_->OkuriAri() : file_->OkuriNasi();

        bool result = remove_entry(it->first, it->second.ToString(), container);

        if(result) {
            count ++;
        }
    }
    return count;
}

int MacCloudStrategy::restore_entries(DictionaryContainer& entries) {
    int count = 0;
    for(DictionaryContainer::iterator it = entries.begin(); it != entries.end(); ++it) {
        SKKDictionaryEntryContainer& container =
        it->first.IsOkuriAri() ? file_->OkuriAri() : file_->OkuriNasi();

        SKKCandidateSuite suite;
        suite.Add(it->second);

        bool result = add_entry(it->first, suite, container);
        if(result) {
            count ++;
        }
    }
    return count;
}

// ----------------------------------------------------------------------
//  local -> iCloud
// ----------------------------------------------------------------------
int MacCloudStrategy::update_remote_dictionary(NSArray* okuri_ari, NSArray* okuri_nasi, NSArray* removed_entries) {
    int count = 0;

    // 更新分をリモートに送信する
    count += send_entries(@"OkuriAri", file_->OkuriAri(), okuri_ari);
    count += send_entries(@"OkuriNasi", file_->OkuriNasi(), okuri_nasi);

    // 空になった項目はリモートから削除する
    remove_remote_entries(file_->OkuriAri(), okuri_ari);
    remove_remote_entries(file_->OkuriNasi(), okuri_nasi);

    send_removed_entries();
    send_restored_entries(removed_entries);

    // キャッシュを破棄する
    restored_entries_.clear();
    removed_entries_.clear();

    return count;
}

int MacCloudStrategy::send_entries(NSString* record_type, SKKDictionaryEntryContainer& container, NSArray* records) {
    int count = 0;
    for(SKKDictionaryEntryIterator it = container.begin();
        it != container.end();
        ++it) {
        NSString* entry = ObjC::nsstring(it->first);
        NSString* candidates = ObjC::nsstring(it->second);

        CKRecord* record = [CloudKitDatabase find:entry key:@"entry" in:records];

        if(!record) {
            NSLog(@"<- %@ %@", entry, candidates);

            CKRecord* newRecord = [[CKRecord alloc] initWithRecordType:record_type];
            newRecord[@"entry"] = entry;
            newRecord[@"candidates"] = candidates;

            [database_ create:newRecord];

            [newRecord release];

            count ++;
        } else if(![candidates isEqualToString: record[@"candidates"]]) {
            NSLog(@"%@ okuri=%@ <= %@ %@", record[@"candidates"], record[@"okuri"], entry, candidates);

            record[@"candidates"] = candidates;
            [database_ update:record];

            count ++;
        }
    }

    return count;
}

int MacCloudStrategy::remove_remote_entries(SKKDictionaryEntryContainer& container, NSArray* records) {
    int count = 0;
    for (CKRecord* record in records) {
        SKKDictionaryEntryIterator it = find_entry(container, [record[@"entry"] UTF8String]);

        if(it == container.end()) {
            [database_ remove:record];
            count ++;
        }
    }
    return count;
}

int MacCloudStrategy::send_removed_entries() {
    int count = removed_entries_.size();
    for(DictionaryContainer::iterator it = removed_entries_.begin();
        it != removed_entries_.end();
        ++it) {
        CKRecord* record = [[CKRecord alloc] initWithRecordType:@"DeletedDictionaryEntry"];
        record[@"entry"] = ObjC::nsstring(it->first.EntryString());
        record[@"candidate"] = ObjC::nsstring(it->second.ToString());
        record[@"okuri"] = it->first.IsOkuriAri() ? @1 : @0;

        [database_ update:record];

        [record release];
    }
    return count;
}

int MacCloudStrategy::send_restored_entries(NSArray* records) {
    int count = 0;

    for(DictionaryContainer::iterator it = restored_entries_.begin(); it != restored_entries_.end(); ++it) {
        NSString* entry = ObjC::nsstring(it->first.EntryString());
        NSString* candidate = ObjC::nsstring(it->second.ToString());
        NSNumber* okuri = it->first.IsOkuriAri() ? @1 : @0;

        for (CKRecord* record in records) {
            if([entry isEqualToString:record[@"entry"]] &&
               [candidate isEqualToString:record[@"candidate"]] &&
               [okuri isEqualToNumber:record[@"okuri"]]) {
                [database_ remove:record];
                count ++;
            }
        }
    }

    return count;
}

// ----------------------------------------------------------------------
//  cleanup
// ----------------------------------------------------------------------
void MacCloudStrategy::cleanup(NSArray* records) {
    std::set<std::string> already;

    for (CKRecord* record in records) {
        std::string entry([record[@"entry"] UTF8String]);

        if(already.find(entry) != already.end()) {
            [database_ remove:record];
        }
        already.insert(entry);
    }
}