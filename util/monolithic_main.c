
#include "monolithic_examples.h"

#define USAGE_NAME   "re2"

#include "monolithic_main_internal_defs.h"

MONOLITHIC_CMD_TABLE_START()

	{ "benchmark", {.fa = re2_benchmark_main } },
	{ "basic_test", { .f = re2_basic_test_main } },
	{ "orpheus", { .fa = re2_orpheus_test_main } },

MONOLITHIC_CMD_TABLE_END();

#include "monolithic_main_tpl.h"
