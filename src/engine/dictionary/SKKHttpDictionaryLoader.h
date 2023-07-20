/* -*- C++ -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2010 Tomotaka SUWA <tomotaka.suwa@gmail.com>

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

#ifndef SKKHttpDictionaryLoader_h
#define SKKHttpDictionaryLoader_h

#include "SKKDictionaryLoader.h"
#include "socketutil.h"

class SKKHttpDictionaryLoader : public SKKDictionaryLoader {
    net::socket::endpoint remote_;
    std::string url_;
    std::string path_;
    std::string tmp_path_;

    virtual bool NeedsUpdate();
    virtual const std::string& FilePath() const;

    bool request(net::socket::tcpstream& http);
    int content_length(net::socket::tcpstream& http);
    long long file_size(const std::string& path) const;
    bool download(net::socket::tcpstream& http, int length);

public:
    SKKHttpDictionaryLoader();

    //
    // 引数の形式は "host:port url path" とする。":port" は省略化。
    //
    // 例)
    //
    // "openlab.jp /skk/skk/dic/SKK-JISYO.L /path/to/the/SKK-JISYO.L"
    //
    virtual void Initialize(const std::string& location);

    virtual int Interval() const { return 60 * 60 * 6; }

    virtual int Timeout() const { return 3; }
};

#endif
