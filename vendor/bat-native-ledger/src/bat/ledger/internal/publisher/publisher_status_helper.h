/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVELEDGER_PUBLISHER_PUBLISHER_STATUS_HELPER_H_
#define BRAVELEDGER_PUBLISHER_PUBLISHER_STATUS_HELPER_H_

#include <functional>
#include <map>
#include <string>
#include <utility>

#include "bat/ledger/ledger.h"

namespace bat_ledger {
class LedgerImpl;
}

namespace braveledger_publisher {

struct PublisherStatusData {
  ledger::PublisherStatus status;
  uint64_t updated_at;
};

using PublisherStatusMap = std::map<std::string, PublisherStatusData>;

void RefreshPublisherStatus(
    bat_ledger::LedgerImpl* ledger,
    PublisherStatusMap&& status_map,
    std::function<void(PublisherStatusMap)> callback);

void RefreshPublisherStatus(
    bat_ledger::LedgerImpl* ledger,
    PublisherStatusMap&& status_map,
    ledger::PublisherInfoList&& info_list,
    ledger::PublisherInfoListCallback callback);

void RefreshPublisherStatus(
    bat_ledger::LedgerImpl* ledger,
    PublisherStatusMap&& status_map,
    ledger::PendingContributionInfoList&& list,
    ledger::PendingContributionInfoListCallback callback);


}  // namespace braveledger_publisher

#endif  // BRAVELEDGER_PUBLISHER_PUBLISHER_STATUS_HELPER_H_
