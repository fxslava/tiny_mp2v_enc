// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#pragma once
#include <stdint.h>

/* start codes */
constexpr uint8_t picture_start_code   = 0x00;
constexpr uint8_t slice_start_code_min = 0x01;
constexpr uint8_t slice_start_code_max = 0xaf;
constexpr uint8_t user_data_start_code = 0xb2;
constexpr uint8_t sequence_header_code = 0xb3;
constexpr uint8_t sequence_error_code  = 0xb4;
constexpr uint8_t extension_start_code = 0xb5;
constexpr uint8_t sequence_end_code    = 0xb7;
constexpr uint8_t group_start_code     = 0xb8;

/* extension ids */
constexpr uint8_t sequence_extension_id                  = 1;
constexpr uint8_t sequence_display_extension_id          = 2;
constexpr uint8_t quant_matrix_extension_id              = 3;
constexpr uint8_t copiright_extension_id                 = 4;
constexpr uint8_t sequence_scalable_extension_id         = 5;
constexpr uint8_t picture_display_extension_id           = 7;
constexpr uint8_t picture_coding_extension_id            = 8; //?
constexpr uint8_t picture_spatial_scalable_extension_id  = 9;
constexpr uint8_t picture_temporal_scalable_extension_id = 10;

// ISO/IEC 13818-2 : 2000 (E) 6.2.2.1
struct sequence_header_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t sequence_header_code;                     // | 32          | bslbf
    uint32_t horizontal_size_value;                    // | 12          | uimsbf
    uint32_t vertical_size_value;                      // | 12          | uimsbf
    uint32_t aspect_ratio_information;                 // | 4           | uimsbf
    uint32_t frame_rate_code;                          // | 4           | uimsbf
    uint32_t bit_rate_value;                           // | 18          | uimsbf
    uint32_t vbv_buffer_size_value;                    // | 10          | uimsbf
    uint32_t constrained_parameters_flag;              // | 1           | bslbf
    uint32_t load_intra_quantiser_matrix;              // | 1           | uimsbf
    uint8_t  intra_quantiser_matrix[64];               // | 8 * 64      | uimsbf
    uint32_t load_non_intra_quantiser_matrix;          // | 1           | uimsbf
    uint8_t  non_intra_quantiser_matrix[64];           // | 8 * 64      | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.2.2
// extension_and_user_data( i )

// ISO/IEC 13818-2 : 2000 (E) 6.2.2.2.1
// extension_data( i )

// ISO/IEC 13818-2 : 2000 (E) 6.2.2.2.2
// user_data()

// ISO/IEC 13818-2 : 2000 (E) 6.2.2.3
struct sequence_extension_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t extension_start_code;                     // | 32          | bslbf
    uint32_t extension_start_code_identifier;          // | 4           | uimsbf
    uint32_t profile_and_level_indication;             // | 8           | uimsbf
    uint32_t progressive_sequence;                     // | 1           | uimsbf
    uint32_t chroma_format;                            // | 2           | uimsbf
    uint32_t horizontal_size_extension;                // | 2           | uimsbf
    uint32_t vertical_size_extension;                  // | 2           | uimsbf
    uint32_t bit_rate_extension;                       // | 12          | uimsbf
    uint32_t vbv_buffer_size_extension;                // | 8           | uimsbf
    uint32_t low_delay;                                // | 1           | uimsbf
    uint32_t frame_rate_extension_n;                   // | 2           | uimsbf
    uint32_t frame_rate_extension_d;                   // | 5           | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.2.4
struct sequence_display_extension_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t extension_start_code_identifier;          // | 4           | uimsbf
    uint32_t video_format;                             // | 3           | uimsbf
    uint32_t colour_description;                       // | 1           | uimsbf
    uint32_t colour_primaries;                         // | 8           | uimsbf
    uint32_t transfer_characteristics;                 // | 8           | uimsbf
    uint32_t matrix_coefficients;                      // | 8           | uimsbf
    uint32_t display_horizontal_size;                  // | 14          | uimsbf
    uint32_t display_vertical_size;                    // | 14          | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.2.5
struct sequence_scalable_extension_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t extension_start_code_identifier;          // | 4           | uimsbf
    uint32_t scalable_mode;                            // | 2           | uimsbf
    uint32_t layer_id;                                 // | 4           | uimsbf
    uint32_t lower_layer_prediction_horizontal_size;   // | 14          | uimsbf
    uint32_t lower_layer_prediction_vertical_size;     // | 14          | uimsbf
    uint32_t horizontal_subsampling_factor_m;          // | 5           | uimsbf
    uint32_t horizontal_subsampling_factor_n;          // | 5           | uimsbf
    uint32_t vertical_subsampling_factor_m;            // | 5           | uimsbf
    uint32_t vertical_subsampling_factor_n;            // | 5           | uimsbf
    uint32_t picture_mux_enable;                       // | 1           | uimsbf
    uint32_t mux_to_progressive_sequence;              // | 1           | uimsbf
    uint32_t picture_mux_order;                        // | 3           | uimsbf
    uint32_t picture_mux_factor;                       // | 3           | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.2.6
