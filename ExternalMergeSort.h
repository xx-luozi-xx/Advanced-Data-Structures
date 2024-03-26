#pragma once
#include <stdio.h>
#include <stddef.h>
#include <tuple>

#include "MinLoserTree.h"
#include "LZ_queue.h"

#define MEMORY_SIZE 1000
#define BLOCK_SIZE 200
#define BLOCK_COUNT (MEMORY_SIZE/BLOCK_SIZE)

#define _EXTERNAL_MERGE_SORT_VIEW__1

#define DISK_MONITOR

#define EXTERNAL_MERGE_SORT_LOG

void externalMergeSort(FILE* input, FILE* output);
