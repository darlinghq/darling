/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// CGLCopyContext: copies the state of the OpenGL attribute groups that the mask names (the glPushAttrib groups, with
// the same bits) from one context to another. Host OpenGL has no call that copies a context, so the state of each
// group is read from the source context and set in the destination. The contexts are made current in turn through
// the public CGL calls; the context that was current on the thread is current again at the end. What the groups
// hold that is not state of the context alone is not copied: the contents of texture objects and the bindings of
// textures (copied only between contexts that share them), the current matrices, evaluators, the raster position,
// and the client state (pixel storage, vertex arrays).

#define GL_GLEXT_PROTOTYPES 1
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <string.h>

#define MAX_LIGHTS 8
#define MAX_CLIP_PLANES 6

static const struct {
	GLenum capability;
	GLbitfield group;
} kCapabilities[] = {
	{ GL_ALPHA_TEST, GL_COLOR_BUFFER_BIT }, { GL_AUTO_NORMAL, GL_EVAL_BIT }, { GL_BLEND, GL_COLOR_BUFFER_BIT },
	{ GL_CLIP_PLANE0, GL_TRANSFORM_BIT }, { GL_CLIP_PLANE1, GL_TRANSFORM_BIT }, { GL_CLIP_PLANE2, GL_TRANSFORM_BIT },
	{ GL_CLIP_PLANE3, GL_TRANSFORM_BIT }, { GL_CLIP_PLANE4, GL_TRANSFORM_BIT }, { GL_CLIP_PLANE5, GL_TRANSFORM_BIT },
	{ GL_COLOR_LOGIC_OP, GL_COLOR_BUFFER_BIT }, { GL_COLOR_MATERIAL, GL_LIGHTING_BIT },
	{ GL_CULL_FACE, GL_POLYGON_BIT }, { GL_DEPTH_TEST, GL_DEPTH_BUFFER_BIT }, { GL_DITHER, GL_COLOR_BUFFER_BIT },
	{ GL_FOG, GL_FOG_BIT },
	{ GL_LIGHT0, GL_LIGHTING_BIT }, { GL_LIGHT1, GL_LIGHTING_BIT }, { GL_LIGHT2, GL_LIGHTING_BIT },
	{ GL_LIGHT3, GL_LIGHTING_BIT }, { GL_LIGHT4, GL_LIGHTING_BIT }, { GL_LIGHT5, GL_LIGHTING_BIT },
	{ GL_LIGHT6, GL_LIGHTING_BIT }, { GL_LIGHT7, GL_LIGHTING_BIT }, { GL_LIGHTING, GL_LIGHTING_BIT },
	{ GL_LINE_SMOOTH, GL_LINE_BIT }, { GL_LINE_STIPPLE, GL_LINE_BIT }, { GL_INDEX_LOGIC_OP, GL_COLOR_BUFFER_BIT },
	{ GL_MULTISAMPLE, GL_MULTISAMPLE_BIT }, { GL_NORMALIZE, GL_TRANSFORM_BIT },
	{ GL_POINT_SMOOTH, GL_POINT_BIT },
	{ GL_POLYGON_OFFSET_FILL, GL_POLYGON_BIT }, { GL_POLYGON_OFFSET_LINE, GL_POLYGON_BIT },
	{ GL_POLYGON_OFFSET_POINT, GL_POLYGON_BIT }, { GL_POLYGON_SMOOTH, GL_POLYGON_BIT },
	{ GL_POLYGON_STIPPLE, GL_POLYGON_BIT }, { GL_RESCALE_NORMAL, GL_TRANSFORM_BIT },
	{ GL_SAMPLE_ALPHA_TO_COVERAGE, GL_MULTISAMPLE_BIT }, { GL_SAMPLE_ALPHA_TO_ONE, GL_MULTISAMPLE_BIT },
	{ GL_SAMPLE_COVERAGE, GL_MULTISAMPLE_BIT }, { GL_SCISSOR_TEST, GL_SCISSOR_BIT },
	{ GL_STENCIL_TEST, GL_STENCIL_BUFFER_BIT },
	{ GL_TEXTURE_1D, GL_TEXTURE_BIT }, { GL_TEXTURE_2D, GL_TEXTURE_BIT }, { GL_TEXTURE_3D, GL_TEXTURE_BIT },
	{ GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BIT },
	{ GL_TEXTURE_GEN_S, GL_TEXTURE_BIT }, { GL_TEXTURE_GEN_T, GL_TEXTURE_BIT }, { GL_TEXTURE_GEN_R, GL_TEXTURE_BIT },
	{ GL_TEXTURE_GEN_Q, GL_TEXTURE_BIT }, { GL_COLOR_SUM, 0 },
};
#define CAPABILITY_COUNT (sizeof(kCapabilities) / sizeof(kCapabilities[0]))

