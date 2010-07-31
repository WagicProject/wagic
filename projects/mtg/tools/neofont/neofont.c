// font generator for WFBFont by linshier

#include <stdlib.h>
#include <iconv.h>
#include <ft2build.h>
#include FT_FREETYPE_H 

FT_Library m_library;
iconv_t ic;

// gb2312 to unicode
unsigned short to_unicode(char * szStr) 
{ 
  static wchar_t target[3]; 
#if 0
  if((MultiByteToWideChar(CP_ACP, 0, szStr, 2, target, 3)) != 0) 
    return (unsigned short)target[0]; 
#else
  char *inp = szStr;
  char *outp = (char *)target;
  unsigned char *op = (char *)target;
  int inlen = strlen(szStr), outlen = 6;
  if (iconv(ic,(char **)&inp, &inlen, (char **)&outp, &outlen) != -1) 
    return (unsigned short)target[0]; 
  else
    return 0;
#endif
} 

int render_cn12(const char * szFilename, const char * szOutputname) 
{ 
  unsigned char zslot[12 * 12 / 2]; 
  FT_Face face; 
  FILE * fp; 
  unsigned char i, j, y; 
  int ctr = 0;

  if (FT_New_Face(m_library, szFilename, 0, &face) != 0) 
    return -1; 
 
  if (FT_Set_Char_Size(face, 0, 12 * 64, 72, 0) != 0 || (fp = fopen(szOutputname,"w")) == NULL) { 
    FT_Done_Face(face); 
    return -1; 
  } 
#if 0
  // GBK encoding
  for (i = 0x81; i < 0xFF; i ++) { 
    for (j = 0x40; j < 0xFF; j ++) { 
#else
  // GB2312 encoding
  for (i = 0xA1; i < 0xF8; i ++) { 
    for (j = 0xA1; j < 0xFF; j ++) { 
#endif
      unsigned char s[3] = {(unsigned char)i, (unsigned char)j, 0}; 
      unsigned short code = to_unicode((char *)s);
      memset(zslot, 0, 12 * 12 / 2); 
      if (code && FT_Load_Char(face, code, FT_LOAD_RENDER) == 0) {
        if (face->glyph->bitmap.buffer != NULL && face->glyph->bitmap_top > 0) { 
          unsigned char left = face->glyph->bitmap_left > 0 ? face->glyph->bitmap_left : 0;
          for (y = 0; y < face->glyph->bitmap.rows && y < 12 && y - face->glyph->bitmap_top < 2; y++) {
                  unsigned char h = 12 - 2 - face->glyph->bitmap_top + y;
            unsigned char l = left + face->glyph->bitmap.pitch > 12 ? 
                12 - left : face->glyph->bitmap.pitch;
            unsigned char u = 0;
            if (face->glyph->bitmap_top < 12 - 2) {
              for (u = 0; u < l; u++)
                zslot[(h * 12 + left + u) / 2]
                |= (face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + u]
                & 0xF0) >> 4 * (1 - (left + u) & 1);
            }
            else {
              for (u = 0; u < l; u++)
                zslot[(y * 12 + left + u) / 2]
                |= (face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + u]
                & 0xF0) >> 4 * (1 - (left + u) & 1);
            }
          }
        }
      }
      if (i == 0xA1 && j == 0xA1) memset(zslot, 0, 12 * 12 / 2); 
      fwrite(zslot, sizeof(unsigned char), 12 * 12 / 2, fp);
    } 
  } 
  fclose(fp);
  FT_Done_Face(face); 
 
  return 0; 
} 

