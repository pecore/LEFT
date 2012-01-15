/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "GLResources.h"
#include <windows.h>
#include "png.h"
#include <fstream>

#include "SoundPlayer.h"
#include "GLFont.h"

GLResources::GLResources()
{
  WIN32_FIND_DATA ffd;
  SoundPlayer::init();

  HANDLE hFind = FindFirstFile(".\\data\\*.*", &ffd);
  assert(INVALID_HANDLE_VALUE != hFind);

  do {
    if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      char filename[256];
      char name[256];
      char ext[4]; 
      strncpy(name, ffd.cFileName, strlen(ffd.cFileName) - 4);
      strncpy(ext, ffd.cFileName + strlen(ffd.cFileName) - 3, 3);
      sprintf(filename, "data\\%s", ffd.cFileName);
      if(get(filename)) continue;

      if(strcmp(ext, "png") == 0) {
        loadTexture(filename);
      }
      if(strcmp(ext, "wav") == 0) {
        Sound * sound = SoundPlayer::load(filename);
        if(sound) {
          ResourcePair * rp = new ResourcePair;
          strncpy(rp->path, filename, sizeof(rp->path)-1);
          rp->value = new GLSoundResource(sound);
          mResources.push_back(rp);
        }
      }
      if(strcmp(ext, "fnt") == 0) {
        bm_font * font = 0;
        glFontCreate(name, &font);
        if(font) {
          char tfilename[256];
          sprintf(tfilename, "data\\%s.png", name);
          font->fontTex = loadTexture(tfilename);
          ResourcePair * rp = new ResourcePair;
          strncpy(rp->path, filename, sizeof(rp->path)-1);
          rp->value = new GLFontResource(font);
          mResources.push_back(rp);
        }
      }
      if(strcmp(ext, "ply") == 0) {
        std::ifstream f(filename);
        if(f.good()) {
          Polygons p;
          f >> p;
          if(p.size() > 0) {
            ResourcePair * rp = new ResourcePair;
            strncpy(rp->path, filename, sizeof(rp->path)-1);
            rp->value = new GLPolygonResource(p);
            mResources.push_back(rp);
          }
        }
        f.close();
      }
    }
  } while(FindNextFile(hFind, &ffd) != 0);
}

GLResources::~GLResources()
{
  ResourcePair * rp = 0;
  foreach(ResourceList, rp, mResources) {
    switch(rp->value->type) {
    case GL_RESOURCE_TEXTURE: {
        GLTextureResource * t = (GLTextureResource *) rp->value;
        glDeleteTextures(1, &t->texture);
      } break;
    case GL_RESOURCE_FONT: {
        GLFontResource * f = (GLFontResource *) rp->value;
        glFontDestroy(f->font);
      } break;
    }
    delete rp->value;
    delete rp;
  }
  SoundPlayer::clear();
}

GLResource * GLResources::get(const char * path)
{
  if(mResources.size() <= 0) return 0;
  ResourcePair * rp = 0;
  foreach(ResourceList, rp, mResources) {
    if(strncmp(rp->path, path, sizeof(rp->path)-1) == 0) {
      return rp->value;
    }
  }
  return 0;
}

GLuint GLResources::loadTexture(const char * filename)
{
  GLuint texture;
  int width = 0;
  int height = 0;
  unsigned char * data = 0;
  unsigned int size = 0;

  assert(load(filename, &data, width, height, size));
  if(data) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    ResourcePair * rp = new ResourcePair;
    strncpy(rp->path, filename, sizeof(rp->path)-1);
    rp->value = new GLTextureResource(texture, (GLfloat)width, (GLfloat)height);
    mResources.push_back(rp);
    delete data;
  } else {
    data = data;
  }

  return texture;
}

bool GLResources::load(const char * filename, unsigned char ** data, int & width, int & height, unsigned int & size)
{
  png_byte header[8];
  png_structp png_ptr;
  png_infop info_ptr;
  int color_type;
  int bit_depth;

  FILE * fp = 0; fopen_s(&fp, filename, "rb");
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

  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
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

  unsigned int row_count = (unsigned int) height;
  unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
  *data = new unsigned char[row_bytes * row_count];
  size = row_bytes * row_count;

  png_bytepp row_pointers = new png_bytep[row_count];
  for(unsigned int y = 0; y < row_count; y++)
    row_pointers[row_count - y - 1] = *data + (y * row_bytes);
  png_read_image(png_ptr, row_pointers);
  delete row_pointers;

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(fp);

  return true;
}