static const GLenum kPixelTransfers[] = {
	GL_RED_SCALE, GL_RED_BIAS, GL_GREEN_SCALE, GL_GREEN_BIAS, GL_BLUE_SCALE, GL_BLUE_BIAS, GL_ALPHA_SCALE,
	GL_ALPHA_BIAS, GL_DEPTH_SCALE, GL_DEPTH_BIAS, GL_INDEX_SHIFT, GL_INDEX_OFFSET,
};
#define PIXEL_TRANSFER_COUNT (sizeof(kPixelTransfers) / sizeof(kPixelTransfers[0]))

static const GLenum kHints[] = {
	GL_PERSPECTIVE_CORRECTION_HINT, GL_POINT_SMOOTH_HINT, GL_LINE_SMOOTH_HINT, GL_POLYGON_SMOOTH_HINT, GL_FOG_HINT,
	GL_GENERATE_MIPMAP_HINT, GL_TEXTURE_COMPRESSION_HINT,
};
#define HINT_COUNT (sizeof(kHints) / sizeof(kHints[0]))

struct light_state {
	GLfloat ambient[4], diffuse[4], specular[4], position[4], direction[3];
	GLfloat exponent, cutoff, constant, linear, quadratic;
};

struct material_state {
	GLfloat ambient[4], diffuse[4], specular[4], emission[4], shininess;
};

struct state {
	GLbitfield mask;
	GLboolean enabled[CAPABILITY_COUNT];
	// GL_CURRENT_BIT
	GLfloat color[4], secondary_color[4], normal[3], texcoord[4], fog_coord;
	GLboolean edge_flag;
	// GL_POINT_BIT, GL_LINE_BIT
	GLfloat point_size, line_width;
	GLint line_stipple_factor, line_stipple_pattern;
	// GL_POLYGON_BIT, GL_POLYGON_STIPPLE_BIT
	GLint cull_face_mode, front_face, polygon_mode[2];
	GLfloat polygon_offset_factor, polygon_offset_units;
	GLubyte polygon_stipple[128];
	// GL_PIXEL_MODE_BIT
	GLfloat transfer[PIXEL_TRANSFER_COUNT];
	GLfloat zoom_x, zoom_y;
	GLboolean map_color, map_stencil;
	// GL_LIGHTING_BIT
	GLint shade_model, color_material_face, color_material_mode;
	GLfloat model_ambient[4];
	GLboolean model_local_viewer, model_two_side;
	GLint model_color_control;
	struct light_state lights[MAX_LIGHTS];
	struct material_state front, back;
	// GL_FOG_BIT
	GLint fog_mode;
	GLfloat fog_density, fog_start, fog_end, fog_color[4];
	// GL_DEPTH_BUFFER_BIT, GL_ACCUM_BUFFER_BIT, GL_STENCIL_BUFFER_BIT
	GLint depth_func;
	GLfloat depth_clear;
	GLboolean depth_mask;
	GLfloat accum_clear[4];
	GLint stencil_func, stencil_ref, stencil_value_mask, stencil_fail, stencil_zfail, stencil_zpass;
	GLint stencil_write_mask, stencil_clear;
	// GL_VIEWPORT_BIT, GL_SCISSOR_BIT
	GLint viewport[4], scissor[4];
	GLfloat depth_range[2];
	// GL_TRANSFORM_BIT
	GLint matrix_mode;
	GLdouble clip_planes[MAX_CLIP_PLANES][4];
	// GL_COLOR_BUFFER_BIT
	GLint alpha_func;
	GLfloat alpha_ref;
	GLint blend_src_rgb, blend_dst_rgb, blend_src_alpha, blend_dst_alpha, blend_eq_rgb, blend_eq_alpha;
	GLfloat blend_color[4], clear_color[4];
	GLint logic_op;
	GLboolean color_mask[4];
	// GL_HINT_BIT, GL_LIST_BIT
	GLint hints[HINT_COUNT];
	GLint list_base;
	// GL_TEXTURE_BIT: the first texture unit
	GLint env_mode;
	GLfloat env_color[4];
	GLint gen_mode[4];
	GLfloat object_plane[4][4], eye_plane[4][4];
	// GL_MULTISAMPLE_BIT
	GLfloat sample_coverage_value;
	GLboolean sample_coverage_invert;
};

