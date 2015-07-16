/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "perf_regs.h"

#include <unordered_map>
#include <base/logging.h>
#include <base/stringprintf.h>
#include <base/strings.h>

constexpr ArchType GetBuildArch() {
#if defined(__i386__)
  return ARCH_X86_32;
#elif defined(__x86_64__)
  return ARCH_X86_64;
#elif defined(__aarch64__)
  return ARCH_ARM64;
#elif defined(__arm__)
  return ARCH_ARM;
#else
  return ARCH_UNSUPPORTED;
#endif
}

static ArchType current_arch = GetBuildArch();

ArchType GetCurrentArch() {
  return current_arch;
}

bool SetCurrentArch(const std::string& arch) {
  if (arch == "x86") {
    current_arch = ARCH_X86_32;
  } else if (arch == "x86_64") {
    current_arch = ARCH_X86_64;
  } else if (arch == "aarch64") {
    current_arch = ARCH_ARM64;
  } else if (android::base::StartsWith(arch, "arm")) {
    current_arch = ARCH_ARM;
  } else {
    LOG(ERROR) << "unsupported arch: " << arch;
    return false;
  }
  return true;
}

uint64_t GetSupportedRegMask() {
  switch (GetCurrentArch()) {
    case ARCH_X86_32:
      return ((1ULL << PERF_REG_X86_32_MAX) - 1);
    case ARCH_X86_64:
      return (((1ULL << PERF_REG_X86_64_MAX) - 1) & ~(1ULL << PERF_REG_X86_DS) &
              ~(1ULL << PERF_REG_X86_ES) & ~(1ULL << PERF_REG_X86_FS) & ~(1ULL << PERF_REG_X86_GS));
    case ARCH_ARM:
      return ((1ULL << PERF_REG_ARM_MAX) - 1);
    case ARCH_ARM64:
      return ((1ULL << PERF_REG_ARM64_MAX) - 1);
    default:
      return 0;
  }
  return 0;
}

static std::unordered_map<size_t, std::string> x86_reg_map = {
    {PERF_REG_X86_AX, "ax"},       {PERF_REG_X86_BX, "bx"}, {PERF_REG_X86_CX, "cx"},
    {PERF_REG_X86_DX, "dx"},       {PERF_REG_X86_SI, "si"}, {PERF_REG_X86_DI, "di"},
    {PERF_REG_X86_BP, "bp"},       {PERF_REG_X86_SP, "sp"}, {PERF_REG_X86_IP, "ip"},
    {PERF_REG_X86_FLAGS, "flags"}, {PERF_REG_X86_CS, "cs"}, {PERF_REG_X86_SS, "ss"},
    {PERF_REG_X86_DS, "ds"},       {PERF_REG_X86_ES, "es"}, {PERF_REG_X86_FS, "fs"},
    {PERF_REG_X86_GS, "gs"},
};

static std::unordered_map<size_t, std::string> arm_reg_map = {
    {PERF_REG_ARM_FP, "fp"}, {PERF_REG_ARM_IP, "ip"}, {PERF_REG_ARM_SP, "sp"},
    {PERF_REG_ARM_LR, "lr"}, {PERF_REG_ARM_PC, "pc"},
};

static std::unordered_map<size_t, std::string> arm64_reg_map = {
    {PERF_REG_ARM64_LR, "lr"}, {PERF_REG_ARM64_SP, "sp"}, {PERF_REG_ARM64_PC, "pc"},
};

std::string GetRegName(size_t reg) {
  switch (GetCurrentArch()) {
    case ARCH_X86_64: {
      if (reg >= PERF_REG_X86_R8 && reg <= PERF_REG_X86_R15) {
        return android::base::StringPrintf("r%zu", reg - PERF_REG_X86_R8 + 8);
      }
    }  // go through
    case ARCH_X86_32: {
      auto it = x86_reg_map.find(reg);
      CHECK(it != x86_reg_map.end()) << "unknown reg " << reg;
      return it->second;
    }
    case ARCH_ARM: {
      if (reg >= PERF_REG_ARM_R0 && reg <= PERF_REG_ARM_R10) {
        return android::base::StringPrintf("r%zu", reg - PERF_REG_ARM_R0);
      }
      auto it = arm_reg_map.find(reg);
      CHECK(it != arm_reg_map.end()) << "unknown reg " << reg;
      return it->second;
    }
    case ARCH_ARM64: {
      if (reg >= PERF_REG_ARM64_X0 && reg <= PERF_REG_ARM64_X29) {
        return android::base::StringPrintf("r%zu", reg - PERF_REG_ARM64_X0);
      }
      auto it = arm64_reg_map.find(reg);
      CHECK(it != arm64_reg_map.end()) << "unknown reg " << reg;
      return it->second;
    }
    case ARCH_UNSUPPORTED:
      return "unknown";
  }
}