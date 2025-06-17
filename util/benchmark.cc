// Copyright 2009 The RE2 Authors.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#define BENCHMARK_FAMILY_ID    "re2"

#include <benchmark/benchmark.h>

#include "monolithic_examples.h"


#if defined(BUILD_MONOLITHIC)
#define main  re2_benchmark_main
#endif

BENCHMARK_MAIN();
