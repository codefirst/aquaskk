/* -*- ObjC -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2008 Tomotaka SUWA <t.suwa@mac.com>

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

#include "PreferenceController.h"
#include "DictionaryTypeTransformer.h" 
#include "SubRuleDescriptions.h"
#include "SKKServerProxy.h"
#include "SKKConstVars.h"
#include <Carbon/Carbon.h>

namespace {
    const NSString* SUB_RULE_FOLDER = @"folder";
    const NSString* SUB_RULE_PATH = @"path";
    const NSString* SUB_RULE_SWITCH = @"active";
    const NSString* SUB_RULE_KEYMAP = @"keymap";
    const NSString* SUB_RULE_DESCRIPTION = @"description";
    const NSString* SUB_RULE_TYPE = @"type";
}

@interface PreferenceController (Local)
- (NSArray*)collectKeyboardLayout;
- (NSMenuItem*)menuItemWithInputSource:(TISInputSourceRef)inputSource imageSize:(NSSize)size;
- (void)initializeVersion;
- (void)initializeSubRulesAtPath:(NSString*)path withType:(NSString*)type;
- (void)setupKeyboardLayout;
- (void)updatePopUpButton;
- (void)updateFontButton;
- (void)saveChanges;
- (void)reloadServer;
@end

@implementation PreferenceController

- (id)init {
    if(self = [super init]) {
        layoutNames_ = [[NSMutableArray alloc] init];
        preferences_ = [[NSMutableDictionary
                            dictionaryWithContentsOfFile:SKKFilePaths::UserDefaults] retain];
        blacklistApps_ = [[NSMutableArray
                           arrayWithContentsOfFile:SKKFilePaths::BlacklistApps] retain];
        dictionarySet_ = [[NSMutableArray
                              arrayWithContentsOfFile:SKKFilePaths::DictionarySet] retain];

        NSString* fontName = [preferences_ objectForKey:SKKUserDefaultKeys::candidate_window_font_name];
        NSNumber* fontSize =  [preferences_ objectForKey:SKKUserDefaultKeys::candidate_window_font_size];
        candidateWindowFont_ =
            [NSFont fontWithName:fontName size:[fontSize floatValue]] ?:
            [NSFont labelFontOfSize:[fontSize floatValue]];

        [candidateWindowFont_ retain];

        proxy_ = [[SKKServerProxy alloc] init];

        NSValueTransformer* transformer
            = [[DictionaryTypeTransformer alloc] initWithDictionaryTypes:[proxy_ createDictionaryTypes]];

        [NSValueTransformer setValueTransformer:transformer forName:@"DictionaryTypeTransformer"];

        [transformer release];
    }

    return self;
}

- (void)dealloc {
    [proxy_ release];
    [candidateWindowFont_ release];
    [blacklistApps_ release];
    [dictionarySet_ release];
    [preferences_ release];
    [layoutNames_ release];

    [super dealloc];
}

- (void)awakeFromNib {
    [NSApp activateIgnoringOtherApps:YES];

    [objController_ setContent:preferences_];
    [arrayController_ setContent:dictionarySet_];
    [blacklistArrayController_ setContent:blacklistApps_];
    [dictionaryTypes_ setContent:[proxy_ createDictionaryTypes]];

    [self initializeVersion];
    [self initializeSubRulesAtPath:SKKFilePaths::SystemResourceFolder
                          withType:@"システム"];
    [self initializeSubRulesAtPath:SKKFilePaths::ApplicationSupportFolder
                          withType:@"ユーザー"];
    [self setupKeyboardLayout];
    [self updatePopUpButton];
    [self updateFontButton];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return YES;
}

- (void)applicationWillTerminate:(NSNotification*)aNotification {
    [self saveChanges];

    [self reloadServer];
}

- (void)changeFont:(id)sender {
    [candidateWindowFont_ release];
    candidateWindowFont_ = [[sender convertFont:[NSFont systemFontOfSize:14]] retain];

    // Cocoa Bindings により、ボタンのフォント属性も連動して変更される
    [preferences_ setObject:[candidateWindowFont_ fontName]
                  forKey:SKKUserDefaultKeys::candidate_window_font_name];
    [preferences_ setObject:[NSNumber numberWithFloat:[candidateWindowFont_ pointSize]]
                  forKey:SKKUserDefaultKeys::candidate_window_font_size];

    [self updateFontButton];
}

- (IBAction)showFontPanel:(id)sender {
    NSFontPanel* panel = [NSFontPanel sharedFontPanel];

    [panel setPanelFont:[fontButton_ font] isMultiple:NO];
    [panel makeKeyAndOrderFront:self];
}

- (void)keyboardLayoutDidChange:(id)sender {
    NSInteger index = [layoutPopUp_ indexOfSelectedItem];
    NSString* selectedLayout = [layoutNames_ objectAtIndex:index];

    if(selectedLayout) {
        [preferences_ setObject:selectedLayout forKey:SKKUserDefaultKeys::keyboard_layout];
    }
}

- (IBAction)browseLocation:(id)sender {
    NSOpenPanel* panel = [NSOpenPanel openPanel];

    NSString* path = [preferences_ valueForKey:SKKUserDefaultKeys::user_dictionary_path];
    NSString* dir = [path stringByDeletingLastPathComponent];
    NSURL* dirurl = [NSURL fileURLWithPath:dir];

    [panel setDirectoryURL:dirurl];
    [panel beginSheetModalForWindow:prefWindow_ completionHandler:^(NSInteger result) {
        if(result == NSModalResponseOK) {
            [preferences_ setObject:[[panel URL] path]
                             forKey:SKKUserDefaultKeys::user_dictionary_path];
        }
    }];
}

@end

@implementation PreferenceController (Local)

static NSInteger compareInputSource(id obj1, id obj2, void *context) {
    NSString* lhs = (NSString*)TISGetInputSourceProperty((TISInputSourceRef)obj1, kTISPropertyLocalizedName);
    NSString* rhs = (NSString*)TISGetInputSourceProperty((TISInputSourceRef)obj2, kTISPropertyLocalizedName);

    return [lhs compare:rhs];
}

- (NSArray*)collectKeyboardLayout {
    NSArray* result = 0;

    // 検索条件(ASCII 入力可能なキーボードレイアウト)
    CFMutableDictionaryRef conditions = CFDictionaryCreateMutable(0, 2, 0, 0);
    CFDictionaryAddValue(conditions, kTISPropertyInputSourceType, kTISTypeKeyboardLayout);
    CFDictionaryAddValue(conditions, kTISPropertyInputSourceIsASCIICapable, kCFBooleanTrue);

    // リストして名前でソートする
    if(NSArray* array = (NSArray*)TISCreateInputSourceList(conditions, true)) {
        result = [array sortedArrayUsingFunction:compareInputSource context:0];
        [array release];
    }

    CFRelease(conditions);

    return result;
}

- (NSMenuItem*)menuItemWithInputSource:(TISInputSourceRef)inputSource imageSize:(NSSize)size {
    NSString* title = (NSString*)TISGetInputSourceProperty(inputSource, kTISPropertyLocalizedName);
    IconRef iconref = (IconRef)TISGetInputSourceProperty(inputSource, kTISPropertyIconRef);
    NSImage* image = [[NSImage alloc] initWithIconRef:iconref];
    [image setSize:size];

    NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:title
                                           action:@selector(keyboardLayoutDidChange:) keyEquivalent:@""];
    [item setImage:image];
    [image release];

    return [item autorelease];
}

- (void)initializeVersion {
    NSDictionary *info = [[NSBundle mainBundle] infoDictionary];
    NSString* version = [NSString stringWithFormat:@"AquaSKK %@ (%@)",
				  [info objectForKey:@"CFBundleShortVersionString"],
				  [info objectForKey:@"CFBundleVersion"]];

    [version_ setStringValue:version];
    [copyright_ setStringValue:[info objectForKey:@"CFBundleGetInfoString"]];
}

- (void)initializeSubRulesAtPath:(NSString*)folder withType:(NSString*)type {
    SubRuleDescriptions* table = new SubRuleDescriptions([folder UTF8String]);
    NSArray* active_rules = (NSArray*)[preferences_ objectForKey:SKKUserDefaultKeys::sub_rules];
    NSDirectoryEnumerator* files = [[NSFileManager defaultManager] enumeratorAtPath:folder];

    while(NSString* file = [files nextObject]) {
        if([[file pathExtension] isEqualToString:@"rule"]) {
            NSMutableDictionary* rule = [[NSMutableDictionary alloc] init];

            [rule setObject:folder forKey:SUB_RULE_FOLDER];

            [rule setObject:file forKey:SUB_RULE_PATH];

            if(const char* keymap = table->Keymap([file UTF8String])) {
                [rule setObject:[NSString stringWithUTF8String:keymap] forKey:SUB_RULE_KEYMAP];
            }

            [rule setObject:[NSString stringWithUTF8String:table->Description([file UTF8String])]
                     forKey:SUB_RULE_DESCRIPTION];

            BOOL flag = active_rules != nil
                ? [active_rules containsObject:[folder stringByAppendingPathComponent:file]]
                : NO;

            [rule setObject:[NSNumber numberWithBool:flag] forKey:SUB_RULE_SWITCH];

            [rule setObject:type forKey:SUB_RULE_TYPE];

            [subRuleController_ addObject:rule];

            [rule release];
        }
    }

    delete table;

    [subRuleController_ setSelectionIndex:0];
}

- (void)setupKeyboardLayout {
    NSArray* array = [self collectKeyboardLayout];
    if(!array) return;

    // PopUpButton のフォントからアイコンのサイズを決めておく
    NSSize size;
    NSFont* font = [layoutPopUp_ font];
    size.height = size.width = [font ascender] - [font descender];

    NSMenu* menu = [[NSMenu alloc] initWithTitle:@""];

    NSEnumerator* enumerator = [array objectEnumerator];
    while(TISInputSourceRef inputSource = (TISInputSourceRef)[enumerator nextObject]) {
        [menu addItem:[self menuItemWithInputSource:inputSource imageSize:size]];

        // "com.apple.keylayout.US" 等の ID 文字列を配列に追加しておく
        [layoutNames_ addObject:(NSString*)TISGetInputSourceProperty(inputSource, kTISPropertyInputSourceID)];
    }

    // PopUpButton にメニューを貼り付ける
    [layoutPopUp_ setMenu:menu];
    [menu release];
}

- (void)updatePopUpButton {
    NSString* selectedLayout = [preferences_ objectForKey:SKKUserDefaultKeys::keyboard_layout];
    NSUInteger index = [layoutNames_ indexOfObject:selectedLayout];

    if(index == NSNotFound) {
        index = 0;
    }

    [layoutPopUp_ selectItemAtIndex:index];
}

- (void)updateFontButton {
    [fontButton_ setTitle:[NSString stringWithFormat:@"%@ - %2.1f",
                                    [candidateWindowFont_ displayName],
                                    [candidateWindowFont_ pointSize]]];
}

- (void)saveChanges {
    NSMutableArray* active_subrules = [[NSMutableArray alloc] init];
    NSMutableArray* active_keymaps = [[NSMutableArray alloc] init];

    NSLog(@"saving changes ...");

    for(NSDictionary* rule in [subRuleController_ arrangedObjects]) {
        NSNumber* active = [rule objectForKey:SUB_RULE_SWITCH];

        if([active boolValue]) {
            NSString* folder = [rule objectForKey:SUB_RULE_FOLDER];
            NSString* subrule = [rule objectForKey:SUB_RULE_PATH];
            NSString* keymap = [rule objectForKey:SUB_RULE_KEYMAP];

            NSLog(@"activating sub rule: %@", subrule);
	    [active_subrules addObject:[folder stringByAppendingPathComponent:subrule]];
		
            if(keymap != nil) {
                NSLog(@"activating sub keymap: %@", keymap);
                [active_keymaps addObject:[folder stringByAppendingPathComponent:keymap]];
            }
        }
    }

    [preferences_ setObject:active_subrules forKey:SKKUserDefaultKeys::sub_rules];
    [preferences_ setObject:active_keymaps forKey:SKKUserDefaultKeys::sub_keymaps];
    
    [preferences_ writeToFile:SKKFilePaths::UserDefaults atomically:YES];
    [blacklistApps_ writeToFile:SKKFilePaths::BlacklistApps atomically:YES];
    [dictionarySet_ writeToFile:SKKFilePaths::DictionarySet atomically:YES];

    [active_subrules release];
    [active_keymaps release];
}

- (void)reloadServer {
    [proxy_ reloadBlacklistApps];
    [proxy_ reloadUserDefaults];
    [proxy_ reloadDictionarySet];
    [proxy_ reloadComponents];
}

@end
