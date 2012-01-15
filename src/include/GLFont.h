/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _GL_FONT_H
#define _GL_FONT_H

#include "GLDefines.h"

typedef struct S_BMchar {
    char ch;
    GLint	 y_ofs;
		GLuint x,
           y,
           w,
           h;
		GLint	 x_ofs;
    
		GLuint x_advance;
} BMchar;
typedef struct S_bm_font {
	float  scale;
	GLuint base, 
         line_h, 
         w, 
         h, 
         pages;
	GLuint fontTex;
	GLuint base_list;
	BMchar * chars;
	float mmat[16];
} bm_font;

void glFontCreate(const char * name, bm_font ** pfont);
void glFontDestroy(bm_font * font);
void glFontPrint(bm_font * font, GLvector2f pos, const char * fmt, ...);
GLfloat glFontDrawChar(bm_font * font, GLvector2f pos, char chr);

#endif
