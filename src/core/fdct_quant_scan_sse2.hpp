// Copyright ? 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>
#include <algorithm>
#include <emmintrin.h>
#include "common/cpu.hpp"
#include "quant_scan_c.hpp"

ALIGN(16) int16_t quant_mult_intra_tbl[31][64] = {
    { 4096, 2048, 1725, 1489, 1489, 1260, 1260, 1214, 2048, 2048, 1489, 1489, 1260, 1214, 1214, 1130, 1725, 1489, 1260, 1260, 1214, 1130, 1130, 936, 1489, 1365, 1214, 1214, 1130, 1024, 964, 862, 1260, 1214, 1130, 1130, 1024, 936, 862, 712, 1214, 1130, 964, 964, 936, 819, 712, 585, 1130, 964, 964, 886, 819, 683, 585, 475, 964, 886, 862, 819, 683, 565, 475, 395, },
    { 2048, 1024, 862, 745, 745, 630, 630, 607, 1024, 1024, 745, 745, 630, 607, 607, 565, 862, 745, 630, 630, 607, 565, 565, 468, 745, 683, 607, 607, 565, 512, 482, 431, 630, 607, 565, 565, 512, 468, 431, 356, 607, 565, 482, 482, 468, 410, 356, 293, 565, 482, 482, 443, 410, 341, 293, 237, 482, 443, 431, 410, 341, 282, 237, 197, },
    { 1365, 683, 575, 496, 496, 420, 420, 405, 683, 683, 496, 496, 420, 405, 405, 377, 575, 496, 420, 420, 405, 377, 377, 312, 496, 455, 405, 405, 377, 341, 321, 287, 420, 405, 377, 377, 341, 312, 287, 237, 405, 377, 321, 321, 312, 273, 237, 195, 377, 321, 321, 295, 273, 228, 195, 158, 321, 295, 287, 273, 228, 188, 158, 132, },
    { 1024, 512, 431, 372, 372, 315, 315, 303, 512, 512, 372, 372, 315, 303, 303, 282, 431, 372, 315, 315, 303, 282, 282, 234, 372, 341, 303, 303, 282, 256, 241, 216, 315, 303, 282, 282, 256, 234, 216, 178, 303, 282, 241, 241, 234, 205, 178, 146, 282, 241, 241, 221, 205, 171, 146, 119, 241, 221, 216, 205, 171, 141, 119, 99, },
    { 819, 410, 345, 298, 298, 252, 252, 243, 410, 410, 298, 298, 252, 243, 243, 226, 345, 298, 252, 252, 243, 226, 226, 187, 298, 273, 243, 243, 226, 205, 193, 172, 252, 243, 226, 226, 205, 187, 172, 142, 243, 226, 193, 193, 187, 164, 142, 117, 226, 193, 193, 177, 164, 137, 117, 95, 193, 177, 172, 164, 137, 113, 95, 79, },
    { 683, 341, 287, 248, 248, 210, 210, 202, 341, 341, 248, 248, 210, 202, 202, 188, 287, 248, 210, 210, 202, 188, 188, 156, 248, 228, 202, 202, 188, 171, 161, 144, 210, 202, 188, 188, 171, 156, 144, 119, 202, 188, 161, 161, 156, 137, 119, 98, 188, 161, 161, 148, 137, 114, 98, 79, 161, 148, 144, 137, 114, 94, 79, 66, },
    { 585, 293, 246, 213, 213, 180, 180, 173, 293, 293, 213, 213, 180, 173, 173, 161, 246, 213, 180, 180, 173, 161, 161, 134, 213, 195, 173, 173, 161, 146, 138, 123, 180, 173, 161, 161, 146, 134, 123, 102, 173, 161, 138, 138, 134, 117, 102, 84, 161, 138, 138, 127, 117, 98, 84, 68, 138, 127, 123, 117, 98, 81, 68, 56, },
    { 512, 256, 216, 186, 186, 158, 158, 152, 256, 256, 186, 186, 158, 152, 152, 141, 216, 186, 158, 158, 152, 141, 141, 117, 186, 171, 152, 152, 141, 128, 120, 108, 158, 152, 141, 141, 128, 117, 108, 89, 152, 141, 120, 120, 117, 102, 89, 73, 141, 120, 120, 111, 102, 85, 73, 59, 120, 111, 108, 102, 85, 71, 59, 49, },
    { 410, 205, 172, 149, 149, 126, 126, 121, 205, 205, 149, 149, 126, 121, 121, 113, 172, 149, 126, 126, 121, 113, 113, 94, 149, 137, 121, 121, 113, 102, 96, 86, 126, 121, 113, 113, 102, 94, 86, 71, 121, 113, 96, 96, 94, 82, 71, 59, 113, 96, 96, 89, 82, 68, 59, 47, 96, 89, 86, 82, 68, 56, 47, 39, },
    { 341, 171, 144, 124, 124, 105, 105, 101, 171, 171, 124, 124, 105, 101, 101, 94, 144, 124, 105, 105, 101, 94, 94, 78, 124, 114, 101, 101, 94, 85, 80, 72, 105, 101, 94, 94, 85, 78, 72, 59, 101, 94, 80, 80, 78, 68, 59, 49, 94, 80, 80, 74, 68, 57, 49, 40, 80, 74, 72, 68, 57, 47, 40, 33, },
    { 293, 146, 123, 106, 106, 90, 90, 87, 146, 146, 106, 106, 90, 87, 87, 81, 123, 106, 90, 90, 87, 81, 81, 67, 106, 98, 87, 87, 81, 73, 69, 62, 90, 87, 81, 81, 73, 67, 62, 51, 87, 81, 69, 69, 67, 59, 51, 42, 81, 69, 69, 63, 59, 49, 42, 34, 69, 63, 62, 59, 49, 40, 34, 28, },
    { 256, 128, 108, 93, 93, 79, 79, 76, 128, 128, 93, 93, 79, 76, 76, 71, 108, 93, 79, 79, 76, 71, 71, 59, 93, 85, 76, 76, 71, 64, 60, 54, 79, 76, 71, 71, 64, 59, 54, 45, 76, 71, 60, 60, 59, 51, 45, 37, 71, 60, 60, 55, 51, 43, 37, 30, 60, 55, 54, 51, 43, 35, 30, 25, },
    { 228, 114, 96, 83, 83, 70, 70, 67, 114, 114, 83, 83, 70, 67, 67, 63, 96, 83, 70, 70, 67, 63, 63, 52, 83, 76, 67, 67, 63, 57, 54, 48, 70, 67, 63, 63, 57, 52, 48, 40, 67, 63, 54, 54, 52, 46, 40, 33, 63, 54, 54, 49, 46, 38, 33, 26, 54, 49, 48, 46, 38, 31, 26, 22, },
    { 205, 102, 86, 74, 74, 63, 63, 61, 102, 102, 74, 74, 63, 61, 61, 56, 86, 74, 63, 63, 61, 56, 56, 47, 74, 68, 61, 61, 56, 51, 48, 43, 63, 61, 56, 56, 51, 47, 43, 36, 61, 56, 48, 48, 47, 41, 36, 29, 56, 48, 48, 44, 41, 34, 29, 24, 48, 44, 43, 41, 34, 28, 24, 20, },
    { 186, 93, 78, 68, 68, 57, 57, 55, 93, 93, 68, 68, 57, 55, 55, 51, 78, 68, 57, 57, 55, 51, 51, 43, 68, 62, 55, 55, 51, 47, 44, 39, 57, 55, 51, 51, 47, 43, 39, 32, 55, 51, 44, 44, 43, 37, 32, 27, 51, 44, 44, 40, 37, 31, 27, 22, 44, 40, 39, 37, 31, 26, 22, 18, },
    { 171, 85, 72, 62, 62, 53, 53, 51, 85, 85, 62, 62, 53, 51, 51, 47, 72, 62, 53, 53, 51, 47, 47, 39, 62, 57, 51, 51, 47, 43, 40, 36, 53, 51, 47, 47, 43, 39, 36, 30, 51, 47, 40, 40, 39, 34, 30, 24, 47, 40, 40, 37, 34, 28, 24, 20, 40, 37, 36, 34, 28, 24, 20, 16, },
    { 146, 73, 62, 53, 53, 45, 45, 43, 73, 73, 53, 53, 45, 43, 43, 40, 62, 53, 45, 45, 43, 40, 40, 33, 53, 49, 43, 43, 40, 37, 34, 31, 45, 43, 40, 40, 37, 33, 31, 25, 43, 40, 34, 34, 33, 29, 25, 21, 40, 34, 34, 32, 29, 24, 21, 17, 34, 32, 31, 29, 24, 20, 17, 14, },
    { 128, 64, 54, 47, 47, 39, 39, 38, 64, 64, 47, 47, 39, 38, 38, 35, 54, 47, 39, 39, 38, 35, 35, 29, 47, 43, 38, 38, 35, 32, 30, 27, 39, 38, 35, 35, 32, 29, 27, 22, 38, 35, 30, 30, 29, 26, 22, 18, 35, 30, 30, 28, 26, 21, 18, 15, 30, 28, 27, 26, 21, 18, 15, 12, },
    { 114, 57, 48, 41, 41, 35, 35, 34, 57, 57, 41, 41, 35, 34, 34, 31, 48, 41, 35, 35, 34, 31, 31, 26, 41, 38, 34, 34, 31, 28, 27, 24, 35, 34, 31, 31, 28, 26, 24, 20, 34, 31, 27, 27, 26, 23, 20, 16, 31, 27, 27, 25, 23, 19, 16, 13, 27, 25, 24, 23, 19, 16, 13, 11, },
    { 102, 51, 43, 37, 37, 32, 32, 30, 51, 51, 37, 37, 32, 30, 30, 28, 43, 37, 32, 32, 30, 28, 28, 23, 37, 34, 30, 30, 28, 26, 24, 22, 32, 30, 28, 28, 26, 23, 22, 18, 30, 28, 24, 24, 23, 20, 18, 15, 28, 24, 24, 22, 20, 17, 15, 12, 24, 22, 22, 20, 17, 14, 12, 10, },
    { 93, 47, 39, 34, 34, 29, 29, 28, 47, 47, 34, 34, 29, 28, 28, 26, 39, 34, 29, 29, 28, 26, 26, 21, 34, 31, 28, 28, 26, 23, 22, 20, 29, 28, 26, 26, 23, 21, 20, 16, 28, 26, 22, 22, 21, 19, 16, 13, 26, 22, 22, 20, 19, 16, 13, 11, 22, 20, 20, 19, 16, 13, 11, 9, },
    { 85, 43, 36, 31, 31, 26, 26, 25, 43, 43, 31, 31, 26, 25, 25, 24, 36, 31, 26, 26, 25, 24, 24, 20, 31, 28, 25, 25, 24, 21, 20, 18, 26, 25, 24, 24, 21, 20, 18, 15, 25, 24, 20, 20, 20, 17, 15, 12, 24, 20, 20, 18, 17, 14, 12, 10, 20, 18, 18, 17, 14, 12, 10, 8, },
    { 79, 39, 33, 29, 29, 24, 24, 23, 39, 39, 29, 29, 24, 23, 23, 22, 33, 29, 24, 24, 23, 22, 22, 18, 29, 26, 23, 23, 22, 20, 19, 17, 24, 23, 22, 22, 20, 18, 17, 14, 23, 22, 19, 19, 18, 16, 14, 11, 22, 19, 19, 17, 16, 13, 11, 9, 19, 17, 17, 16, 13, 11, 9, 8, },
    { 73, 37, 31, 27, 27, 23, 23, 22, 37, 37, 27, 27, 23, 22, 22, 20, 31, 27, 23, 23, 22, 20, 20, 17, 27, 24, 22, 22, 20, 18, 17, 15, 23, 22, 20, 20, 18, 17, 15, 13, 22, 20, 17, 17, 17, 15, 13, 10, 20, 17, 17, 16, 15, 12, 10, 8, 17, 16, 15, 15, 12, 10, 8, 7, },
    { 64, 32, 27, 23, 23, 20, 20, 19, 32, 32, 23, 23, 20, 19, 19, 18, 27, 23, 20, 20, 19, 18, 18, 15, 23, 21, 19, 19, 18, 16, 15, 13, 20, 19, 18, 18, 16, 15, 13, 11, 19, 18, 15, 15, 15, 13, 11, 9, 18, 15, 15, 14, 13, 11, 9, 7, 15, 14, 13, 13, 11, 9, 7, 6, },
    { 57, 28, 24, 21, 21, 18, 18, 17, 28, 28, 21, 21, 18, 17, 17, 16, 24, 21, 18, 18, 17, 16, 16, 13, 21, 19, 17, 17, 16, 14, 13, 12, 18, 17, 16, 16, 14, 13, 12, 10, 17, 16, 13, 13, 13, 11, 10, 8, 16, 13, 13, 12, 11, 9, 8, 7, 13, 12, 12, 11, 9, 8, 7, 5, },
    { 51, 26, 22, 19, 19, 16, 16, 15, 26, 26, 19, 19, 16, 15, 15, 14, 22, 19, 16, 16, 15, 14, 14, 12, 19, 17, 15, 15, 14, 13, 12, 11, 16, 15, 14, 14, 13, 12, 11, 9, 15, 14, 12, 12, 12, 10, 9, 7, 14, 12, 12, 11, 10, 9, 7, 6, 12, 11, 11, 10, 9, 7, 6, 5, },
    { 47, 23, 20, 17, 17, 14, 14, 14, 23, 23, 17, 17, 14, 14, 14, 13, 20, 17, 14, 14, 14, 13, 13, 11, 17, 16, 14, 14, 13, 12, 11, 10, 14, 14, 13, 13, 12, 11, 10, 8, 14, 13, 11, 11, 11, 9, 8, 7, 13, 11, 11, 10, 9, 8, 7, 5, 11, 10, 10, 9, 8, 6, 5, 4, },
    { 43, 21, 18, 16, 16, 13, 13, 13, 21, 21, 16, 16, 13, 13, 13, 12, 18, 16, 13, 13, 13, 12, 12, 10, 16, 14, 13, 13, 12, 11, 10, 9, 13, 13, 12, 12, 11, 10, 9, 7, 13, 12, 10, 10, 10, 9, 7, 6, 12, 10, 10, 9, 9, 7, 6, 5, 10, 9, 9, 9, 7, 6, 5, 4, },
    { 39, 20, 17, 14, 14, 12, 12, 12, 20, 20, 14, 14, 12, 12, 12, 11, 17, 14, 12, 12, 12, 11, 11, 9, 14, 13, 12, 12, 11, 10, 9, 8, 12, 12, 11, 11, 10, 9, 8, 7, 12, 11, 9, 9, 9, 8, 7, 6, 11, 9, 9, 9, 8, 7, 6, 5, 9, 9, 8, 8, 7, 5, 5, 4, },
    { 37, 18, 15, 13, 13, 11, 11, 11, 18, 18, 13, 13, 11, 11, 11, 10, 15, 13, 11, 11, 11, 10, 10, 8, 13, 12, 11, 11, 10, 9, 9, 8, 11, 11, 10, 10, 9, 8, 8, 6, 11, 10, 9, 9, 8, 7, 6, 5, 10, 9, 9, 8, 7, 6, 5, 4, 9, 8, 8, 7, 6, 5, 4, 4, },
};

