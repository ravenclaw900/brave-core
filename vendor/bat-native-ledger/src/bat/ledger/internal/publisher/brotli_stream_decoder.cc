/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/publisher/brotli_stream_decoder.h"

namespace {

void* Allocate(void* opaque, size_t size) {
  size_t* array = reinterpret_cast<size_t*>(malloc(size + sizeof(size_t)));
  if (!array) {
    return nullptr;
  }
  array[0] = size;
  return &array[1];
}

void Free(void* opaque, void* address) {
  if (!address) {
    return;
  }
  size_t* array = reinterpret_cast<size_t*>(address);
  free(&array[-1]);
}

}  // namespace

namespace braveledger_publisher {

BrotliStreamDecoder::BrotliStreamDecoder(size_t buffer_size) {
  brotli_state_ = BrotliDecoderCreateInstance(&Allocate, &Free, this);
  out_vector_.reserve(buffer_size);
}

BrotliStreamDecoder::~BrotliStreamDecoder() {
  BrotliDecoderDestroyInstance(brotli_state_);
  brotli_state_ = nullptr;
}

BrotliStreamDecoder::Result BrotliStreamDecoder::DecodeString(
    const std::string_view input,
    std::function<void(std::string_view)> callback) {
  static_assert(sizeof(char) == sizeof(uint8_t), "safe reinterpret_cast");
  if (input.length() == 0) {
    return Result::Error;
  }
  return Decode(
      reinterpret_cast<const uint8_t*>(input.data()),
      input.length(),
      [&callback](const uint8_t* data, size_t length) {
        std::string_view result(reinterpret_cast<const char*>(data), length);
        callback(result);
      });
}

BrotliStreamDecoder::Result BrotliStreamDecoder::Decode(
    const uint8_t* input_buffer,
    size_t input_length,
    std::function<void(const uint8_t* data, size_t length)> callback) {
  if (!input_buffer || input_length == 0) {
    return Result::Error;
  }

  uint8_t* output_buffer = out_vector_.data();
  size_t output_length = out_vector_.capacity();

  for (;;) {
    auto brotli_result = BrotliDecoderDecompressStream(
      brotli_state_,
      &input_length,
      &input_buffer,
      &output_length,
      &output_buffer,
      nullptr);

    switch (brotli_result) {
      case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT: {
        callback(out_vector_.data(), out_vector_.capacity() - output_length);
        output_buffer = out_vector_.data();
        output_length = out_vector_.capacity();
        break;
      }
      case BROTLI_DECODER_RESULT_SUCCESS: {
        callback(out_vector_.data(), out_vector_.capacity() - output_length);
        return Result::Done;
      }
      case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT: {
        return Result::InputRequired;
      }
      default: {
        return Result::Error;
      }
    }
  }
}

}  // namespace braveledger_publisher
