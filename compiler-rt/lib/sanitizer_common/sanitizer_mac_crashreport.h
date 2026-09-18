//===-- sanitizer_mac_crashreport.h --------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Darwin crash-reporter payload format for sanitizer reports. Populated
// through the ReportReceiver interface (see sanitizer_report_receiver.h).
//
//===----------------------------------------------------------------------===//
#ifndef SANITIZER_MAC_CRASHREPORT_H
#define SANITIZER_MAC_CRASHREPORT_H

#include "sanitizer_platform.h"

#if SANITIZER_APPLE

#  include <stddef.h>
#  include <stdint.h>

#  define DARWIN_SANITIZER_V1_CATEGORY_MAXLEN 32
#  define DARWIN_SANITIZER_V1_TYPE_MAXLEN 32
#  define DARWIN_SANITIZER_V1_STACK_DESCRIPTION_MAXLEN 128
#  define DARWIN_SANITIZER_V1_MAXSTACKS 4
#  define DARWIN_SANITIZER_V1_FRAMES_PER_STACK 64

#  define DARWIN_SANITIZER_V1_STACK_TYPE_OTHER 0
#  define DARWIN_SANITIZER_V1_STACK_TYPE_ALLOCATION 1
#  define DARWIN_SANITIZER_V1_STACK_TYPE_DEALLOCATION 2

typedef struct {
  uint64_t thread_id;
  uint64_t time;
  uint32_t num_frames;
  uintptr_t frames[DARWIN_SANITIZER_V1_FRAMES_PER_STACK];
} sanitizers_stack_trace_t;

typedef struct {
  uint32_t type;
  char description[DARWIN_SANITIZER_V1_STACK_DESCRIPTION_MAXLEN];
  sanitizers_stack_trace_t stack;
} darwin_sanitizer_report_payload_stack_v1;

typedef struct {
  char category[DARWIN_SANITIZER_V1_CATEGORY_MAXLEN];
  char type[DARWIN_SANITIZER_V1_TYPE_MAXLEN];

  // These three fields may be 0 for non-heap errors.
  uintptr_t fault_address;
  uintptr_t allocation_address;
  size_t allocation_size;

  uint16_t nstacks;
  darwin_sanitizer_report_payload_stack_v1 stacks[DARWIN_SANITIZER_V1_MAXSTACKS];
} darwin_sanitizer_report_payload_v1;

typedef struct __attribute__((packed)) {
  uint16_t vers;
  union {
    darwin_sanitizer_report_payload_v1 v1;
  };
} darwin_sanitizer_report_payload;

extern darwin_sanitizer_report_payload darwin_sanitizer_report_global;

namespace __sanitizer {

// Install the Darwin ReportReceiver. Called at library init.
void InitDarwinReportReceiver();

}  // namespace __sanitizer

#endif  // SANITIZER_APPLE
#endif  // SANITIZER_MAC_CRASHREPORT_H