ALIGN(16) int16_t quant_rounder_intra_tbl[31][64] = {
    { 8, 16, 19, 22, 22, 26, 26, 27, 16, 16, 22, 22, 26, 27, 27, 29, 19, 22, 26, 26, 27, 29, 29, 35, 22, 24, 27, 27, 29, 32, 34, 38, 26, 27, 29, 29, 32, 35, 38, 46, 27, 29, 34, 34, 35, 40, 46, 56, 29, 34, 34, 37, 40, 48, 56, 69, 34, 37, 38, 40, 48, 58, 69, 83, },
    { 16, 32, 38, 44, 44, 52, 52, 54, 32, 32, 44, 44, 52, 54, 54, 58, 38, 44, 52, 52, 54, 58, 58, 70, 44, 48, 54, 54, 58, 64, 68, 76, 52, 54, 58, 58, 64, 70, 76, 92, 54, 58, 68, 68, 70, 80, 92, 112, 58, 68, 68, 74, 80, 96, 112, 138, 68, 74, 76, 80, 96, 116, 138, 166, },
    { 24, 48, 57, 66, 66, 78, 78, 81, 48, 48, 66, 66, 78, 81, 81, 87, 57, 66, 78, 78, 81, 87, 87, 105, 66, 72, 81, 81, 87, 96, 102, 114, 78, 81, 87, 87, 96, 105, 114, 138, 81, 87, 102, 102, 105, 120, 138, 168, 87, 102, 102, 111, 120, 144, 168, 207, 102, 111, 114, 120, 144, 174, 207, 249, },
    { 32, 64, 76, 88, 88, 104, 104, 108, 64, 64, 88, 88, 104, 108, 108, 116, 76, 88, 104, 104, 108, 116, 116, 140, 88, 96, 108, 108, 116, 128, 136, 152, 104, 108, 116, 116, 128, 140, 152, 184, 108, 116, 136, 136, 140, 160, 184, 224, 116, 136, 136, 148, 160, 192, 224, 276, 136, 148, 152, 160, 192, 232, 276, 332, },
    { 40, 80, 95, 110, 110, 130, 130, 135, 80, 80, 110, 110, 130, 135, 135, 145, 95, 110, 130, 130, 135, 145, 145, 175, 110, 120, 135, 135, 145, 160, 170, 190, 130, 135, 145, 145, 160, 175, 190, 230, 135, 145, 170, 170, 175, 200, 230, 280, 145, 170, 170, 185, 200, 240, 280, 345, 170, 185, 190, 200, 240, 290, 345, 415, },
    { 48, 96, 114, 132, 132, 156, 156, 162, 96, 96, 132, 132, 156, 162, 162, 174, 114, 132, 156, 156, 162, 174, 174, 210, 132, 144, 162, 162, 174, 192, 204, 228, 156, 162, 174, 174, 192, 210, 228, 276, 162, 174, 204, 204, 210, 240, 276, 336, 174, 204, 204, 222, 240, 288, 336, 414, 204, 222, 228, 240, 288, 348, 414, 498, },
    { 56, 112, 133, 154, 154, 182, 182, 189, 112, 112, 154, 154, 182, 189, 189, 203, 133, 154, 182, 182, 189, 203, 203, 245, 154, 168, 189, 189, 203, 224, 238, 266, 182, 189, 203, 203, 224, 245, 266, 322, 189, 203, 238, 238, 245, 280, 322, 392, 203, 238, 238, 259, 280, 336, 392, 483, 238, 259, 266, 280, 336, 406, 483, 581, },
    { 64, 128, 152, 176, 176, 208, 208, 216, 128, 128, 176, 176, 208, 216, 216, 232, 152, 176, 208, 208, 216, 232, 232, 280, 176, 192, 216, 216, 232, 256, 272, 304, 208, 216, 232, 232, 256, 280, 304, 368, 216, 232, 272, 272, 280, 320, 368, 448, 232, 272, 272, 296, 320, 384, 448, 552, 272, 296, 304, 320, 384, 464, 552, 664, },
    { 80, 160, 190, 220, 220, 260, 260, 270, 160, 160, 220, 220, 260, 270, 270, 290, 190, 220, 260, 260, 270, 290, 290, 350, 220, 240, 270, 270, 290, 320, 340, 380, 260, 270, 290, 290, 320, 350, 380, 460, 270, 290, 340, 340, 350, 400, 460, 560, 290, 340, 340, 370, 400, 480, 560, 690, 340, 370, 380, 400, 480, 580, 690, 830, },
    { 96, 192, 228, 264, 264, 312, 312, 324, 192, 192, 264, 264, 312, 324, 324, 348, 228, 264, 312, 312, 324, 348, 348, 420, 264, 288, 324, 324, 348, 384, 408, 456, 312, 324, 348, 348, 384, 420, 456, 552, 324, 348, 408, 408, 420, 480, 552, 672, 348, 408, 408, 444, 480, 576, 672, 828, 408, 444, 456, 480, 576, 696, 828, 996, },
    { 112, 224, 266, 308, 308, 364, 364, 378, 224, 224, 308, 308, 364, 378, 378, 406, 266, 308, 364, 364, 378, 406, 406, 490, 308, 336, 378, 378, 406, 448, 476, 532, 364, 378, 406, 406, 448, 490, 532, 644, 378, 406, 476, 476, 490, 560, 644, 784, 406, 476, 476, 518, 560, 672, 784, 966, 476, 518, 532, 560, 672, 812, 966, 1162, },
    { 128, 256, 304, 352, 352, 416, 416, 432, 256, 256, 352, 352, 416, 432, 432, 464, 304, 352, 416, 416, 432, 464, 464, 560, 352, 384, 432, 432, 464, 512, 544, 608, 416, 432, 464, 464, 512, 560, 608, 736, 432, 464, 544, 544, 560, 640, 736, 896, 464, 544, 544, 592, 640, 768, 896, 1104, 544, 592, 608, 640, 768, 928, 1104, 1328, },
    { 144, 288, 342, 396, 396, 468, 468, 486, 288, 288, 396, 396, 468, 486, 486, 522, 342, 396, 468, 468, 486, 522, 522, 630, 396, 432, 486, 486, 522, 576, 612, 684, 468, 486, 522, 522, 576, 630, 684, 828, 486, 522, 612, 612, 630, 720, 828, 1008, 522, 612, 612, 666, 720, 864, 1008, 1242, 612, 666, 684, 720, 864, 1044, 1242, 1494, },
    { 160, 320, 380, 440, 440, 520, 520, 540, 320, 320, 440, 440, 520, 540, 540, 580, 380, 440, 520, 520, 540, 580, 580, 700, 440, 480, 540, 540, 580, 640, 680, 760, 520, 540, 580, 580, 640, 700, 760, 920, 540, 580, 680, 680, 700, 800, 920, 1120, 580, 680, 680, 740, 800, 960, 1120, 1380, 680, 740, 760, 800, 960, 1160, 1380, 1660, },
    { 176, 352, 418, 484, 484, 572, 572, 594, 352, 352, 484, 484, 572, 594, 594, 638, 418, 484, 572, 572, 594, 638, 638, 770, 484, 528, 594, 594, 638, 704, 748, 836, 572, 594, 638, 638, 704, 770, 836, 1012, 594, 638, 748, 748, 770, 880, 1012, 1232, 638, 748, 748, 814, 880, 1056, 1232, 1518, 748, 814, 836, 880, 1056, 1276, 1518, 1826, },
    { 192, 384, 456, 528, 528, 624, 624, 648, 384, 384, 528, 528, 624, 648, 648, 696, 456, 528, 624, 624, 648, 696, 696, 840, 528, 576, 648, 648, 696, 768, 816, 912, 624, 648, 696, 696, 768, 840, 912, 1104, 648, 696, 816, 816, 840, 960, 1104, 1344, 696, 816, 816, 888, 960, 1152, 1344, 1656, 816, 888, 912, 960, 1152, 1392, 1656, 1992, },
    { 224, 448, 532, 616, 616, 728, 728, 756, 448, 448, 616, 616, 728, 756, 756, 812, 532, 616, 728, 728, 756, 812, 812, 980, 616, 672, 756, 756, 812, 896, 952, 1064, 728, 756, 812, 812, 896, 980, 1064, 1288, 756, 812, 952, 952, 980, 1120, 1288, 1568, 812, 952, 952, 1036, 1120, 1344, 1568, 1932, 952, 1036, 1064, 1120, 1344, 1624, 1932, 2324, },
    { 256, 512, 608, 704, 704, 832, 832, 864, 512, 512, 704, 704, 832, 864, 864, 928, 608, 704, 832, 832, 864, 928, 928, 1120, 704, 768, 864, 864, 928, 1024, 1088, 1216, 832, 864, 928, 928, 1024, 1120, 1216, 1472, 864, 928, 1088, 1088, 1120, 1280, 1472, 1792, 928, 1088, 1088, 1184, 1280, 1536, 1792, 2208, 1088, 1184, 1216, 1280, 1536, 1856, 2208, 2656, },
    { 288, 576, 684, 792, 792, 936, 936, 972, 576, 576, 792, 792, 936, 972, 972, 1044, 684, 792, 936, 936, 972, 1044, 1044, 1260, 792, 864, 972, 972, 1044, 1152, 1224, 1368, 936, 972, 1044, 1044, 1152, 1260, 1368, 1656, 972, 1044, 1224, 1224, 1260, 1440, 1656, 2016, 1044, 1224, 1224, 1332, 1440, 1728, 2016, 2484, 1224, 1332, 1368, 1440, 1728, 2088, 2484, 2988, },
    { 320, 640, 760, 880, 880, 1040, 1040, 1080, 640, 640, 880, 880, 1040, 1080, 1080, 1160, 760, 880, 1040, 1040, 1080, 1160, 1160, 1400, 880, 960, 1080, 1080, 1160, 1280, 1360, 1520, 1040, 1080, 1160, 1160, 1280, 1400, 1520, 1840, 1080, 1160, 1360, 1360, 1400, 1600, 1840, 2240, 1160, 1360, 1360, 1480, 1600, 1920, 2240, 2760, 1360, 1480, 1520, 1600, 1920, 2320, 2760, 3320, },
    { 352, 704, 836, 968, 968, 1144, 1144, 1188, 704, 704, 968, 968, 1144, 1188, 1188, 1276, 836, 968, 1144, 1144, 1188, 1276, 1276, 1540, 968, 1056, 1188, 1188, 1276, 1408, 1496, 1672, 1144, 1188, 1276, 1276, 1408, 1540, 1672, 2024, 1188, 1276, 1496, 1496, 1540, 1760, 2024, 2464, 1276, 1496, 1496, 1628, 1760, 2112, 2464, 3036, 1496, 1628, 1672, 1760, 2112, 2552, 3036, 3652, },
    { 384, 768, 912, 1056, 1056, 1248, 1248, 1296, 768, 768, 1056, 1056, 1248, 1296, 1296, 1392, 912, 1056, 1248, 1248, 1296, 1392, 1392, 1680, 1056, 1152, 1296, 1296, 1392, 1536, 1632, 1824, 1248, 1296, 1392, 1392, 1536, 1680, 1824, 2208, 1296, 1392, 1632, 1632, 1680, 1920, 2208, 2688, 1392, 1632, 1632, 1776, 1920, 2304, 2688, 3312, 1632, 1776, 1824, 1920, 2304, 2784, 3312, 3984, },
    { 416, 832, 988, 1144, 1144, 1352, 1352, 1404, 832, 832, 1144, 1144, 1352, 1404, 1404, 1508, 988, 1144, 1352, 1352, 1404, 1508, 1508, 1820, 1144, 1248, 1404, 1404, 1508, 1664, 1768, 1976, 1352, 1404, 1508, 1508, 1664, 1820, 1976, 2392, 1404, 1508, 1768, 1768, 1820, 2080, 2392, 2912, 1508, 1768, 1768, 1924, 2080, 2496, 2912, 3588, 1768, 1924, 1976, 2080, 2496, 3016, 3588, 4316, },
    { 448, 896, 1064, 1232, 1232, 1456, 1456, 1512, 896, 896, 1232, 1232, 1456, 1512, 1512, 1624, 1064, 1232, 1456, 1456, 1512, 1624, 1624, 1960, 1232, 1344, 1512, 1512, 1624, 1792, 1904, 2128, 1456, 1512, 1624, 1624, 1792, 1960, 2128, 2576, 1512, 1624, 1904, 1904, 1960, 2240, 2576, 3136, 1624, 1904, 1904, 2072, 2240, 2688, 3136, 3864, 1904, 2072, 2128, 2240, 2688, 3248, 3864, 4648, },
    { 512, 1024, 1216, 1408, 1408, 1664, 1664, 1728, 1024, 1024, 1408, 1408, 1664, 1728, 1728, 1856, 1216, 1408, 1664, 1664, 1728, 1856, 1856, 2240, 1408, 1536, 1728, 1728, 1856, 2048, 2176, 2432, 1664, 1728, 1856, 1856, 2048, 2240, 2432, 2944, 1728, 1856, 2176, 2176, 2240, 2560, 2944, 3584, 1856, 2176, 2176, 2368, 2560, 3072, 3584, 4416, 2176, 2368, 2432, 2560, 3072, 3712, 4416, 5312, },
    { 576, 1152, 1368, 1584, 1584, 1872, 1872, 1944, 1152, 1152, 1584, 1584, 1872, 1944, 1944, 2088, 1368, 1584, 1872, 1872, 1944, 2088, 2088, 2520, 1584, 1728, 1944, 1944, 2088, 2304, 2448, 2736, 1872, 1944, 2088, 2088, 2304, 2520, 2736, 3312, 1944, 2088, 2448, 2448, 2520, 2880, 3312, 4032, 2088, 2448, 2448, 2664, 2880, 3456, 4032, 4968, 2448, 2664, 2736, 2880, 3456, 4176, 4968, 5976, },
    { 640, 1280, 1520, 1760, 1760, 2080, 2080, 2160, 1280, 1280, 1760, 1760, 2080, 2160, 2160, 2320, 1520, 1760, 2080, 2080, 2160, 2320, 2320, 2800, 1760, 1920, 2160, 2160, 2320, 2560, 2720, 3040, 2080, 2160, 2320, 2320, 2560, 2800, 3040, 3680, 2160, 2320, 2720, 2720, 2800, 3200, 3680, 4480, 2320, 2720, 2720, 2960, 3200, 3840, 4480, 5520, 2720, 2960, 3040, 3200, 3840, 4640, 5520, 6640, },
    { 704, 1408, 1672, 1936, 1936, 2288, 2288, 2376, 1408, 1408, 1936, 1936, 2288, 2376, 2376, 2552, 1672, 1936, 2288, 2288, 2376, 2552, 2552, 3080, 1936, 2112, 2376, 2376, 2552, 2816, 2992, 3344, 2288, 2376, 2552, 2552, 2816, 3080, 3344, 4048, 2376, 2552, 2992, 2992, 3080, 3520, 4048, 4928, 2552, 2992, 2992, 3256, 3520, 4224, 4928, 6072, 2992, 3256, 3344, 3520, 4224, 5104, 6072, 7304, },
    { 768, 1536, 1824, 2112, 2112, 2496, 2496, 2592, 1536, 1536, 2112, 2112, 2496, 2592, 2592, 2784, 1824, 2112, 2496, 2496, 2592, 2784, 2784, 3360, 2112, 2304, 2592, 2592, 2784, 3072, 3264, 3648, 2496, 2592, 2784, 2784, 3072, 3360, 3648, 4416, 2592, 2784, 3264, 3264, 3360, 3840, 4416, 5376, 2784, 3264, 3264, 3552, 3840, 4608, 5376, 6624, 3264, 3552, 3648, 3840, 4608, 5568, 6624, 7968, },
    { 832, 1664, 1976, 2288, 2288, 2704, 2704, 2808, 1664, 1664, 2288, 2288, 2704, 2808, 2808, 3016, 1976, 2288, 2704, 2704, 2808, 3016, 3016, 3640, 2288, 2496, 2808, 2808, 3016, 3328, 3536, 3952, 2704, 2808, 3016, 3016, 3328, 3640, 3952, 4784, 2808, 3016, 3536, 3536, 3640, 4160, 4784, 5824, 3016, 3536, 3536, 3848, 4160, 4992, 5824, 7176, 3536, 3848, 3952, 4160, 4992, 6032, 7176, 8632, },
    { 896, 1792, 2128, 2464, 2464, 2912, 2912, 3024, 1792, 1792, 2464, 2464, 2912, 3024, 3024, 3248, 2128, 2464, 2912, 2912, 3024, 3248, 3248, 3920, 2464, 2688, 3024, 3024, 3248, 3584, 3808, 4256, 2912, 3024, 3248, 3248, 3584, 3920, 4256, 5152, 3024, 3248, 3808, 3808, 3920, 4480, 5152, 6272, 3248, 3808, 3808, 4144, 4480, 5376, 6272, 7728, 3808, 4144, 4256, 4480, 5376, 6496, 7728, 9296, },
};

