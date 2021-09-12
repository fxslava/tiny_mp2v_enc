// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.
#include "scan.h"

uint8_t g_scan_trans[2][64] = {
  { 0,	17,	12,	27,	35,	29,	58,	53,
    1,	24,	19,	20,	42,	22,	59,	60,
    8,	32,	26,	13,	49,	15,	52,	61,
    16,	25,	33,	6,	56,	23,	45,	54,
    9,	18,	40,	7,	57,	30,	38,	47,
    2,	11,	48,	14,	50,	37,	31,	55,
    3,	4,	41,	21,	43,	44,	39,	62,
    10,	5,	34,	28,	36,	51,	46,	63 },
  { 0,	17,	41,	19,	51,	21,	53,	38, // alternate scan
    8,	25,	33,	27,	59,	29,	61,	46,
    16,	32,	26,	34,	20,	36,	22,	54,
    24,	40,	18,	42,	28,	44,	30,	62,
    1,	48,	3,	50,	5,	52,	7,	39,
    9,	56,	11,	58,	13,	60,	15,	47,
    2,	57,	4,	35,	6,	37,	23,	55,
    10,	49,	12,	43,	14,	45,	31,	63 } };