static const GLenum kLights[MAX_LIGHTS] = {
	GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7,
};
static const GLenum kGenCoords[4] = { GL_S, GL_T, GL_R, GL_Q };

static void readMaterial(GLenum face, struct material_state *material) {
	glGetMaterialfv(face, GL_AMBIENT, material->ambient);
	glGetMaterialfv(face, GL_DIFFUSE, material->diffuse);
	glGetMaterialfv(face, GL_SPECULAR, material->specular);
	glGetMaterialfv(face, GL_EMISSION, material->emission);
	glGetMaterialfv(face, GL_SHININESS, &material->shininess);
}

static void writeMaterial(GLenum face, const struct material_state *material) {
	glMaterialfv(face, GL_AMBIENT, material->ambient);
	glMaterialfv(face, GL_DIFFUSE, material->diffuse);
	glMaterialfv(face, GL_SPECULAR, material->specular);
	glMaterialfv(face, GL_EMISSION, material->emission);
	glMaterialfv(face, GL_SHININESS, &material->shininess);
}

// Reads the groups of mask from the current context.
static void readState(struct state *state) {
	GLbitfield mask = state->mask;
	GLint value[4];
	for (size_t i = 0; i < CAPABILITY_COUNT; i++)
		if (mask & (kCapabilities[i].group | GL_ENABLE_BIT))
			state->enabled[i] = glIsEnabled(kCapabilities[i].capability);

	if (mask & GL_CURRENT_BIT) {
		glGetFloatv(GL_CURRENT_COLOR, state->color);
		glGetFloatv(GL_CURRENT_SECONDARY_COLOR, state->secondary_color);
		glGetFloatv(GL_CURRENT_NORMAL, state->normal);
		glGetFloatv(GL_CURRENT_TEXTURE_COORDS, state->texcoord);
		glGetFloatv(GL_CURRENT_FOG_COORD, &state->fog_coord);
		glGetBooleanv(GL_EDGE_FLAG, &state->edge_flag);
	}
	if (mask & GL_POINT_BIT)
		glGetFloatv(GL_POINT_SIZE, &state->point_size);
	if (mask & GL_LINE_BIT) {
		glGetFloatv(GL_LINE_WIDTH, &state->line_width);
		glGetIntegerv(GL_LINE_STIPPLE_REPEAT, &state->line_stipple_factor);
		glGetIntegerv(GL_LINE_STIPPLE_PATTERN, &state->line_stipple_pattern);
	}
	if (mask & GL_POLYGON_BIT) {
		glGetIntegerv(GL_CULL_FACE_MODE, &state->cull_face_mode);
		glGetIntegerv(GL_FRONT_FACE, &state->front_face);
		glGetIntegerv(GL_POLYGON_MODE, state->polygon_mode);
		glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &state->polygon_offset_factor);
		glGetFloatv(GL_POLYGON_OFFSET_UNITS, &state->polygon_offset_units);
	}
	if (mask & GL_POLYGON_STIPPLE_BIT) {
		GLint alignment;
		glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glGetPolygonStipple(state->polygon_stipple);
		glPixelStorei(GL_PACK_ALIGNMENT, alignment);
	}
	if (mask & GL_PIXEL_MODE_BIT) {
		for (size_t i = 0; i < PIXEL_TRANSFER_COUNT; i++)
			glGetFloatv(kPixelTransfers[i], &state->transfer[i]);
		glGetFloatv(GL_ZOOM_X, &state->zoom_x);
		glGetFloatv(GL_ZOOM_Y, &state->zoom_y);
		glGetBooleanv(GL_MAP_COLOR, &state->map_color);
		glGetBooleanv(GL_MAP_STENCIL, &state->map_stencil);
	}
	if (mask & GL_LIGHTING_BIT) {
		glGetIntegerv(GL_SHADE_MODEL, &state->shade_model);
		glGetIntegerv(GL_COLOR_MATERIAL_FACE, &state->color_material_face);
		glGetIntegerv(GL_COLOR_MATERIAL_PARAMETER, &state->color_material_mode);
		glGetFloatv(GL_LIGHT_MODEL_AMBIENT, state->model_ambient);
		glGetBooleanv(GL_LIGHT_MODEL_LOCAL_VIEWER, &state->model_local_viewer);
		glGetBooleanv(GL_LIGHT_MODEL_TWO_SIDE, &state->model_two_side);
		glGetIntegerv(GL_LIGHT_MODEL_COLOR_CONTROL, &state->model_color_control);
		for (int i = 0; i < MAX_LIGHTS; i++) {
			struct light_state *light = &state->lights[i];
			glGetLightfv(kLights[i], GL_AMBIENT, light->ambient);
			glGetLightfv(kLights[i], GL_DIFFUSE, light->diffuse);
			glGetLightfv(kLights[i], GL_SPECULAR, light->specular);
			glGetLightfv(kLights[i], GL_POSITION, light->position);
			glGetLightfv(kLights[i], GL_SPOT_DIRECTION, light->direction);
			glGetLightfv(kLights[i], GL_SPOT_EXPONENT, &light->exponent);
			glGetLightfv(kLights[i], GL_SPOT_CUTOFF, &light->cutoff);
			glGetLightfv(kLights[i], GL_CONSTANT_ATTENUATION, &light->constant);
			glGetLightfv(kLights[i], GL_LINEAR_ATTENUATION, &light->linear);
			glGetLightfv(kLights[i], GL_QUADRATIC_ATTENUATION, &light->quadratic);
		}
		readMaterial(GL_FRONT, &state->front);
		readMaterial(GL_BACK, &state->back);
	}
	if (mask & GL_FOG_BIT) {
		glGetIntegerv(GL_FOG_MODE, &state->fog_mode);
		glGetFloatv(GL_FOG_DENSITY, &state->fog_density);
		glGetFloatv(GL_FOG_START, &state->fog_start);
		glGetFloatv(GL_FOG_END, &state->fog_end);
		glGetFloatv(GL_FOG_COLOR, state->fog_color);
	}
	if (mask & GL_DEPTH_BUFFER_BIT) {
		glGetIntegerv(GL_DEPTH_FUNC, &state->depth_func);
		glGetFloatv(GL_DEPTH_CLEAR_VALUE, &state->depth_clear);
		glGetBooleanv(GL_DEPTH_WRITEMASK, &state->depth_mask);
	}
	if (mask & GL_ACCUM_BUFFER_BIT)
		glGetFloatv(GL_ACCUM_CLEAR_VALUE, state->accum_clear);
	if (mask & GL_STENCIL_BUFFER_BIT) {
		glGetIntegerv(GL_STENCIL_FUNC, &state->stencil_func);
		glGetIntegerv(GL_STENCIL_REF, &state->stencil_ref);
		glGetIntegerv(GL_STENCIL_VALUE_MASK, &state->stencil_value_mask);
		glGetIntegerv(GL_STENCIL_FAIL, &state->stencil_fail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &state->stencil_zfail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &state->stencil_zpass);
		glGetIntegerv(GL_STENCIL_WRITEMASK, &state->stencil_write_mask);
		glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &state->stencil_clear);
	}
	if (mask & GL_VIEWPORT_BIT) {
		glGetIntegerv(GL_VIEWPORT, state->viewport);
		glGetFloatv(GL_DEPTH_RANGE, state->depth_range);
	}
	if (mask & GL_SCISSOR_BIT)
		glGetIntegerv(GL_SCISSOR_BOX, state->scissor);
	if (mask & GL_TRANSFORM_BIT) {
		glGetIntegerv(GL_MATRIX_MODE, &state->matrix_mode);
		for (int i = 0; i < MAX_CLIP_PLANES; i++)
			glGetClipPlane(GL_CLIP_PLANE0 + i, state->clip_planes[i]);
	}
	if (mask & GL_COLOR_BUFFER_BIT) {
		glGetIntegerv(GL_ALPHA_TEST_FUNC, &state->alpha_func);
		glGetFloatv(GL_ALPHA_TEST_REF, &state->alpha_ref);
		glGetIntegerv(GL_BLEND_SRC_RGB, &state->blend_src_rgb);
		glGetIntegerv(GL_BLEND_DST_RGB, &state->blend_dst_rgb);
		glGetIntegerv(GL_BLEND_SRC_ALPHA, &state->blend_src_alpha);
		glGetIntegerv(GL_BLEND_DST_ALPHA, &state->blend_dst_alpha);
		glGetIntegerv(GL_BLEND_EQUATION_RGB, &state->blend_eq_rgb);
		glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &state->blend_eq_alpha);
		glGetFloatv(GL_BLEND_COLOR, state->blend_color);
		glGetFloatv(GL_COLOR_CLEAR_VALUE, state->clear_color);
		glGetIntegerv(GL_LOGIC_OP_MODE, &state->logic_op);
		glGetBooleanv(GL_COLOR_WRITEMASK, state->color_mask);
	}
	if (mask & GL_HINT_BIT)
		for (size_t i = 0; i < HINT_COUNT; i++)
			glGetIntegerv(kHints[i], &state->hints[i]);
	if (mask & GL_LIST_BIT)
		glGetIntegerv(GL_LIST_BASE, &state->list_base);
	if (mask & GL_TEXTURE_BIT) {
		GLint unit;
		glGetIntegerv(GL_ACTIVE_TEXTURE, &unit);
		glActiveTexture(GL_TEXTURE0);
		glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &state->env_mode);
		glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, state->env_color);
		for (int i = 0; i < 4; i++) {
			glGetTexGeniv(kGenCoords[i], GL_TEXTURE_GEN_MODE, &state->gen_mode[i]);
			glGetTexGenfv(kGenCoords[i], GL_OBJECT_PLANE, state->object_plane[i]);
			glGetTexGenfv(kGenCoords[i], GL_EYE_PLANE, state->eye_plane[i]);
		}
		glActiveTexture((GLenum) unit);
	}
	if (mask & GL_MULTISAMPLE_BIT) {
		glGetFloatv(GL_SAMPLE_COVERAGE_VALUE, &state->sample_coverage_value);
		glGetBooleanv(GL_SAMPLE_COVERAGE_INVERT, &state->sample_coverage_invert);
	}
	(void) value;
}