int16_t quant_mult_nonintra_tbl[31] = { 4096, 2048, 1365, 1024, 819, 683, 585, 512, 410, 341, 293, 256, 228, 205, 186, 171, 146, 128, 114, 102, 93, 85, 79, 73, 64, 57, 51, 47, 43, 39, 37 };

MP2V_INLINE __m128i _mm_tmp_op1_epi16(__m128i src) {
    const __m128i tmp = _mm_mulhi_epi16(src, _mm_set1_epi16(9598));
    return _mm_subs_epi16(src, _mm_adds_epi16(tmp, tmp));
}

MP2V_INLINE __m128i _mm_tmp_op2_epi16(__m128i src) {
    const __m128i tmp = _mm_mulhi_epi16(src, _mm_set1_epi16(15034));
    return _mm_subs_epi16(src, _mm_adds_epi16(tmp, tmp));
}

MP2V_INLINE __m128i _mm_tmp_op4_epi16(__m128i src) {
    const __m128i tmp = _mm_mulhi_epi16(src, _mm_set1_epi16(10045));
    return _mm_adds_epi16(src, _mm_adds_epi16(tmp, tmp));
}

MP2V_INLINE __m128i _mm_tmp_op5_epi16(__m128i src) {
    const __m128i tmp = _mm_mulhi_epi16(src, _mm_set1_epi16(12540));
    return _mm_adds_epi16(tmp, tmp);
}

