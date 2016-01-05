/* -*- ObjC -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2007-2010 Tomotaka SUWA <tomotaka.suwa@gmail.com>

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

#ifndef SKKConstVars_h
#define SKKConstVars_h

@class NSString;

// UserDefaults 用のキー
namespace SKKUserDefaultKeys {
    extern NSString* suppress_newline_on_commit;
    extern NSString* use_numeric_conversion;
    extern NSString* show_input_mode_icon;
    extern NSString* use_individual_input_mode;
    extern NSString* beep_on_registration;
    extern NSString* inline_backspace_implies_commit;
    extern NSString* delete_okuri_when_quit;

    extern NSString* keyboard_layout;

    extern NSString* sub_rules;
    extern NSString* sub_keymaps;

    extern NSString* enable_extended_completion;
    extern NSString* enable_dynamic_completion;
    extern NSString* dynamic_completion_range;
    extern NSString* minimum_completion_length;

    extern NSString* max_count_of_inline_candidates;
    extern NSString* candidate_window_labels;
    extern NSString* candidate_window_font_name;
    extern NSString* candidate_window_font_size;
    extern NSString* put_candidate_window_upward;
    extern NSString* enable_annotation;

    extern NSString* user_dictionary_path;
    extern NSString* enable_icloud;

    extern NSString* enable_skkserv;
    extern NSString* skkserv_localonly;
    extern NSString* skkserv_port;

    extern NSString* enable_private_mode;

    extern NSString* fix_intermediate_conversion;
    extern NSString* display_shortest_match_of_kana_conversions;
    extern NSString* handle_recursive_entry_as_okuri;

    extern NSString* openlab_host;
    extern NSString* openlab_path;

    extern NSString* direct_clients;

    extern NSString* enable_skkdap;
    extern NSString* skkdap_folder;
    extern NSString* skkdap_port;
};

// DictionarySet 用のキー
namespace SKKDictionarySetKeys {
    extern NSString* active;
    extern NSString* type;
    extern NSString* location;
};

// DictionaryTypes 用のキー
namespace SKKDictionaryTypeKeys {
    extern NSString* type;
    extern NSString* name;
};

// 重要なパス
namespace SKKFilePaths {
    extern NSString* SystemResourceFolder;
    extern NSString* ApplicationSupportFolder;
    extern NSString* DictionarySet;
    extern NSString* UserDefaults;
};

#endif
