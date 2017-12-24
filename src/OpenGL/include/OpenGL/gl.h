#ifndef __gl_h_
#define __gl_h_

#if defined __gl3_h_ && !(defined GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED)
#warning gl.h and gl3.h are both included.  Compiler will not invoke errors if using removed OpenGL functionality.
#endif

/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: This software was created using the
** OpenGL(R) version 1.2.1 Sample Implementation published by SGI, but has
** not been independently verified as being compliant with the OpenGL(R)
** version 1.2.1 Specification.
*/

// switches to providing function pointers
//#define GL_GLEXT_FUNCTION_POINTERS 1

#include <OpenGL/gltypes.h>
	
#ifndef GL_GLEXT_LEGACY
#include <OpenGL/glext.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* For compatibility with OpenGL v1.0 */
#define GL_LOGIC_OP GL_INDEX_LOGIC_OP
#define GL_TEXTURE_COMPONENTS GL_TEXTURE_INTERNAL_FORMAT

/*************************************************************/

/* Version */
#define GL_VERSION_1_1                    1
#define GL_VERSION_1_2                    1
#define GL_VERSION_1_3                    1
#define GL_VERSION_1_4                    1
#define GL_VERSION_1_5                    1
#define GL_VERSION_2_0                    1
#define GL_VERSION_2_1                    1

/* AccumOp */
#define GL_ACCUM                          0x0100
#define GL_LOAD                           0x0101
#define GL_RETURN                         0x0102
#define GL_MULT                           0x0103
#define GL_ADD                            0x0104

/* AlphaFunction */
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207

/* AttribMask */
#define GL_CURRENT_BIT                    0x00000001
#define GL_POINT_BIT                      0x00000002
#define GL_LINE_BIT                       0x00000004
#define GL_POLYGON_BIT                    0x00000008
#define GL_POLYGON_STIPPLE_BIT            0x00000010
#define GL_PIXEL_MODE_BIT                 0x00000020
#define GL_LIGHTING_BIT                   0x00000040
#define GL_FOG_BIT                        0x00000080
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_ACCUM_BUFFER_BIT               0x00000200
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_VIEWPORT_BIT                   0x00000800
#define GL_TRANSFORM_BIT                  0x00001000
#define GL_ENABLE_BIT                     0x00002000
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_HINT_BIT                       0x00008000
#define GL_EVAL_BIT                       0x00010000
#define GL_LIST_BIT                       0x00020000
#define GL_TEXTURE_BIT                    0x00040000
#define GL_SCISSOR_BIT                    0x00080000
#define GL_ALL_ATTRIB_BITS                0x000fffff

/* BeginMode */
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                          0x0007
#define GL_QUAD_STRIP                     0x0008
#define GL_POLYGON                        0x0009

/* BlendEquationMode */
/*      GL_LOGIC_OP */
/*      GL_FUNC_ADD */
/*      GL_MIN */
/*      GL_MAX */
/*      GL_FUNC_SUBTRACT */
/*      GL_FUNC_REVERSE_SUBTRACT */

/* BlendingFactorDest */
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305
/*      GL_CONSTANT_COLOR */
/*      GL_ONE_MINUS_CONSTANT_COLOR */
/*      GL_CONSTANT_ALPHA */
/*      GL_ONE_MINUS_CONSTANT_ALPHA */

/* BlendingFactorSrc */
/*      GL_ZERO */
/*      GL_ONE */
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
/*      GL_SRC_ALPHA */
/*      GL_ONE_MINUS_SRC_ALPHA */
/*      GL_DST_ALPHA */
/*      GL_ONE_MINUS_DST_ALPHA */
/*      GL_CONSTANT_COLOR */
/*      GL_ONE_MINUS_CONSTANT_COLOR */
/*      GL_CONSTANT_ALPHA */
/*      GL_ONE_MINUS_CONSTANT_ALPHA */

/* Boolean */
#define GL_TRUE                           1
#define GL_FALSE                          0

/* ClearBufferMask */
/*      GL_COLOR_BUFFER_BIT */
/*      GL_ACCUM_BUFFER_BIT */
/*      GL_STENCIL_BUFFER_BIT */
/*      GL_DEPTH_BUFFER_BIT */

/* ClientArrayType */
/*      GL_VERTEX_ARRAY */
/*      GL_NORMAL_ARRAY */
/*      GL_COLOR_ARRAY */
/*      GL_INDEX_ARRAY */
/*      GL_TEXTURE_COORD_ARRAY */
/*      GL_EDGE_FLAG_ARRAY */

/* ClipPlaneName */
#define GL_CLIP_PLANE0                    0x3000
#define GL_CLIP_PLANE1                    0x3001
#define GL_CLIP_PLANE2                    0x3002
#define GL_CLIP_PLANE3                    0x3003
#define GL_CLIP_PLANE4                    0x3004
#define GL_CLIP_PLANE5                    0x3005

/* ColorMaterialFace */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_FRONT_AND_BACK */

/* ColorMaterialParameter */
/*      GL_AMBIENT */
/*      GL_DIFFUSE */
/*      GL_SPECULAR */
/*      GL_EMISSION */
/*      GL_AMBIENT_AND_DIFFUSE */

/* ColorPointerType */
/*      GL_BYTE */
/*      GL_UNSIGNED_BYTE */
/*      GL_SHORT */
/*      GL_UNSIGNED_SHORT */
/*      GL_INT */
/*      GL_UNSIGNED_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* ColorTableParameterPName */
/*      GL_COLOR_TABLE_SCALE */
/*      GL_COLOR_TABLE_BIAS */

/* ColorTableTarget */
/*      GL_COLOR_TABLE */
/*      GL_POST_CONVOLUTION_COLOR_TABLE */
/*      GL_POST_COLOR_MATRIX_COLOR_TABLE */
/*      GL_PROXY_COLOR_TABLE */
/*      GL_PROXY_POST_CONVOLUTION_COLOR_TABLE */
/*      GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE */

/* ConvolutionBorderMode */
/*      GL_REDUCE */
/*      GL_IGNORE_BORDER */
/*      GL_CONSTANT_BORDER */

/* ConvolutionParameter */
/*      GL_CONVOLUTION_BORDER_MODE */
/*      GL_CONVOLUTION_FILTER_SCALE */
/*      GL_CONVOLUTION_FILTER_BIAS */

/* ConvolutionTarget */
/*      GL_CONVOLUTION_1D */
/*      GL_CONVOLUTION_2D */

/* CullFaceMode */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_FRONT_AND_BACK */

/* DataType */
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_2_BYTES                        0x1407
#define GL_3_BYTES                        0x1408
#define GL_4_BYTES                        0x1409
#define GL_DOUBLE                         0x140A

/* DepthFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* DrawBufferMode */
#define GL_NONE                           0
#define GL_FRONT_LEFT                     0x0400
#define GL_FRONT_RIGHT                    0x0401
#define GL_BACK_LEFT                      0x0402
#define GL_BACK_RIGHT                     0x0403
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_LEFT                           0x0406
#define GL_RIGHT                          0x0407
#define GL_FRONT_AND_BACK                 0x0408
#define GL_AUX0                           0x0409
#define GL_AUX1                           0x040A
#define GL_AUX2                           0x040B
#define GL_AUX3                           0x040C

/* Enable */
/*      GL_FOG */
/*      GL_LIGHTING */
/*      GL_TEXTURE_1D */
/*      GL_TEXTURE_2D */
/*      GL_LINE_STIPPLE */
/*      GL_POLYGON_STIPPLE */
/*      GL_CULL_FACE */
/*      GL_ALPHA_TEST */
/*      GL_BLEND */
/*      GL_INDEX_LOGIC_OP */
/*      GL_COLOR_LOGIC_OP */
/*      GL_DITHER */
/*      GL_STENCIL_TEST */
/*      GL_DEPTH_TEST */
/*      GL_CLIP_PLANE0 */
/*      GL_CLIP_PLANE1 */
/*      GL_CLIP_PLANE2 */
/*      GL_CLIP_PLANE3 */
/*      GL_CLIP_PLANE4 */
/*      GL_CLIP_PLANE5 */
/*      GL_LIGHT0 */
/*      GL_LIGHT1 */
/*      GL_LIGHT2 */
/*      GL_LIGHT3 */
/*      GL_LIGHT4 */
/*      GL_LIGHT5 */
/*      GL_LIGHT6 */
/*      GL_LIGHT7 */
/*      GL_TEXTURE_GEN_S */
/*      GL_TEXTURE_GEN_T */
/*      GL_TEXTURE_GEN_R */
/*      GL_TEXTURE_GEN_Q */
/*      GL_MAP1_VERTEX_3 */
/*      GL_MAP1_VERTEX_4 */
/*      GL_MAP1_COLOR_4 */
/*      GL_MAP1_INDEX */
/*      GL_MAP1_NORMAL */
/*      GL_MAP1_TEXTURE_COORD_1 */
/*      GL_MAP1_TEXTURE_COORD_2 */
/*      GL_MAP1_TEXTURE_COORD_3 */
/*      GL_MAP1_TEXTURE_COORD_4 */
/*      GL_MAP2_VERTEX_3 */
/*      GL_MAP2_VERTEX_4 */
/*      GL_MAP2_COLOR_4 */
/*      GL_MAP2_INDEX */
/*      GL_MAP2_NORMAL */
/*      GL_MAP2_TEXTURE_COORD_1 */
/*      GL_MAP2_TEXTURE_COORD_2 */
/*      GL_MAP2_TEXTURE_COORD_3 */
/*      GL_MAP2_TEXTURE_COORD_4 */
/*      GL_POINT_SMOOTH */
/*      GL_LINE_SMOOTH */
/*      GL_POLYGON_SMOOTH */
/*      GL_SCISSOR_TEST */
/*      GL_COLOR_MATERIAL */
/*      GL_NORMALIZE */
/*      GL_AUTO_NORMAL */
/*      GL_VERTEX_ARRAY */
/*      GL_NORMAL_ARRAY */
/*      GL_COLOR_ARRAY */
/*      GL_INDEX_ARRAY */
/*      GL_TEXTURE_COORD_ARRAY */
/*      GL_EDGE_FLAG_ARRAY */
/*      GL_POLYGON_OFFSET_POINT */
/*      GL_POLYGON_OFFSET_LINE */
/*      GL_POLYGON_OFFSET_FILL */
/*      GL_COLOR_TABLE */
/*      GL_POST_CONVOLUTION_COLOR_TABLE */
/*      GL_POST_COLOR_MATRIX_COLOR_TABLE */
/*      GL_CONVOLUTION_1D */
/*      GL_CONVOLUTION_2D */
/*      GL_SEPARABLE_2D */
/*      GL_HISTOGRAM */
/*      GL_MINMAX */
/*      GL_RESCALE_NORMAL */
/*      GL_TEXTURE_3D */

/* ErrorCode */
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_OUT_OF_MEMORY                  0x0505
/*      GL_TABLE_TOO_LARGE */

/* FeedBackMode */
#define GL_2D                             0x0600
#define GL_3D                             0x0601
#define GL_3D_COLOR                       0x0602
#define GL_3D_COLOR_TEXTURE               0x0603
#define GL_4D_COLOR_TEXTURE               0x0604

/* FeedBackToken */
#define GL_PASS_THROUGH_TOKEN             0x0700
#define GL_POINT_TOKEN                    0x0701
#define GL_LINE_TOKEN                     0x0702
#define GL_POLYGON_TOKEN                  0x0703
#define GL_BITMAP_TOKEN                   0x0704
#define GL_DRAW_PIXEL_TOKEN               0x0705
#define GL_COPY_PIXEL_TOKEN               0x0706
#define GL_LINE_RESET_TOKEN               0x0707

/* FogMode */
/*      GL_LINEAR */
#define GL_EXP                            0x0800
#define GL_EXP2                           0x0801

/* FogParameter */
/*      GL_FOG_COLOR */
/*      GL_FOG_DENSITY */
/*      GL_FOG_END */
/*      GL_FOG_INDEX */
/*      GL_FOG_MODE */
/*      GL_FOG_START */

/* FrontFaceDirection */
#define GL_CW                             0x0900
#define GL_CCW                            0x0901

/* GetColorTableParameterPName */
/*      GL_COLOR_TABLE_SCALE */
/*      GL_COLOR_TABLE_BIAS */
/*      GL_COLOR_TABLE_FORMAT */
/*      GL_COLOR_TABLE_WIDTH */
/*      GL_COLOR_TABLE_RED_SIZE */
/*      GL_COLOR_TABLE_GREEN_SIZE */
/*      GL_COLOR_TABLE_BLUE_SIZE */
/*      GL_COLOR_TABLE_ALPHA_SIZE */
/*      GL_COLOR_TABLE_LUMINANCE_SIZE */
/*      GL_COLOR_TABLE_INTENSITY_SIZE */

/* GetConvolutionParameterPName */
/*      GL_CONVOLUTION_BORDER_COLOR */
/*      GL_CONVOLUTION_BORDER_MODE */
/*      GL_CONVOLUTION_FILTER_SCALE */
/*      GL_CONVOLUTION_FILTER_BIAS */
/*      GL_CONVOLUTION_FORMAT */
/*      GL_CONVOLUTION_WIDTH */
/*      GL_CONVOLUTION_HEIGHT */
/*      GL_MAX_CONVOLUTION_WIDTH */
/*      GL_MAX_CONVOLUTION_HEIGHT */

/* GetHistogramParameterPName */
/*      GL_HISTOGRAM_WIDTH */
/*      GL_HISTOGRAM_FORMAT */
/*      GL_HISTOGRAM_RED_SIZE */
/*      GL_HISTOGRAM_GREEN_SIZE */
/*      GL_HISTOGRAM_BLUE_SIZE */
/*      GL_HISTOGRAM_ALPHA_SIZE */
/*      GL_HISTOGRAM_LUMINANCE_SIZE */
/*      GL_HISTOGRAM_SINK */

/* GetMapTarget */
#define GL_COEFF                          0x0A00
#define GL_ORDER                          0x0A01
#define GL_DOMAIN                         0x0A02

/* GetMinmaxParameterPName */
/*      GL_MINMAX_FORMAT */
/*      GL_MINMAX_SINK */

/* GetPixelMap */
/*      GL_PIXEL_MAP_I_TO_I */
/*      GL_PIXEL_MAP_S_TO_S */
/*      GL_PIXEL_MAP_I_TO_R */
/*      GL_PIXEL_MAP_I_TO_G */
/*      GL_PIXEL_MAP_I_TO_B */
/*      GL_PIXEL_MAP_I_TO_A */
/*      GL_PIXEL_MAP_R_TO_R */
/*      GL_PIXEL_MAP_G_TO_G */
/*      GL_PIXEL_MAP_B_TO_B */
/*      GL_PIXEL_MAP_A_TO_A */

/* GetPointerTarget */
/*      GL_VERTEX_ARRAY_POINTER */
/*      GL_NORMAL_ARRAY_POINTER */
/*      GL_COLOR_ARRAY_POINTER */
/*      GL_INDEX_ARRAY_POINTER */
/*      GL_TEXTURE_COORD_ARRAY_POINTER */
/*      GL_EDGE_FLAG_ARRAY_POINTER */