template<bool sat>
MP2V_INLINE __m128i _mm_x2_epi16(__m128i tmp) {
    const __m128i res = _mm_adds_epi16(tmp, tmp);
    return sat ? _mm_max_epi16(_mm_min_epi16(res, _mm_set1_epi16(2047)), _mm_set1_epi16(-2048)) : res;
}

template<bool sat = false>
MP2V_INLINE void fdct_1d_sse2(__m128i (&src)[8]) {
    const __m128i v0 = _mm_adds_epi16(src[0], src[7]);
    const __m128i v1 = _mm_adds_epi16(src[1], src[6]);
    const __m128i v2 = _mm_adds_epi16(src[2], src[5]);
    const __m128i v3 = _mm_adds_epi16(src[3], src[4]);
    const __m128i v4 = _mm_subs_epi16(src[3], src[4]);
    const __m128i v5 = _mm_subs_epi16(src[2], src[5]);
    const __m128i v6 = _mm_subs_epi16(src[1], src[6]);
    const __m128i v7 = _mm_subs_epi16(src[0], src[7]);

    const __m128i v8 = _mm_adds_epi16(v0, v3);
    const __m128i v9 = _mm_adds_epi16(v1, v2);
    const __m128i v10 = _mm_subs_epi16(v1, v2);
    const __m128i v11 = _mm_subs_epi16(v0, v3);
    const __m128i v12 = _mm_adds_epi16(v4, v5);
    const __m128i v13 = _mm_tmp_op1_epi16(_mm_adds_epi16(v5, v6));
    const __m128i v14 = _mm_adds_epi16(v6, v7);
    const __m128i v15 = _mm_adds_epi16(v8, v9);
    const __m128i v16 = _mm_subs_epi16(v8, v9);
    const __m128i v17 = _mm_tmp_op1_epi16(_mm_adds_epi16(v10, v11));
    const __m128i v18 = _mm_tmp_op5_epi16(_mm_subs_epi16(v14, v12));
    const __m128i v19 = _mm_subs_epi16(_mm_tmp_op2_epi16(v12), v18);
    const __m128i v20 = _mm_subs_epi16(_mm_tmp_op4_epi16(v14), v18);
    const __m128i v21 = _mm_adds_epi16(v17, v11);
    const __m128i v22 = _mm_subs_epi16(v11, v17);
    const __m128i v23 = _mm_adds_epi16(v13, v7);
    const __m128i v24 = _mm_subs_epi16(v7, v13);
    const __m128i v25 = _mm_adds_epi16(v19, v24);
    const __m128i v26 = _mm_adds_epi16(v23, v20);
    const __m128i v27 = _mm_subs_epi16(v23, v20);
    const __m128i v28 = _mm_subs_epi16(v24, v19);

    src[0] = _mm_x2_epi16<sat>(_mm_mulhi_epi16(v15, _mm_set1_epi16(11585)));
    src[1] = _mm_x2_epi16<sat>(_mm_mulhi_epi16(v26, _mm_set1_epi16(8352)));
    src[2] = _mm_x2_epi16<sat>(_mm_mulhi_epi16(v21, _mm_set1_epi16(8867)));
    src[3] = _mm_x2_epi16<sat>(_mm_mulhi_epi16(v28, _mm_set1_epi16(9852)));
    src[4] = _mm_x2_epi16<sat>(_mm_mulhi_epi16(v16, _mm_set1_epi16(11585)));
    src[5] = _mm_x2_epi16<sat>(_mm_mulhi_epi16(v25, _mm_set1_epi16(14745)));
    src[6] = _mm_x2_epi16<sat>(_mm_subs_epi16(v22, _mm_mulhi_epi16(v22, _mm_set1_epi16(11361))));
    src[7] = _mm_x2_epi16<sat>(_mm_adds_epi16(v27, _mm_mulhi_epi16(v27, _mm_set1_epi16(9223))));
}

