/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/strings/stringprintf.h"
#include "bat/ledger/internal/request/request_publisher.h"
#include "bat/ledger/internal/request/request_util.h"

namespace {

constexpr char kPrefix[] = "";

}  // namespace

namespace braveledger_request_util {

std::string GetPublisherListUrl() {
  return BuildUrl(
      "/prefixes",
      kPrefix,
      ServerTypes::kPublisher);
}

std::string GetPublisherInfoUrl(const std::string& hash_prefix) {
  return BuildUrl(
      base::StringPrintf("/prefix/%s", hash_prefix.data()),
      kPrefix,
      ServerTypes::kPublisher);
}

}  // namespace braveledger_request_util
