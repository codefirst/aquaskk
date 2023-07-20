/*

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

#include "SKKHttpDictionaryLoader.h"
#include <fstream>
#include <sys/stat.h>

SKKHttpDictionaryLoader::SKKHttpDictionaryLoader() {}

void SKKHttpDictionaryLoader::Initialize(const std::string& location) {
    std::istringstream buf(location);
    std::string addr;

    buf >> addr >> url_;

    buf.ignore(1);
    std::getline(buf, path_);

    remote_.parse(addr, "80");

    tmp_path_ = path_ + ".download";
}

bool SKKHttpDictionaryLoader::NeedsUpdate() {
    net::socket::tcpstream http(remote_);

    if(request(http)) {
        return download(http, content_length(http));
    }

    return false;
}

const std::string& SKKHttpDictionaryLoader::FilePath() const {
    return path_;
}

// ----------------------------------------------------------------------

bool SKKHttpDictionaryLoader::request(net::socket::tcpstream& http) {
    char timestamp[64];
    struct stat st;

    if(stat(path_.c_str(), &st) != 0) {
        st.st_mtime = 0;
    }

    // HTTP 日付を生成する(RFC 822, updated by RFC 1123)
    //
    // 例) "Sun, 06 Nov 1994 08:49:37 GMT"
    strftime(timestamp, sizeof(timestamp),
             "%a, %d %b %Y %T GMT", gmtime(&st.st_mtime));

    http << "GET " << url_ << " HTTP/1.1\r\n";
    http << "Host: " << remote_.node() << "\r\n";
    http << "If-Modified-Since: " << timestamp << "\r\n";
    http << "Connection: close\r\n";
    http << "\r\n" << std::flush;

    return (bool)http;
}

int SKKHttpDictionaryLoader::content_length(net::socket::tcpstream& http) {
    int length = 0;
    std::string response;

    while(std::getline(http, response) && response != "\r") {
        if(response.find("HTTP/1.1") != std::string::npos) {
            std::istringstream buf(response);

            // "HTTP/1.1 200" を期待する
            buf >> response >> response;
            if(response != "200") {
                while(std::getline(http, response)) {}
                break;
            }
        }

        if(response.find("Content-Length") != std::string::npos) {
            std::istringstream buf(response);
            buf >> response >> length;
        }
    }

    return length;
}

long long SKKHttpDictionaryLoader::file_size(const std::string& path) const {
    struct stat st;

    if(stat(path.c_str(), &st) == 0) {
        return st.st_size;
    }

    return 0;
}

bool SKKHttpDictionaryLoader::download(net::socket::tcpstream& http, int length) {
    if(!length) return false;

    std::string line;
    std::ofstream ofs(tmp_path_.c_str());

    int readed = 0;
    while(std::getline(http, line)) {
        readed += line.size() + 1;
        ofs << line << std::endl;
        if(readed >= length) { break; }
    }

    // ダウンロードに失敗したか？
    long long new_size = file_size(tmp_path_);
    if(new_size != length) {
        std::cerr << "SKKHttpDictionaryLoader::download(): size conflict: expected="
                  << length << ", actual=" << new_size << std::endl;
        return false;
    }

    // 既存の辞書と比較して小さすぎないか？
    long long old_size = file_size(path_);
    if(old_size != 0) {
        const int safety_margin = 32 * 1024; // 32KB

        if(new_size + safety_margin < old_size) {
            std::cerr << "SKKHttpDictionaryLoader::download(): too small: size="
                      << new_size << std::endl;
            return false;
        }
    }

    if(rename(tmp_path_.c_str(), path_.c_str()) != 0) {
        std::cerr << "SKKHttpDictionaryLoader::download(): rename failed: errno="
                  << errno << std::endl;
        return false;
    }

    return true;
}