// Sets the groups of mask in the current context. Positions and directions of lights and clip planes are held in
// eye coordinates, which the modelview matrix is applied to when they are set: they are set with an identity one.
static void writeState(const struct state *state) {
	GLbitfield mask = state->mask;
	for (size_t i = 0; i < CAPABILITY_COUNT; i++) {
		if (mask & (kCapabilities[i].group | GL_ENABLE_BIT)) {
			if (state->enabled[i])
				glEnable(kCapabilities[i].capability);
			else
				glDisable(kCapabilities[i].capability);
		}
	}

	if (mask & GL_CURRENT_BIT) {
		glColor4fv(state->color);
		glSecondaryColor3fv(state->secondary_color);
		glNormal3fv(state->normal);
		glTexCoord4fv(state->texcoord);
		glFogCoordf(state->fog_coord);
		glEdgeFlag(state->edge_flag);
	}
	if (mask & GL_POINT_BIT)
		glPointSize(state->point_size);
	if (mask & GL_LINE_BIT) {
		glLineWidth(state->line_width);
		glLineStipple(state->line_stipple_factor, (GLushort) state->line_stipple_pattern);
	}
	if (mask & GL_POLYGON_BIT) {
		glCullFace((GLenum) state->cull_face_mode);
		glFrontFace((GLenum) state->front_face);
		// the polygon modes of the front and back faces; both are the same unless set apart
		glPolygonMode(GL_FRONT, (GLenum) state->polygon_mode[0]);
		glPolygonMode(GL_BACK, (GLenum) state->polygon_mode[1]);
		glPolygonOffset(state->polygon_offset_factor, state->polygon_offset_units);
	}
	if (mask & GL_POLYGON_STIPPLE_BIT) {
		GLint alignment;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPolygonStipple(state->polygon_stipple);
		glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	}
	if (mask & GL_PIXEL_MODE_BIT) {
		for (size_t i = 0; i < PIXEL_TRANSFER_COUNT; i++)
			glPixelTransferf(kPixelTransfers[i], state->transfer[i]);
		glPixelTransferi(GL_MAP_COLOR, state->map_color);
		glPixelTransferi(GL_MAP_STENCIL, state->map_stencil);
		glPixelZoom(state->zoom_x, state->zoom_y);
	}

	GLint matrix_mode = 0;
	int identity = (mask & (GL_LIGHTING_BIT | GL_TRANSFORM_BIT | GL_TEXTURE_BIT)) != 0;
	if (identity) {
		glGetIntegerv(GL_MATRIX_MODE, &matrix_mode);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
	}
	if (mask & GL_LIGHTING_BIT) {
		glShadeModel((GLenum) state->shade_model);
		glColorMaterial((GLenum) state->color_material_face, (GLenum) state->color_material_mode);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, state->model_ambient);
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, state->model_local_viewer);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, state->model_two_side);
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, state->model_color_control);
		for (int i = 0; i < MAX_LIGHTS; i++) {
			const struct light_state *light = &state->lights[i];
			glLightfv(kLights[i], GL_AMBIENT, light->ambient);
			glLightfv(kLights[i], GL_DIFFUSE, light->diffuse);
			glLightfv(kLights[i], GL_SPECULAR, light->specular);
			glLightfv(kLights[i], GL_POSITION, light->position);
			glLightfv(kLights[i], GL_SPOT_DIRECTION, light->direction);
			glLightf(kLights[i], GL_SPOT_EXPONENT, light->exponent);
			glLightf(kLights[i], GL_SPOT_CUTOFF, light->cutoff);
			glLightf(kLights[i], GL_CONSTANT_ATTENUATION, light->constant);
			glLightf(kLights[i], GL_LINEAR_ATTENUATION, light->linear);
			glLightf(kLights[i], GL_QUADRATIC_ATTENUATION, light->quadratic);
		}
		writeMaterial(GL_FRONT, &state->front);
		writeMaterial(GL_BACK, &state->back);
	}
	if (mask & GL_TRANSFORM_BIT)
		for (int i = 0; i < MAX_CLIP_PLANES; i++)
			glClipPlane(GL_CLIP_PLANE0 + i, state->clip_planes[i]);
	if (mask & GL_TEXTURE_BIT) {
		GLint unit;
		glGetIntegerv(GL_ACTIVE_TEXTURE, &unit);
		glActiveTexture(GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, state->env_mode);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, state->env_color);
		for (int i = 0; i < 4; i++) {
			glTexGeni(kGenCoords[i], GL_TEXTURE_GEN_MODE, state->gen_mode[i]);
			glTexGenfv(kGenCoords[i], GL_OBJECT_PLANE, state->object_plane[i]);
			glTexGenfv(kGenCoords[i], GL_EYE_PLANE, state->eye_plane[i]);
		}
		glActiveTexture((GLenum) unit);
	}
	if (identity) {
		glPopMatrix();
		glMatrixMode((GLenum) matrix_mode);
	}
	if (mask & GL_TRANSFORM_BIT)
		glMatrixMode((GLenum) state->matrix_mode);

	if (mask & GL_FOG_BIT) {
		glFogi(GL_FOG_MODE, state->fog_mode);
		glFogf(GL_FOG_DENSITY, state->fog_density);
		glFogf(GL_FOG_START, state->fog_start);
		glFogf(GL_FOG_END, state->fog_end);
		glFogfv(GL_FOG_COLOR, state->fog_color);
	}
	if (mask & GL_DEPTH_BUFFER_BIT) {
		glDepthFunc((GLenum) state->depth_func);
		glClearDepth(state->depth_clear);
		glDepthMask(state->depth_mask);
	}
	if (mask & GL_ACCUM_BUFFER_BIT)
		glClearAccum(state->accum_clear[0], state->accum_clear[1], state->accum_clear[2], state->accum_clear[3]);
	if (mask & GL_STENCIL_BUFFER_BIT) {
		glStencilFunc((GLenum) state->stencil_func, state->stencil_ref, (GLuint) state->stencil_value_mask);
		glStencilOp((GLenum) state->stencil_fail, (GLenum) state->stencil_zfail, (GLenum) state->stencil_zpass);
		glStencilMask((GLuint) state->stencil_write_mask);
		glClearStencil(state->stencil_clear);
	}
	if (mask & GL_VIEWPORT_BIT) {
		glViewport(state->viewport[0], state->viewport[1], state->viewport[2], state->viewport[3]);
		glDepthRange(state->depth_range[0], state->depth_range[1]);
	}
	if (mask & GL_SCISSOR_BIT)
		glScissor(state->scissor[0], state->scissor[1], state->scissor[2], state->scissor[3]);
	if (mask & GL_COLOR_BUFFER_BIT) {
		glAlphaFunc((GLenum) state->alpha_func, state->alpha_ref);
		glBlendFuncSeparate((GLenum) state->blend_src_rgb, (GLenum) state->blend_dst_rgb,
		                    (GLenum) state->blend_src_alpha, (GLenum) state->blend_dst_alpha);
		glBlendEquationSeparate((GLenum) state->blend_eq_rgb, (GLenum) state->blend_eq_alpha);
		glBlendColor(state->blend_color[0], state->blend_color[1], state->blend_color[2], state->blend_color[3]);
		glClearColor(state->clear_color[0], state->clear_color[1], state->clear_color[2], state->clear_color[3]);
		glLogicOp((GLenum) state->logic_op);
		glColorMask(state->color_mask[0], state->color_mask[1], state->color_mask[2], state->color_mask[3]);
	}
	if (mask & GL_HINT_BIT)
		for (size_t i = 0; i < HINT_COUNT; i++)
			glHint(kHints[i], (GLenum) state->hints[i]);
	if (mask & GL_LIST_BIT)
		glListBase((GLuint) state->list_base);
	if (mask & GL_MULTISAMPLE_BIT)
		glSampleCoverage(state->sample_coverage_value, state->sample_coverage_invert);
}