/* GetTarget */
#define GL_CURRENT_COLOR                  0x0B00
#define GL_CURRENT_INDEX                  0x0B01
#define GL_CURRENT_NORMAL                 0x0B02
#define GL_CURRENT_TEXTURE_COORDS         0x0B03
#define GL_CURRENT_RASTER_COLOR           0x0B04
#define GL_CURRENT_RASTER_INDEX           0x0B05
#define GL_CURRENT_RASTER_TEXTURE_COORDS  0x0B06
#define GL_CURRENT_RASTER_POSITION        0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID  0x0B08
#define GL_CURRENT_RASTER_DISTANCE        0x0B09
#define GL_POINT_SMOOTH                   0x0B10
#define GL_POINT_SIZE                     0x0B11
#define GL_POINT_SIZE_RANGE               0x0B12
#define GL_POINT_SIZE_GRANULARITY         0x0B13
#define GL_LINE_SMOOTH                    0x0B20
#define GL_LINE_WIDTH                     0x0B21
#define GL_LINE_WIDTH_RANGE               0x0B22
#define GL_LINE_WIDTH_GRANULARITY         0x0B23
#define GL_LINE_STIPPLE                   0x0B24
#define GL_LINE_STIPPLE_PATTERN           0x0B25
#define GL_LINE_STIPPLE_REPEAT            0x0B26
/*      GL_SMOOTH_POINT_SIZE_RANGE */
/*      GL_SMOOTH_POINT_SIZE_GRANULARITY */
/*      GL_SMOOTH_LINE_WIDTH_RANGE */
/*      GL_SMOOTH_LINE_WIDTH_GRANULARITY */
/*      GL_ALIASED_POINT_SIZE_RANGE */
/*      GL_ALIASED_LINE_WIDTH_RANGE */
#define GL_LIST_MODE                      0x0B30
#define GL_MAX_LIST_NESTING               0x0B31
#define GL_LIST_BASE                      0x0B32
#define GL_LIST_INDEX                     0x0B33
#define GL_POLYGON_MODE                   0x0B40
#define GL_POLYGON_SMOOTH                 0x0B41
#define GL_POLYGON_STIPPLE                0x0B42
#define GL_EDGE_FLAG                      0x0B43
#define GL_CULL_FACE                      0x0B44
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_LIGHTING                       0x0B50
#define GL_LIGHT_MODEL_LOCAL_VIEWER       0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE           0x0B52
#define GL_LIGHT_MODEL_AMBIENT            0x0B53
#define GL_SHADE_MODEL                    0x0B54
#define GL_COLOR_MATERIAL_FACE            0x0B55
#define GL_COLOR_MATERIAL_PARAMETER       0x0B56
#define GL_COLOR_MATERIAL                 0x0B57
#define GL_FOG                            0x0B60
#define GL_FOG_INDEX                      0x0B61
#define GL_FOG_DENSITY                    0x0B62
#define GL_FOG_START                      0x0B63
#define GL_FOG_END                        0x0B64
#define GL_FOG_MODE                       0x0B65
#define GL_FOG_COLOR                      0x0B66
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_TEST                     0x0B71
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_ACCUM_CLEAR_VALUE              0x0B80
#define GL_STENCIL_TEST                   0x0B90
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_MATRIX_MODE                    0x0BA0
#define GL_NORMALIZE                      0x0BA1
#define GL_VIEWPORT                       0x0BA2
#define GL_MODELVIEW_STACK_DEPTH          0x0BA3
#define GL_PROJECTION_STACK_DEPTH         0x0BA4
#define GL_TEXTURE_STACK_DEPTH            0x0BA5
#define GL_MODELVIEW_MATRIX               0x0BA6
#define GL_PROJECTION_MATRIX              0x0BA7
#define GL_TEXTURE_MATRIX                 0x0BA8
#define GL_ATTRIB_STACK_DEPTH             0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH      0x0BB1
#define GL_ALPHA_TEST                     0x0BC0
#define GL_ALPHA_TEST_FUNC                0x0BC1
#define GL_ALPHA_TEST_REF                 0x0BC2
#define GL_DITHER                         0x0BD0
#define GL_BLEND_DST                      0x0BE0
#define GL_BLEND_SRC                      0x0BE1
#define GL_BLEND                          0x0BE2
#define GL_LOGIC_OP_MODE                  0x0BF0
#define GL_INDEX_LOGIC_OP                 0x0BF1
#define GL_COLOR_LOGIC_OP                 0x0BF2
#define GL_AUX_BUFFERS                    0x0C00
#define GL_DRAW_BUFFER                    0x0C01
#define GL_READ_BUFFER                    0x0C02
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_INDEX_CLEAR_VALUE              0x0C20
#define GL_INDEX_WRITEMASK                0x0C21
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_INDEX_MODE                     0x0C30
#define GL_RGBA_MODE                      0x0C31
#define GL_DOUBLEBUFFER                   0x0C32
#define GL_STEREO                         0x0C33
#define GL_RENDER_MODE                    0x0C40
#define GL_PERSPECTIVE_CORRECTION_HINT    0x0C50
#define GL_POINT_SMOOTH_HINT              0x0C51
#define GL_LINE_SMOOTH_HINT               0x0C52
#define GL_POLYGON_SMOOTH_HINT            0x0C53
#define GL_FOG_HINT                       0x0C54
#define GL_TEXTURE_GEN_S                  0x0C60
#define GL_TEXTURE_GEN_T                  0x0C61
#define GL_TEXTURE_GEN_R                  0x0C62
#define GL_TEXTURE_GEN_Q                  0x0C63
#define GL_PIXEL_MAP_I_TO_I               0x0C70
#define GL_PIXEL_MAP_S_TO_S               0x0C71
#define GL_PIXEL_MAP_I_TO_R               0x0C72
#define GL_PIXEL_MAP_I_TO_G               0x0C73
#define GL_PIXEL_MAP_I_TO_B               0x0C74
#define GL_PIXEL_MAP_I_TO_A               0x0C75
#define GL_PIXEL_MAP_R_TO_R               0x0C76
#define GL_PIXEL_MAP_G_TO_G               0x0C77
#define GL_PIXEL_MAP_B_TO_B               0x0C78
#define GL_PIXEL_MAP_A_TO_A               0x0C79
#define GL_PIXEL_MAP_I_TO_I_SIZE          0x0CB0
#define GL_PIXEL_MAP_S_TO_S_SIZE          0x0CB1
#define GL_PIXEL_MAP_I_TO_R_SIZE          0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE          0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE          0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE          0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE          0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE          0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE          0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE          0x0CB9
#define GL_UNPACK_SWAP_BYTES              0x0CF0
#define GL_UNPACK_LSB_FIRST               0x0CF1
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_UNPACK_SKIP_ROWS               0x0CF3
#define GL_UNPACK_SKIP_PIXELS             0x0CF4
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_SWAP_BYTES                0x0D00
#define GL_PACK_LSB_FIRST                 0x0D01
#define GL_PACK_ROW_LENGTH                0x0D02
#define GL_PACK_SKIP_ROWS                 0x0D03
#define GL_PACK_SKIP_PIXELS               0x0D04
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAP_COLOR                      0x0D10
#define GL_MAP_STENCIL                    0x0D11
#define GL_INDEX_SHIFT                    0x0D12
#define GL_INDEX_OFFSET                   0x0D13
#define GL_RED_SCALE                      0x0D14
#define GL_RED_BIAS                       0x0D15
#define GL_ZOOM_X                         0x0D16
#define GL_ZOOM_Y                         0x0D17
#define GL_GREEN_SCALE                    0x0D18
#define GL_GREEN_BIAS                     0x0D19
#define GL_BLUE_SCALE                     0x0D1A
#define GL_BLUE_BIAS                      0x0D1B
#define GL_ALPHA_SCALE                    0x0D1C
#define GL_ALPHA_BIAS                     0x0D1D
#define GL_DEPTH_SCALE                    0x0D1E
#define GL_DEPTH_BIAS                     0x0D1F
#define GL_MAX_EVAL_ORDER                 0x0D30
#define GL_MAX_LIGHTS                     0x0D31
#define GL_MAX_CLIP_PLANES                0x0D32
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_PIXEL_MAP_TABLE            0x0D34
#define GL_MAX_ATTRIB_STACK_DEPTH         0x0D35
#define GL_MAX_MODELVIEW_STACK_DEPTH      0x0D36
#define GL_MAX_NAME_STACK_DEPTH           0x0D37
#define GL_MAX_PROJECTION_STACK_DEPTH     0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH        0x0D39
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH  0x0D3B
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_INDEX_BITS                     0x0D51
#define GL_RED_BITS                       0x0D52
#define GL_GREEN_BITS                     0x0D53
#define GL_BLUE_BITS                      0x0D54
#define GL_ALPHA_BITS                     0x0D55
#define GL_DEPTH_BITS                     0x0D56
#define GL_STENCIL_BITS                   0x0D57
#define GL_ACCUM_RED_BITS                 0x0D58
#define GL_ACCUM_GREEN_BITS               0x0D59
#define GL_ACCUM_BLUE_BITS                0x0D5A
#define GL_ACCUM_ALPHA_BITS               0x0D5B
#define GL_NAME_STACK_DEPTH               0x0D70
#define GL_AUTO_NORMAL                    0x0D80
#define GL_MAP1_COLOR_4                   0x0D90
#define GL_MAP1_INDEX                     0x0D91
#define GL_MAP1_NORMAL                    0x0D92
#define GL_MAP1_TEXTURE_COORD_1           0x0D93
#define GL_MAP1_TEXTURE_COORD_2           0x0D94
#define GL_MAP1_TEXTURE_COORD_3           0x0D95
#define GL_MAP1_TEXTURE_COORD_4           0x0D96
#define GL_MAP1_VERTEX_3                  0x0D97
#define GL_MAP1_VERTEX_4                  0x0D98
#define GL_MAP2_COLOR_4                   0x0DB0
#define GL_MAP2_INDEX                     0x0DB1
#define GL_MAP2_NORMAL                    0x0DB2
#define GL_MAP2_TEXTURE_COORD_1           0x0DB3
#define GL_MAP2_TEXTURE_COORD_2           0x0DB4
#define GL_MAP2_TEXTURE_COORD_3           0x0DB5
#define GL_MAP2_TEXTURE_COORD_4           0x0DB6
#define GL_MAP2_VERTEX_3                  0x0DB7
#define GL_MAP2_VERTEX_4                  0x0DB8
#define GL_MAP1_GRID_DOMAIN               0x0DD0
#define GL_MAP1_GRID_SEGMENTS             0x0DD1
#define GL_MAP2_GRID_DOMAIN               0x0DD2
#define GL_MAP2_GRID_SEGMENTS             0x0DD3
#define GL_TEXTURE_1D                     0x0DE0
#define GL_TEXTURE_2D                     0x0DE1
#define GL_FEEDBACK_BUFFER_POINTER        0x0DF0
#define GL_FEEDBACK_BUFFER_SIZE           0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE           0x0DF2
#define GL_SELECTION_BUFFER_POINTER       0x0DF3
#define GL_SELECTION_BUFFER_SIZE          0x0DF4
/*      GL_TEXTURE_BINDING_1D */
/*      GL_TEXTURE_BINDING_2D */
/*      GL_TEXTURE_BINDING_3D */
/*      GL_VERTEX_ARRAY */
/*      GL_NORMAL_ARRAY */
/*      GL_COLOR_ARRAY */
/*      GL_INDEX_ARRAY */
/*      GL_TEXTURE_COORD_ARRAY */
/*      GL_EDGE_FLAG_ARRAY */
/*      GL_VERTEX_ARRAY_SIZE */
/*      GL_VERTEX_ARRAY_TYPE */
/*      GL_VERTEX_ARRAY_STRIDE */
/*      GL_NORMAL_ARRAY_TYPE */
/*      GL_NORMAL_ARRAY_STRIDE */
/*      GL_COLOR_ARRAY_SIZE */
/*      GL_COLOR_ARRAY_TYPE */
/*      GL_COLOR_ARRAY_STRIDE */
/*      GL_INDEX_ARRAY_TYPE */
/*      GL_INDEX_ARRAY_STRIDE */
/*      GL_TEXTURE_COORD_ARRAY_SIZE */
/*      GL_TEXTURE_COORD_ARRAY_TYPE */
/*      GL_TEXTURE_COORD_ARRAY_STRIDE */
/*      GL_EDGE_FLAG_ARRAY_STRIDE */
/*      GL_POLYGON_OFFSET_FACTOR */
/*      GL_POLYGON_OFFSET_UNITS */
/*      GL_COLOR_TABLE */
/*      GL_POST_CONVOLUTION_COLOR_TABLE */
/*      GL_POST_COLOR_MATRIX_COLOR_TABLE */
/*      GL_CONVOLUTION_1D */
/*      GL_CONVOLUTION_2D */
/*      GL_SEPARABLE_2D */
/*      GL_POST_CONVOLUTION_RED_SCALE */
/*      GL_POST_CONVOLUTION_GREEN_SCALE */
/*      GL_POST_CONVOLUTION_BLUE_SCALE */
/*      GL_POST_CONVOLUTION_ALPHA_SCALE */
/*      GL_POST_CONVOLUTION_RED_BIAS */
/*      GL_POST_CONVOLUTION_GREEN_BIAS */
/*      GL_POST_CONVOLUTION_BLUE_BIAS */
/*      GL_POST_CONVOLUTION_ALPHA_BIAS */
/*      GL_COLOR_MATRIX */
/*      GL_COLOR_MATRIX_STACK_DEPTH */
/*      GL_MAX_COLOR_MATRIX_STACK_DEPTH */
/*      GL_POST_COLOR_MATRIX_RED_SCALE */
/*      GL_POST_COLOR_MATRIX_GREEN_SCALE */
/*      GL_POST_COLOR_MATRIX_BLUE_SCALE */
/*      GL_POST_COLOR_MATRIX_ALPHA_SCALE */
/*      GL_POST_COLOR_MATRIX_RED_BIAS */
/*      GL_POST_COLOR_MATRIX_GREEN_BIAS */
/*      GL_POST_COLOR_MATRIX_BLUE_BIAS */
/*      GL_POST_COLOR_MATRIX_ALPHA_BIAS */
/*      GL_HISTOGRAM */
/*      GL_MINMAX */
/*      GL_MAX_ELEMENTS_VERTICES */
/*      GL_MAX_ELEMENTS_INDICES */
/*      GL_RESCALE_NORMAL */
/*      GL_LIGHT_MODEL_COLOR_CONTROL */
/*      GL_PACK_SKIP_IMAGES */
/*      GL_PACK_IMAGE_HEIGHT */
/*      GL_UNPACK_SKIP_IMAGES */
/*      GL_UNPACK_IMAGE_HEIGHT */
/*      GL_TEXTURE_3D */
/*      GL_MAX_3D_TEXTURE_SIZE */
/*      GL_BLEND_COLOR */
/*      GL_BLEND_EQUATION */

/* GetTextureParameter */
/*      GL_TEXTURE_MAG_FILTER */
/*      GL_TEXTURE_MIN_FILTER */
/*      GL_TEXTURE_WRAP_S */
/*      GL_TEXTURE_WRAP_T */
#define GL_TEXTURE_WIDTH                  0x1000
#define GL_TEXTURE_HEIGHT                 0x1001
#define GL_TEXTURE_INTERNAL_FORMAT        0x1003
#define GL_TEXTURE_BORDER_COLOR           0x1004
#define GL_TEXTURE_BORDER                 0x1005
/*      GL_TEXTURE_RED_SIZE */
/*      GL_TEXTURE_GREEN_SIZE */
/*      GL_TEXTURE_BLUE_SIZE */
/*      GL_TEXTURE_ALPHA_SIZE */
/*      GL_TEXTURE_LUMINANCE_SIZE */
/*      GL_TEXTURE_INTENSITY_SIZE */
/*      GL_TEXTURE_PRIORITY */
/*      GL_TEXTURE_RESIDENT */
/*      GL_TEXTURE_DEPTH */
/*      GL_TEXTURE_WRAP_R */
/*      GL_TEXTURE_MIN_LOD */
/*      GL_TEXTURE_MAX_LOD */
/*      GL_TEXTURE_BASE_LEVEL */
/*      GL_TEXTURE_MAX_LEVEL */

/* HintMode */
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102

/* HintTarget */
/*      GL_PERSPECTIVE_CORRECTION_HINT */
/*      GL_POINT_SMOOTH_HINT */
/*      GL_LINE_SMOOTH_HINT */
/*      GL_POLYGON_SMOOTH_HINT */
/*      GL_FOG_HINT */

/* HistogramTarget */
/*      GL_HISTOGRAM */
/*      GL_PROXY_HISTOGRAM */

/* IndexPointerType */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* LightModelColorControl */
/*      GL_SINGLE_COLOR */
/*      GL_SEPARATE_SPECULAR_COLOR */

/* LightModelParameter */
/*      GL_LIGHT_MODEL_AMBIENT */
/*      GL_LIGHT_MODEL_LOCAL_VIEWER */
/*      GL_LIGHT_MODEL_TWO_SIDE */
/*      GL_LIGHT_MODEL_COLOR_CONTROL */

/* LightName */
#define GL_LIGHT0                         0x4000
#define GL_LIGHT1                         0x4001
#define GL_LIGHT2                         0x4002
#define GL_LIGHT3                         0x4003
#define GL_LIGHT4                         0x4004
#define GL_LIGHT5                         0x4005
#define GL_LIGHT6                         0x4006
#define GL_LIGHT7                         0x4007

/* LightParameter */
#define GL_AMBIENT                        0x1200
#define GL_DIFFUSE                        0x1201
#define GL_SPECULAR                       0x1202
#define GL_POSITION                       0x1203
#define GL_SPOT_DIRECTION                 0x1204
#define GL_SPOT_EXPONENT                  0x1205
#define GL_SPOT_CUTOFF                    0x1206
#define GL_CONSTANT_ATTENUATION           0x1207
#define GL_LINEAR_ATTENUATION             0x1208
#define GL_QUADRATIC_ATTENUATION          0x1209

/* InterleavedArrays */
/*      GL_V2F */
/*      GL_V3F */
/*      GL_C4UB_V2F */
/*      GL_C4UB_V3F */
/*      GL_C3F_V3F */
/*      GL_N3F_V3F */
/*      GL_C4F_N3F_V3F */
/*      GL_T2F_V3F */
/*      GL_T4F_V4F */
/*      GL_T2F_C4UB_V3F */
/*      GL_T2F_C3F_V3F */
/*      GL_T2F_N3F_V3F */
/*      GL_T2F_C4F_N3F_V3F */
/*      GL_T4F_C4F_N3F_V4F */

/* ListMode */
#define GL_COMPILE                        0x1300
#define GL_COMPILE_AND_EXECUTE            0x1301

/* ListNameType */
/*      GL_BYTE */
/*      GL_UNSIGNED_BYTE */
/*      GL_SHORT */
/*      GL_UNSIGNED_SHORT */
/*      GL_INT */
/*      GL_UNSIGNED_INT */
/*      GL_FLOAT */
/*      GL_2_BYTES */
/*      GL_3_BYTES */
/*      GL_4_BYTES */

/* LogicOp */
#define GL_CLEAR                          0x1500
#define GL_AND                            0x1501
#define GL_AND_REVERSE                    0x1502
#define GL_COPY                           0x1503
#define GL_AND_INVERTED                   0x1504
#define GL_NOOP                           0x1505
#define GL_XOR                            0x1506
#define GL_OR                             0x1507
#define GL_NOR                            0x1508
#define GL_EQUIV                          0x1509
#define GL_INVERT                         0x150A
#define GL_OR_REVERSE                     0x150B
#define GL_COPY_INVERTED                  0x150C
#define GL_OR_INVERTED                    0x150D
#define GL_NAND                           0x150E
#define GL_SET                            0x150F

/* MapTarget */
/*      GL_MAP1_COLOR_4 */
/*      GL_MAP1_INDEX */
/*      GL_MAP1_NORMAL */
/*      GL_MAP1_TEXTURE_COORD_1 */
/*      GL_MAP1_TEXTURE_COORD_2 */
/*      GL_MAP1_TEXTURE_COORD_3 */
/*      GL_MAP1_TEXTURE_COORD_4 */
/*      GL_MAP1_VERTEX_3 */
/*      GL_MAP1_VERTEX_4 */
/*      GL_MAP2_COLOR_4 */
/*      GL_MAP2_INDEX */
/*      GL_MAP2_NORMAL */
/*      GL_MAP2_TEXTURE_COORD_1 */
/*      GL_MAP2_TEXTURE_COORD_2 */
/*      GL_MAP2_TEXTURE_COORD_3 */
/*      GL_MAP2_TEXTURE_COORD_4 */
/*      GL_MAP2_VERTEX_3 */
/*      GL_MAP2_VERTEX_4 */

