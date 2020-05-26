/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/publisher/publisher_status_helper.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "base/time/time.h"
#include "bat/ledger/internal/ledger_impl.h"

namespace {

using braveledger_publisher::PublisherStatusMap;

struct RefreshTaskInfo {
  RefreshTaskInfo(
      bat_ledger::LedgerImpl* ledger,
      PublisherStatusMap&& status_map,
      std::function<void(PublisherStatusMap)> callback)
      : ledger(ledger),
        map(std::move(status_map)),
        current(map.begin()),
        callback(callback) {}

  bat_ledger::LedgerImpl* ledger;
  PublisherStatusMap map;
  PublisherStatusMap::iterator current;
  std::function<void(PublisherStatusMap)> callback;
};

void RefreshNext(std::shared_ptr<RefreshTaskInfo> task_info) {
  // Find the first map element that has an expired status.
  task_info->current = std::find_if(
      task_info->current,
      task_info->map.end(),
      [&task_info](auto& key_value) {
        return task_info->ledger->ShouldFetchServerPublisherInfo(
            base::Time::FromDoubleT(key_value.second.updated_at));
      });

  // Execute the callback if no more expired elements are found.
  if (task_info->current == task_info->map.end()) {
    task_info->callback(std::move(task_info->map));
    return;
  }

  // Look for publisher key in hash index.
  auto& key = task_info->current->first;
  task_info->ledger->SearchPublisherList(key, [task_info](bool exists) {
    // If the publisher key does not exist in the hash index look for
    // next expired entry.
    if (!exists) {
      ++task_info->current;
      RefreshNext(task_info);
      return;
    }
    // Fetch current publisher info.
    auto& key = task_info->current->first;
    task_info->ledger->RefreshPublisher(key, [task_info](auto status) {
      // Update status map and continue looking for expired entries.
      task_info->current->second.status = status;
      ++task_info->current;
      RefreshNext(task_info);
    });
  });
}

}  // namespace

namespace braveledger_publisher {

void RefreshPublisherStatus(
    bat_ledger::LedgerImpl* ledger,
    PublisherStatusMap&& status_map,
    std::function<void(PublisherStatusMap)> callback) {
  RefreshNext(std::make_shared<RefreshTaskInfo>(
      ledger,
      std::move(status_map),
      callback));
}

void RefreshPublisherStatus(
    bat_ledger::LedgerImpl* ledger,
    PublisherStatusMap&& status_map,
    ledger::PublisherInfoList&& info_list,
    ledger::PublisherInfoListCallback callback) {
  auto shared_list = std::make_shared<ledger::PublisherInfoList>(
      std::move(info_list));

  RefreshPublisherStatus(ledger, std::move(status_map),
      [shared_list, callback](auto status_map) {
        for (auto& info : *shared_list) {
          info->status = status_map[info->id].status;
        }
        callback(std::move(*shared_list));
      });
}

void RefreshPublisherStatus(
    bat_ledger::LedgerImpl* ledger,
    PublisherStatusMap&& status_map,
    ledger::PendingContributionInfoList&& info_list,
    ledger::PendingContributionInfoListCallback callback) {
  auto shared_list = std::make_shared<ledger::PendingContributionInfoList>(
      std::move(info_list));

  RefreshPublisherStatus(ledger, std::move(status_map),
      [shared_list, callback](auto status_map) {
        for (auto& info : *shared_list) {
          info->status = status_map[info->publisher_key].status;
        }
        callback(std::move(*shared_list));
      });
}

}  // namespace braveledger_publisher
