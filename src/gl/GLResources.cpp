#include "GLResources.h"
#include <windows.h>
#include "png.h"

#include "SoundPlayer.h"

void GLResources::init()
{
  WIN32_FIND_DATA ffd;
  SoundPlayer::init();

  HANDLE hFind = FindFirstFile(".\\data\\*.*", &ffd);
  assert(INVALID_HANDLE_VALUE != hFind);

  do {
    if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      char ext[4]; strncpy(ext, ffd.cFileName + strlen(ffd.cFileName) - 3, 4); ext[3] = 0;
      char filename[256]; sprintf(filename, "data\\%s", ffd.cFileName);

      if(strcmp(ext, "png") == 0) {
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
          rp->value = new GLTextureResource(texture, width, height);;
          mResources.push_back(rp);
          delete data;
        }
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
      if(strcmp(ext, "glf") == 0) {
        GLFONT font;
        GLuint texture;
        glGenTextures(1, &texture);
        if(glFontCreate(&font, filename, texture)) {
          ResourcePair * rp = new ResourcePair;
          strncpy(rp->path, filename, sizeof(rp->path)-1);
          rp->value = new GLFontResource(font, texture);
          mResources.push_back(rp);
        } else {
          glDeleteTextures(1, &texture);
        }
      }
    }
  } while(FindNextFile(hFind, &ffd) != 0);
}

void GLResources::clear()
{
  ResourcePair * rp = 0;
  foreach(ResourceList, rp, mResources) {
    switch(rp->value->type) {
    case GL_RESOURCE_TEXTURE:
      glDeleteTextures(1, &(((GLTextureResource *)rp)->texture));
      break;
    case GL_RESOURCE_FONT:
      glFontDestroy(&(((GLFontResource *)rp)->font));
      glDeleteTextures(1, &(((GLFontResource *)rp)->texture));
      break;
    }
    delete rp->value;
    delete rp;
  }
  SoundPlayer::clear();
}

GLResource * GLResources::get(const char * path)
{
  ResourcePair * rp = 0;
  foreach(ResourceList, rp, mResources) {
    if(strncmp(rp->path, path, sizeof(rp->path)-1) == 0) {
      return rp->value;
    }
  }
  return 0;
}

bool GLResources::load(const char * filename, unsigned char ** data, int & width, int & height, unsigned int & size)
{
  png_byte header[8];
  png_structp png_ptr;
  png_infop info_ptr;
  int color_type, interlace_type;
  int bit_depth;

  FILE * fp = NULL;
  fp = fopen(filename, "rb");
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
  for(int y = 0; y < row_count; y++)
    row_pointers[row_count - y - 1] = *data + (y * row_bytes);
  png_read_image(png_ptr, row_pointers);
  delete row_pointers;

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(fp);

  return true;
}