// A context of a core profile has no fixed-function state to copy.
static int isLegacyContext(CGLContextObj context) {
	GLint profile = 0;
	CGLPixelFormatObj format = CGLGetPixelFormat(context);
	if (format && CGLDescribePixelFormat(format, 0, kCGLPFAOpenGLProfile, &profile) == kCGLNoError)
		return profile == 0 || profile == kCGLOGLPVersion_Legacy;
	return 1;
}

static void discardErrors(void) {
	for (int i = 0; i < 16 && glGetError() != GL_NO_ERROR; i++)
		;
}

CGLError CGLCopyContext(CGLContextObj src, CGLContextObj dst, GLbitfield mask) {
	if (src == NULL || dst == NULL)
		return kCGLBadContext;
	if (src == dst || mask == 0)
		return kCGLNoError;
	// the same state of the group, set through the same calls, only in contexts of the legacy profile
	if (!isLegacyContext(src) || !isLegacyContext(dst))
		return kCGLNoError;

	struct state *state = calloc(1, sizeof(*state));
	if (state == NULL)
		return kCGLBadAlloc;
	state->mask = mask;
	CGLContextObj previous = CGLGetCurrentContext();
	CGLError error = CGLSetCurrentContext(src);
	if (error == kCGLNoError) {
		readState(state);
		discardErrors();
		error = CGLSetCurrentContext(dst);
	}
	if (error == kCGLNoError) {
		writeState(state);
		discardErrors();
	}
	CGLSetCurrentContext(previous);
	free(state);
	return error;
}