int render_cn16(const char * szFilename, const char * szOutputname) 
{ 
  unsigned char zslot[16 * 16 / 2]; 
  FT_Face face; 
  FILE * fp; 
  unsigned char i, j, y; 
  int ctr = 0;

  if (FT_New_Face(m_library, szFilename, 0, &face) != 0) 
    return -1; 
 
  if (FT_Set_Char_Size(face, 0, 16 * 64, 72, 0) != 0 || (fp = fopen(szOutputname,"w")) == NULL) { 
    FT_Done_Face(face); 
    return -1; 
  } 
#if 0
  // GBK encoding
  for (i = 0x81; i < 0xFF; i ++) { 
    for (j = 0x40; j < 0xFF; j ++) { 
#else
  // GB2312 encoding
  for (i = 0xA1; i < 0xF8; i ++) { 
    for (j = 0xA1; j < 0xFF; j ++) { 
#endif
      unsigned char s[3] = {(unsigned char)i, (unsigned char)j, 0}; 
      unsigned short code = to_unicode((char *)s);
      memset(zslot, 0, 16 * 16 / 2); 
      if (code && FT_Load_Char(face, code, FT_LOAD_RENDER) == 0) {
        if (face->glyph->bitmap.buffer != NULL && face->glyph->bitmap_top > 0) { 
          unsigned char left = face->glyph->bitmap_left > 0 ? face->glyph->bitmap_left : 0;
          for (y = 0; y < face->glyph->bitmap.rows && y < 16 && y - face->glyph->bitmap_top < 2; y++) {
                  unsigned char h = 16 - 2 - face->glyph->bitmap_top + y;
            unsigned char l = left + face->glyph->bitmap.pitch > 16 ? 
                16 - left : face->glyph->bitmap.pitch;
            unsigned char u = 0;
            if (face->glyph->bitmap_top < 16 - 2) {
              for (u = 0; u < l; u++)
                zslot[(h * 16 + left + u) / 2]
                |= (face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + u]
                & 0xF0) >> 4 * (1 - (left + u) & 1);
            }
            else {
              for (u = 0; u < l; u++)
                zslot[(y * 16 + left + u) / 2]
                |= (face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + u]
                & 0xF0) >> 4 * (1 - (left + u) & 1);
            }
          }
        }
      }
      if (i == 0xA1 && j == 0xA1) memset(zslot, 0, 16 * 16 / 2); 
      fwrite(zslot, sizeof(unsigned char), 16 * 16 / 2, fp);
    } 
  } 
  fclose(fp);
#if 0
  if ((fp = fopen("fallback.gbk.3","w")) == NULL) { 
    FT_Done_Face(face); 
    return -1; 
  } 
  for (i = 0x81; i < 0xA1; i ++) { 
    for (j = 0x40; j < 0xFF; j ++) { 
      unsigned char s[3] = {(unsigned char)i, (unsigned char)j, 0}; 
      unsigned short code = to_unicode((char *)s);
      memset(zslot, 0, 16 * 16 / 4); 
      if (code && FT_Load_Char(face, code, FT_LOAD_RENDER) == 0) {
        if (face->glyph->bitmap.buffer != NULL && face->glyph->bitmap_top > 0) { 
          unsigned char left = face->glyph->bitmap_left > 0 ? face->glyph->bitmap_left : 0;
          for (y = 0; y < face->glyph->bitmap.rows && y < 16 && y - face->glyph->bitmap_top < 2; y++) {
                  unsigned char h = 16 - 2 - face->glyph->bitmap_top + y;
            unsigned char l = left + face->glyph->bitmap.pitch > 16 ? 
                16 - left : face->glyph->bitmap.pitch;
            unsigned char u = 0;
            if (face->glyph->bitmap_top < 16 - 2) {
              for (u = 0; u < l; u++)
                zslot[(h * 16 + left + u) / 4]
                |= (face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + u]
                & 0xC0) >> 2 * (3 - (left + u) & 3);
            }
            else {
              for (u = 0; u < l; u++)
                zslot[(y * 16 + left + u) / 4]
                |= (face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + u]
                & 0xC0) >> 2 * (3 - (left + u) & 3);
            }
          }
        }
      }
      fwrite(zslot, sizeof(unsigned char), 16 * 16 / 4, fp);
    } 
  } 
  fclose(fp);
#endif
  FT_Done_Face(face); 
 
  return 0; 
} 