/* MaterialFace */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_FRONT_AND_BACK */

/* MaterialParameter */
#define GL_EMISSION                       0x1600
#define GL_SHININESS                      0x1601
#define GL_AMBIENT_AND_DIFFUSE            0x1602
#define GL_COLOR_INDEXES                  0x1603
/*      GL_AMBIENT */
/*      GL_DIFFUSE */
/*      GL_SPECULAR */

/* MatrixMode */
#define GL_MODELVIEW                      0x1700
#define GL_PROJECTION                     0x1701
#define GL_TEXTURE                        0x1702

/* MeshMode1 */
/*      GL_POINT */
/*      GL_LINE */

/* MeshMode2 */
/*      GL_POINT */
/*      GL_LINE */
/*      GL_FILL */

/* MinmaxTarget */
/*      GL_MINMAX */

/* NormalPointerType */
/*      GL_BYTE */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* PixelCopyType */
#define GL_COLOR                          0x1800
#define GL_DEPTH                          0x1801
#define GL_STENCIL                        0x1802

/* PixelFormat */
#define GL_COLOR_INDEX                    0x1900
#define GL_STENCIL_INDEX                  0x1901
#define GL_DEPTH_COMPONENT                0x1902
#define GL_RED                            0x1903
#define GL_GREEN                          0x1904
#define GL_BLUE                           0x1905
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A
/*      GL_ABGR */

/* PixelInternalFormat */
/*      GL_ALPHA4 */
/*      GL_ALPHA8 */
/*      GL_ALPHA12 */
/*      GL_ALPHA16 */
/*      GL_LUMINANCE4 */
/*      GL_LUMINANCE8 */
/*      GL_LUMINANCE12 */
/*      GL_LUMINANCE16 */
/*      GL_LUMINANCE4_ALPHA4 */
/*      GL_LUMINANCE6_ALPHA2 */
/*      GL_LUMINANCE8_ALPHA8 */
/*      GL_LUMINANCE12_ALPHA4 */
/*      GL_LUMINANCE12_ALPHA12 */
/*      GL_LUMINANCE16_ALPHA16 */
/*      GL_INTENSITY */
/*      GL_INTENSITY4 */
/*      GL_INTENSITY8 */
/*      GL_INTENSITY12 */
/*      GL_INTENSITY16 */
/*      GL_R3_G3_B2 */
/*      GL_RGB4 */
/*      GL_RGB5 */
/*      GL_RGB8 */
/*      GL_RGB10 */
/*      GL_RGB12 */
/*      GL_RGB16 */
/*      GL_RGBA2 */
/*      GL_RGBA4 */
/*      GL_RGB5_A1 */
/*      GL_RGBA8 */
/*      GL_RGB10_A2 */
/*      GL_RGBA12 */
/*      GL_RGBA16 */

/* PixelMap */
/*      GL_PIXEL_MAP_I_TO_I */
/*      GL_PIXEL_MAP_S_TO_S */
/*      GL_PIXEL_MAP_I_TO_R */
/*      GL_PIXEL_MAP_I_TO_G */
/*      GL_PIXEL_MAP_I_TO_B */
/*      GL_PIXEL_MAP_I_TO_A */
/*      GL_PIXEL_MAP_R_TO_R */
/*      GL_PIXEL_MAP_G_TO_G */
/*      GL_PIXEL_MAP_B_TO_B */
/*      GL_PIXEL_MAP_A_TO_A */

/* PixelStore */
/*      GL_UNPACK_SWAP_BYTES */
/*      GL_UNPACK_LSB_FIRST */
/*      GL_UNPACK_ROW_LENGTH */
/*      GL_UNPACK_SKIP_ROWS */
/*      GL_UNPACK_SKIP_PIXELS */
/*      GL_UNPACK_ALIGNMENT */
/*      GL_PACK_SWAP_BYTES */
/*      GL_PACK_LSB_FIRST */
/*      GL_PACK_ROW_LENGTH */
/*      GL_PACK_SKIP_ROWS */
/*      GL_PACK_SKIP_PIXELS */
/*      GL_PACK_ALIGNMENT */
/*      GL_PACK_SKIP_IMAGES */
/*      GL_PACK_IMAGE_HEIGHT */
/*      GL_UNPACK_SKIP_IMAGES */
/*      GL_UNPACK_IMAGE_HEIGHT */

/* PixelTransfer */
/*      GL_MAP_COLOR */
/*      GL_MAP_STENCIL */
/*      GL_INDEX_SHIFT */
/*      GL_INDEX_OFFSET */
/*      GL_RED_SCALE */
/*      GL_RED_BIAS */
/*      GL_GREEN_SCALE */
/*      GL_GREEN_BIAS */
/*      GL_BLUE_SCALE */
/*      GL_BLUE_BIAS */
/*      GL_ALPHA_SCALE */
/*      GL_ALPHA_BIAS */
/*      GL_DEPTH_SCALE */
/*      GL_DEPTH_BIAS */
/*      GL_POST_CONVOLUTION_RED_SCALE */
/*      GL_POST_CONVOLUTION_GREEN_SCALE */
/*      GL_POST_CONVOLUTION_BLUE_SCALE */
/*      GL_POST_CONVOLUTION_ALPHA_SCALE */
/*      GL_POST_CONVOLUTION_RED_BIAS */
/*      GL_POST_CONVOLUTION_GREEN_BIAS */
/*      GL_POST_CONVOLUTION_BLUE_BIAS */
/*      GL_POST_CONVOLUTION_ALPHA_BIAS */
/*      GL_POST_COLOR_MATRIX_RED_SCALE */
/*      GL_POST_COLOR_MATRIX_GREEN_SCALE */
/*      GL_POST_COLOR_MATRIX_BLUE_SCALE */
/*      GL_POST_COLOR_MATRIX_ALPHA_SCALE */
/*      GL_POST_COLOR_MATRIX_RED_BIAS */
/*      GL_POST_COLOR_MATRIX_GREEN_BIAS */
/*      GL_POST_COLOR_MATRIX_BLUE_BIAS */
/*      GL_POST_COLOR_MATRIX_ALPHA_BIAS */

/* PixelType */
#define GL_BITMAP                         0x1A00
/*      GL_BYTE */
/*      GL_UNSIGNED_BYTE */
/*      GL_SHORT */
/*      GL_UNSIGNED_SHORT */
/*      GL_INT */
/*      GL_UNSIGNED_INT */
/*      GL_FLOAT */
/*      GL_BGR */
/*      GL_BGRA */
/*      GL_UNSIGNED_BYTE_3_3_2 */
/*      GL_UNSIGNED_SHORT_4_4_4_4 */
/*      GL_UNSIGNED_SHORT_5_5_5_1 */
/*      GL_UNSIGNED_INT_8_8_8_8 */
/*      GL_UNSIGNED_INT_10_10_10_2 */
/*      GL_UNSIGNED_SHORT_5_6_5 */
/*      GL_UNSIGNED_BYTE_2_3_3_REV */
/*      GL_UNSIGNED_SHORT_5_6_5_REV */
/*      GL_UNSIGNED_SHORT_4_4_4_4_REV */
/*      GL_UNSIGNED_SHORT_1_5_5_5_REV */
/*      GL_UNSIGNED_INT_8_8_8_8_REV */
/*      GL_UNSIGNED_INT_2_10_10_10_REV */

/* PolygonMode */
#define GL_POINT                          0x1B00
#define GL_LINE                           0x1B01
#define GL_FILL                           0x1B02

/* ReadBufferMode */
/*      GL_FRONT_LEFT */
/*      GL_FRONT_RIGHT */
/*      GL_BACK_LEFT */
/*      GL_BACK_RIGHT */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_LEFT */
/*      GL_RIGHT */
/*      GL_AUX0 */
/*      GL_AUX1 */
/*      GL_AUX2 */
/*      GL_AUX3 */

/* RenderingMode */
#define GL_RENDER                         0x1C00
#define GL_FEEDBACK                       0x1C01
#define GL_SELECT                         0x1C02

/* SeparableTarget */
/*      GL_SEPARABLE_2D */

/* ShadingModel */
#define GL_FLAT                           0x1D00
#define GL_SMOOTH                         0x1D01

/* StencilFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* StencilOp */
/*      GL_ZERO */
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
/*      GL_INVERT */

/* StringName */
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

/* TextureCoordName */
#define GL_S                              0x2000
#define GL_T                              0x2001
#define GL_R                              0x2002
#define GL_Q                              0x2003

/* TexCoordPointerType */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* TextureEnvMode */
#define GL_MODULATE                       0x2100
#define GL_DECAL                          0x2101
/*      GL_BLEND */
/*      GL_REPLACE */

/* TextureEnvParameter */
#define GL_TEXTURE_ENV_MODE               0x2200
#define GL_TEXTURE_ENV_COLOR              0x2201

/* TextureEnvTarget */
#define GL_TEXTURE_ENV                    0x2300

/* TextureGenMode */
#define GL_EYE_LINEAR                     0x2400
#define GL_OBJECT_LINEAR                  0x2401
#define GL_SPHERE_MAP                     0x2402

/* TextureGenParameter */
#define GL_TEXTURE_GEN_MODE               0x2500
#define GL_OBJECT_PLANE                   0x2501
#define GL_EYE_PLANE                      0x2502

/* TextureMagFilter */
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

/* TextureMinFilter */
/*      GL_NEAREST */
/*      GL_LINEAR */
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703

/* TextureParameterName */
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
/*      GL_TEXTURE_BORDER_COLOR */
/*      GL_TEXTURE_PRIORITY */
/*      GL_TEXTURE_WRAP_R */
/*      GL_TEXTURE_MIN_LOD */
/*      GL_TEXTURE_MAX_LOD */
/*      GL_TEXTURE_BASE_LEVEL */
/*      GL_TEXTURE_MAX_LEVEL */

/* TextureTarget */
/*      GL_TEXTURE_1D */
/*      GL_TEXTURE_2D */
/*      GL_PROXY_TEXTURE_1D */
/*      GL_PROXY_TEXTURE_2D */
/*      GL_TEXTURE_3D */
/*      GL_PROXY_TEXTURE_3D */

/* TextureWrapMode */
#define GL_CLAMP                          0x2900
#define GL_REPEAT                         0x2901
/*      GL_CLAMP_TO_EDGE */

/* VertexPointerType */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* ClientAttribMask */
#define GL_CLIENT_PIXEL_STORE_BIT         0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT        0x00000002
#define GL_CLIENT_ALL_ATTRIB_BITS         0xffffffff

/* polygon_offset */
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_POLYGON_OFFSET_UNITS           0x2A00
#define GL_POLYGON_OFFSET_POINT           0x2A01
#define GL_POLYGON_OFFSET_LINE            0x2A02
#define GL_POLYGON_OFFSET_FILL            0x8037

/* texture */
#define GL_ALPHA4                         0x803B
#define GL_ALPHA8                         0x803C
#define GL_ALPHA12                        0x803D
#define GL_ALPHA16                        0x803E
#define GL_LUMINANCE4                     0x803F
#define GL_LUMINANCE8                     0x8040
#define GL_LUMINANCE12                    0x8041
#define GL_LUMINANCE16                    0x8042
#define GL_LUMINANCE4_ALPHA4              0x8043
#define GL_LUMINANCE6_ALPHA2              0x8044
#define GL_LUMINANCE8_ALPHA8              0x8045
#define GL_LUMINANCE12_ALPHA4             0x8046
#define GL_LUMINANCE12_ALPHA12            0x8047
#define GL_LUMINANCE16_ALPHA16            0x8048
#define GL_INTENSITY                      0x8049
#define GL_INTENSITY4                     0x804A
#define GL_INTENSITY8                     0x804B
#define GL_INTENSITY12                    0x804C
#define GL_INTENSITY16                    0x804D
#define GL_R3_G3_B2                       0x2A10
#define GL_RGB4                           0x804F
#define GL_RGB5                           0x8050
#define GL_RGB8                           0x8051
#define GL_RGB10                          0x8052
#define GL_RGB12                          0x8053
#define GL_RGB16                          0x8054
#define GL_RGBA2                          0x8055
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_RGBA12                         0x805A
#define GL_RGBA16                         0x805B
#define GL_TEXTURE_RED_SIZE               0x805C
#define GL_TEXTURE_GREEN_SIZE             0x805D
#define GL_TEXTURE_BLUE_SIZE              0x805E
#define GL_TEXTURE_ALPHA_SIZE             0x805F
#define GL_TEXTURE_LUMINANCE_SIZE         0x8060
#define GL_TEXTURE_INTENSITY_SIZE         0x8061
#define GL_PROXY_TEXTURE_1D               0x8063
#define GL_PROXY_TEXTURE_2D               0x8064

/* texture_object */
#define GL_TEXTURE_PRIORITY               0x8066
#define GL_TEXTURE_RESIDENT               0x8067
#define GL_TEXTURE_BINDING_1D             0x8068
#define GL_TEXTURE_BINDING_2D             0x8069
#define GL_TEXTURE_BINDING_3D             0x806A

/* vertex_array */
#define GL_VERTEX_ARRAY                   0x8074
#define GL_NORMAL_ARRAY                   0x8075
#define GL_COLOR_ARRAY                    0x8076
#define GL_INDEX_ARRAY                    0x8077
#define GL_TEXTURE_COORD_ARRAY            0x8078
#define GL_EDGE_FLAG_ARRAY                0x8079
#define GL_VERTEX_ARRAY_SIZE              0x807A
#define GL_VERTEX_ARRAY_TYPE              0x807B
#define GL_VERTEX_ARRAY_STRIDE            0x807C
#define GL_NORMAL_ARRAY_TYPE              0x807E
#define GL_NORMAL_ARRAY_STRIDE            0x807F
#define GL_COLOR_ARRAY_SIZE               0x8081
#define GL_COLOR_ARRAY_TYPE               0x8082
#define GL_COLOR_ARRAY_STRIDE             0x8083
#define GL_INDEX_ARRAY_TYPE               0x8085
#define GL_INDEX_ARRAY_STRIDE             0x8086
#define GL_TEXTURE_COORD_ARRAY_SIZE       0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE       0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE     0x808A
#define GL_EDGE_FLAG_ARRAY_STRIDE         0x808C
#define GL_VERTEX_ARRAY_POINTER           0x808E
#define GL_NORMAL_ARRAY_POINTER           0x808F
#define GL_COLOR_ARRAY_POINTER            0x8090
#define GL_INDEX_ARRAY_POINTER            0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER    0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER        0x8093
#define GL_V2F                            0x2A20
#define GL_V3F                            0x2A21
#define GL_C4UB_V2F                       0x2A22
#define GL_C4UB_V3F                       0x2A23
#define GL_C3F_V3F                        0x2A24
#define GL_N3F_V3F                        0x2A25
#define GL_C4F_N3F_V3F                    0x2A26
#define GL_T2F_V3F                        0x2A27
#define GL_T4F_V4F                        0x2A28
#define GL_T2F_C4UB_V3F                   0x2A29
#define GL_T2F_C3F_V3F                    0x2A2A
#define GL_T2F_N3F_V3F                    0x2A2B
#define GL_T2F_C4F_N3F_V3F                0x2A2C
#define GL_T4F_C4F_N3F_V4F                0x2A2D

/* bgra */
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1

/* blend_color */
#define GL_CONSTANT_COLOR                 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR       0x8002
#define GL_CONSTANT_ALPHA                 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA       0x8004
#define GL_BLEND_COLOR                    0x8005

/* blend_minmax */
#define GL_FUNC_ADD                       0x8006
#define GL_MIN                            0x8007
#define GL_MAX                            0x8008
#define GL_BLEND_EQUATION                 0x8009

/* blend_equation_separate */
#define GL_BLEND_EQUATION_RGB             0x8009
#define GL_BLEND_EQUATION_ALPHA           0x883D

/* blend_subtract */
#define GL_FUNC_SUBTRACT                  0x800A
#define GL_FUNC_REVERSE_SUBTRACT          0x800B

/* color_matrix */
#define GL_COLOR_MATRIX                   0x80B1
#define GL_COLOR_MATRIX_STACK_DEPTH       0x80B2
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH   0x80B3
#define GL_POST_COLOR_MATRIX_RED_SCALE    0x80B4
#define GL_POST_COLOR_MATRIX_GREEN_SCALE  0x80B5
#define GL_POST_COLOR_MATRIX_BLUE_SCALE   0x80B6
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE  0x80B7
#define GL_POST_COLOR_MATRIX_RED_BIAS     0x80B8
#define GL_POST_COLOR_MATRIX_GREEN_BIAS   0x80B9
#define GL_POST_COLOR_MATRIX_BLUE_BIAS    0x80BA
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS   0x80BB

/* color_table */
#define GL_COLOR_TABLE                    0x80D0
#define GL_POST_CONVOLUTION_COLOR_TABLE   0x80D1
#define GL_POST_COLOR_MATRIX_COLOR_TABLE  0x80D2
#define GL_PROXY_COLOR_TABLE              0x80D3
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE 0x80D4
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE 0x80D5
#define GL_COLOR_TABLE_SCALE              0x80D6
#define GL_COLOR_TABLE_BIAS               0x80D7
#define GL_COLOR_TABLE_FORMAT             0x80D8
#define GL_COLOR_TABLE_WIDTH              0x80D9
#define GL_COLOR_TABLE_RED_SIZE           0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE         0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE          0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE         0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE     0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE     0x80DF

