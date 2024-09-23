/* -*- C++ -*-
 *
 * utf8util.h - UTF-8 utilities
 *
 *   Copyright (c) 2007 Tomotaka SUWA, All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef	utf8util_h
#define utf8util_h

#include <string>
#include <algorithm>
#include <cstdlib>

// UTF8 イテレータ
template <typename Iterator>
class utf8iterator {
    Iterator curr_;

    unsigned size() const {
	static unsigned table[] = {
	    1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1, // 0x00
	    1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	    1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	    1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	    1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	    1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	    1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	    1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,
	    0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0, // 0x80
	    0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	    0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,
	    2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2, // 0xc0
	    2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2,
	    3, 3, 3, 3, 3, 3, 3, 3,	3, 3, 3, 3, 3, 3, 3, 3, // 0xe0
	    4, 4, 4, 4, 4, 4, 4, 4,	5, 5, 5, 5, 6, 6, 0, 0  // 0xf0
	};

	return table[(unsigned char)*curr_];
    }

    bool leadbyte() const { return size() != 0; }

    utf8iterator& next(int count = 1) {
	for(int i = 0; i < count; ++ i) {
	    do { ++ curr_; } while(!leadbyte());
	}
	return *this;
    }

    utf8iterator& prev(int count = 1) {
	for(int i = 0; i < count; ++ i) {
	    do { -- curr_; } while(!leadbyte());
	}
	return *this;
    }

public:
    utf8iterator() : curr_(0) {}
    utf8iterator(Iterator iter) : curr_(iter) {}
    utf8iterator& operator=(Iterator iter) {
	curr_ = iter;
	return *this;
    }

    Iterator iterator() { return curr_; }
    unsigned charsize() const { return size(); }

    std::string operator*() const {
	return std::string(curr_, curr_ + size());
    }

    utf8iterator& operator++() { return next(); }
    utf8iterator operator++(int) {
	utf8iterator iter(*this);
	++ curr_;
	return iter;
    }

    utf8iterator& operator--() { return prev(); }
    utf8iterator operator--(int) {
	utf8iterator iter(*this);
	-- curr_;
	return iter;
    }

    utf8iterator& operator+=(int count) { return 0 < count ? next(count) : prev(std::abs(count)); }
    utf8iterator& operator-=(int count) { return 0 < count ? prev(count) : next(std::abs(count)); }

    friend bool operator==(const utf8iterator& lhs, const utf8iterator& rhs) { return lhs.curr_ == rhs.curr_; }
    friend bool operator!=(const utf8iterator& lhs, const utf8iterator& rhs) { return lhs.curr_ != rhs.curr_; }
    friend bool operator<(const utf8iterator& lhs, const utf8iterator& rhs) { return lhs.curr_ < rhs.curr_; }
};

template <typename Iterator>
int operator-(utf8iterator<Iterator> end, utf8iterator<Iterator> beg) {
    int size = 0;
    for(auto cur = beg; cur != end; ++ cur) {
        ++ size;
    }
    return size;
}

template <typename Iterator>
utf8iterator<Iterator> operator+(utf8iterator<Iterator> beg, int count) { return beg += count; }
template <typename Iterator>
utf8iterator<Iterator> operator-(utf8iterator<Iterator> beg, int count) { return beg -= count; }

// 原始的な手続き
struct utf8 {
    typedef utf8iterator<std::string::iterator> iterator;
    typedef utf8iterator<std::string::const_iterator> const_iterator;

    // UTF8 文字列長取得
    static unsigned length(const std::string& str) {
	return const_iterator(str.end()) - const_iterator(str.begin());
    }

    // 指定位置に文字列挿入
    //
    // 例：
    //      std::string str = "文字列";
    //      utf8::push(str, "a", -3); // "a文字列";
    //      utf8::push(str, "b");     // "a文字列b";
    // 
    static void push(std::string& target, const std::string& str, int offset = 0) {
	if(0 <= offset || target.empty()) {
	    target += str;
	} else {
	    iterator end(target.end());
	    iterator pos(end + offset);

	    if(pos.iterator() < target.begin()) {
		pos = target.begin();
	    }
	    target.insert(pos.iterator() - target.begin(), str);
	}
    }

    // 指定位置から一文字後退して削除
    //
    // 例：
    //      std::string str = "文字列";
    //      utf8::pop(str);           // "文字";
    //      utf8::pop(str, -1);       // "字";
    // 
    static void pop(std::string& target, int offset = 0) {
	iterator end(target.end());
	iterator pos(end + (0 < offset ? 0 : offset)  - 1);

	if(!(pos.iterator() < target.begin())) {
	    target.erase(pos.iterator() - target.begin(), pos.charsize());
	}
    }

    // 左部分文字列の取得
    //
    // 例：
    //      std::string str = "文字列";
    //	    utf8::left(str);	      // "文字列";
    //	    utf8::left(str, -1);      // "文字";
    //
    static std::string left(const std::string& str, int offset = 0) {
	if(offset == 0) return str;

	std::string result;
	const_iterator end(str.end());
	const_iterator pos(end + offset);

	if(pos.iterator() < str.begin()) {
	    pos = str.begin();
	}

	std::copy(str.begin(), pos.iterator(), std::back_inserter(result));

	return result;
    }

    // 右部分文字列の取得
    //
    // 例：
    //      std::string str = "文字列";
    //	    utf8::right(str);	      // "";
    //	    utf8::right(str, -1);     // "列";
    //
    static std::string right(const std::string& str, int offset = 0) {
	if(offset == 0) return "";

	std::string result;
	const_iterator end(str.end());
	const_iterator pos(end + offset);

	if(pos.iterator() < str.begin()) {
	    pos = str.begin();
	}

	std::copy(pos.iterator(), str.end(), std::back_inserter(result));

	return result;
    }

    // 共通プレフィクス取得
    //
    // 例：
    //      if(common_prefix("1漢字2", "1漢字3") == "1漢字") { ...
    //
    static std::string common_prefix(const std::string&s1, const std::string& s2) {
        const unsigned long max = std::min(s1.size(), s2.size());
        utf8::const_iterator iter1(s1.begin());
        utf8::const_iterator iter2(s2.begin());

        while(iter1.iterator() - s1.begin() < max && *iter1 == *iter2) {
            ++ iter1;
            ++ iter2;
        }

        return s1.substr(0, iter1.iterator() - s1.begin());
    }
};

#endif
