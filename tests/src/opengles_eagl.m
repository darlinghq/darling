/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

/*
 * OpenGLES.framework: EAGLContext and EAGLSharegroup, and the GL ES entry
 * points drawing into a framebuffer of the application's own. The contract is
 * the public EAGL and OpenGL ES 2.0/3.0 documentation. 64-bit only, as iOS's
 * OpenGL ES is.
 */

#import <Foundation/Foundation.h>
#import <QuartzCore/CALayer.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES3/gl.h>

#include <stdio.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL: line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

@interface TestLayer : CALayer <EAGLDrawable>
@property (nullable, copy) NSDictionary<NSString *, id> *drawableProperties;
@end
@implementation TestLayer
@synthesize drawableProperties;
@end

/* A framebuffer of size w x h with an RGBA8 renderbuffer. */
static void make_target(GLuint *fbo, GLuint *rbo, int w, int h)
{
	glGenFramebuffers(1, fbo);
	glGenRenderbuffers(1, rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, *rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
	glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *rbo);
}

int main(void)
{
	@autoreleasepool {
		/* Contexts. */
		CHECK([EAGLContext currentContext] == nil);
		CHECK([[EAGLContext alloc] initWithAPI:(EAGLRenderingAPI)0] == nil);
		CHECK([[EAGLContext alloc] initWithAPI:(EAGLRenderingAPI)4] == nil);

		EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
		if (context == nil)
			context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		CHECK(context != nil);
		if (context == nil) {
			printf("%d failure(s)\n", failures + 1);
			return 1;
		}
		CHECK([context API] >= kEAGLRenderingAPIOpenGLES2);
		CHECK([context sharegroup] != nil);
		CHECK([EAGLContext setCurrentContext:context]);
		CHECK([EAGLContext currentContext] == context);

		EAGLContext *sibling = [[EAGLContext alloc] initWithAPI:[context API] sharegroup:[context sharegroup]];
		CHECK(sibling != nil);
		CHECK([sibling sharegroup] == [context sharegroup]);
		context.debugLabel = @"label";
		CHECK([context.debugLabel isEqualToString:@"label"]);

		/* Drawing into the application's own framebuffer. */
		GLuint fbo, rbo;
		make_target(&fbo, &rbo, 8, 4);
		CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glClearColor(1, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_SCISSOR_TEST);
		glScissor(0, 0, 4, 4);
		glClearColor(0, 0, 1, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		unsigned char px[8 * 4 * 4];
		glReadPixels(0, 0, 8, 4, GL_RGBA, GL_UNSIGNED_BYTE, px);
		CHECK(glGetError() == GL_NO_ERROR);
		CHECK(px[0] == 0 && px[1] == 0 && px[2] == 255 && px[3] >= 127 && px[3] <= 128);
		CHECK(px[4 * 7] == 255 && px[4 * 7 + 1] == 0 && px[4 * 7 + 2] == 0 && px[4 * 7 + 3] == 255);
		/* The last row, too: the first read reached all 4 rows. */
		CHECK(px[4 * (3 * 8 + 7)] == 255);

		/* A shader program with a uniform. */
		const GLchar *vs = "attribute vec4 p; void main(){ gl_Position = p; }";
		const GLchar *fs = "precision mediump float; uniform float v; void main(){ gl_FragColor = vec4(0.0, v, 0.0, 1.0); }";
		GLuint vso = glCreateShader(GL_VERTEX_SHADER), fso = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(vso, 1, &vs, NULL);
		glShaderSource(fso, 1, &fs, NULL);
		glCompileShader(vso);
		glCompileShader(fso);
		GLuint prog = glCreateProgram();
		glAttachShader(prog, vso);
		glAttachShader(prog, fso);
		glBindAttribLocation(prog, 0, "p");
		glLinkProgram(prog);
		GLint linked = 0;
		glGetProgramiv(prog, GL_LINK_STATUS, &linked);
		CHECK(linked == GL_TRUE);
		glUseProgram(prog);
		glUniform1f(glGetUniformLocation(prog, "v"), 1.0f);
		static const GLfloat quad[] = { -1, -1, 1, -1, -1, 1, 1, 1 };
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, quad);
		glEnableVertexAttribArray(0);
		glViewport(0, 0, 8, 4);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glReadPixels(0, 0, 8, 4, GL_RGBA, GL_UNSIGNED_BYTE, px);
		CHECK(px[0] == 0 && px[1] == 255 && px[2] == 0 && px[3] == 255);

		/* Errors are the host's: an invalid enum. */
		glBindTexture(0x1234, 0);
		CHECK(glGetError() == GL_INVALID_ENUM);

		/* A layer as renderbuffer storage, and presenting into it. */
		TestLayer *layer = [TestLayer layer];
		layer.bounds = CGRectMake(0, 0, 16, 8);
		layer.contentsScale = 2;
		layer.drawableProperties = @{ kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8 };
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		CHECK([context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer]);
		GLint w = 0, h = 0;
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &w);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &h);
		CHECK(w == 32 && h == 16);
		glClearColor(0, 1, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		CHECK(layer.contents == nil);
		CHECK([context presentRenderbuffer:GL_RENDERBUFFER]);
		CHECK(layer.contents != nil);
		CHECK(CGImageGetWidth((__bridge CGImageRef)layer.contents) == 32);

		/* Invalid uses report NO. */
		CHECK(![context renderbufferStorage:GL_RENDERBUFFER fromDrawable:nil]);
		CHECK(![context renderbufferStorage:GL_FRAMEBUFFER fromDrawable:layer]);
		CHECK(![context presentRenderbuffer:GL_FRAMEBUFFER]);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		CHECK(![context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer]);

		/* Current contexts are per thread. */
		__block BOOL other = YES;
		dispatch_semaphore_t done = dispatch_semaphore_create(0);
		dispatch_async(dispatch_get_global_queue(0, 0), ^{
			other = [EAGLContext currentContext] != nil;
			dispatch_semaphore_signal(done);
		});
		dispatch_semaphore_wait(done, DISPATCH_TIME_FOREVER);
		CHECK(!other);

		CHECK([EAGLContext setCurrentContext:nil]);
		CHECK([EAGLContext currentContext] == nil);
	}
	if (failures != 0) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
