/* -*- C++ -*-

 MacOS X implementation of the SKK input method.

 Copyright (C) 2015 mzp <mzpppp@gmail.com>

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

#include "SKKCloudUserDictionary.h"

SKKCloudUserDictionary::SKKCloudUserDictionary(SKKLocalUserDictionary* dictionary, SKKSyncStrategy* strategy)
: dictionary_(dictionary), strategy_(strategy) {
}

SKKCloudUserDictionary::~SKKCloudUserDictionary() {
    strategy_->Save();
}

void SKKCloudUserDictionary::Initialize(const std::string& path) {
    dictionary_->Initialize(path);
    strategy_->Initialize(dictionary_->GetDicionaryFile());
}

void SKKCloudUserDictionary::Find(const SKKEntry& entry, SKKCandidateSuite& result) {
    dictionary_->Find(entry, result);
}

std::string SKKCloudUserDictionary::ReverseLookup(const std::string& candidate) {
    return dictionary_->ReverseLookup(candidate);
}

void SKKCloudUserDictionary::Complete(SKKCompletionHelper& helper) {
    dictionary_->Complete(helper);
}

void SKKCloudUserDictionary::Register(const SKKEntry& entry, const SKKCandidate& candidate) {
    dictionary_->Register(entry, candidate);
    strategy_->Register(entry, candidate);
}

void SKKCloudUserDictionary::Remove(const SKKEntry& entry, const SKKCandidate& candidate) {
    dictionary_->Remove(entry, candidate);
    strategy_->Remove(entry, candidate);
}

void SKKCloudUserDictionary::SetPrivateMode(bool flag) {
    dictionary_->SetPrivateMode(flag);
}