struct group_of_pictures_header_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t group_start_code;                         // | 32          | bslbf
    uint32_t time_code;                                // | 25          | uimsbf
    uint32_t closed_gop;                               // | 1           | uimsbf
    uint32_t broken_link;                              // | 1           | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.3
struct picture_header_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t picture_start_code;                       // | 32          | bslbf
    uint32_t temporal_reference;                       // | 10          | uimsbf
    uint32_t picture_coding_type;                      // | 3           | uimsbf
    uint32_t vbv_delay;                                // | 16          | uimsbf
    uint32_t full_pel_forward_vector;                  // | 1           | bslbf
    uint32_t forward_f_code;                           // | 3           | bslbf
    uint32_t full_pel_backward_vector;                 // | 1           | bslbf
    uint32_t backward_f_code;                          // | 3           | bslbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.3.1
struct picture_coding_extension_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t extension_start_code;                     // | 32          | bslbf
    uint32_t extension_start_code_identifier;          // | 4           | uimsbf
    uint32_t f_code[4];                                // | 4           | uimsbf
    uint32_t intra_dc_precision;                       // | 2           | uimsbf
    uint32_t picture_structure;                        // | 2           | uimsbf
    uint32_t top_field_first;                          // | 1           | uimsbf
    uint32_t frame_pred_frame_dct;                     // | 1           | uimsbf
    uint32_t concealment_motion_vectors;               // | 1           | uimsbf
    uint32_t q_scale_type;                             // | 1           | uimsbf
    uint32_t intra_vlc_format;                         // | 1           | uimsbf
    uint32_t alternate_scan;                           // | 1           | uimsbf
    uint32_t repeat_first_field;                       // | 1           | uimsbf
    uint32_t chroma_420_type;                          // | 1           | uimsbf
    uint32_t progressive_frame;                        // | 1           | uimsbf
    uint32_t composite_display_flag;                   // | 1           | uimsbf
    uint32_t v_axis;                                   // | 1           | uimsbf
    uint32_t field_sequence;                           // | 3           | uimsbf
    uint32_t sub_carrier;                              // | 1           | uimsbf
    uint32_t burst_amplitude;                          // | 7           | uimsbf
    uint32_t sub_carrier_phase;                        // | 8           | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.3.2
struct quant_matrix_extension_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t extension_start_code_identifier;          // | 4           | uimsbf
    uint32_t load_intra_quantiser_matrix;              // | 1           | uimsbf
    uint8_t  intra_quantiser_matrix[64];               // | 8 * 64      | uimsbf
    uint32_t load_non_intra_quantiser_matrix;          // | 1           | uimsbf
    uint8_t  non_intra_quantiser_matrix[64];           // | 8 * 64      | uimsbf
    uint32_t load_chroma_intra_quantiser_matrix;       // | 1           | uimsbf
    uint8_t  chroma_intra_quantiser_matrix[64];        // | 8 * 64      | uimsbf
    uint32_t load_chroma_non_intra_quantiser_matrix;   // | 1           | uimsbf
    uint8_t  chroma_non_intra_quantiser_matrix[64];    // | 8 * 64      | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.3.3
struct picture_display_extension_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t extension_start_code_identifier;          // | 4           | uimsbf
    uint32_t frame_centre_horizontal_offset[3];        // | 16          | simsbf
    uint32_t frame_centre_vertical_offset[3];          // | 16          | simsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.3.4
struct picture_temporal_scalable_extension_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t extension_start_code_identifier;          // | 4           | uimsbf
    uint32_t reference_select_code;                    // | 2           | uimsbf
    uint32_t forward_temporal_reference;               // | 10          | uimsbf
    uint32_t backward_temporal_reference;              // | 10          | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.3.5
struct picture_spatial_scalable_extension_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t extension_start_code_identifier;          // | 4           | uimsbf
    uint32_t lower_layer_temporal_reference;           // | 10          | uimsbf
    uint32_t lower_layer_horizontal_offset;            // | 15          | simsbf
    uint32_t lower_layer_vertical_offset;              // | 15          | simsbf
    uint32_t spatial_temporal_weight_code_table_index; // | 2           | uimsbf
    uint32_t lower_layer_progressive_frame;            // | 1           | uimsbf
    uint32_t lower_layer_deinterlaced_field_select;    // | 1           | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.3.6
