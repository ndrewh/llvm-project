//===-- sanitizer_mac_crashreport.cpp ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "sanitizer_mac_crashreport.h"

#if SANITIZER_APPLE

#  include "sanitizer_common.h"
#  include "sanitizer_libc.h"
#  include "sanitizer_placement_new.h"
#  include "sanitizer_report_receiver.h"

darwin_sanitizer_report_payload darwin_sanitizer_report_global;

namespace __sanitizer {

namespace {

// Translates ReportReceiver events into darwin_sanitizer_report_global.
class DarwinReportReceiver : public ReportReceiver {
 public:
  void OnStart(const char *sanitizer, const char *type) override {
    internal_memset(&darwin_sanitizer_report_global, 0,
                    sizeof(darwin_sanitizer_report_global));
    darwin_sanitizer_report_global.vers = 1;
    if (sanitizer)
      CopyTruncated(darwin_sanitizer_report_global.v1.category, sanitizer,
                    DARWIN_SANITIZER_V1_CATEGORY_MAXLEN);
    if (type)
      CopyTruncated(darwin_sanitizer_report_global.v1.type, type,
                    DARWIN_SANITIZER_V1_TYPE_MAXLEN);
  }

  void OnAddress(uptr addr, uptr size, ReportAddressKind kind,
                 const char *desc, uptr desc_len) override {
    darwin_sanitizer_report_payload_v1 &v1 = darwin_sanitizer_report_global.v1;
    switch (kind) {
      case kReportAddressFault:
        v1.fault_address = addr;
        break;
      case kReportAddressAllocation:
        v1.allocation_address = addr;
        v1.allocation_size = size;
        break;
      default:
        // Fill fault_address opportunistically if not yet set.
        if (!v1.fault_address)
          v1.fault_address = addr;
        break;
    }
  }

  void OnStack(const uptr *frames, uptr num_frames, ReportStackKind kind,
               const char *desc, uptr desc_len) override {
    // Skip the offending-op stack: it's already captured as the process's
    // crash stack via macOS crash reporting, so recording it again in the
    // payload would be redundant.
    if (kind == kReportStackFault)
      return;
    darwin_sanitizer_report_payload_v1 &v1 = darwin_sanitizer_report_global.v1;
    if (v1.nstacks >= DARWIN_SANITIZER_V1_MAXSTACKS)
      return;
    darwin_sanitizer_report_payload_stack_v1 &out = v1.stacks[v1.nstacks++];
    internal_memset(&out, 0, sizeof(out));
    switch (kind) {
      case kReportStackAllocation:
        out.type = DARWIN_SANITIZER_V1_STACK_TYPE_ALLOCATION;
        break;
      case kReportStackDeallocation:
        out.type = DARWIN_SANITIZER_V1_STACK_TYPE_DEALLOCATION;
        break;
      default:
        out.type = DARWIN_SANITIZER_V1_STACK_TYPE_OTHER;
        break;
    }
    if (desc)
      CopyTruncated(out.description, desc,
                    DARWIN_SANITIZER_V1_STACK_DESCRIPTION_MAXLEN);
    // Reverse to top-of-stack-last for the payload format.
    const uptr cap =
        sizeof(out.stack.frames) / sizeof(out.stack.frames[0]);
    uptr n = num_frames < cap ? num_frames : cap;
    out.stack.num_frames = n;
    for (uptr i = 0; i < n; i++)
      out.stack.frames[i] = frames[n - 1 - i];
  }

  void OnFinish() override {
    // Nothing further to do; the global is left populated for external
    // crash-reporting infrastructure to snapshot.
  }

 private:
  static void CopyTruncated(char *dst, const char *src, uptr dst_cap) {
    if (!dst_cap)
      return;
    internal_strncpy(dst, src, dst_cap);
    dst[dst_cap - 1] = 0;
  }
};

// Storage for a lazy-constructed DarwinReportReceiver. Placement-new'd from
// InitDarwinReportReceiver at platform init time so we avoid the global
// constructor/destructor that a plain file-scope object would require.
alignas(DarwinReportReceiver) char
    g_darwin_receiver_storage[sizeof(DarwinReportReceiver)];
DarwinReportReceiver *g_darwin_receiver = nullptr;

}  // namespace

void InitDarwinReportReceiver() {
  if (g_darwin_receiver)
    return;
  g_darwin_receiver = new (g_darwin_receiver_storage) DarwinReportReceiver();
  RegisterReportReceiver(g_darwin_receiver);
}

}  // namespace __sanitizer

#endif  // SANITIZER_APPLE
