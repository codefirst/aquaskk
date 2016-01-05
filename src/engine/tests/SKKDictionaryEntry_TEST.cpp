#include "SKKDictionaryEntry.h"
#include <cassert>
#include <iostream>

int main() {
  SKKDictionaryEntryContainer container;
  SKKDictionaryEntryIterator it;

  it = find_entry(container, "こうほ");
  assert(it == container.end());

  SKKEntry entry("こうほ");
  SKKCandidateSuite suite("/候補/");
  add_entry(entry, suite, container);
  it = find_entry(container, "こうほ");
  assert(it->second == "/候補/");

  update_entry(entry, std::string("コーホ"), container);
  it = find_entry(container, "こうほ");
  assert(it->second == "/コーホ/候補/");

  remove_entry(entry, std::string("コーホ"), container);
  it = find_entry(container, "こうほ");
  assert(it->second == "/候補/");

  remove_entry(entry, std::string("候補"), container);
  it = find_entry(container, "こうほ");
  assert(it == container.end());
}
