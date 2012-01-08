/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "GLSprite.h"
#include "png.h"

GLSprite::GLSprite()
{
  mFilename = "";
  mScale = 1.0f;

  mpData = 0;
  mAngle = 0.0f;
  mPos.x = 0.0f;
  mPos.y = 0.0f;

  mInitialized = false;
}

GLSprite::GLSprite(const char * filename, int width, int height)
{
  mFilename = filename;
  mWidth = width;
  mHeight = height;
  mScale = 1.0f;

  mpData = 0;
  mPos.x = 0.0f;
  mPos.y = 0.0f;
  mAngle = 0.0f;
  mRotation.x = 0.0f;
  mRotation.y = 0.0f;

  mInitialized = load();
  if(mInitialized) {
    mInitialized = prepare();
  }
}

GLSprite::~GLSprite()
{
  if(mpData) { 
    delete mpData;
  }
  glDeleteTextures(1, &mpTextures[0]);
  mpData = 0;
}

void GLSprite::colorMask()
{
}

void GLSprite::draw()
{
  if(!mInitialized) return;
  GLfloat width = mWidth * mScale;
  GLfloat height = mHeight * mScale;

  glPushMatrix();
  glTranslatef(mRotation.x, mRotation.y, 0.0f);
  glRotatef(mAngle, 0.0f, 0.0f, 1.0f);
  glTranslatef(-mRotation.x, -mRotation.y, 0.0f);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  colorMask();
  glBindTexture(GL_TEXTURE_2D, mpTextures[0]);
  glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); 
    glVertex3f(mPos.x - (width / 2), mPos.y - (height / 2),  0.0f);
		
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(mPos.x + (width / 2), mPos.y - (height / 2),  0.0f);

    glTexCoord2f(1.0f, 1.0f); 
    glVertex3f(mPos.x + (width / 2), mPos.y + (height / 2),  0.0f);
		
    glTexCoord2f(0.0f, 1.0f); 
    glVertex3f(mPos.x - (width / 2), mPos.y + (height / 2),  0.0f);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glColor3f(1.0f, 1.0f, 1.0f);
  glDisable(GL_BLEND);
  
  glPopMatrix();
}

bool GLSprite::prepare()
{
  bool result = true;

  if(!mpData || (mWidth == 0)) { 
    result = false;
  }

  if(result) {
    glGenTextures(1, mpTextures);
    GL_CHECK_ERROR(result);

    if(result) {
      glBindTexture(GL_TEXTURE_2D, mpTextures[0]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mSizeX, mSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, mpData);
      GL_CHECK_ERROR(result);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    delete mpData;
    mpData = 0;
  }

  return result;
}

bool GLSprite::load()
{
  png_byte header[8];
  png_structp png_ptr;
  png_infop info_ptr;
  int color_type, interlace_type;
  int bit_depth;

  FILE * fp = NULL;
  fp = fopen(mFilename, "rb");
  if(!fp) {
    return false;
  }

  fread(header, 1, 8, fp);
  if(png_sig_cmp(header, 0, 8)) {
    fclose(fp);
    return false;
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png_ptr) {
    fclose(fp);
    return false;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    fclose(fp);
    return false;
  }

  if(setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return false;
  }
  
  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  mWidth = mSizeX = png_get_image_width(png_ptr, info_ptr);
  mHeight = mSizeY = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  switch(color_type) {
  case PNG_COLOR_TYPE_RGBA:
    break;
  case PNG_COLOR_TYPE_RGB:
  default:
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return false;
  }
  png_read_update_info(png_ptr, info_ptr);

  if(setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return false;
  }

  unsigned int row_count = (unsigned int) mHeight;
  unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
  mpData = new unsigned char[row_bytes * row_count];

  png_bytepp row_pointers = new png_bytep[row_count];
  for(int y = 0; y < row_count; y++)
    row_pointers[row_count - y - 1] = mpData + (y * row_bytes);
  png_read_image(png_ptr, row_pointers);
  delete row_pointers;

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(fp);

  return true;
}

GLAnimatedSprite::GLAnimatedSprite(const char * filename, GLvector2f pos, GLfloat width, GLfloat height)
{
  mSprite = new GLSprite(filename);
  assert(mSprite->isInitialized());
  mWidth = width;
  mHeight = height;
  mPos = pos;
}

GLAnimatedSprite::~GLAnimatedSprite()
{
  delete mSprite;
}

bool GLAnimatedSprite::draw(int index)
{
  GLfloat width = mWidth;
  GLfloat height = mHeight;
  int colcount = mSprite->w() / mWidth;
  int rowcount = mSprite->h() / mHeight;
  if(index >= colcount * rowcount) {
    return false;
  }
  int xindex = index % colcount;
  int yindex = index / colcount;

  glPushMatrix();
  glTranslatef(mRotation.x, mRotation.y, 0.0f);
  glRotatef(mAngle, 0.0f, 0.0f, 1.0f);
  glTranslatef(-mRotation.x, -mRotation.y, 0.0f);
  glPopMatrix();

  GLvector2f tsize(mWidth / mSprite->w(), mHeight / mSprite->h());
  GLvector2f tindex(xindex * tsize.x, yindex * tsize.y);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, mSprite->texture());
  glBegin(GL_QUADS);
		glTexCoord2f(tindex.x, tindex.y); 
    glVertex3f(mPos.x - (width / 2), mPos.y - (height / 2),  0.0f);
		
    glTexCoord2f(tindex.x + tsize.x, tindex.y);
    glVertex3f(mPos.x + (width / 2), mPos.y - (height / 2),  0.0f);

    glTexCoord2f(tindex.x + tsize.x, tindex.y + tsize.y); 
    glVertex3f(mPos.x + (width / 2), mPos.y + (height / 2),  0.0f);
		
    glTexCoord2f(tindex.x, tindex.y + tsize.y); 
    glVertex3f(mPos.x - (width / 2), mPos.y + (height / 2),  0.0f);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glColor3f(1.0f, 1.0f, 1.0f);
  glDisable(GL_BLEND);

  return true;
}