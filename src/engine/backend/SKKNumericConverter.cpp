/*

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

#include "SKKNumericConverter.h"
#include "SKKCandidate.h"

// ======================================================================
// 数値変換を実装するユーティリティ関数
// ======================================================================

// 1024 → １０２４
static std::string ConvertType1(const std::string& src) {
    std::string result;

    for(unsigned i = 0; i < src.size(); ++ i) {
	switch(src[i]) {
	case '0':
	    result += "０";
	    break;
	case '1':
	    result += "１";
	    break;
	case '2':
	    result += "２";
	    break;
	case '3':
	    result += "３";
	    break;
	case '4':
	    result += "４";
	    break;
	case '5':
	    result += "５";
	    break;
	case '6':
	    result += "６";
	    break;
	case '7':
	    result += "７";
	    break;
	case '8':
	    result += "８";
	    break;
	case '9':
	    result += "９";
	    break;
	}
    }

    return result;
}

// 1024 → 一〇二四
static std::string ConvertType2(const std::string& src) {
    std::string result;

    for(unsigned i = 0; i < src.size(); ++ i) {
	switch(src[i]) {
	case '0':
	    result += "〇";
	    break;
	case '1':
	    result += "一";
	    break;
	case '2':
	    result += "二";
	    break;
	case '3':
	    result += "三";
	    break;
	case '4':
	    result += "四";
	    break;
	case '5':
	    result += "五";
	    break;
	case '6':
	    result += "六";
	    break;
	case '7':
	    result += "七";
	    break;
	case '8':
	    result += "八";
	    break;
	case '9':
	    result += "九";
	    break;
	}
    }

    return result;
}

// 1024 → 千二十四
static std::string ConvertType3(const std::string& src) {
    const char* unit1[] = { "", "万", "億", "兆", "京", "垓" };
    const char* unit2[] = { "十", "百", "千" };
    std::string result;
    unsigned long previous_size = 0;

    if(src.size() == 1 && src[0] == '0') {
	return "〇";
    }

    for(unsigned long i = src.find_first_not_of("0"); i < src.size(); ++ i) {
	switch(src[i]) {
	case '2':
	    result += "二";
	    break;
	case '3':
	    result += "三";
	    break;
	case '4':
	    result += "四";
	    break;
	case '5':
	    result += "五";
	    break;
	case '6':
	    result += "六";
	    break;
	case '7':
	    result += "七";
	    break;
	case '8':
	    result += "八";
	    break;
	case '9':
	    result += "九";
	    break;
	}

	unsigned long distance = src.size() - i;

	// 「十、百、千」以外の位
	if(distance > 4 && (distance - 1) % 4 == 0) {
	    if(src[i] == '1') {
		result += "一";
	    }
	    // 「垓、京、兆、億、万」が連続しない場合に追加
	    if(previous_size < result.size()) {
	        result += unit1[(distance - 1) / 4];
	        previous_size = result.size();
	    }
	} else {
	    // 十の位以上
	    if(distance > 1) {
		if(src[i] != '0') {
		    // 「一千万」の処理
		    if(src[i] == '1' && distance > 4 && (distance - 2) % 4 == 2) {
			result += "一";
		    }
		    result += unit2[(distance - 2) % 4];
		}
	    } else {
		// 一の位
		if(src[i] == '1') {
		    result += "一";
		}
	    }
	}
    }

    return result;
}

// 数値再変換(AquaSKK では無視)
static std::string ConvertType4(const std::string& src) {
    return src;
}

// 1024 → 壱阡弐拾四
static std::string ConvertType5(const std::string& src) {
    const char* unit1[] = { "", "萬", "億", "兆", "京", "垓" };
    const char* unit2[] = { "拾", "百", "阡" };
    std::string result;
    unsigned long previous_size = 0;

    if(src.size() == 1 && src[0] == '0') {
	return "零";
    }

    for(unsigned long i = src.find_first_not_of("0"); i < src.size(); ++ i) {
	switch(src[i]) {
	case '1':
	    result += "壱";
	    break;
	case '2':
	    result += "弐";
	    break;
	case '3':
	    result += "参";
	    break;
	case '4':
	    result += "四";
	    break;
	case '5':
	    result += "伍";
	    break;
	case '6':
	    result += "六";
	    break;
	case '7':
	    result += "七";
	    break;
	case '8':
	    result += "八";
	    break;
	case '9':
	    result += "九";
	    break;
	}

	unsigned long distance = src.size() - i;

	// 「十、百、千」以外の位
	if(distance > 4 && (distance - 1) % 4 == 0) {
	    // 「垓、京、兆、億、萬」が連続しない場合に追加
	    if (previous_size < result.size()){
	        result += unit1[(distance - 1) / 4];
	        previous_size = result.size();
	    }
	} else {
	    // 十の位以上
	    if(distance > 1) {
		if(src[i] != '0') {
		    result += unit2[(distance - 2) % 4];
		}
	    }
	}
    }

    return result;
}

// 34 → ３四
static std::string ConvertType9(const std::string& src) {
    return  ConvertType1(src.substr(0, 1)) + ConvertType2(src.substr(1, 1));
}

// ======================================================================
// クラスインタフェース
// ======================================================================

// 検索キーの正規化
bool SKKNumericConverter::Setup(const std::string& query) {
    params_.clear();
    original_ = query;

    const char* numbers = "0123456789";
    std::string src(query);
    std::string::size_type from = src.find_first_of(numbers);

    // 連続した数値を見つけたら vector に格納し、"#" に正規化
    while(from != std::string::npos) {
	std::string::size_type to = src.find_first_not_of(numbers, from);
	params_.push_back(src.substr(from, to - from));
	src.replace(from, to - from, "#");

	from = src.find_first_of(numbers, from);
    }

    normalized_ = src;

    return !params_.empty();
}

// オリジナルのキー
const std::string& SKKNumericConverter::OriginalKey() const {
    return original_;
}

// 正規化されたキー
const std::string& SKKNumericConverter::NormalizedKey() const {
    if(params_.empty()) return original_;

    return normalized_;
}

// 数値変換を適用する
void SKKNumericConverter::Apply(SKKCandidate& candidate) const {
    if(params_.empty()) return;

    const char* numbers = "0123459";
    std::string result;
    std::string src(candidate.Word());
    std::string::size_type pos = 0;

    for(unsigned index = 0; index < params_.size(); ++ index) {
	pos = src.find_first_of(numbers, pos + 1);
        if(pos == std::string::npos) break;

	if(src[pos - 1] == '#') {
	    switch(src[pos]) {
	    case '0':		// 無変換
		src.replace(pos - 1, 2, params_[index]);
		break;

	    case '1':		// 半角→全角変換
		src.replace(pos - 1, 2, ConvertType1(params_[index]));
		break;

	    case '2':		// 漢数字位取りなし
		src.replace(pos - 1, 2, ConvertType2(params_[index]));
		break;

	    case '3':		// 漢数字位取りあり
		src.replace(pos - 1, 2, ConvertType3(params_[index]));
		break;

	    case '4':		// 数値再変換(AquaSKK では無変換)
		src.replace(pos - 1, 2, ConvertType4(params_[index]));
		break;

	    case '5':		// 小切手・手形
		src.replace(pos - 1, 2, ConvertType5(params_[index]));
		break;

	    case '9':		// 棋譜入力用
		src.replace(pos - 1, 2, ConvertType9(params_[index]));
		break;
	    }

	    pos = pos - 1 + params_[index].size();
	}
    }

    candidate.SetVariant(src);
}
