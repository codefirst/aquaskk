/* -*- C++ -*-
 *
 * subrange.h - subrange of STL sequence container.
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

#ifndef subrange_h
#define subrange_h

template <typename Container, typename Iter = typename Container::iterator>
class subrange {
    typedef unsigned size_type;

    Iter begin_;
    Iter end_;

    void adjust(Container& container, size_type pos, size_type length) {
	unsigned long size = container.size();

	if(pos < size) {
	    begin_ = container.begin() + pos;
	} else {
	    begin_ = end_ = container.end();
	    return;
	}

	if(length == -1U ||  size < pos + length) {
	    end_ = container.end();
	} else {
      	    end_ = begin_ + length;
	}
    }

public:
    typedef typename Container::value_type value_type;
    typedef Iter iterator;

    subrange() {}
    subrange(Iter begin, Iter end) : begin_(begin), end_(end) {}
    subrange(Container& container, size_type pos, size_type length = -1U) {
	adjust(container, pos, length);
    }

    void set(Iter begin, Iter end) {
	begin_ = begin;
	end_ = end;
    }

    void set(Container& container, size_type pos, size_type length = -1U) {
	adjust(container, pos, length);
    }

    size_type size() const { return end() - begin(); }
    bool empty() const { return begin() == end(); }

    Iter begin() { return begin_; }
    Iter end() { return end_; }

    Iter begin() const { return begin_; }
    Iter end() const { return end_; }

    value_type& front() { return *begin(); }
    value_type& back() { return *(end() - 1); }

    value_type& operator[](size_type pos) { return *(begin() + pos); }
    const value_type& operator[](size_type pos) const { return *(begin() + pos); }
};

#endif