MP2V_INLINE void transpose_8x8_sse2(__m128i (&src)[8]) {
    __m128i a03b03 = _mm_unpacklo_epi16(src[0], src[1]);
    __m128i c03d03 = _mm_unpacklo_epi16(src[2], src[3]);
    __m128i e03f03 = _mm_unpacklo_epi16(src[4], src[5]);
    __m128i g03h03 = _mm_unpacklo_epi16(src[6], src[7]);
    __m128i a47b47 = _mm_unpackhi_epi16(src[0], src[1]);
    __m128i c47d47 = _mm_unpackhi_epi16(src[2], src[3]);
    __m128i e47f47 = _mm_unpackhi_epi16(src[4], src[5]);
    __m128i g47h47 = _mm_unpackhi_epi16(src[6], src[7]);

    __m128i a01b01c01d01 = _mm_unpacklo_epi32(a03b03, c03d03);
    __m128i a23b23c23d23 = _mm_unpackhi_epi32(a03b03, c03d03);
    __m128i e01f01g01h01 = _mm_unpacklo_epi32(e03f03, g03h03);
    __m128i e23f23g23h23 = _mm_unpackhi_epi32(e03f03, g03h03);
    __m128i a45b45c45d45 = _mm_unpacklo_epi32(a47b47, c47d47);
    __m128i a67b67c67d67 = _mm_unpackhi_epi32(a47b47, c47d47);
    __m128i e45f45g45h45 = _mm_unpacklo_epi32(e47f47, g47h47);
    __m128i e67f67g67h67 = _mm_unpackhi_epi32(e47f47, g47h47);

    src[0] = _mm_unpacklo_epi64(a01b01c01d01, e01f01g01h01);
    src[1] = _mm_unpackhi_epi64(a01b01c01d01, e01f01g01h01);
    src[2] = _mm_unpacklo_epi64(a23b23c23d23, e23f23g23h23);
    src[3] = _mm_unpackhi_epi64(a23b23c23d23, e23f23g23h23);
    src[4] = _mm_unpacklo_epi64(a45b45c45d45, e45f45g45h45);
    src[5] = _mm_unpackhi_epi64(a45b45c45d45, e45f45g45h45);
    src[6] = _mm_unpacklo_epi64(a67b67c67d67, e67f67g67h67);
    src[7] = _mm_unpackhi_epi64(a67b67c67d67, e67f67g67h67);
}

