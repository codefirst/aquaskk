/* -*- ObjC -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2009 Tomotaka SUWA <t.suwa@mac.com>

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

#include "MacConfig.h"
#include "SKKConstVars.h"

MacConfig::MacConfig() {
}

bool MacConfig::FixIntermediateConversion() {
    return boolConfig(SKKUserDefaultKeys::fix_intermediate_conversion);
}

bool MacConfig::EnableDynamicCompletion() {
    return boolConfig(SKKUserDefaultKeys::enable_dynamic_completion);
}

int MacConfig::DynamicCompletionRange() {
    return integerConfig(SKKUserDefaultKeys::dynamic_completion_range);
}

bool MacConfig::EnableAnnotation() {
    return boolConfig(SKKUserDefaultKeys::enable_annotation);
}

bool MacConfig::DisplayShortestMatchOfKanaConversions() {
    return boolConfig(SKKUserDefaultKeys::display_shortest_match_of_kana_conversions);
}

bool MacConfig::SuppressNewlineOnCommit() {
    return boolConfig(SKKUserDefaultKeys::suppress_newline_on_commit);
}

int MacConfig::MaxCountOfInlineCandidates() {
    return integerConfig(SKKUserDefaultKeys::max_count_of_inline_candidates);
}

bool MacConfig::HandleRecursiveEntryAsOkuri() {
    return boolConfig(SKKUserDefaultKeys::handle_recursive_entry_as_okuri);
}

bool MacConfig::InlineBackSpaceImpliesCommit() {
    return boolConfig(SKKUserDefaultKeys::inline_backspace_implies_commit);
}

bool MacConfig::DeleteOkuriWhenQuit() {
    return boolConfig(SKKUserDefaultKeys::delete_okuri_when_quit);
}

// private methods

int MacConfig::integerConfig(NSString* key) {
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    return (int)[defaults integerForKey:key];
}

bool MacConfig::boolConfig(NSString* key) {
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    return [defaults boolForKey:key] == YES;
}