/* convolution */
#define GL_CONVOLUTION_1D                 0x8010
#define GL_CONVOLUTION_2D                 0x8011
#define GL_SEPARABLE_2D                   0x8012
#define GL_CONVOLUTION_BORDER_MODE        0x8013
#define GL_CONVOLUTION_FILTER_SCALE       0x8014
#define GL_CONVOLUTION_FILTER_BIAS        0x8015
#define GL_REDUCE                         0x8016
#define GL_CONVOLUTION_FORMAT             0x8017
#define GL_CONVOLUTION_WIDTH              0x8018
#define GL_CONVOLUTION_HEIGHT             0x8019
#define GL_MAX_CONVOLUTION_WIDTH          0x801A
#define GL_MAX_CONVOLUTION_HEIGHT         0x801B
#define GL_POST_CONVOLUTION_RED_SCALE     0x801C
#define GL_POST_CONVOLUTION_GREEN_SCALE   0x801D
#define GL_POST_CONVOLUTION_BLUE_SCALE    0x801E
#define GL_POST_CONVOLUTION_ALPHA_SCALE   0x801F
#define GL_POST_CONVOLUTION_RED_BIAS      0x8020
#define GL_POST_CONVOLUTION_GREEN_BIAS    0x8021
#define GL_POST_CONVOLUTION_BLUE_BIAS     0x8022
#define GL_POST_CONVOLUTION_ALPHA_BIAS    0x8023
#define GL_CONSTANT_BORDER                0x8151
#define GL_REPLICATE_BORDER               0x8153
#define GL_CONVOLUTION_BORDER_COLOR       0x8154

/* draw_range_elements */
#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9

/* histogram */
#define GL_HISTOGRAM                      0x8024
#define GL_PROXY_HISTOGRAM                0x8025
#define GL_HISTOGRAM_WIDTH                0x8026
#define GL_HISTOGRAM_FORMAT               0x8027
#define GL_HISTOGRAM_RED_SIZE             0x8028
#define GL_HISTOGRAM_GREEN_SIZE           0x8029
#define GL_HISTOGRAM_BLUE_SIZE            0x802A
#define GL_HISTOGRAM_ALPHA_SIZE           0x802B
#define GL_HISTOGRAM_LUMINANCE_SIZE       0x802C
#define GL_HISTOGRAM_SINK                 0x802D
#define GL_MINMAX                         0x802E
#define GL_MINMAX_FORMAT                  0x802F
#define GL_MINMAX_SINK                    0x8030
#define GL_TABLE_TOO_LARGE                0x8031

/* packed_pixels */
#define GL_UNSIGNED_BYTE_3_3_2            0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_UNSIGNED_INT_10_10_10_2        0x8036
#define GL_UNSIGNED_BYTE_2_3_3_REV        0x8362
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV     0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368

/* rescale_normal */
#define GL_RESCALE_NORMAL                 0x803A

/* separate_specular_color */
#define GL_LIGHT_MODEL_COLOR_CONTROL      0x81F8
#define GL_SINGLE_COLOR                   0x81F9
#define GL_SEPARATE_SPECULAR_COLOR        0x81FA

/* texture3D */
#define GL_PACK_SKIP_IMAGES               0x806B
#define GL_PACK_IMAGE_HEIGHT              0x806C
#define GL_UNPACK_SKIP_IMAGES             0x806D
#define GL_UNPACK_IMAGE_HEIGHT            0x806E
#define GL_TEXTURE_3D                     0x806F
#define GL_PROXY_TEXTURE_3D               0x8070
#define GL_TEXTURE_DEPTH                  0x8071
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_MAX_3D_TEXTURE_SIZE            0x8073

/* texture_edge_clamp */
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_CLAMP_TO_BORDER                0x812D

/* texture_lod */
#define GL_TEXTURE_MIN_LOD                0x813A
#define GL_TEXTURE_MAX_LOD                0x813B
#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D

/* GetTarget1_2 */
#define GL_SMOOTH_POINT_SIZE_RANGE        0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY  0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE        0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY  0x0B23
#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF
#define GL_ACTIVE_TEXTURE                 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE          0x84E1
#define GL_MAX_TEXTURE_UNITS              0x84E2

#define GL_COMBINE                        0x8570
#define GL_COMBINE_RGB                    0x8571
#define GL_COMBINE_ALPHA                  0x8572
#define GL_RGB_SCALE                      0x8573
#define GL_ADD_SIGNED                     0x8574
#define GL_INTERPOLATE                    0x8575
#define GL_CONSTANT                       0x8576
#define GL_PRIMARY_COLOR                  0x8577
#define GL_PREVIOUS                       0x8578
#define GL_SUBTRACT                       0x84E7

#define GL_SRC0_RGB                       0x8580
#define GL_SRC1_RGB                       0x8581
#define GL_SRC2_RGB                       0x8582
#define GL_SRC0_ALPHA                     0x8588
#define GL_SRC1_ALPHA                     0x8589
#define GL_SRC2_ALPHA                     0x858A

/* Obsolete */
#define GL_SOURCE0_RGB                    0x8580
#define GL_SOURCE1_RGB                    0x8581
#define GL_SOURCE2_RGB                    0x8582
#define GL_SOURCE0_ALPHA                  0x8588
#define GL_SOURCE1_ALPHA                  0x8589
#define GL_SOURCE2_ALPHA                  0x858A

#define GL_OPERAND0_RGB                   0x8590
#define GL_OPERAND1_RGB                   0x8591
#define GL_OPERAND2_RGB                   0x8592
#define GL_OPERAND0_ALPHA                 0x8598
#define GL_OPERAND1_ALPHA                 0x8599
#define GL_OPERAND2_ALPHA                 0x859A

#define GL_DOT3_RGB                       0x86AE
#define GL_DOT3_RGBA                      0x86AF

#define GL_TRANSPOSE_MODELVIEW_MATRIX     0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX    0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX       0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX         0x84E6

#define GL_NORMAL_MAP                     0x8511
#define GL_REFLECTION_MAP                 0x8512
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP         0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C

#define GL_COMPRESSED_ALPHA               0x84E9
#define GL_COMPRESSED_LUMINANCE           0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA     0x84EB
#define GL_COMPRESSED_INTENSITY           0x84EC
#define GL_COMPRESSED_RGB                 0x84ED
#define GL_COMPRESSED_RGBA                0x84EE
#define GL_TEXTURE_COMPRESSION_HINT       0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE  0x86A0
#define GL_TEXTURE_COMPRESSED             0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS     0x86A3

#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLE_COVERAGE                0x80A0
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
#define GL_MULTISAMPLE_BIT                0x20000000

#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_TEXTURE_DEPTH_SIZE             0x884A
#define GL_DEPTH_TEXTURE_MODE             0x884B

#define GL_TEXTURE_COMPARE_MODE           0x884C
#define GL_TEXTURE_COMPARE_FUNC           0x884D
#define GL_COMPARE_R_TO_TEXTURE           0x884E

/* occlusion_query */
#define GL_QUERY_COUNTER_BITS             0x8864
#define GL_CURRENT_QUERY                  0x8865
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867
#define GL_SAMPLES_PASSED                 0x8914

#define GL_FOG_COORD_SRC                  0x8450
#define GL_FOG_COORD                      0x8451
#define GL_FRAGMENT_DEPTH                 0x8452
#define GL_CURRENT_FOG_COORD              0x8453  
#define GL_FOG_COORD_ARRAY_TYPE           0x8454
#define GL_FOG_COORD_ARRAY_STRIDE         0x8455
#define GL_FOG_COORD_ARRAY_POINTER        0x8456
#define GL_FOG_COORD_ARRAY                0x8457

/* Obsolete */
#define GL_FOG_COORDINATE_SOURCE          0x8450
#define GL_FOG_COORDINATE                 0x8451
#define GL_CURRENT_FOG_COORDINATE         0x8453  
#define GL_FOG_COORDINATE_ARRAY_TYPE      0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE    0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER   0x8456
#define GL_FOG_COORDINATE_ARRAY           0x8457

#define GL_COLOR_SUM                      0x8458
#define GL_CURRENT_SECONDARY_COLOR        0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE     0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE     0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE   0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER  0x845D
#define GL_SECONDARY_COLOR_ARRAY          0x845E

#define GL_POINT_SIZE_MIN                 0x8126
#define GL_POINT_SIZE_MAX                 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE      0x8128
#define GL_POINT_DISTANCE_ATTENUATION     0x8129

#define GL_BLEND_DST_RGB                  0x80C8
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB

#define GL_GENERATE_MIPMAP                0x8191
#define GL_GENERATE_MIPMAP_HINT           0x8192

#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508

#define GL_MIRRORED_REPEAT                0x8370

#define GL_MAX_TEXTURE_LOD_BIAS           0x84FD
#define GL_TEXTURE_FILTER_CONTROL         0x8500
#define GL_TEXTURE_LOD_BIAS               0x8501

/* vertex_buffer_object */
#define GL_ARRAY_BUFFER                                0x8892
#define GL_ELEMENT_ARRAY_BUFFER                        0x8893
#define GL_ARRAY_BUFFER_BINDING                        0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING                0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING                 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING                 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING                  0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING                  0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING          0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING              0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING        0x889C
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING              0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING                 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING          0x889F
#define GL_STREAM_DRAW                                 0x88E0
#define GL_STREAM_READ                                 0x88E1
#define GL_STREAM_COPY                                 0x88E2
#define GL_STATIC_DRAW                                 0x88E4
#define GL_STATIC_READ                                 0x88E5
#define GL_STATIC_COPY                                 0x88E6
#define GL_DYNAMIC_DRAW                                0x88E8
#define GL_DYNAMIC_READ                                0x88E9
#define GL_DYNAMIC_COPY                                0x88EA
#define GL_READ_ONLY                                   0x88B8
#define GL_WRITE_ONLY                                  0x88B9
#define GL_READ_WRITE                                  0x88BA
#define GL_BUFFER_SIZE                                 0x8764
#define GL_BUFFER_USAGE                                0x8765
#define GL_BUFFER_ACCESS                               0x88BB
#define GL_BUFFER_MAPPED                               0x88BC
#define GL_BUFFER_MAP_POINTER                          0x88BD
/* Obsolete */
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING         0x889D

/* OpenGL 2.0 */
#define GL_CURRENT_PROGRAM                0x8B8D
#define GL_SHADER_TYPE                    0x8B4F
#define GL_DELETE_STATUS                  0x8B80
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_ATTACHED_SHADERS               0x8B85
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH      0x8B87
#define GL_SHADER_SOURCE_LENGTH           0x8B88
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_1D                     0x8B5D
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_3D                     0x8B5F
#define GL_SAMPLER_CUBE                   0x8B60
#define GL_SAMPLER_1D_SHADOW              0x8B61
#define GL_SAMPLER_2D_SHADOW              0x8B62
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_VERTEX_SHADER                  0x8B31
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS  0x8B4A
#define GL_MAX_VARYING_FLOATS             0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH    0x8B8A
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_MAX_VERTEX_ATTRIBS             0x8869
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED    0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE       0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE     0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE       0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_CURRENT_VERTEX_ATTRIB          0x8626
#define GL_VERTEX_ATTRIB_ARRAY_POINTER    0x8645
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE        0x8643
#define GL_MAX_TEXTURE_COORDS             0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS        0x8872
#define GL_MAX_DRAW_BUFFERS               0x8824
#define GL_DRAW_BUFFER0                   0x8825
#define GL_DRAW_BUFFER1                   0x8826
#define GL_DRAW_BUFFER2                   0x8827
#define GL_DRAW_BUFFER3                   0x8828
#define GL_DRAW_BUFFER4                   0x8829
#define GL_DRAW_BUFFER5                   0x882A
#define GL_DRAW_BUFFER6                   0x882B
#define GL_DRAW_BUFFER7                   0x882C
#define GL_DRAW_BUFFER8                   0x882D
#define GL_DRAW_BUFFER9                   0x882E
#define GL_DRAW_BUFFER10                  0x882F
#define GL_DRAW_BUFFER11                  0x8830
#define GL_DRAW_BUFFER12                  0x8831
#define GL_DRAW_BUFFER13                  0x8832
#define GL_DRAW_BUFFER14                  0x8833
#define GL_DRAW_BUFFER15                  0x8834
#define GL_POINT_SPRITE                   0x8861
#define GL_COORD_REPLACE                  0x8862
#define GL_POINT_SPRITE_COORD_ORIGIN      0x8CA0
#define GL_LOWER_LEFT                     0x8CA1
#define GL_UPPER_LEFT                     0x8CA2
#define GL_STENCIL_BACK_FUNC              0x8800
#define GL_STENCIL_BACK_VALUE_MASK        0x8CA4
#define GL_STENCIL_BACK_REF               0x8CA3
#define GL_STENCIL_BACK_FAIL              0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
#define GL_STENCIL_BACK_WRITEMASK         0x8CA5

/* OpenGL 2.1 */
#define GL_CURRENT_RASTER_SECONDARY_COLOR 0x845F
#define GL_PIXEL_PACK_BUFFER              0x88EB
#define GL_PIXEL_UNPACK_BUFFER            0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING      0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING    0x88EF
#define GL_FLOAT_MAT2x3                   0x8B65
#define GL_FLOAT_MAT2x4                   0x8B66
#define GL_FLOAT_MAT3x2                   0x8B67
#define GL_FLOAT_MAT3x4                   0x8B68
#define GL_FLOAT_MAT4x2                   0x8B69
#define GL_FLOAT_MAT4x3                   0x8B6A
#define GL_SRGB                           0x8C40
#define GL_SRGB8                          0x8C41
#define GL_SRGB_ALPHA                     0x8C42
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_SLUMINANCE_ALPHA               0x8C44
#define GL_SLUMINANCE8_ALPHA8             0x8C45
#define GL_SLUMINANCE                     0x8C46
#define GL_SLUMINANCE8                    0x8C47
#define GL_COMPRESSED_SRGB                0x8C48
#define GL_COMPRESSED_SRGB_ALPHA          0x8C49
#define GL_COMPRESSED_SLUMINANCE          0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA    0x8C4B

/*************************************************************/

