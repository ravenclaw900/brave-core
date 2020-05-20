/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "bat/ads/internal/ads_locale_helper.h"

#include "base/time/time.h"
#include "bat/ads/internal/static_values.h"
#include "bat/ads/internal/logging.h"
#include "bat/ads/internal/time_util.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace ads {

AdsLocaleHelper::AdsLocaleHelper(
    AdsClient* ads_client)
    : ads_client_(ads_client) {
  BuildUrl();
}

AdsLocaleHelper::~AdsLocaleHelper() = default;

void AdsLocaleHelper::GetState() {
  auto callback = std::bind(&AdsLocaleHelper::OnStateReceived,
      this, url_, _1, _2, _3);
  ads_client_->URLRequest(url_, {}, "", "", URLRequestMethod::GET, callback);

  return;
}

void AdsLocaleHelper::OnStateReceived(
    const std::string& url,
    const int response_status_code,
    const std::string& response,
    const std::map<std::string, std::string>& headers) {
  // TODO(Moritz Haller): Impl.
  BLOG(7, UrlResponseToString(url, response_status_code, response, headers));
}

void AdsLocaleHelper::BuildUrl() {
  switch (_environment) {
    case Environment::PRODUCTION: {
      url_ = PRODUCTION_SERVER;
      break;
    }

    case Environment::STAGING: {
      url_ = STAGING_SERVER;
      break;
    }

    case Environment::DEVELOPMENT: {
      url_ = DEVELOPMENT_SERVER;
      break;
    }
  }

  url_ += GETSTATE_PATH;
}

}  // namespace ads
