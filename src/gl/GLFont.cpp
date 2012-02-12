/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer

    Thanks:
    legolas558 <legolas558@users.sourceforge.net>
    
*/

#include "GLDefines.h"
#include "GLFont.h"
int bmfont_load(const char * filename, bm_font * font);
void bmfont_infoblock(FILE * f, bm_font * font, int size);
void bmfont_commonblock(FILE * f, bm_font * font, int size);
void bmfont_charsblock(FILE * f, bm_font * font, int size);

#define DWORD int
#define WORD  short
#define BYTE  char

void glFontCreate(const char * name, bm_font ** pfont)
{
  char filename[128];
  sprintf(filename, "data\\%s.fnt", name);
  (*pfont) = new bm_font;
  bmfont_load(filename, (*pfont));
}

#define GL_Y_FIX(f) (1 - (f))
GLfloat glFontDrawChar(bm_font * font, GLvector2f pos, char chr)
{
  BMchar ch = font->chars[chr];
  pos.x += font->chars[chr].x_ofs;  
  int ysize = font->chars[chr].y_ofs + font->chars[chr].h;
  pos.y += (GLfloat) font->base - ((GLfloat) font->chars[chr].y_ofs + (GLfloat) font->chars[chr].h);
  GLvector2f size((GLfloat) font->chars[chr].w, (GLfloat) font->chars[chr].h);

  GLvector2f tsize((GLfloat) font->chars[chr].w / (GLfloat) font->w, (GLfloat) font->chars[chr].h / (GLfloat) font->h);
  GLvector2f tpos = GLvector2f((GLfloat) font->chars[chr].x / (GLfloat) font->w, (GLfloat) font->chars[chr].y / (GLfloat) font->h);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, font->fontTex);
  glBegin(GL_QUADS);
    glTexCoord2f(tpos.x          , GL_Y_FIX(tpos.y));
    glVertex2f(	 pos.x           , pos.y + size.y);
    glTexCoord2f(tpos.x + tsize.x, GL_Y_FIX(tpos.y));
    glVertex2f(	 pos.x + size.x  , pos.y + size.y);
    glTexCoord2f(tpos.x + tsize.x, GL_Y_FIX(tpos.y + tsize.y));
    glVertex2f(	 pos.x + size.x  , pos.y);
    glTexCoord2f(tpos.x          , GL_Y_FIX(tpos.y + tsize.y));
    glVertex2f(	 pos.x           , pos.y);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_BLEND);
  return (GLfloat) font->chars[chr].x_advance;
}

void glFontPrint(bm_font * font, GLvector2f pos, const char * fmt, ...)
{
  char msg[256];
  va_list args;
  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  for(unsigned int i = 0; i < strlen(msg); i++) {
    pos.x += glFontDrawChar(font, pos, msg[i] - 32);
  }
  va_end(args);
}

void glFontDestroy(bm_font * font)
{
  delete font->chars;
  delete font;
}

int bmfont_load(const char * filename, bm_font * font)
{
  FILE * f = 0;
  fopen_s(&f, filename, "rb");
	char magicString[4];
	fread(magicString, 4, 1, f);
	if( strncmp(magicString, "BMF\003", 4) != 0 ) {
		fclose(f);
		return -1;
	}

	// Read each block
	char blockType;
	int blockSize;
	while( fread(&blockType, 1, 1, f) )
	{
		// Read the blockSize
		fread(&blockSize, 4, 1, f);
		switch( blockType ) {
		case 1: // info
			bmfont_infoblock(f, font, blockSize);
			break;
		case 2: // common
			bmfont_commonblock(f, font, blockSize);
			break;
		case 4: // chars
			bmfont_charsblock(f, font, blockSize);
			break;
    case 3: // pages
    case 5: // kerning pairs
      { // skip
        char *buffer = new char[blockSize];
	      fread(buffer, blockSize, 1, f);
	      delete[] buffer;
      } break;
    default:
			fclose(f);
			return -1;
		}
	}
	fclose(f);
	return 0;
}

void bmfont_infoblock(FILE * f, bm_font * font, int size)
{
#pragma pack(push)
#pragma pack(1)
struct infoBlock
{
    WORD fontSize;
    BYTE reserved:4;
    BYTE bold    :1;
    BYTE italic  :1;
    BYTE unicode :1;
    BYTE smooth  :1;
    BYTE charSet;
    WORD stretchH;
    BYTE aa;
    BYTE paddingUp;
    BYTE paddingRight;
    BYTE paddingDown;
    BYTE paddingLeft;
    BYTE spacingHoriz;
    BYTE spacingVert;
    BYTE outline;         // Added with version 2
    char fontName[1];
};
#pragma pack(pop)

	char *buffer = new char[size];
	fread(buffer, size, 1, f);
	infoBlock *blk = (infoBlock*)buffer;
  // outline
	delete[] buffer;
}

void bmfont_commonblock(FILE * f, bm_font * font, int size)
{
#pragma pack(push)
#pragma pack(1)
struct commonBlock
{
    WORD lineHeight;
    WORD base;
    WORD scaleW;
    WORD scaleH;
    WORD pages;
    BYTE packed  :1;
    BYTE reserved:7;
	BYTE alphaChnl;
	BYTE redChnl;
	BYTE greenChnl;
	BYTE blueChnl;
}; 
#pragma pack(pop)

	char *buffer = new char[size];
	fread(buffer, size, 1, f);
	commonBlock *blk = (commonBlock*)buffer;
  font->base =  blk->base;
  font->line_h = blk->lineHeight; 
  font->w = blk->scaleW;
  font->h = blk->scaleH;
  font->pages = blk->pages;
	delete[] buffer;
}

void bmfont_charsblock(FILE * f, bm_font * font, int size)
{
#pragma pack(push)
#pragma pack(1)
struct charsBlock
{
    struct charInfo
    {
        DWORD id;
        WORD  x;
        WORD  y;
        WORD  width;
        WORD  height;
        short xoffset;
        short yoffset;
        short xadvance;
        BYTE  page;
        BYTE  chnl;
    } chars[1];
};
#pragma pack(pop)
	char *buffer = new char[size];
	fread(buffer, size, 1, f);
	charsBlock *blk = (charsBlock*)buffer;
  font->chars = (BMchar *) malloc(256 * sizeof(BMchar));
	for( int n = 0; n < 256 && int(n*sizeof(charsBlock::charInfo)) < size; n++ )
	{
    font->chars[n].ch = n > 32 ? n - 32 : '?';
    font->chars[n].x = blk->chars[n].x;
		font->chars[n].y = blk->chars[n].y;
    font->chars[n].w = blk->chars[n].width;
		font->chars[n].h = blk->chars[n].height;
    font->chars[n].x_ofs = blk->chars[n].xoffset;
		font->chars[n].y_ofs = blk->chars[n].yoffset;
    font->chars[n].x_advance = blk->chars[n].xadvance;
	}
	delete[] buffer;
}