int render_en12(const char * szFilename, const char * szOutputname) 
{ 
  unsigned char zslot[12 * 6 / 2]; 
  FT_Face face; 
  FILE * fp; 
  unsigned char i, j, y; 
  int ctr = 0;

  if (FT_New_Face(m_library, szFilename, 0, &face) != 0) 
    return -1; 
 
  if (FT_Set_Char_Size(face, 0, 10 * 64, 72, 0) != 0 || (fp = fopen(szOutputname,"w")) == NULL) { 
    FT_Done_Face(face); 
    return -1; 
  } 
  for (i = 0x00; i < 0x7F; i ++) { 
    unsigned short code = i;
    memset(zslot, 0, 12 * 6 / 2); 
    if (code > 0x20 && FT_Load_Char(face, code, FT_LOAD_RENDER) == 0) {
      if (face->glyph->bitmap.buffer != NULL && face->glyph->bitmap_top > 0) { 
        for (y = 0; y < face->glyph->bitmap.rows && y < 12 && y - face->glyph->bitmap_top < 2; y++) {
          unsigned char h = 12 - 2 - face->glyph->bitmap_top + y;
          unsigned char u = 0;
          unsigned char left = face->glyph->bitmap.pitch > 6 ? 1 : 0;
          for (u = 0; u < left + face->glyph->bitmap.pitch; u++)
            zslot[(h * 6 + u) / 2]
              |= (face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + left + u]
                  & 0xF0) >> 4 * (1 - u & 1);
        }
      }
    }
    fwrite(zslot, sizeof(unsigned char), 12 * 6 / 2, fp);
  } 

  fclose(fp);
  FT_Done_Face(face); 
 
  return 0; 
} 

int render_en16(const char * szFilename, const char * szOutputname) 
{ 
  unsigned char zslot[16 * 8 / 2]; 
  FT_Face face; 
  FILE * fp; 
  unsigned char i, j, y; 
  int ctr = 0;

  if (FT_New_Face(m_library, szFilename, 0, &face) != 0) 
    return -1; 
 
  if (FT_Set_Char_Size(face, 0, 12 * 64, 72, 0) != 0 || (fp = fopen(szOutputname,"w")) == NULL) { 
    FT_Done_Face(face); 
    return -1; 
  } 
  for (i = 0x00; i < 0x7F; i ++) { 
    unsigned short code = i;
    memset(zslot, 0, 16 * 8 / 2); 
    if (code > 0x20 && FT_Load_Char(face, code, FT_LOAD_RENDER) == 0) {
      if (face->glyph->bitmap.buffer != NULL && face->glyph->bitmap_top > 0) { 
        for (y = 0; y < face->glyph->bitmap.rows && y < 16 && y - face->glyph->bitmap_top < 2; y++) {
          unsigned char h = 16 - 2 - face->glyph->bitmap_top + y;
          unsigned char u = 0;
          unsigned char left = face->glyph->bitmap.pitch > 8 ? 1 : 0;
          for (u = 0; u < left + face->glyph->bitmap.pitch; u++)
            zslot[(h * 8 + u) / 2]
              |= (face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + left + u]
                  & 0xF0) >> 4 * (1 - u & 1);
        }
      }
    }
    fwrite(zslot, sizeof(unsigned char), 16 * 8 / 2, fp);
  } 

  fclose(fp);
  FT_Done_Face(face); 
 
  return 0; 
} 

int main()
{
  if (FT_Init_FreeType(&m_library) != 0) {
    printf("Error initializing freetype library!\n"); 
    return 1;
  }
  ic = iconv_open("unicode", "gb2312");
  if (ic == 0) printf("Error initializing iconv!\n");

  render_cn12("fallback.ttf", "simon.gbk"); 
  render_en12("fallback.ttf", "simon.asc"); 
  render_cn16("fallback.ttf", "f3.gbk"); 
  render_en16("fallback.ttf", "f3.asc"); 
  render_cn16("gkai00mp.ttf", "magic.gbk"); 
  render_en16("gkai00mp.ttf", "magic.asc"); 
  render_cn12("gkai00mp.ttf", "smallface.gbk"); 
  render_en12("gkai00mp.ttf", "smallface.asc"); 

  iconv_close(ic);
  FT_Done_FreeType(m_library);
  printf("done\n");
  return 0;
}