struct copyright_extension_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t extension_start_code_identifier;          // | 4           | uimsbf
    uint32_t copyright_flag;                           // | 1           | uimsbf
    uint32_t copyright_identifier;                     // | 8           | uimsbf
    uint32_t original_or_copy;                         // | 1           | uimsbf
    uint32_t reserved;                                 // | 7           | bslbf
    uint32_t copyright_number_1;                       // | 20          | uimsbf
    uint32_t copyright_number_2;                       // | 22          | uimsbf
    uint32_t copyright_number_3;                       // | 22          | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.3.6
// picture_data()

// ISO/IEC 13818-2 : 2000 (E) 6.2.3.7.1
struct camera_parameters_extension_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t extension_start_code_identifier;          // | 4           | uimsbf
    uint32_t reserved0;                                // | 1           | uimsbf
    uint32_t camera_id;                                // | 7           | simsbf
    uint32_t height_of_image_device;                   // | 22          | uimsbf
    uint32_t focal_length;                             // | 22          | uimsbf
    uint32_t f_number;                                 // | 22          | uimsbf
    uint32_t vertical_angle_of_view;                   // | 22          | uimsbf
    uint32_t camera_position_x;                        // | 16 * 2      | simsbf
    uint32_t camera_position_y;                        // | 16 * 2      | simsbf
    uint32_t camera_position_z;                        // | 16 * 2      | simsbf
    uint32_t camera_direction_x;                       // | 22          | simsbf
    uint32_t camera_direction_y;                       // | 22          | simsbf
    uint32_t camera_direction_z;                       // | 22          | simsbf
    uint32_t image_plane_vertical_x;                   // | 22          | simsbf
    uint32_t image_plane_vertical_y;                   // | 22          | simsbf
    uint32_t image_plane_vertical_z;                   // | 22          | simsbf
    uint32_t reserved1;                                // | 32          | bslbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.3.7.2
// ITU-T_extension()

// ISO/IEC 13818-2 : 2000 (E) 6.2.4
struct slice_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t slice_start_code;                         // | 32          | bslbf
    uint32_t slice_vertical_position_extension;        // | 3           | uimsbf
    uint32_t priority_breakpoint;                      // | 7           | uimsbf
    uint32_t quantiser_scale_code;                     // | 5           | uimsbf
    uint32_t slice_extension_flag;                     // | 1           | bslbf
    uint32_t intra_slice;                              // | 1           | uimsbf
    uint32_t slice_picture_id_enable;                  // | 1           | uimsbf
    uint32_t slice_picture_id;                         // | 6           | uimbsf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.5.1
struct macroblock_modes_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t macroblock_type;                          // | 1 - 9       | vlclbf
    uint32_t spatial_temporal_weight_code;             // | 2           | uimsbf
    uint32_t frame_motion_type;                        // | 2           | uimsbf
    uint32_t field_motion_type;                        // | 2           | uimsbf
    uint32_t dct_type;                                 // | 1           | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.5.2.1
struct motion_vector_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t motion_code[2][2][2];                     // | 1 - 11      | vlclbf
    uint32_t motion_residual[2][2][2];                 // | 1 - 8       | uimsbf
    uint32_t dmvector[2];                              // | 1 - 2       | vlclbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.5.2
struct motion_vectors_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t motion_vertical_field_select[2][2];       // | 1           | uimsbf
    motion_vector_t motion_vector[2];                  // |             |
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.5
struct coded_block_pattern_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    uint32_t coded_block_pattern_420;                  // | 3 - 9       | vlclbf
    uint32_t coded_block_pattern_1;                    // | 2           | uimsbf
    uint32_t coded_block_pattern_2;                    // | 6           | uimsbf
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.5
struct macroblock_t {
    //     | Syntax element                               | No. of bits | Mnemonic
    //while (nextbits() = = '0000 0001 000')              |             |
    //  macroblock_escape                                 | 11          | bslbf
    //macroblock_address_increment                        | 1 - 11      | vlclbf
    uint32_t macroblock_address_increment;             // |             |
    macroblock_modes_t macroblock_modes;               // |             |
    uint32_t quantiser_scale_code;                     // | 5           | uimsbf
    motion_vectors_t motion_vectors[2];                // |             |
    coded_block_pattern_t coded_block_pattern;         // |             |
};

// ISO/IEC 13818-2 : 2000 (E) 6.2.2
struct video_sequence_t {
    sequence_header_t    sequence_header;
    sequence_extension_t sequence_extension;
};