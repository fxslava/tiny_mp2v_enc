// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>

uint32_t bit_scan_reverse(uint32_t x);
uint32_t bit_scan_forward(uint32_t x);
uint64_t bit_scan_forward64(uint64_t x);