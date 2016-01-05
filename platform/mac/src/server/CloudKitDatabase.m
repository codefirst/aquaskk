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

#import <Foundation/Foundation.h>
#import "CloudKitDatabase.h"

@implementation CloudKitDatabase

- (id)init {
    if(self = [super init]) {
        database_ = [[CKContainer defaultContainer] privateCloudDatabase];
    }
    return self;
}

- (void)fetch:(NSString*)recordName with:(void (^)(NSArray* records))with {
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"updatedAt > %@", [NSDate dateWithTimeIntervalSince1970:0]];
    CKQuery* query = [[CKQuery alloc] initWithRecordType:recordName predicate:predicate];

    CKQueryOperation* operation = [[CKQueryOperation alloc] initWithQuery:query];

    [self fetch:operation records:[[NSMutableArray alloc] init] with:with];

    [query release];
    [operation release];
}

- (void)fetch:(CKQueryOperation*)operation records:(NSMutableArray*)records with:(void (^)(NSArray* records))with {
    operation.recordFetchedBlock = ^(CKRecord* record) {
        [records addObject:record];
    };

    operation.queryCompletionBlock = ^(CKQueryCursor* cursor, NSError* error) {
        if(error) {
            NSLog(@"fetch error: %@", error);
            return;
        }

        if(cursor) {
            // 途中の場合は取得を継続する
            CKQueryOperation* operation = [[CKQueryOperation alloc] initWithCursor:cursor];
            [self fetch:operation records:records with:with];
            [operation release];
        } else {
            // 最後まで来た
            with(records);
            [records release];
        }
    };

    [database_ addOperation:operation];
}

- (void)update:(CKRecord*)record {
    record[@"updatedAt"] = [NSDate date];
    [database_ saveRecord:record completionHandler:^(CKRecord *record, NSError *error) {
        if(error) {
            NSLog(@"[CloudKit]Update new record error: %@", error);
        }
    }];
}

- (void)create:(CKRecord*)record {
    record[@"updatedAt"] = [NSDate date];
    [database_ saveRecord:record completionHandler:^(CKRecord *record, NSError *error) {
        if(error) {
            NSLog(@"[CloudKit]Create new record error: %@", error);
        }
    }];
}

- (void)remove:(CKRecord*)record {
    [database_ deleteRecordWithID:record.recordID completionHandler: ^(CKRecordID* recordID, NSError* error) {
        if(error) {
            NSLog(@"[CloudKit]Delete new record error: %@", error);
        }
    }];
}

+ (CKRecord*)find:(NSString*)recordID key:(NSString*)key in:(NSArray*)records {
    for (CKRecord* record in records) {
        if([record[key] isEqualToString:recordID]) {
            return record;
        }
    }
    return nil;
}

@end