#ifdef GL_GLEXT_FUNCTION_POINTERS
typedef void (* glAccumProcPtr) (GLenum op, GLfloat value);
typedef void (* glAlphaFuncProcPtr) (GLenum func, GLclampf ref);
typedef GLboolean (* glAreTexturesResidentProcPtr) (GLsizei n, const GLuint *textures, GLboolean *residences);
typedef void (* glArrayElementProcPtr) (GLint i);
typedef void (* glBeginProcPtr) (GLenum mode);
typedef void (* glBindTextureProcPtr) (GLenum target, GLuint texture);
typedef void (* glBitmapProcPtr) (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
typedef void (* glBlendColorProcPtr) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (* glBlendEquationProcPtr) (GLenum mode);
typedef void (* glBlendEquationSeparateProcPtr) (GLenum modeRGB, GLenum modeAlpha);
typedef void (* glBlendFuncProcPtr) (GLenum sfactor, GLenum dfactor);
typedef void (* glCallListProcPtr) (GLuint list);
typedef void (* glCallListsProcPtr) (GLsizei n, GLenum type, const GLvoid *lists);
typedef void (* glClearProcPtr) (GLbitfield mask);
typedef void (* glClearAccumProcPtr) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (* glClearColorProcPtr) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (* glClearDepthProcPtr) (GLclampd depth);
typedef void (* glClearIndexProcPtr) (GLfloat c);
typedef void (* glClearStencilProcPtr) (GLint s);
typedef void (* glClipPlaneProcPtr) (GLenum plane, const GLdouble *equation);
typedef void (* glColor3bProcPtr) (GLbyte red, GLbyte green, GLbyte blue);
typedef void (* glColor3bvProcPtr) (const GLbyte *v);
typedef void (* glColor3dProcPtr) (GLdouble red, GLdouble green, GLdouble blue);
typedef void (* glColor3dvProcPtr) (const GLdouble *v);
typedef void (* glColor3fProcPtr) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (* glColor3fvProcPtr) (const GLfloat *v);
typedef void (* glColor3iProcPtr) (GLint red, GLint green, GLint blue);
typedef void (* glColor3ivProcPtr) (const GLint *v);
typedef void (* glColor3sProcPtr) (GLshort red, GLshort green, GLshort blue);
typedef void (* glColor3svProcPtr) (const GLshort *v);
typedef void (* glColor3ubProcPtr) (GLubyte red, GLubyte green, GLubyte blue);
typedef void (* glColor3ubvProcPtr) (const GLubyte *v);
typedef void (* glColor3uiProcPtr) (GLuint red, GLuint green, GLuint blue);
typedef void (* glColor3uivProcPtr) (const GLuint *v);
typedef void (* glColor3usProcPtr) (GLushort red, GLushort green, GLushort blue);
typedef void (* glColor3usvProcPtr) (const GLushort *v);
typedef void (* glColor4bProcPtr) (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
typedef void (* glColor4bvProcPtr) (const GLbyte *v);
typedef void (* glColor4dProcPtr) (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
typedef void (* glColor4dvProcPtr) (const GLdouble *v);
typedef void (* glColor4fProcPtr) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (* glColor4fvProcPtr) (const GLfloat *v);
typedef void (* glColor4iProcPtr) (GLint red, GLint green, GLint blue, GLint alpha);
typedef void (* glColor4ivProcPtr) (const GLint *v);
typedef void (* glColor4sProcPtr) (GLshort red, GLshort green, GLshort blue, GLshort alpha);
typedef void (* glColor4svProcPtr) (const GLshort *v);
typedef void (* glColor4ubProcPtr) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef void (* glColor4ubvProcPtr) (const GLubyte *v);
typedef void (* glColor4uiProcPtr) (GLuint red, GLuint green, GLuint blue, GLuint alpha);
typedef void (* glColor4uivProcPtr) (const GLuint *v);
typedef void (* glColor4usProcPtr) (GLushort red, GLushort green, GLushort blue, GLushort alpha);
typedef void (* glColor4usvProcPtr) (const GLushort *v);
typedef void (* glColorMaskProcPtr) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (* glColorMaterialProcPtr) (GLenum face, GLenum mode);
typedef void (* glColorPointerProcPtr) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (* glColorSubTableProcPtr) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (* glColorTableProcPtr) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (* glColorTableParameterfvProcPtr) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (* glColorTableParameterivProcPtr) (GLenum target, GLenum pname, const GLint *params);
typedef void (* glConvolutionFilter1DProcPtr) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef void (* glConvolutionFilter2DProcPtr) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef void (* glConvolutionParameterfProcPtr) (GLenum target, GLenum pname, GLfloat params);
typedef void (* glConvolutionParameterfvProcPtr) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (* glConvolutionParameteriProcPtr) (GLenum target, GLenum pname, GLint params);
typedef void (* glConvolutionParameterivProcPtr) (GLenum target, GLenum pname, const GLint *params);
typedef void (* glCopyColorSubTableProcPtr) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef void (* glCopyColorTableProcPtr) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (* glCopyConvolutionFilter1DProcPtr) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (* glCopyConvolutionFilter2DProcPtr) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (* glCopyPixelsProcPtr) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
typedef void (* glCopyTexImage1DProcPtr) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (* glCopyTexImage2DProcPtr) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (* glCopyTexSubImage1DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (* glCopyTexSubImage2DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (* glCopyTexSubImage3DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (* glCullFaceProcPtr) (GLenum mode);
typedef void (* glDeleteListsProcPtr) (GLuint list, GLsizei range);
typedef void (* glDeleteTexturesProcPtr) (GLsizei n, const GLuint *textures);
typedef void (* glDepthFuncProcPtr) (GLenum func);
typedef void (* glDepthMaskProcPtr) (GLboolean flag);
typedef void (* glDepthRangeProcPtr) (GLclampd zNear, GLclampd zFar);
typedef void (* glDisableProcPtr) (GLenum cap);
typedef void (* glDisableClientStateProcPtr) (GLenum array);
typedef void (* glDrawArraysProcPtr) (GLenum mode, GLint first, GLsizei count);
typedef void (* glDrawBufferProcPtr) (GLenum mode);
typedef void (* glDrawElementsProcPtr) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (* glDrawPixelsProcPtr) (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glDrawRangeElementsProcPtr) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (* glEdgeFlagProcPtr) (GLboolean flag);
typedef void (* glEdgeFlagPointerProcPtr) (GLsizei stride, const GLvoid *pointer);
typedef void (* glEdgeFlagvProcPtr) (const GLboolean *flag);
typedef void (* glEnableProcPtr) (GLenum cap);
typedef void (* glEnableClientStateProcPtr) (GLenum array);
typedef void (* glEndProcPtr) (void);
typedef void (* glEndListProcPtr) (void);
typedef void (* glEvalCoord1dProcPtr) (GLdouble u);
typedef void (* glEvalCoord1dvProcPtr) (const GLdouble *u);
typedef void (* glEvalCoord1fProcPtr) (GLfloat u);
typedef void (* glEvalCoord1fvProcPtr) (const GLfloat *u);
typedef void (* glEvalCoord2dProcPtr) (GLdouble u, GLdouble v);
typedef void (* glEvalCoord2dvProcPtr) (const GLdouble *u);
typedef void (* glEvalCoord2fProcPtr) (GLfloat u, GLfloat v);
typedef void (* glEvalCoord2fvProcPtr) (const GLfloat *u);
typedef void (* glEvalMesh1ProcPtr) (GLenum mode, GLint i1, GLint i2);
typedef void (* glEvalMesh2ProcPtr) (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
typedef void (* glEvalPoint1ProcPtr) (GLint i);
typedef void (* glEvalPoint2ProcPtr) (GLint i, GLint j);
typedef void (* glFeedbackBufferProcPtr) (GLsizei size, GLenum type, GLfloat *buffer);
typedef void (* glFinishProcPtr) (void);
typedef void (* glFlushProcPtr) (void);
typedef void (* glFogfProcPtr) (GLenum pname, GLfloat param);
typedef void (* glFogfvProcPtr) (GLenum pname, const GLfloat *params);
typedef void (* glFogiProcPtr) (GLenum pname, GLint param);
typedef void (* glFogivProcPtr) (GLenum pname, const GLint *params);
typedef void (* glFrontFaceProcPtr) (GLenum mode);
typedef void (* glFrustumProcPtr) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef GLuint (* glGenListsProcPtr) (GLsizei range);
typedef void (* glGenTexturesProcPtr) (GLsizei n, GLuint *textures);
typedef void (* glGetBooleanvProcPtr) (GLenum pname, GLboolean *params);
typedef void (* glGetClipPlaneProcPtr) (GLenum plane, GLdouble *equation);
typedef void (* glGetColorTableProcPtr) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (* glGetColorTableParameterfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetColorTableParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetConvolutionFilterProcPtr) (GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef void (* glGetConvolutionParameterfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetConvolutionParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetDoublevProcPtr) (GLenum pname, GLdouble *params);
typedef GLenum (* glGetErrorProcPtr) (void);
typedef void (* glGetFloatvProcPtr) (GLenum pname, GLfloat *params);
typedef void (* glGetHistogramProcPtr) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (* glGetHistogramParameterfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetHistogramParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetIntegervProcPtr) (GLenum pname, GLint *params);
typedef void (* glGetLightfvProcPtr) (GLenum light, GLenum pname, GLfloat *params);
typedef void (* glGetLightivProcPtr) (GLenum light, GLenum pname, GLint *params);
typedef void (* glGetMapdvProcPtr) (GLenum target, GLenum query, GLdouble *v);
typedef void (* glGetMapfvProcPtr) (GLenum target, GLenum query, GLfloat *v);
typedef void (* glGetMapivProcPtr) (GLenum target, GLenum query, GLint *v);
typedef void (* glGetMaterialfvProcPtr) (GLenum face, GLenum pname, GLfloat *params);
typedef void (* glGetMaterialivProcPtr) (GLenum face, GLenum pname, GLint *params);
typedef void (* glGetMinmaxProcPtr) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (* glGetMinmaxParameterfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetMinmaxParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetPixelMapfvProcPtr) (GLenum map, GLfloat *values);
typedef void (* glGetPixelMapuivProcPtr) (GLenum map, GLuint *values);
typedef void (* glGetPixelMapusvProcPtr) (GLenum map, GLushort *values);
typedef void (* glGetPointervProcPtr) (GLenum pname, GLvoid **params);
typedef void (* glGetPolygonStippleProcPtr) (GLubyte *mask);
typedef void (* glGetSeparableFilterProcPtr) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef const GLubyte *(* glGetStringProcPtr) (GLenum name);
typedef void (* glGetTexEnvfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetTexEnvivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetTexGendvProcPtr) (GLenum coord, GLenum pname, GLdouble *params);
typedef void (* glGetTexGenfvProcPtr) (GLenum coord, GLenum pname, GLfloat *params);
typedef void (* glGetTexGenivProcPtr) (GLenum coord, GLenum pname, GLint *params);
typedef void (* glGetTexImageProcPtr) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
typedef void (* glGetTexLevelParameterfvProcPtr) (GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void (* glGetTexLevelParameterivProcPtr) (GLenum target, GLint level, GLenum pname, GLint *params);
typedef void (* glGetTexParameterfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetTexParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glHintProcPtr) (GLenum target, GLenum mode);
typedef void (* glHistogramProcPtr) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef void (* glIndexMaskProcPtr) (GLuint mask);
typedef void (* glIndexPointerProcPtr) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (* glIndexdProcPtr) (GLdouble c);
typedef void (* glIndexdvProcPtr) (const GLdouble *c);
typedef void (* glIndexfProcPtr) (GLfloat c);
typedef void (* glIndexfvProcPtr) (const GLfloat *c);
typedef void (* glIndexiProcPtr) (GLint c);
typedef void (* glIndexivProcPtr) (const GLint *c);
typedef void (* glIndexsProcPtr) (GLshort c);
typedef void (* glIndexsvProcPtr) (const GLshort *c);
typedef void (* glIndexubProcPtr) (GLubyte c);
typedef void (* glIndexubvProcPtr) (const GLubyte *c);
typedef void (* glInitNamesProcPtr) (void);
typedef void (* glInterleavedArraysProcPtr) (GLenum format, GLsizei stride, const GLvoid *pointer);
typedef GLboolean (* glIsEnabledProcPtr) (GLenum cap);
typedef GLboolean (* glIsListProcPtr) (GLuint list);
typedef GLboolean (* glIsTextureProcPtr) (GLuint texture);
typedef void (* glLightModelfProcPtr) (GLenum pname, GLfloat param);
typedef void (* glLightModelfvProcPtr) (GLenum pname, const GLfloat *params);
typedef void (* glLightModeliProcPtr) (GLenum pname, GLint param);
typedef void (* glLightModelivProcPtr) (GLenum pname, const GLint *params);
typedef void (* glLightfProcPtr) (GLenum light, GLenum pname, GLfloat param);
typedef void (* glLightfvProcPtr) (GLenum light, GLenum pname, const GLfloat *params);
typedef void (* glLightiProcPtr) (GLenum light, GLenum pname, GLint param);
typedef void (* glLightivProcPtr) (GLenum light, GLenum pname, const GLint *params);
typedef void (* glLineStippleProcPtr) (GLint factor, GLushort pattern);
typedef void (* glLineWidthProcPtr) (GLfloat width);
typedef void (* glListBaseProcPtr) (GLuint base);
typedef void (* glLoadIdentityProcPtr) (void);
typedef void (* glLoadMatrixdProcPtr) (const GLdouble *m);
typedef void (* glLoadMatrixfProcPtr) (const GLfloat *m);
typedef void (* glLoadNameProcPtr) (GLuint name);
typedef void (* glLogicOpProcPtr) (GLenum opcode);
typedef void (* glMap1dProcPtr) (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
typedef void (* glMap1fProcPtr) (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
typedef void (* glMap2dProcPtr) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
typedef void (* glMap2fProcPtr) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
typedef void (* glMapGrid1dProcPtr) (GLint un, GLdouble u1, GLdouble u2);
typedef void (* glMapGrid1fProcPtr) (GLint un, GLfloat u1, GLfloat u2);
typedef void (* glMapGrid2dProcPtr) (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
typedef void (* glMapGrid2fProcPtr) (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
typedef void (* glMaterialfProcPtr) (GLenum face, GLenum pname, GLfloat param);
typedef void (* glMaterialfvProcPtr) (GLenum face, GLenum pname, const GLfloat *params);
typedef void (* glMaterialiProcPtr) (GLenum face, GLenum pname, GLint param);
typedef void (* glMaterialivProcPtr) (GLenum face, GLenum pname, const GLint *params);
typedef void (* glMatrixModeProcPtr) (GLenum mode);
typedef void (* glMinmaxProcPtr) (GLenum target, GLenum internalformat, GLboolean sink);
typedef void (* glMultMatrixdProcPtr) (const GLdouble *m);
typedef void (* glMultMatrixfProcPtr) (const GLfloat *m);
typedef void (* glNewListProcPtr) (GLuint list, GLenum mode);
typedef void (* glNormal3bProcPtr) (GLbyte nx, GLbyte ny, GLbyte nz);
typedef void (* glNormal3bvProcPtr) (const GLbyte *v);
typedef void (* glNormal3dProcPtr) (GLdouble nx, GLdouble ny, GLdouble nz);
typedef void (* glNormal3dvProcPtr) (const GLdouble *v);
typedef void (* glNormal3fProcPtr) (GLfloat nx, GLfloat ny, GLfloat nz);
typedef void (* glNormal3fvProcPtr) (const GLfloat *v);
typedef void (* glNormal3iProcPtr) (GLint nx, GLint ny, GLint nz);
typedef void (* glNormal3ivProcPtr) (const GLint *v);
typedef void (* glNormal3sProcPtr) (GLshort nx, GLshort ny, GLshort nz);
typedef void (* glNormal3svProcPtr) (const GLshort *v);
typedef void (* glNormalPointerProcPtr) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (* glOrthoProcPtr) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef void (* glPassThroughProcPtr) (GLfloat token);
typedef void (* glPixelMapfvProcPtr) (GLenum map, GLint mapsize, const GLfloat *values);
typedef void (* glPixelMapuivProcPtr) (GLenum map, GLint mapsize, const GLuint *values);
typedef void (* glPixelMapusvProcPtr) (GLenum map, GLint mapsize, const GLushort *values);
typedef void (* glPixelStorefProcPtr) (GLenum pname, GLfloat param);
typedef void (* glPixelStoreiProcPtr) (GLenum pname, GLint param);
typedef void (* glPixelTransferfProcPtr) (GLenum pname, GLfloat param);
typedef void (* glPixelTransferiProcPtr) (GLenum pname, GLint param);
typedef void (* glPixelZoomProcPtr) (GLfloat xfactor, GLfloat yfactor);
typedef void (* glPointSizeProcPtr) (GLfloat size);
typedef void (* glPolygonModeProcPtr) (GLenum face, GLenum mode);
typedef void (* glPolygonOffsetProcPtr) (GLfloat factor, GLfloat units);
typedef void (* glPolygonStippleProcPtr) (const GLubyte *mask);
typedef void (* glPopAttribProcPtr) (void);
typedef void (* glPopClientAttribProcPtr) (void);
typedef void (* glPopMatrixProcPtr) (void);
typedef void (* glPopNameProcPtr) (void);
typedef void (* glPrioritizeTexturesProcPtr) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef void (* glPushAttribProcPtr) (GLbitfield mask);
typedef void (* glPushClientAttribProcPtr) (GLbitfield mask);
typedef void (* glPushMatrixProcPtr) (void);
typedef void (* glPushNameProcPtr) (GLuint name);
typedef void (* glRasterPos2dProcPtr) (GLdouble x, GLdouble y);
typedef void (* glRasterPos2dvProcPtr) (const GLdouble *v);
typedef void (* glRasterPos2fProcPtr) (GLfloat x, GLfloat y);
typedef void (* glRasterPos2fvProcPtr) (const GLfloat *v);
typedef void (* glRasterPos2iProcPtr) (GLint x, GLint y);
typedef void (* glRasterPos2ivProcPtr) (const GLint *v);
typedef void (* glRasterPos2sProcPtr) (GLshort x, GLshort y);
typedef void (* glRasterPos2svProcPtr) (const GLshort *v);
typedef void (* glRasterPos3dProcPtr) (GLdouble x, GLdouble y, GLdouble z);
typedef void (* glRasterPos3dvProcPtr) (const GLdouble *v);
typedef void (* glRasterPos3fProcPtr) (GLfloat x, GLfloat y, GLfloat z);
typedef void (* glRasterPos3fvProcPtr) (const GLfloat *v);
typedef void (* glRasterPos3iProcPtr) (GLint x, GLint y, GLint z);
typedef void (* glRasterPos3ivProcPtr) (const GLint *v);
typedef void (* glRasterPos3sProcPtr) (GLshort x, GLshort y, GLshort z);
typedef void (* glRasterPos3svProcPtr) (const GLshort *v);
typedef void (* glRasterPos4dProcPtr) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (* glRasterPos4dvProcPtr) (const GLdouble *v);
typedef void (* glRasterPos4fProcPtr) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (* glRasterPos4fvProcPtr) (const GLfloat *v);
typedef void (* glRasterPos4iProcPtr) (GLint x, GLint y, GLint z, GLint w);
typedef void (* glRasterPos4ivProcPtr) (const GLint *v);
typedef void (* glRasterPos4sProcPtr) (GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (* glRasterPos4svProcPtr) (const GLshort *v);
typedef void (* glReadBufferProcPtr) (GLenum mode);
typedef void (* glReadPixelsProcPtr) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
typedef void (* glRectdProcPtr) (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
typedef void (* glRectdvProcPtr) (const GLdouble *v1, const GLdouble *v2);
typedef void (* glRectfProcPtr) (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
typedef void (* glRectfvProcPtr) (const GLfloat *v1, const GLfloat *v2);
typedef void (* glRectiProcPtr) (GLint x1, GLint y1, GLint x2, GLint y2);
typedef void (* glRectivProcPtr) (const GLint *v1, const GLint *v2);
typedef void (* glRectsProcPtr) (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
typedef void (* glRectsvProcPtr) (const GLshort *v1, const GLshort *v2);
typedef GLint (* glRenderModeProcPtr) (GLenum mode);
typedef void (* glResetHistogramProcPtr) (GLenum target);
typedef void (* glResetMinmaxProcPtr) (GLenum target);
typedef void (* glRotatedProcPtr) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
typedef void (* glRotatefProcPtr) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
typedef void (* glScaledProcPtr) (GLdouble x, GLdouble y, GLdouble z);
typedef void (* glScalefProcPtr) (GLfloat x, GLfloat y, GLfloat z);
typedef void (* glScissorProcPtr) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (* glSelectBufferProcPtr) (GLsizei size, GLuint *buffer);
typedef void (* glSeparableFilter2DProcPtr) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef void (* glShadeModelProcPtr) (GLenum mode);
typedef void (* glStencilFuncProcPtr) (GLenum func, GLint ref, GLuint mask);
typedef void (* glStencilMaskProcPtr) (GLuint mask);
typedef void (* glStencilOpProcPtr) (GLenum fail, GLenum zfail, GLenum zpass);
typedef void (* glTexCoord1dProcPtr) (GLdouble s);
typedef void (* glTexCoord1dvProcPtr) (const GLdouble *v);
typedef void (* glTexCoord1fProcPtr) (GLfloat s);
typedef void (* glTexCoord1fvProcPtr) (const GLfloat *v);
typedef void (* glTexCoord1iProcPtr) (GLint s);
typedef void (* glTexCoord1ivProcPtr) (const GLint *v);
typedef void (* glTexCoord1sProcPtr) (GLshort s);
typedef void (* glTexCoord1svProcPtr) (const GLshort *v);
typedef void (* glTexCoord2dProcPtr) (GLdouble s, GLdouble t);
typedef void (* glTexCoord2dvProcPtr) (const GLdouble *v);
typedef void (* glTexCoord2fProcPtr) (GLfloat s, GLfloat t);
typedef void (* glTexCoord2fvProcPtr) (const GLfloat *v);
typedef void (* glTexCoord2iProcPtr) (GLint s, GLint t);
typedef void (* glTexCoord2ivProcPtr) (const GLint *v);
typedef void (* glTexCoord2sProcPtr) (GLshort s, GLshort t);
typedef void (* glTexCoord2svProcPtr) (const GLshort *v);
typedef void (* glTexCoord3dProcPtr) (GLdouble s, GLdouble t, GLdouble r);
typedef void (* glTexCoord3dvProcPtr) (const GLdouble *v);
typedef void (* glTexCoord3fProcPtr) (GLfloat s, GLfloat t, GLfloat r);
typedef void (* glTexCoord3fvProcPtr) (const GLfloat *v);
typedef void (* glTexCoord3iProcPtr) (GLint s, GLint t, GLint r);
typedef void (* glTexCoord3ivProcPtr) (const GLint *v);
typedef void (* glTexCoord3sProcPtr) (GLshort s, GLshort t, GLshort r);
typedef void (* glTexCoord3svProcPtr) (const GLshort *v);
typedef void (* glTexCoord4dProcPtr) (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (* glTexCoord4dvProcPtr) (const GLdouble *v);
typedef void (* glTexCoord4fProcPtr) (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (* glTexCoord4fvProcPtr) (const GLfloat *v);
typedef void (* glTexCoord4iProcPtr) (GLint s, GLint t, GLint r, GLint q);
typedef void (* glTexCoord4ivProcPtr) (const GLint *v);
typedef void (* glTexCoord4sProcPtr) (GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (* glTexCoord4svProcPtr) (const GLshort *v);
typedef void (* glTexCoordPointerProcPtr) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (* glTexEnvfProcPtr) (GLenum target, GLenum pname, GLfloat param);
typedef void (* glTexEnvfvProcPtr) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (* glTexEnviProcPtr) (GLenum target, GLenum pname, GLint param);
typedef void (* glTexEnvivProcPtr) (GLenum target, GLenum pname, const GLint *params);
typedef void (* glTexGendProcPtr) (GLenum coord, GLenum pname, GLdouble param);
typedef void (* glTexGendvProcPtr) (GLenum coord, GLenum pname, const GLdouble *params);
typedef void (* glTexGenfProcPtr) (GLenum coord, GLenum pname, GLfloat param);
typedef void (* glTexGenfvProcPtr) (GLenum coord, GLenum pname, const GLfloat *params);
typedef void (* glTexGeniProcPtr) (GLenum coord, GLenum pname, GLint param);
typedef void (* glTexGenivProcPtr) (GLenum coord, GLenum pname, const GLint *params);
typedef void (* glTexImage1DProcPtr) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTexImage2DProcPtr) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTexImage3DProcPtr) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTexParameterfProcPtr) (GLenum target, GLenum pname, GLfloat param);
typedef void (* glTexParameterfvProcPtr) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (* glTexParameteriProcPtr) (GLenum target, GLenum pname, GLint param);
typedef void (* glTexParameterivProcPtr) (GLenum target, GLenum pname, const GLint *params);
typedef void (* glTexSubImage1DProcPtr) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTexSubImage2DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTexSubImage3DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTranslatedProcPtr) (GLdouble x, GLdouble y, GLdouble z);
typedef void (* glTranslatefProcPtr) (GLfloat x, GLfloat y, GLfloat z);
typedef void (* glVertex2dProcPtr) (GLdouble x, GLdouble y);
typedef void (* glVertex2dvProcPtr) (const GLdouble *v);
typedef void (* glVertex2fProcPtr) (GLfloat x, GLfloat y);
typedef void (* glVertex2fvProcPtr) (const GLfloat *v);
typedef void (* glVertex2iProcPtr) (GLint x, GLint y);
typedef void (* glVertex2ivProcPtr) (const GLint *v);
typedef void (* glVertex2sProcPtr) (GLshort x, GLshort y);
typedef void (* glVertex2svProcPtr) (const GLshort *v);
typedef void (* glVertex3dProcPtr) (GLdouble x, GLdouble y, GLdouble z);
typedef void (* glVertex3dvProcPtr) (const GLdouble *v);
typedef void (* glVertex3fProcPtr) (GLfloat x, GLfloat y, GLfloat z);
typedef void (* glVertex3fvProcPtr) (const GLfloat *v);
typedef void (* glVertex3iProcPtr) (GLint x, GLint y, GLint z);
typedef void (* glVertex3ivProcPtr) (const GLint *v);
typedef void (* glVertex3sProcPtr) (GLshort x, GLshort y, GLshort z);
typedef void (* glVertex3svProcPtr) (const GLshort *v);
typedef void (* glVertex4dProcPtr) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (* glVertex4dvProcPtr) (const GLdouble *v);
typedef void (* glVertex4fProcPtr) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (* glVertex4fvProcPtr) (const GLfloat *v);
typedef void (* glVertex4iProcPtr) (GLint x, GLint y, GLint z, GLint w);
typedef void (* glVertex4ivProcPtr) (const GLint *v);
typedef void (* glVertex4sProcPtr) (GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (* glVertex4svProcPtr) (const GLshort *v);
typedef void (* glVertexPointerProcPtr) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (* glViewportProcPtr) (GLint x, GLint y, GLsizei width, GLsizei height);

typedef void (* glSampleCoverageProcPtr) (GLclampf value, GLboolean invert);

typedef void (* glLoadTransposeMatrixfProcPtr) (const GLfloat *m);
typedef void (* glLoadTransposeMatrixdProcPtr) (const GLdouble *m);
typedef void (* glMultTransposeMatrixfProcPtr) (const GLfloat *m);
typedef void (* glMultTransposeMatrixdProcPtr) (const GLdouble *m);

typedef void (* glCompressedTexImage3DProcPtr) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (* glCompressedTexImage2DProcPtr) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (* glCompressedTexImage1DProcPtr) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (* glCompressedTexSubImage3DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (* glCompressedTexSubImage2DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (* glCompressedTexSubImage1DProcPtr) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (* glGetCompressedTexImageProcPtr) (GLenum target, GLint lod, GLvoid *img);

typedef void (* glActiveTextureProcPtr) (GLenum texture);
typedef void (* glClientActiveTextureProcPtr) (GLenum texture);
typedef void (* glMultiTexCoord1dProcPtr) (GLenum target, GLdouble s);
typedef void (* glMultiTexCoord1dvProcPtr) (GLenum target, const GLdouble *v);
typedef void (* glMultiTexCoord1fProcPtr) (GLenum target, GLfloat s);
typedef void (* glMultiTexCoord1fvProcPtr) (GLenum target, const GLfloat *v);
typedef void (* glMultiTexCoord1iProcPtr) (GLenum target, GLint s);
typedef void (* glMultiTexCoord1ivProcPtr) (GLenum target, const GLint *v);
typedef void (* glMultiTexCoord1sProcPtr) (GLenum target, GLshort s);
typedef void (* glMultiTexCoord1svProcPtr) (GLenum target, const GLshort *v);
typedef void (* glMultiTexCoord2dProcPtr) (GLenum target, GLdouble s, GLdouble t);
typedef void (* glMultiTexCoord2dvProcPtr) (GLenum target, const GLdouble *v);
typedef void (* glMultiTexCoord2fProcPtr) (GLenum target, GLfloat s, GLfloat t);
typedef void (* glMultiTexCoord2fvProcPtr) (GLenum target, const GLfloat *v);
typedef void (* glMultiTexCoord2iProcPtr) (GLenum target, GLint s, GLint t);
typedef void (* glMultiTexCoord2ivProcPtr) (GLenum target, const GLint *v);
typedef void (* glMultiTexCoord2sProcPtr) (GLenum target, GLshort s, GLshort t);
typedef void (* glMultiTexCoord2svProcPtr) (GLenum target, const GLshort *v);
typedef void (* glMultiTexCoord3dProcPtr) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (* glMultiTexCoord3dvProcPtr) (GLenum target, const GLdouble *v);
typedef void (* glMultiTexCoord3fProcPtr) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (* glMultiTexCoord3fvProcPtr) (GLenum target, const GLfloat *v);
typedef void (* glMultiTexCoord3iProcPtr) (GLenum target, GLint s, GLint t, GLint r);
typedef void (* glMultiTexCoord3ivProcPtr) (GLenum target, const GLint *v);
typedef void (* glMultiTexCoord3sProcPtr) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (* glMultiTexCoord3svProcPtr) (GLenum target, const GLshort *v);
typedef void (* glMultiTexCoord4dProcPtr) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (* glMultiTexCoord4dvProcPtr) (GLenum target, const GLdouble *v);
typedef void (* glMultiTexCoord4fProcPtr) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (* glMultiTexCoord4fvProcPtr) (GLenum target, const GLfloat *v);
typedef void (* glMultiTexCoord4iProcPtr) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (* glMultiTexCoord4ivProcPtr) (GLenum target, const GLint *v);
typedef void (* glMultiTexCoord4sProcPtr) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (* glMultiTexCoord4svProcPtr) (GLenum target, const GLshort *v);

typedef void (* glFogCoordfProcPtr) (GLfloat coord);
typedef void (* glFogCoordfvProcPtr) (const GLfloat *coord);  
typedef void (* glFogCoorddProcPtr) (GLdouble coord);
typedef void (* glFogCoorddvProcPtr) (const GLdouble *coord);   
typedef void (* glFogCoordPointerProcPtr) (GLenum type, GLsizei stride, const GLvoid *pointer);

typedef void (* glSecondaryColor3bProcPtr) (GLbyte red, GLbyte green, GLbyte blue);
typedef void (* glSecondaryColor3bvProcPtr) (const GLbyte *v);
typedef void (* glSecondaryColor3dProcPtr) (GLdouble red, GLdouble green, GLdouble blue);
typedef void (* glSecondaryColor3dvProcPtr) (const GLdouble *v);
typedef void (* glSecondaryColor3fProcPtr) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (* glSecondaryColor3fvProcPtr) (const GLfloat *v);
typedef void (* glSecondaryColor3iProcPtr) (GLint red, GLint green, GLint blue);
typedef void (* glSecondaryColor3ivProcPtr) (const GLint *v);
typedef void (* glSecondaryColor3sProcPtr) (GLshort red, GLshort green, GLshort blue);
typedef void (* glSecondaryColor3svProcPtr) (const GLshort *v);
typedef void (* glSecondaryColor3ubProcPtr) (GLubyte red, GLubyte green, GLubyte blue);
typedef void (* glSecondaryColor3ubvProcPtr) (const GLubyte *v);
typedef void (* glSecondaryColor3uiProcPtr) (GLuint red, GLuint green, GLuint blue);
typedef void (* glSecondaryColor3uivProcPtr) (const GLuint *v);
typedef void (* glSecondaryColor3usProcPtr) (GLushort red, GLushort green, GLushort blue);
typedef void (* glSecondaryColor3usvProcPtr) (const GLushort *v);
typedef void (* glSecondaryColorPointerProcPtr) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

typedef void (* glPointParameterfProcPtr) (GLenum pname, GLfloat param); 
typedef void (* glPointParameterfvProcPtr) (GLenum pname, const GLfloat *params);
typedef void (* glPointParameteriProcPtr) (GLenum pname, GLint param); 
typedef void (* glPointParameterivProcPtr) (GLenum pname, const GLint *params);

typedef void (* glBlendFuncSeparateProcPtr) (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

typedef void (* glMultiDrawArraysProcPtr) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
typedef void (* glMultiDrawElementsProcPtr) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* const *indices, GLsizei primcount);

typedef void (* glWindowPos2dProcPtr) (GLdouble x, GLdouble y);
typedef void (* glWindowPos2dvProcPtr) (const GLdouble *v);
typedef void (* glWindowPos2fProcPtr) (GLfloat x, GLfloat y);
typedef void (* glWindowPos2fvProcPtr) (const GLfloat *v);
typedef void (* glWindowPos2iProcPtr) (GLint x, GLint y); 
typedef void (* glWindowPos2ivProcPtr) (const GLint *v);
typedef void (* glWindowPos2sProcPtr) (GLshort x, GLshort y);
typedef void (* glWindowPos2svProcPtr) (const GLshort *v);
typedef void (* glWindowPos3dProcPtr) (GLdouble x, GLdouble y, GLdouble z);
typedef void (* glWindowPos3dvProcPtr) (const GLdouble *v);
typedef void (* glWindowPos3fProcPtr) (GLfloat x, GLfloat y, GLfloat z);
typedef void (* glWindowPos3fvProcPtr) (const GLfloat *v);
typedef void (* glWindowPos3iProcPtr) (GLint x, GLint y, GLint z);
typedef void (* glWindowPos3ivProcPtr) (const GLint *v);
typedef void (* glWindowPos3sProcPtr) (GLshort x, GLshort y, GLshort z);
typedef void (* glWindowPos3svProcPtr) (const GLshort *v);

typedef void (* glGenQueriesProcPtr) (GLsizei n, GLuint *ids);
typedef void (* glDeleteQueriesProcPtr) (GLsizei n, const GLuint *ids);
typedef GLboolean (* glIsQueryProcPtr) (GLuint id);
typedef void (* glBeginQueryProcPtr) (GLenum target, GLuint id);
typedef void (* glEndQueryProcPtr) (GLenum target);
typedef void (* glGetQueryivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetQueryObjectivProcPtr) (GLuint id, GLenum pname, GLint *params);
typedef void (* glGetQueryObjectuivProcPtr) (GLuint id, GLenum pname, GLuint *params);

typedef void (* glBindBufferProcPtr) (GLenum target, GLuint buffer);
typedef void (* glDeleteBuffersProcPtr) (GLsizei n, const GLuint *buffers);
typedef void (* glGenBuffersProcPtr) (GLsizei n, GLuint *buffers);
typedef GLboolean (* glIsBufferProcPtr) (GLuint buffer);
typedef void (* glBufferDataProcPtr) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void (* glBufferSubDataProcPtr) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
typedef void (* glGetBufferSubDataProcPtr) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
typedef GLvoid *(* glMapBufferProcPtr) (GLenum target, GLenum access);
typedef GLboolean (* glUnmapBufferProcPtr) (GLenum target);
typedef void (* glGetBufferParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetBufferPointervProcPtr) (GLenum target, GLenum pname, GLvoid **params);

typedef void (* glDrawBuffersProcPtr) (GLsizei n, const GLenum *bufs);
typedef void (* glVertexAttrib1dProcPtr) (GLuint index, GLdouble x);
typedef void (* glVertexAttrib1dvProcPtr) (GLuint index, const GLdouble *v);
typedef void (* glVertexAttrib1fProcPtr) (GLuint index, GLfloat x);
typedef void (* glVertexAttrib1fvProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib1sProcPtr) (GLuint index, GLshort x);
typedef void (* glVertexAttrib1svProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib2dProcPtr) (GLuint index, GLdouble x, GLdouble y);
typedef void (* glVertexAttrib2dvProcPtr) (GLuint index, const GLdouble *v);
typedef void (* glVertexAttrib2fProcPtr) (GLuint index, GLfloat x, GLfloat y);
typedef void (* glVertexAttrib2fvProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib2sProcPtr) (GLuint index, GLshort x, GLshort y);
typedef void (* glVertexAttrib2svProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib3dProcPtr) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (* glVertexAttrib3dvProcPtr) (GLuint index, const GLdouble *v);
typedef void (* glVertexAttrib3fProcPtr) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (* glVertexAttrib3fvProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib3sProcPtr) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (* glVertexAttrib3svProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib4NbvProcPtr) (GLuint index, const GLbyte *v);
typedef void (* glVertexAttrib4NivProcPtr) (GLuint index, const GLint *v);
typedef void (* glVertexAttrib4NsvProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib4NubProcPtr) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (* glVertexAttrib4NubvProcPtr) (GLuint index, const GLubyte *v);
typedef void (* glVertexAttrib4NuivProcPtr) (GLuint index, const GLuint *v);
typedef void (* glVertexAttrib4NusvProcPtr) (GLuint index, const GLushort *v);
typedef void (* glVertexAttrib4bvProcPtr) (GLuint index, const GLbyte *v);
typedef void (* glVertexAttrib4dProcPtr) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (* glVertexAttrib4dvProcPtr) (GLuint index, const GLdouble *v);
typedef void (* glVertexAttrib4fProcPtr) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (* glVertexAttrib4fvProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib4ivProcPtr) (GLuint index, const GLint *v);
typedef void (* glVertexAttrib4sProcPtr) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (* glVertexAttrib4svProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib4ubvProcPtr) (GLuint index, const GLubyte *v);
typedef void (* glVertexAttrib4uivProcPtr) (GLuint index, const GLuint *v);
typedef void (* glVertexAttrib4usvProcPtr) (GLuint index, const GLushort *v);
typedef void (* glVertexAttribPointerProcPtr) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (* glEnableVertexAttribArrayProcPtr) (GLuint index);
typedef void (* glDisableVertexAttribArrayProcPtr) (GLuint index);
typedef void (* glGetVertexAttribdvProcPtr) (GLuint index, GLenum pname, GLdouble *params);
typedef void (* glGetVertexAttribfvProcPtr) (GLuint index, GLenum pname, GLfloat *params);
typedef void (* glGetVertexAttribivProcPtr) (GLuint index, GLenum pname, GLint *params);
typedef void (* glGetVertexAttribPointervProcPtr) (GLuint index, GLenum pname, GLvoid **pointer);
typedef void (* glDeleteShaderProcPtr) (GLuint shader);
typedef void (* glDetachShaderProcPtr) (GLuint program, GLuint shader);
typedef GLuint (* glCreateShaderProcPtr) (GLenum type);
typedef void (* glShaderSourceProcPtr) (GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length);
typedef void (* glCompileShaderProcPtr) (GLuint shader);
typedef GLuint (* glCreateProgramProcPtr) (void);
typedef void (* glAttachShaderProcPtr) (GLuint program, GLuint shader);
typedef void (* glLinkProgramProcPtr) (GLuint program);
typedef void (* glUseProgramProcPtr) (GLuint program);
typedef void (* glDeleteProgramProcPtr) (GLuint program);
typedef void (* glValidateProgramProcPtr) (GLuint program);
typedef void (* glUniform1fProcPtr) (GLint location, GLfloat v0);
typedef void (* glUniform2fProcPtr) (GLint location, GLfloat v0, GLfloat v1);
typedef void (* glUniform3fProcPtr) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (* glUniform4fProcPtr) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (* glUniform1iProcPtr) (GLint location, GLint v0);
typedef void (* glUniform2iProcPtr) (GLint location, GLint v0, GLint v1);
typedef void (* glUniform3iProcPtr) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (* glUniform4iProcPtr) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (* glUniform1fvProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform2fvProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform3fvProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform4fvProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform1ivProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniform2ivProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniform3ivProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniform4ivProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniformMatrix2fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix3fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix4fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef GLboolean (* glIsShaderProcPtr) (GLuint shader);
typedef GLboolean (* glIsProgramProcPtr) (GLuint program);
typedef void (* glGetShaderivProcPtr) (GLuint shader, GLenum pname, GLint *params);
typedef void (* glGetProgramivProcPtr) (GLuint program, GLenum pname, GLint *params);
typedef void (* glGetAttachedShadersProcPtr) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
typedef void (* glGetShaderInfoLogProcPtr) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (* glGetProgramInfoLogProcPtr) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLint (* glGetUniformLocationProcPtr) (GLuint program, const GLchar *name);
typedef void (* glGetActiveUniformProcPtr) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (* glGetUniformfvProcPtr) (GLuint program, GLint location, GLfloat *params);
typedef void (* glGetUniformivProcPtr) (GLuint program, GLint location, GLint *params);
typedef void (* glGetShaderSourceProcPtr) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
typedef void (* glBindAttribLocationProcPtr) (GLuint program, GLuint index, const GLchar *name);
typedef void (* glGetActiveAttribProcPtr) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef GLint (* glGetAttribLocationProcPtr) (GLuint program, const GLchar *name);
typedef void (* glStencilFuncSeparateProcPtr) (GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void (* glStencilOpSeparateProcPtr) (GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
typedef void (* glStencilMaskSeparateProcPtr) (GLenum face, GLuint mask);

typedef void (* glUniformMatrix2x3fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix3x2fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix2x4fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix4x2fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix3x4fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix4x3fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

#else /* GL_GLEXT_FUNCTION_POINTERS */

extern void glAccum (GLenum op, GLfloat value);
extern void glAlphaFunc (GLenum func, GLclampf ref);
extern GLboolean glAreTexturesResident (GLsizei n, const GLuint *textures, GLboolean *residences);
extern void glArrayElement (GLint i);
extern void glBegin (GLenum mode);
extern void glBindTexture (GLenum target, GLuint texture);
extern void glBitmap (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
extern void glBlendColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern void glBlendEquation (GLenum mode);
extern void glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha);
extern void glBlendFunc (GLenum sfactor, GLenum dfactor);
extern void glCallList (GLuint list);
extern void glCallLists (GLsizei n, GLenum type, const GLvoid *lists);
extern void glClear (GLbitfield mask);
extern void glClearAccum (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern void glClearDepth (GLclampd depth);
extern void glClearIndex (GLfloat c);
extern void glClearStencil (GLint s);
extern void glClipPlane (GLenum plane, const GLdouble *equation);
extern void glColor3b (GLbyte red, GLbyte green, GLbyte blue);
extern void glColor3bv (const GLbyte *v);
extern void glColor3d (GLdouble red, GLdouble green, GLdouble blue);
extern void glColor3dv (const GLdouble *v);
extern void glColor3f (GLfloat red, GLfloat green, GLfloat blue);
extern void glColor3fv (const GLfloat *v);
extern void glColor3i (GLint red, GLint green, GLint blue);
extern void glColor3iv (const GLint *v);
extern void glColor3s (GLshort red, GLshort green, GLshort blue);
extern void glColor3sv (const GLshort *v);
extern void glColor3ub (GLubyte red, GLubyte green, GLubyte blue);
extern void glColor3ubv (const GLubyte *v);
extern void glColor3ui (GLuint red, GLuint green, GLuint blue);
extern void glColor3uiv (const GLuint *v);
extern void glColor3us (GLushort red, GLushort green, GLushort blue);
extern void glColor3usv (const GLushort *v);
extern void glColor4b (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
extern void glColor4bv (const GLbyte *v);
extern void glColor4d (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
extern void glColor4dv (const GLdouble *v);
extern void glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void glColor4fv (const GLfloat *v);
extern void glColor4i (GLint red, GLint green, GLint blue, GLint alpha);
extern void glColor4iv (const GLint *v);
extern void glColor4s (GLshort red, GLshort green, GLshort blue, GLshort alpha);
extern void glColor4sv (const GLshort *v);
extern void glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
extern void glColor4ubv (const GLubyte *v);
extern void glColor4ui (GLuint red, GLuint green, GLuint blue, GLuint alpha);
extern void glColor4uiv (const GLuint *v);
extern void glColor4us (GLushort red, GLushort green, GLushort blue, GLushort alpha);
extern void glColor4usv (const GLushort *v);
extern void glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern void glColorMaterial (GLenum face, GLenum mode);
extern void glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void glColorSubTable (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
extern void glColorTable (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
extern void glColorTableParameterfv (GLenum target, GLenum pname, const GLfloat *params);
extern void glColorTableParameteriv (GLenum target, GLenum pname, const GLint *params);
extern void glConvolutionFilter1D (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
extern void glConvolutionFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
extern void glConvolutionParameterf (GLenum target, GLenum pname, GLfloat params);
extern void glConvolutionParameterfv (GLenum target, GLenum pname, const GLfloat *params);
extern void glConvolutionParameteri (GLenum target, GLenum pname, GLint params);
extern void glConvolutionParameteriv (GLenum target, GLenum pname, const GLint *params);
extern void glCopyColorSubTable (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
extern void glCopyColorTable (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
extern void glCopyConvolutionFilter1D (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
extern void glCopyConvolutionFilter2D (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
extern void glCopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
extern void glCopyTexImage1D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
extern void glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void glCopyTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern void glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void glCopyTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void glCullFace (GLenum mode);
extern void glDeleteLists (GLuint list, GLsizei range);
extern void glDeleteTextures (GLsizei n, const GLuint *textures);
extern void glDepthFunc (GLenum func);
extern void glDepthMask (GLboolean flag);
extern void glDepthRange (GLclampd zNear, GLclampd zFar);
extern void glDisable (GLenum cap);
extern void glDisableClientState (GLenum array);
extern void glDrawArrays (GLenum mode, GLint first, GLsizei count);
extern void glDrawBuffer (GLenum mode);
extern void glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
extern void glDrawPixels (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern void glDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
extern void glEdgeFlag (GLboolean flag);
extern void glEdgeFlagPointer (GLsizei stride, const GLvoid *pointer);
extern void glEdgeFlagv (const GLboolean *flag);
extern void glEnable (GLenum cap);
extern void glEnableClientState (GLenum array);
extern void glEnd (void);
extern void glEndList (void);
extern void glEvalCoord1d (GLdouble u);
extern void glEvalCoord1dv (const GLdouble *u);
extern void glEvalCoord1f (GLfloat u);
extern void glEvalCoord1fv (const GLfloat *u);
extern void glEvalCoord2d (GLdouble u, GLdouble v);
extern void glEvalCoord2dv (const GLdouble *u);
extern void glEvalCoord2f (GLfloat u, GLfloat v);
extern void glEvalCoord2fv (const GLfloat *u);
extern void glEvalMesh1 (GLenum mode, GLint i1, GLint i2);
extern void glEvalMesh2 (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
extern void glEvalPoint1 (GLint i);
extern void glEvalPoint2 (GLint i, GLint j);
extern void glFeedbackBuffer (GLsizei size, GLenum type, GLfloat *buffer);
extern void glFinish (void);
extern void glFlush (void);
extern void glFogf (GLenum pname, GLfloat param);
extern void glFogfv (GLenum pname, const GLfloat *params);
extern void glFogi (GLenum pname, GLint param);
extern void glFogiv (GLenum pname, const GLint *params);
extern void glFrontFace (GLenum mode);
extern void glFrustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern GLuint glGenLists (GLsizei range);
extern void glGenTextures (GLsizei n, GLuint *textures);
extern void glGetBooleanv (GLenum pname, GLboolean *params);
extern void glGetClipPlane (GLenum plane, GLdouble *equation);
extern void glGetColorTable (GLenum target, GLenum format, GLenum type, GLvoid *table);
extern void glGetColorTableParameterfv (GLenum target, GLenum pname, GLfloat *params);
extern void glGetColorTableParameteriv (GLenum target, GLenum pname, GLint *params);
extern void glGetConvolutionFilter (GLenum target, GLenum format, GLenum type, GLvoid *image);
extern void glGetConvolutionParameterfv (GLenum target, GLenum pname, GLfloat *params);
extern void glGetConvolutionParameteriv (GLenum target, GLenum pname, GLint *params);
extern void glGetDoublev (GLenum pname, GLdouble *params);
extern GLenum glGetError (void);
extern void glGetFloatv (GLenum pname, GLfloat *params);
extern void glGetHistogram (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
extern void glGetHistogramParameterfv (GLenum target, GLenum pname, GLfloat *params);
extern void glGetHistogramParameteriv (GLenum target, GLenum pname, GLint *params);
extern void glGetIntegerv (GLenum pname, GLint *params);
extern void glGetLightfv (GLenum light, GLenum pname, GLfloat *params);
extern void glGetLightiv (GLenum light, GLenum pname, GLint *params);
extern void glGetMapdv (GLenum target, GLenum query, GLdouble *v);
extern void glGetMapfv (GLenum target, GLenum query, GLfloat *v);
extern void glGetMapiv (GLenum target, GLenum query, GLint *v);
extern void glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params);
extern void glGetMaterialiv (GLenum face, GLenum pname, GLint *params);
extern void glGetMinmax (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
extern void glGetMinmaxParameterfv (GLenum target, GLenum pname, GLfloat *params);
extern void glGetMinmaxParameteriv (GLenum target, GLenum pname, GLint *params);
extern void glGetPixelMapfv (GLenum map, GLfloat *values);
extern void glGetPixelMapuiv (GLenum map, GLuint *values);
extern void glGetPixelMapusv (GLenum map, GLushort *values);
extern void glGetPointerv (GLenum pname, GLvoid **params);
extern void glGetPolygonStipple (GLubyte *mask);
extern void glGetSeparableFilter (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
extern const GLubyte *glGetString (GLenum name);
extern void glGetTexEnvfv (GLenum target, GLenum pname, GLfloat *params);
extern void glGetTexEnviv (GLenum target, GLenum pname, GLint *params);
extern void glGetTexGendv (GLenum coord, GLenum pname, GLdouble *params);
extern void glGetTexGenfv (GLenum coord, GLenum pname, GLfloat *params);
extern void glGetTexGeniv (GLenum coord, GLenum pname, GLint *params);
extern void glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
extern void glGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params);
extern void glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params);
extern void glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params);
extern void glGetTexParameteriv (GLenum target, GLenum pname, GLint *params);
extern void glHint (GLenum target, GLenum mode);
extern void glHistogram (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
extern void glIndexMask (GLuint mask);
extern void glIndexPointer (GLenum type, GLsizei stride, const GLvoid *pointer);
extern void glIndexd (GLdouble c);
extern void glIndexdv (const GLdouble *c);
extern void glIndexf (GLfloat c);
extern void glIndexfv (const GLfloat *c);
extern void glIndexi (GLint c);
extern void glIndexiv (const GLint *c);
extern void glIndexs (GLshort c);
extern void glIndexsv (const GLshort *c);
extern void glIndexub (GLubyte c);
extern void glIndexubv (const GLubyte *c);
extern void glInitNames (void);
extern void glInterleavedArrays (GLenum format, GLsizei stride, const GLvoid *pointer);
extern GLboolean glIsEnabled (GLenum cap);
extern GLboolean glIsList (GLuint list);
extern GLboolean glIsTexture (GLuint texture);
extern void glLightModelf (GLenum pname, GLfloat param);
extern void glLightModelfv (GLenum pname, const GLfloat *params);
extern void glLightModeli (GLenum pname, GLint param);
extern void glLightModeliv (GLenum pname, const GLint *params);
extern void glLightf (GLenum light, GLenum pname, GLfloat param);
extern void glLightfv (GLenum light, GLenum pname, const GLfloat *params);
extern void glLighti (GLenum light, GLenum pname, GLint param);
extern void glLightiv (GLenum light, GLenum pname, const GLint *params);
extern void glLineStipple (GLint factor, GLushort pattern);
extern void glLineWidth (GLfloat width);
extern void glListBase (GLuint base);
extern void glLoadIdentity (void);
extern void glLoadMatrixd (const GLdouble *m);
extern void glLoadMatrixf (const GLfloat *m);
extern void glLoadName (GLuint name);
extern void glLogicOp (GLenum opcode);
extern void glMap1d (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
extern void glMap1f (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
extern void glMap2d (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
extern void glMap2f (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
extern void glMapGrid1d (GLint un, GLdouble u1, GLdouble u2);
extern void glMapGrid1f (GLint un, GLfloat u1, GLfloat u2);
extern void glMapGrid2d (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
extern void glMapGrid2f (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
extern void glMaterialf (GLenum face, GLenum pname, GLfloat param);
extern void glMaterialfv (GLenum face, GLenum pname, const GLfloat *params);
extern void glMateriali (GLenum face, GLenum pname, GLint param);
extern void glMaterialiv (GLenum face, GLenum pname, const GLint *params);
extern void glMatrixMode (GLenum mode);
extern void glMinmax (GLenum target, GLenum internalformat, GLboolean sink);
extern void glMultMatrixd (const GLdouble *m);
extern void glMultMatrixf (const GLfloat *m);
extern void glNewList (GLuint list, GLenum mode);
extern void glNormal3b (GLbyte nx, GLbyte ny, GLbyte nz);
extern void glNormal3bv (const GLbyte *v);
extern void glNormal3d (GLdouble nx, GLdouble ny, GLdouble nz);
extern void glNormal3dv (const GLdouble *v);
extern void glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz);
extern void glNormal3fv (const GLfloat *v);
extern void glNormal3i (GLint nx, GLint ny, GLint nz);
extern void glNormal3iv (const GLint *v);
extern void glNormal3s (GLshort nx, GLshort ny, GLshort nz);
extern void glNormal3sv (const GLshort *v);
extern void glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer);
extern void glOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void glPassThrough (GLfloat token);
extern void glPixelMapfv (GLenum map, GLint mapsize, const GLfloat *values);
extern void glPixelMapuiv (GLenum map, GLint mapsize, const GLuint *values);
extern void glPixelMapusv (GLenum map, GLint mapsize, const GLushort *values);
extern void glPixelStoref (GLenum pname, GLfloat param);
extern void glPixelStorei (GLenum pname, GLint param);
extern void glPixelTransferf (GLenum pname, GLfloat param);
extern void glPixelTransferi (GLenum pname, GLint param);
extern void glPixelZoom (GLfloat xfactor, GLfloat yfactor);
extern void glPointSize (GLfloat size);
extern void glPolygonMode (GLenum face, GLenum mode);
extern void glPolygonOffset (GLfloat factor, GLfloat units);
extern void glPolygonStipple (const GLubyte *mask);
extern void glPopAttrib (void);
extern void glPopClientAttrib (void);
extern void glPopMatrix (void);
extern void glPopName (void);
extern void glPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities);
extern void glPushAttrib (GLbitfield mask);
extern void glPushClientAttrib (GLbitfield mask);
extern void glPushMatrix (void);
extern void glPushName (GLuint name);
extern void glRasterPos2d (GLdouble x, GLdouble y);
extern void glRasterPos2dv (const GLdouble *v);
extern void glRasterPos2f (GLfloat x, GLfloat y);
extern void glRasterPos2fv (const GLfloat *v);
extern void glRasterPos2i (GLint x, GLint y);
extern void glRasterPos2iv (const GLint *v);
extern void glRasterPos2s (GLshort x, GLshort y);
extern void glRasterPos2sv (const GLshort *v);
extern void glRasterPos3d (GLdouble x, GLdouble y, GLdouble z);
extern void glRasterPos3dv (const GLdouble *v);
extern void glRasterPos3f (GLfloat x, GLfloat y, GLfloat z);
extern void glRasterPos3fv (const GLfloat *v);
extern void glRasterPos3i (GLint x, GLint y, GLint z);
extern void glRasterPos3iv (const GLint *v);
extern void glRasterPos3s (GLshort x, GLshort y, GLshort z);
extern void glRasterPos3sv (const GLshort *v);
extern void glRasterPos4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void glRasterPos4dv (const GLdouble *v);
extern void glRasterPos4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void glRasterPos4fv (const GLfloat *v);
extern void glRasterPos4i (GLint x, GLint y, GLint z, GLint w);
extern void glRasterPos4iv (const GLint *v);
extern void glRasterPos4s (GLshort x, GLshort y, GLshort z, GLshort w);
extern void glRasterPos4sv (const GLshort *v);
extern void glReadBuffer (GLenum mode);
extern void glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
extern void glRectd (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
extern void glRectdv (const GLdouble *v1, const GLdouble *v2);
extern void glRectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
extern void glRectfv (const GLfloat *v1, const GLfloat *v2);
extern void glRecti (GLint x1, GLint y1, GLint x2, GLint y2);
extern void glRectiv (const GLint *v1, const GLint *v2);
extern void glRects (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
extern void glRectsv (const GLshort *v1, const GLshort *v2);
extern GLint glRenderMode (GLenum mode);
extern void glResetHistogram (GLenum target);
extern void glResetMinmax (GLenum target);
extern void glRotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
extern void glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
extern void glScaled (GLdouble x, GLdouble y, GLdouble z);
extern void glScalef (GLfloat x, GLfloat y, GLfloat z);
extern void glScissor (GLint x, GLint y, GLsizei width, GLsizei height);
extern void glSelectBuffer (GLsizei size, GLuint *buffer);
extern void glSeparableFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
extern void glShadeModel (GLenum mode);
extern void glStencilFunc (GLenum func, GLint ref, GLuint mask);
extern void glStencilMask (GLuint mask);
extern void glStencilOp (GLenum fail, GLenum zfail, GLenum zpass);
extern void glTexCoord1d (GLdouble s);
extern void glTexCoord1dv (const GLdouble *v);
extern void glTexCoord1f (GLfloat s);
extern void glTexCoord1fv (const GLfloat *v);
extern void glTexCoord1i (GLint s);
extern void glTexCoord1iv (const GLint *v);
extern void glTexCoord1s (GLshort s);
extern void glTexCoord1sv (const GLshort *v);
extern void glTexCoord2d (GLdouble s, GLdouble t);
extern void glTexCoord2dv (const GLdouble *v);
extern void glTexCoord2f (GLfloat s, GLfloat t);
extern void glTexCoord2fv (const GLfloat *v);
extern void glTexCoord2i (GLint s, GLint t);
extern void glTexCoord2iv (const GLint *v);
extern void glTexCoord2s (GLshort s, GLshort t);
extern void glTexCoord2sv (const GLshort *v);
extern void glTexCoord3d (GLdouble s, GLdouble t, GLdouble r);
extern void glTexCoord3dv (const GLdouble *v);
extern void glTexCoord3f (GLfloat s, GLfloat t, GLfloat r);
extern void glTexCoord3fv (const GLfloat *v);
extern void glTexCoord3i (GLint s, GLint t, GLint r);
extern void glTexCoord3iv (const GLint *v);
extern void glTexCoord3s (GLshort s, GLshort t, GLshort r);
extern void glTexCoord3sv (const GLshort *v);
extern void glTexCoord4d (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void glTexCoord4dv (const GLdouble *v);
extern void glTexCoord4f (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void glTexCoord4fv (const GLfloat *v);
extern void glTexCoord4i (GLint s, GLint t, GLint r, GLint q);
extern void glTexCoord4iv (const GLint *v);
extern void glTexCoord4s (GLshort s, GLshort t, GLshort r, GLshort q);
extern void glTexCoord4sv (const GLshort *v);
extern void glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void glTexEnvf (GLenum target, GLenum pname, GLfloat param);
extern void glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params);
extern void glTexEnvi (GLenum target, GLenum pname, GLint param);
extern void glTexEnviv (GLenum target, GLenum pname, const GLint *params);
extern void glTexGend (GLenum coord, GLenum pname, GLdouble param);
extern void glTexGendv (GLenum coord, GLenum pname, const GLdouble *params);
extern void glTexGenf (GLenum coord, GLenum pname, GLfloat param);
extern void glTexGenfv (GLenum coord, GLenum pname, const GLfloat *params);
extern void glTexGeni (GLenum coord, GLenum pname, GLint param);
extern void glTexGeniv (GLenum coord, GLenum pname, const GLint *params);
extern void glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern void glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern void glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern void glTexParameterf (GLenum target, GLenum pname, GLfloat param);
extern void glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params);
extern void glTexParameteri (GLenum target, GLenum pname, GLint param);
extern void glTexParameteriv (GLenum target, GLenum pname, const GLint *params);
extern void glTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
extern void glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern void glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
extern void glTranslated (GLdouble x, GLdouble y, GLdouble z);
extern void glTranslatef (GLfloat x, GLfloat y, GLfloat z);
extern void glVertex2d (GLdouble x, GLdouble y);
extern void glVertex2dv (const GLdouble *v);
extern void glVertex2f (GLfloat x, GLfloat y);
extern void glVertex2fv (const GLfloat *v);
extern void glVertex2i (GLint x, GLint y);
extern void glVertex2iv (const GLint *v);
extern void glVertex2s (GLshort x, GLshort y);
extern void glVertex2sv (const GLshort *v);
extern void glVertex3d (GLdouble x, GLdouble y, GLdouble z);
extern void glVertex3dv (const GLdouble *v);
extern void glVertex3f (GLfloat x, GLfloat y, GLfloat z);
extern void glVertex3fv (const GLfloat *v);
extern void glVertex3i (GLint x, GLint y, GLint z);
extern void glVertex3iv (const GLint *v);
extern void glVertex3s (GLshort x, GLshort y, GLshort z);
extern void glVertex3sv (const GLshort *v);
extern void glVertex4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void glVertex4dv (const GLdouble *v);
extern void glVertex4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void glVertex4fv (const GLfloat *v);
extern void glVertex4i (GLint x, GLint y, GLint z, GLint w);
extern void glVertex4iv (const GLint *v);
extern void glVertex4s (GLshort x, GLshort y, GLshort z, GLshort w);
extern void glVertex4sv (const GLshort *v);
extern void glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void glViewport (GLint x, GLint y, GLsizei width, GLsizei height);

extern void glSampleCoverage (GLclampf value, GLboolean invert);

extern void glLoadTransposeMatrixf (const GLfloat *m);
extern void glLoadTransposeMatrixd (const GLdouble *m);
extern void glMultTransposeMatrixf (const GLfloat *m);
extern void glMultTransposeMatrixd (const GLdouble *m);

extern void glCompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
extern void glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
extern void glCompressedTexImage1D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
extern void glCompressedTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
extern void glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
extern void glCompressedTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
extern void glGetCompressedTexImage (GLenum target, GLint lod, GLvoid *img);

extern void glActiveTexture (GLenum texture);
extern void glClientActiveTexture (GLenum texture);
extern void glMultiTexCoord1d (GLenum target, GLdouble s);
extern void glMultiTexCoord1dv (GLenum target, const GLdouble *v);
extern void glMultiTexCoord1f (GLenum target, GLfloat s);
extern void glMultiTexCoord1fv (GLenum target, const GLfloat *v);
extern void glMultiTexCoord1i (GLenum target, GLint s);
extern void glMultiTexCoord1iv (GLenum target, const GLint *v);
extern void glMultiTexCoord1s (GLenum target, GLshort s);
extern void glMultiTexCoord1sv (GLenum target, const GLshort *v);
extern void glMultiTexCoord2d (GLenum target, GLdouble s, GLdouble t);
extern void glMultiTexCoord2dv (GLenum target, const GLdouble *v);
extern void glMultiTexCoord2f (GLenum target, GLfloat s, GLfloat t);
extern void glMultiTexCoord2fv (GLenum target, const GLfloat *v);
extern void glMultiTexCoord2i (GLenum target, GLint s, GLint t);
extern void glMultiTexCoord2iv (GLenum target, const GLint *v);
extern void glMultiTexCoord2s (GLenum target, GLshort s, GLshort t);
extern void glMultiTexCoord2sv (GLenum target, const GLshort *v);
extern void glMultiTexCoord3d (GLenum target, GLdouble s, GLdouble t, GLdouble r);
extern void glMultiTexCoord3dv (GLenum target, const GLdouble *v);
extern void glMultiTexCoord3f (GLenum target, GLfloat s, GLfloat t, GLfloat r);
extern void glMultiTexCoord3fv (GLenum target, const GLfloat *v);
extern void glMultiTexCoord3i (GLenum target, GLint s, GLint t, GLint r);
extern void glMultiTexCoord3iv (GLenum target, const GLint *v);
extern void glMultiTexCoord3s (GLenum target, GLshort s, GLshort t, GLshort r);
extern void glMultiTexCoord3sv (GLenum target, const GLshort *v);
extern void glMultiTexCoord4d (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void glMultiTexCoord4dv (GLenum target, const GLdouble *v);
extern void glMultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void glMultiTexCoord4fv (GLenum target, const GLfloat *v);
extern void glMultiTexCoord4i (GLenum target, GLint s, GLint t, GLint r, GLint q);
extern void glMultiTexCoord4iv (GLenum target, const GLint *v);
extern void glMultiTexCoord4s (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
extern void glMultiTexCoord4sv (GLenum target, const GLshort *v);

extern void glFogCoordf (GLfloat coord);
extern void glFogCoordfv (const GLfloat *coord);  
extern void glFogCoordd (GLdouble coord);
extern void glFogCoorddv (const GLdouble *coord);   
extern void glFogCoordPointer (GLenum type, GLsizei stride, const GLvoid *pointer);

extern void glSecondaryColor3b (GLbyte red, GLbyte green, GLbyte blue);
extern void glSecondaryColor3bv (const GLbyte *v);
extern void glSecondaryColor3d (GLdouble red, GLdouble green, GLdouble blue);
extern void glSecondaryColor3dv (const GLdouble *v);
extern void glSecondaryColor3f (GLfloat red, GLfloat green, GLfloat blue);
extern void glSecondaryColor3fv (const GLfloat *v);
extern void glSecondaryColor3i (GLint red, GLint green, GLint blue);
extern void glSecondaryColor3iv (const GLint *v);
extern void glSecondaryColor3s (GLshort red, GLshort green, GLshort blue);
extern void glSecondaryColor3sv (const GLshort *v);
extern void glSecondaryColor3ub (GLubyte red, GLubyte green, GLubyte blue);
extern void glSecondaryColor3ubv (const GLubyte *v);
extern void glSecondaryColor3ui (GLuint red, GLuint green, GLuint blue);
extern void glSecondaryColor3uiv (const GLuint *v);
extern void glSecondaryColor3us (GLushort red, GLushort green, GLushort blue);
extern void glSecondaryColor3usv (const GLushort *v);
extern void glSecondaryColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

extern void glPointParameterf (GLenum pname, GLfloat param); 
extern void glPointParameterfv (GLenum pname, const GLfloat *params);
extern void glPointParameteri (GLenum pname, GLint param); 
extern void glPointParameteriv (GLenum pname, const GLint *params);

extern void glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

extern void glMultiDrawArrays (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
extern void glMultiDrawElements (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* const *indices, GLsizei primcount);

extern void glWindowPos2d (GLdouble x, GLdouble y);
extern void glWindowPos2dv (const GLdouble *v);
extern void glWindowPos2f (GLfloat x, GLfloat y);
extern void glWindowPos2fv (const GLfloat *v);
extern void glWindowPos2i (GLint x, GLint y); 
extern void glWindowPos2iv (const GLint *v);
extern void glWindowPos2s (GLshort x, GLshort y);
extern void glWindowPos2sv (const GLshort *v);
extern void glWindowPos3d (GLdouble x, GLdouble y, GLdouble z);
extern void glWindowPos3dv (const GLdouble *v);
extern void glWindowPos3f (GLfloat x, GLfloat y, GLfloat z);
extern void glWindowPos3fv (const GLfloat *v);
extern void glWindowPos3i (GLint x, GLint y, GLint z);
extern void glWindowPos3iv (const GLint *v);
extern void glWindowPos3s (GLshort x, GLshort y, GLshort z);
extern void glWindowPos3sv (const GLshort *v);

extern void glGenQueries (GLsizei n, GLuint *ids);
extern void glDeleteQueries (GLsizei n, const GLuint *ids);
extern GLboolean glIsQuery (GLuint id);
extern void glBeginQuery (GLenum target, GLuint id);
extern void glEndQuery (GLenum target);
extern void glGetQueryiv (GLenum target, GLenum pname, GLint *params);
extern void glGetQueryObjectiv (GLuint id, GLenum pname, GLint *params);
extern void glGetQueryObjectuiv (GLuint id, GLenum pname, GLuint *params);

extern void glBindBuffer (GLenum target, GLuint buffer);
extern void glDeleteBuffers (GLsizei n, const GLuint *buffers);
extern void glGenBuffers (GLsizei n, GLuint *buffers);
extern GLboolean glIsBuffer (GLuint buffer);
extern void glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
extern void glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
extern void glGetBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
extern GLvoid *glMapBuffer (GLenum target, GLenum access);
extern GLboolean glUnmapBuffer (GLenum target);
extern void glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params);
extern void glGetBufferPointerv (GLenum target, GLenum pname, GLvoid **params);

extern void glDrawBuffers (GLsizei n, const GLenum *bufs);
extern void glVertexAttrib1d (GLuint index, GLdouble x);
extern void glVertexAttrib1dv (GLuint index, const GLdouble *v);
extern void glVertexAttrib1f (GLuint index, GLfloat x);
extern void glVertexAttrib1fv (GLuint index, const GLfloat *v);
extern void glVertexAttrib1s (GLuint index, GLshort x);
extern void glVertexAttrib1sv (GLuint index, const GLshort *v);
extern void glVertexAttrib2d (GLuint index, GLdouble x, GLdouble y);
extern void glVertexAttrib2dv (GLuint index, const GLdouble *v);
extern void glVertexAttrib2f (GLuint index, GLfloat x, GLfloat y);
extern void glVertexAttrib2fv (GLuint index, const GLfloat *v);
extern void glVertexAttrib2s (GLuint index, GLshort x, GLshort y);
extern void glVertexAttrib2sv (GLuint index, const GLshort *v);
extern void glVertexAttrib3d (GLuint index, GLdouble x, GLdouble y, GLdouble z);
extern void glVertexAttrib3dv (GLuint index, const GLdouble *v);
extern void glVertexAttrib3f (GLuint index, GLfloat x, GLfloat y, GLfloat z);
extern void glVertexAttrib3fv (GLuint index, const GLfloat *v);
extern void glVertexAttrib3s (GLuint index, GLshort x, GLshort y, GLshort z);
extern void glVertexAttrib3sv (GLuint index, const GLshort *v);
extern void glVertexAttrib4Nbv (GLuint index, const GLbyte *v);
extern void glVertexAttrib4Niv (GLuint index, const GLint *v);
extern void glVertexAttrib4Nsv (GLuint index, const GLshort *v);
extern void glVertexAttrib4Nub (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
extern void glVertexAttrib4Nubv (GLuint index, const GLubyte *v);
extern void glVertexAttrib4Nuiv (GLuint index, const GLuint *v);
extern void glVertexAttrib4Nusv (GLuint index, const GLushort *v);
extern void glVertexAttrib4bv (GLuint index, const GLbyte *v);
extern void glVertexAttrib4d (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void glVertexAttrib4dv (GLuint index, const GLdouble *v);
extern void glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void glVertexAttrib4fv (GLuint index, const GLfloat *v);
extern void glVertexAttrib4iv (GLuint index, const GLint *v);
extern void glVertexAttrib4s (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
extern void glVertexAttrib4sv (GLuint index, const GLshort *v);
extern void glVertexAttrib4ubv (GLuint index, const GLubyte *v);
extern void glVertexAttrib4uiv (GLuint index, const GLuint *v);
extern void glVertexAttrib4usv (GLuint index, const GLushort *v);
extern void glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
extern void glEnableVertexAttribArray (GLuint index);
extern void glDisableVertexAttribArray (GLuint index);
extern void glGetVertexAttribdv (GLuint index, GLenum pname, GLdouble *params);
extern void glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params);
extern void glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params);
extern void glGetVertexAttribPointerv (GLuint index, GLenum pname, GLvoid **pointer);
extern void glDeleteShader (GLuint shader);
extern void glDetachShader (GLuint program, GLuint shader);
extern GLuint glCreateShader (GLenum type);
extern void glShaderSource (GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length);
extern void glCompileShader (GLuint shader);
extern GLuint glCreateProgram (void);
extern void glAttachShader (GLuint program, GLuint shader);
extern void glLinkProgram (GLuint program);
extern void glUseProgram (GLuint program);
extern void glDeleteProgram (GLuint program);
extern void glValidateProgram (GLuint program);
extern void glUniform1f (GLint location, GLfloat v0);
extern void glUniform2f (GLint location, GLfloat v0, GLfloat v1);
extern void glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void glUniform1i (GLint location, GLint v0);
extern void glUniform2i (GLint location, GLint v0, GLint v1);
extern void glUniform3i (GLint location, GLint v0, GLint v1, GLint v2);
extern void glUniform4i (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void glUniform1fv (GLint location, GLsizei count, const GLfloat *value);
extern void glUniform2fv (GLint location, GLsizei count, const GLfloat *value);
extern void glUniform3fv (GLint location, GLsizei count, const GLfloat *value);
extern void glUniform4fv (GLint location, GLsizei count, const GLfloat *value);
extern void glUniform1iv (GLint location, GLsizei count, const GLint *value);
extern void glUniform2iv (GLint location, GLsizei count, const GLint *value);
extern void glUniform3iv (GLint location, GLsizei count, const GLint *value);
extern void glUniform4iv (GLint location, GLsizei count, const GLint *value);
extern void glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern GLboolean glIsShader (GLuint shader);
extern GLboolean glIsProgram (GLuint program);
extern void glGetShaderiv (GLuint shader, GLenum pname, GLint *params);
extern void glGetProgramiv (GLuint program, GLenum pname, GLint *params);
extern void glGetAttachedShaders (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
extern void glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern GLint glGetUniformLocation (GLuint program, const GLchar *name);
extern void glGetActiveUniform (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern void glGetUniformfv (GLuint program, GLint location, GLfloat *params);
extern void glGetUniformiv (GLuint program, GLint location, GLint *params);
extern void glGetShaderSource (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
extern void glBindAttribLocation (GLuint program, GLuint index, const GLchar *name);
extern void glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern GLint glGetAttribLocation (GLuint program, const GLchar *name);
extern void glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask);
extern void glStencilOpSeparate (GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
extern void glStencilMaskSeparate (GLenum face, GLuint mask);

extern void glUniformMatrix2x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void glUniformMatrix3x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void glUniformMatrix2x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void glUniformMatrix4x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void glUniformMatrix3x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void glUniformMatrix4x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);


#endif /* GL_GLEXT_FUNCTION_POINTERS */

#ifdef __cplusplus
}
#endif

#endif /* __gl_h_ */