MP2V_INLINE void quantize_8x8_sse2(__m128i (&buffer)[8], int quantizer_scale, int dc_prec) {
    int16_t DC = (int16_t)buffer[0].m128i_i16[0];
    int32_t QDC = (DC + 3 - dc_prec) >> (3 - dc_prec); // <-- precise rounding, less pulsating effect

    for (int i = 0; i < 8; i++) {
        __m128i val   = buffer[i];
        __m128i sign  = _mm_srai_epi16(val, 15);
        __m128i absv  = _mm_xor_si128(_mm_add_epi16(val, sign), sign);
        __m128i mulWS = _mm_load_si128((__m128i*) & quant_mult_intra_tbl[quantizer_scale - 1][i * 8]);
        __m128i addWS = _mm_load_si128((__m128i*) & quant_rounder_intra_tbl[quantizer_scale - 1][i * 8]);
        __m128i qabs  = _mm_mulhi_epu16(_mm_adds_epu16(_mm_slli_epi16(absv, 5), addWS), mulWS);
        buffer[i] = _mm_sub_epi16(_mm_xor_si128(qabs, sign), sign);
    }

    buffer[0].m128i_i16[0] = QDC; //tmp0 >> (3 - dc_prec);
}

MP2V_INLINE uint64_t make_altscan_sse2(int16_t(&F)[65], __m128i (&v)[8]) {
    __m128i tmp0 = _mm_unpacklo_epi32(v[1], v[2]);                                                 // 10 11 20 21 12 13 22 23
    __m128i res0 = _mm_unpacklo_epi64(v[0], tmp0);                                                 // 01 02 03 04 10 11 20 21
    _mm_store_si128((__m128i*) & F[0], res0);

            tmp0 = _mm_shufflehi_epi16(v[1], _MM_SHUFFLE(2, 3, 0, 1));                             // 10 11 12 13 15 14 17 16
    __m128i tmp1 = _mm_blend_epi16(_mm_srli_si128(v[1], 4), tmp0, 0b11000000);                     // 12 13 14 15 16 17 17 16
    __m128i res1 = _mm_blend_epi16(_mm_srli_si128(v[0], 4), tmp1, 0b11000011);                     // 12 13 04 05 06 07 17 16
    _mm_store_si128((__m128i*) & F[8], res1);

    uint64_t nnz = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_packs_epi16(res0, res1), _mm_setzero_si128()));

            tmp0 = _mm_srli_si128(tmp0, 8);                                                        // 15 14 17 16 00 00 00 00
            tmp1 = _mm_shufflelo_epi16(v[2], _MM_SHUFFLE(2, 3, 1, 0));                             // 20 21 23 22 24 25 26 27
    __m128i tmp2 = _mm_unpacklo_epi32(v[3], v[4]);                                                 // 30 31 40 41 32 33 42 43
    __m128i res2 = _mm_or_si128(_mm_blend_epi16(tmp0, tmp1, 0b00001100), _mm_slli_si128(tmp2, 8)); // 15 14 23 22 30 31 40 41
    _mm_store_si128((__m128i*) & F[16], res2);

            tmp0 = _mm_shuffle_epi32(v[3], _MM_SHUFFLE(2, 0, 3, 1));                               // 32 33 36 37 30 31 34 35
            tmp1 = _mm_srli_si128(tmp1, 4);                                                        // 23 22 24 25 26 27 00 00
    __m128i res3 = _mm_blend_epi16(tmp0, tmp1, 0b00111100);                                        // 32 33 24 25 26 27 34 35
    _mm_store_si128((__m128i*) & F[24], res3);

    nnz |= (uint64_t)_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_packs_epi16(res2, res3), _mm_setzero_si128())) << 16;

            tmp0 = _mm_blend_epi16(_mm_srli_si128(tmp0, 4), v[4], 0b11111100);                     // 36 37 42 43 44 45 46 47
            tmp1 = _mm_unpacklo_epi32(v[5], v[6]);                                                 // 50 51 60 61 52 53 62 63
    __m128i res4 = _mm_unpacklo_epi64(tmp0, tmp1);                                                 // 36 37 42 43 50 51 60 61
    _mm_store_si128((__m128i*) & F[32], res4);

            tmp0 = _mm_shuffle_epi32(v[5], _MM_SHUFFLE(2, 0, 3, 1));                               // 52 53 56 57 50 51 54 55
            tmp1 = _mm_srli_si128(v[4], 4);                                                        // 42 43 44 45 46 47 00 00
    __m128i res5 = _mm_blend_epi16(tmp0, tmp1, 0b00111100);                                        // 52 53 44 45 46 47 54 55
    _mm_store_si128((__m128i*) & F[40], res5);

    nnz |= (uint64_t)_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_packs_epi16(res4, res5), _mm_setzero_si128())) << 32;

            tmp0 = _mm_srli_si128(_mm_unpacklo_epi32(tmp0, v[6]), 8);                              // 56 57 62 63 00 00 00 00
    __m128i res6 = _mm_or_si128(tmp0, _mm_slli_si128(v[7], 8));                                    // 56 57 62 63 70 71 72 73
    _mm_store_si128((__m128i*) & F[48], res6);
    __m128i res7 = _mm_unpackhi_epi64(v[6], v[7]);                                                 // 64 65 66 67 74 75 76 77
    _mm_store_si128((__m128i*) & F[56], res7);

    nnz |= (uint64_t)_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_packs_epi16(res6, res7), _mm_setzero_si128())) << 48;
    F[64] = 0;
    return nnz;
}

template<bool alt_scan, bool intra = true>
uint64_t forward_dct_scan_quant_template(int16_t(&F)[65], uint8_t* src, int stride, int quantizer_scale, int dc_prec) {
    ALIGN(16) int16_t tmp[64];
    __m128i buffer[8];
    for (int i = 0; i < 8; i++)
        buffer[i] = _mm_cvtepu8_epi16(_mm_loadl_epi64((__m128i*) & src[i * stride]));

    fdct_1d_sse2(buffer);
    transpose_8x8_sse2(buffer);
    fdct_1d_sse2<true>(buffer);
    //transpose_8x8_sse2(buffer);
    quantize_8x8_sse2(buffer, quantizer_scale, dc_prec);

    //transpose_8x8_sse2(buffer);
    return make_altscan_sse2(F, buffer);
}