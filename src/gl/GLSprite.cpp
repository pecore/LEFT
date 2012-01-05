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
  mpData = 0;
}

void GLSprite::colorMask()
{
}

void GLSprite::draw()
{
  if(!mInitialized) return;

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
    glVertex3f(mPos.x - (mWidth / 2), mPos.y - (mHeight / 2),  0.0f);
		
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(mPos.x + (mWidth / 2), mPos.y - (mHeight / 2),  0.0f);

    glTexCoord2f(1.0f, 1.0f); 
    glVertex3f(mPos.x + (mWidth / 2), mPos.y + (mHeight / 2),  0.0f);
		
    glTexCoord2f(0.0f, 1.0f); 
    glVertex3f(mPos.x - (mWidth / 2), mPos.y + (mHeight / 2),  0.0f);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glColor3f(1.0f, 1.0f, 1.0f);
  glDisable(GL_BLEND);
  
  glPopMatrix();
}

bool GLSprite::prepare()
{
  if(!mpData) 
    return false;

  if(mWidth == 0)
    return false;

  glGenTextures(1, mpTextures);
  glBindTexture(GL_TEXTURE_2D, mpTextures[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mSizeX, mSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, mpData);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  delete mpData;
  mpData = 0;

  return true;
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

#if 0
bool GLSprite::load()
{
  BITMAPINFO bminfo;
  HANDLE dibhandle;
  const HDC hDC = wglGetCurrentDC();  
  
  if(mpData) delete mpData;
  mpData = 0;

  bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);  
  bminfo.bmiHeader.biBitCount = 0;
  dibhandle = LoadImage(0, mFilename, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_CREATEDIBSECTION | LR_LOADFROMFILE);
  if(dibhandle) {
    GetDIBits(hDC, (HBITMAP)dibhandle, 0,0, NULL, &bminfo, DIB_RGB_COLORS);

    mSizeX = bminfo.bmiHeader.biWidth;
    mSizeY = bminfo.bmiHeader.biHeight;
    bminfo.bmiHeader.biBitCount = 24; 
    bminfo.bmiHeader.biCompression = BI_RGB;  

    const DWORD tmpsize = mSizeX * mSizeY * 3;
    unsigned char * tmp = new unsigned char[tmpsize];
    if(tmp) {
      if(GetDIBits(hDC, (HBITMAP)dibhandle, 0, mSizeY, tmp, &bminfo, DIB_RGB_COLORS)) {
        if((mWidth == -1) || (mHeight == -1)) {
          mWidth = mSizeX;
          mHeight = mSizeY;
        }
        const DWORD size = mSizeX * mSizeY * 4;
        mpData = new unsigned char[size];
        if(mpData) {
          for(unsigned int i = 0, j = 0; i < tmpsize && j < size; i+=3, j+=4) {
            mpData[j]     = tmp[i + 2];
            mpData[j + 1] = tmp[i + 1];
            mpData[j + 2] = tmp[i];
            if(mpData[j] == 0xFF && mpData[j + 1] == 0x00 && mpData[j + 2] == 0xFF) {
              mpData[j]     = 0xFF;
              mpData[j + 1] = 0x00;
              mpData[j + 2] = 0x00;
              mpData[j + 3] = 0x00;
            } else if(mpData[j] == 0x0F && mpData[j + 1] == 0x00 && mpData[j + 2] == 0x0F) {
              mpData[j]     = 0xFF;
              mpData[j + 1] = 0x00;
              mpData[j + 2] = 0x00;
              mpData[j + 3] = 0x00;
            } else {
              mpData[j + 3] = 0xFF;
            }
          }
        } else {
          DeleteObject(dibhandle);
          return false;
        }
      } else {
        DeleteObject(dibhandle);
        return false;
      }
      
      delete tmp;
    } else {
      DeleteObject(dibhandle);
      return false;
    }
  } else {
    return false;
  }

  DeleteObject(dibhandle);
  return true;
}   
#endif
