/* -*- ObjC -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2006-2008 Tomotaka SUWA <t.suwa@mac.com>

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

#include "DictionarySet.h"
#include "SKKConstVars.h"

// ドラッグ & ドロップ用
static NSString* DictionaryRowsType = @"DictionaryRowsType";

@implementation DictionarySet

- (void)moveObjectsInArrangedObjectsFromIndexes:(NSIndexSet*)indexSet toIndex:(unsigned int)insertIndex {
    NSArray* objects = [self arrangedObjects];
    NSUInteger idx = [indexSet lastIndex];
    NSUInteger aboveInsertIndexCount = 0;
    NSUInteger removeIndex;

    while(NSNotFound != idx) {
	if(idx >= insertIndex) {
	    removeIndex = idx + aboveInsertIndexCount;
	    aboveInsertIndexCount += 1;
	} else {
	    removeIndex = idx;
	    insertIndex -= 1;
	}

	id object = [[objects objectAtIndex:removeIndex] retain];
	[self removeObjectAtArrangedObjectIndex:removeIndex];
	[self insertObject:object atArrangedObjectIndex:insertIndex];
	[object release];

	idx = [indexSet indexLessThanIndex:idx];
    }
}

- (int)rowsAboveRow:(int)row inIndexSet:(NSIndexSet*)indexSet {
    NSUInteger currentIndex = [indexSet firstIndex];
    int i = 0;

    while(currentIndex != NSNotFound) {
	if(currentIndex < row) {
	    ++ i;
	}
	currentIndex = [indexSet indexGreaterThanIndex:currentIndex];
    }

    return i;
}

- (void)awakeFromNib {
    [tableView registerForDraggedTypes:[NSArray arrayWithObjects:DictionaryRowsType, nil]];

    [super awakeFromNib];
}

- (id)newObject {
    id obj = [super newObject];

    // 新規のエントリを初期化する
    if(obj) {
	[obj setValue:[NSNumber numberWithBool:YES] forKey:SKKDictionarySetKeys::active];
	[obj setValue:[NSNumber numberWithInt:0] forKey:SKKDictionarySetKeys::type];
	[obj setValue:@"" forKey:SKKDictionarySetKeys::location];
    }

    return obj;
}

- (IBAction)browseLocation:(id)sender {
    NSOpenPanel* panel = [NSOpenPanel openPanel];

    NSString* path = [[self selection] valueForKey:SKKDictionarySetKeys::location];
    NSString* dir = [path stringByDeletingLastPathComponent];
    NSURL* dirurl = [NSURL fileURLWithPath:dir];

    [panel setDirectoryURL:dirurl];
    [panel beginSheetModalForWindow:prefView completionHandler:^(NSInteger result) {
        if(result == NSModalResponseOK) {
            [[self selection] setValue:[[panel URL] path]
                                forKey:SKKDictionarySetKeys::location];
        }
    }];
}

- (BOOL)tableView:(NSTableView*)tv writeRowsWithIndexes:(NSIndexSet*)rowIndexes toPasteboard:(NSPasteboard*)pboard {
    NSArray* typesArray = [NSArray arrayWithObjects:DictionaryRowsType, nil];
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject:rowIndexes requiringSecureCoding:YES error:nil];

    [pboard declareTypes:typesArray owner:self];
    [pboard setData:data forType:DictionaryRowsType];

    return YES;
}

- (NSDragOperation)tableView:(NSTableView*)tv
		validateDrop:(id <NSDraggingInfo>)info
		 proposedRow:(int)row
       proposedDropOperation:(NSTableViewDropOperation)op {
    NSDragOperation dragOp = NSDragOperationCopy;

    // ドラッグ元が同じなら、移動
    if([info draggingSource] == tableView) {
	dragOp =  NSDragOperationMove;
    }

    [tv setDropRow:row dropOperation:NSTableViewDropAbove];

    return dragOp;
}

- (BOOL)tableView:(NSTableView*)tv
       acceptDrop:(id <NSDraggingInfo>)info
              row:(int)row
    dropOperation:(NSTableViewDropOperation)op {
    if(row < 0) {
	row = 0;
    }

    // ドラッグ元以外なら、何もしない
    if([info draggingSource] != tableView) {
	return NO;
    }

    NSPasteboard* pboard = [info draggingPasteboard];
    NSData* rowData = [pboard dataForType:DictionaryRowsType];
    NSIndexSet* rowIndexes = [NSKeyedUnarchiver unarchivedObjectOfClass:[NSIndexSet class] fromData:rowData error:nil];

    [self moveObjectsInArrangedObjectsFromIndexes:rowIndexes toIndex:row];

    int rowsAbove = [self rowsAboveRow:row inIndexSet:rowIndexes];

    NSRange range = NSMakeRange(row - rowsAbove, [rowIndexes count]);
    [self setSelectionIndexes:[NSIndexSet indexSetWithIndexesInRange:range]];

    return YES;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView*)tv {
    return [[self arrangedObjects] count];
}

- (id)tableView:(NSTableView*)tv objectValueForTableColumn:(NSTableColumn*)column row:(NSInteger)row {
    id obj = [[self arrangedObjects] objectAtIndex:row];
    return [obj valueForKey:[column identifier]];
}

@end
