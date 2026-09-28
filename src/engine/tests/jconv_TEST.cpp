#include <cassert>
#include "jconv.h"

int main() {
    // JIS X 0213 composed characters: one EUC-JP code maps to two UCS
    // characters (base + combining mark).

    // kana + U+309A (combining semi-voiced sound mark)
    assert(jconv::utf8_from_eucj("\xa4\xf7") == "か\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa4\xf8") == "き\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa4\xf9") == "く\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa4\xfa") == "け\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa4\xfb") == "こ\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa5\xf7") == "カ\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa5\xf8") == "キ\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa5\xf9") == "ク\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa5\xfa") == "ケ\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa5\xfb") == "コ\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa5\xfc") == "セ\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa5\xfd") == "ツ\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa5\xfe") == "ト\xe3\x82\x9a");
    assert(jconv::utf8_from_eucj("\xa6\xf8") == "ㇷ\xe3\x82\x9a");

    // IPA + U+0300/U+0301, tone letters
    assert(jconv::utf8_from_eucj("\xab\xc4") == "\xc3\xa6\xcc\x80");
    assert(jconv::utf8_from_eucj("\xab\xc8") == "\xc9\x94\xcc\x80");
    assert(jconv::utf8_from_eucj("\xab\xc9") == "\xc9\x94\xcc\x81");
    assert(jconv::utf8_from_eucj("\xab\xca") == "\xca\x8c\xcc\x80");
    assert(jconv::utf8_from_eucj("\xab\xcb") == "\xca\x8c\xcc\x81");
    assert(jconv::utf8_from_eucj("\xab\xcc") == "\xc9\x99\xcc\x80");
    assert(jconv::utf8_from_eucj("\xab\xcd") == "\xc9\x99\xcc\x81");
    assert(jconv::utf8_from_eucj("\xab\xce") == "\xc9\x9a\xcc\x80");
    assert(jconv::utf8_from_eucj("\xab\xcf") == "\xc9\x9a\xcc\x81");
    assert(jconv::utf8_from_eucj("\xab\xe5") == "\xcb\xa9\xcb\xa5");
    assert(jconv::utf8_from_eucj("\xab\xe6") == "\xcb\xa5\xcb\xa9");

    // surrounded by ordinary characters
    assert(jconv::utf8_from_eucj("\xa4\xa2\xa4\xf7\xa4\xa4") == "あか\xe3\x82\x9aい");
}
