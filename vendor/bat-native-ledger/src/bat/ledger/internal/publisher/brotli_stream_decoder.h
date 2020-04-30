/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVELEDGER_PUBLISHER_BROTLI_STREAM_DECODER_H_
#define BRAVELEDGER_PUBLISHER_BROTLI_STREAM_DECODER_H_

#include <functional>
#include <string_view>
#include <vector>

#include "third_party/brotli/include/brotli/decode.h"

namespace braveledger_publisher {

class BrotliStreamDecoder {
 public:
  explicit BrotliStreamDecoder(size_t buffer_size);

  BrotliStreamDecoder(const BrotliStreamDecoder&) = delete;
  BrotliStreamDecoder& operator=(const BrotliStreamDecoder&) = delete;

  ~BrotliStreamDecoder();

  enum class Result {
    Done = 0,
    InputRequired,
    Error
  };

  Result DecodeString(
      const std::string_view input,
      std::function<void(std::string_view)> callback);

  Result Decode(
      const uint8_t* input_buffer,
      size_t input_length,
      std::function<void(const uint8_t* data, size_t length)> callback);

 private:
  BrotliDecoderState* brotli_state_;
  std::vector<uint8_t> out_vector_;
};

}  // namespace braveledger_publisher

#endif  // BRAVELEDGER_PUBLISHER_BROTLI_STREAM_DECODER_H_
