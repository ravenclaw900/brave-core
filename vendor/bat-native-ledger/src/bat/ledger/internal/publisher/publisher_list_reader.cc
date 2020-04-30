/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/publisher/publisher_list_reader.h"

#include <utility>

#include "base/big_endian.h"
#include "bat/ledger/internal/publisher/prefix_util.h"

namespace {

template<typename T>
std::string ExpandDeltaList(const T& deltas) {
  constexpr size_t prefix_size = sizeof(uint32_t);

  std::string output;
  output.resize(deltas.size() * prefix_size);
  size_t output_pos = 0;

  uint32_t value = 0;
  for (uint32_t delta : deltas) {
    value += delta;
    base::WriteBigEndian(&output[output_pos], value);
    output_pos += prefix_size;
  }

  return output;
}

}  // namespace

namespace braveledger_publisher {

PublisherListReader::PublisherListReader()
    : prefix_size_(kMinPrefixSize) {}

PublisherListReader::~PublisherListReader() = default;

PublisherListReader::ParseError PublisherListReader::Parse(
    const std::string& contents) {
  using publishers_pb::PublisherList;

  PublisherList message;
  if (!message.ParseFromString(contents)) {
    return ParseError::InvalidProtobufMessage;
  }

  size_t prefix_size = message.prefix_size();
  if (prefix_size < kMinPrefixSize || prefix_size > kMaxPrefixSize) {
    return ParseError::InvalidPrefixSize;
  }

  size_t uncompressed_size = message.uncompressed_size();
  if (uncompressed_size == 0) {
    return ParseError::InvalidUncompressedSize;
  }

  std::string uncompressed;
  switch (message.compression_type()) {
    case PublisherList::NO_COMPRESSION: {
      if (message.prefixes().size() % prefix_size != 0) {
        return ParseError::InvalidUncompressedSize;
      }
      uncompressed = std::move(*message.mutable_prefixes());
      break;
    }
    case PublisherList::DELTA_COMPRESSION: {
      if (prefix_size != sizeof(uint32_t)) {
        return ParseError::InvalidPrefixSize;
      }
      uncompressed = ExpandDeltaList(message.deltas());
      break;
    }
    default:
      return ParseError::UnknownCompressionType;
  }

  prefixes_ = std::move(uncompressed);
  prefix_size_ = prefix_size;

  return ParseError::None;
}

}  // namespace braveledger_publisher
