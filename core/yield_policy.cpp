/*
 *  Copyright 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "yield_policy.h"
#include <assert.h>

// #include "absl/base/attributes.h"
// #include "absl/base/config.h"
// #include "rtc_base/checks.h"
#if !defined(CORE_HAVE_THREAD_LOCAL) && defined(CORE_POSIX)
#include <pthread.h>
#endif

// CORE_HAVE_CPP_ATTRIBUTE
//
// A function-like feature checking macro that accepts C++11 style attributes.
// It's a wrapper around `__has_cpp_attribute`, defined by ISO C++ SD-6
// (https://en.cppreference.com/w/cpp/experimental/feature_test). If we don't
// find `__has_cpp_attribute`, will evaluate to 0.
#if defined(__cplusplus) && defined(__has_cpp_attribute)
// NOTE: requiring __cplusplus above should not be necessary, but
// works around https://bugs.llvm.org/show_bug.cgi?id=23435.
#define CORE_HAVE_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define CORE_HAVE_CPP_ATTRIBUTE(x) 0
#endif


#if defined(__cpp_constinit) && __cpp_constinit >= 201907L
#define CORE_CONST_INIT constinit
#elif CORE_HAVE_CPP_ATTRIBUTE(clang::require_constant_initialization)
#define CORE_CONST_INIT [[clang::require_constant_initialization]]
#else
#define CORE_CONST_INIT
#endif

namespace core {
    namespace {

#if defined(CORE_HAVE_THREAD_LOCAL)

        CORE_CONST_INIT thread_local YieldInterface* current_yield_policy = nullptr;

        YieldInterface* GetCurrentYieldPolicy() {
            return current_yield_policy;
        }

        void SetCurrentYieldPolicy(YieldInterface* ptr) {
            current_yield_policy = ptr;
        }

#elif defined(CORE_POSIX)

        // Emscripten does not support the C++11 thread_local keyword but does support
        // the pthread thread-local storage API.
        // https://github.com/emscripten-core/emscripten/issues/3502

        CORE_CONST_INIT pthread_key_t g_current_yield_policy_tls = 0;

        void InitializeTls() {
            assert(pthread_key_create(&g_current_yield_policy_tls, nullptr) == 0);
        }

        pthread_key_t GetCurrentYieldPolicyTls() {
            static pthread_once_t init_once = PTHREAD_ONCE_INIT;
            assert(pthread_once(&init_once, &InitializeTls) == 0);
            return g_current_yield_policy_tls;
        }

        YieldInterface* GetCurrentYieldPolicy() {
            return static_cast<YieldInterface*>(pthread_getspecific(GetCurrentYieldPolicyTls()));
        }

        void SetCurrentYieldPolicy(YieldInterface* ptr) {
            pthread_setspecific(GetCurrentYieldPolicyTls(), ptr);
        }

#else
#error Unsupported platform
#endif

    }  // namespace

    ScopedYieldPolicy::ScopedYieldPolicy(YieldInterface* policy)
    : previous_(GetCurrentYieldPolicy()) {
        SetCurrentYieldPolicy(policy);
    }

    ScopedYieldPolicy::~ScopedYieldPolicy() {
        SetCurrentYieldPolicy(previous_);
    }

    void ScopedYieldPolicy::YieldExecution() {
        YieldInterface* current = GetCurrentYieldPolicy();
        if (current) {
            current->YieldExecution();
        }
    }

}  // namespace core
