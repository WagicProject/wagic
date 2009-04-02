#include "../include/config.h"
#include "../include/utils.h"





int filesize(const char * filename){
  int file_size = 0;
#if defined (WIN32) || defined (LINUX)
  FILE * file = fopen(filename, "rb");
  if (file != NULL)
    {
      fseek(file, 0, SEEK_END);
      file_size = ftell(file);
      fclose(file);
    }

#else
  int file = sceIoOpen(filename,PSP_O_RDONLY, 0777);
  if (file > 0){
    file_size = sceIoLseek(file, 0, PSP_SEEK_END);
    sceIoClose(file);
  }

#endif
  return file_size;
}


int fileExists(const char * filename){
 std::ifstream fichier(filename);
if(fichier){
  fichier.close();
  return 1;
}

char alternateFilename[512];
sprintf(alternateFilename, RESPATH"/%s",filename);
 std::ifstream fichier2(alternateFilename);
if(fichier2){
  fichier2.close();
  return 1;
}
return 0;
}


#ifdef LINUX

#include <execinfo.h>
void dumpStack()
{
  void* buffer[50];
  int s = backtrace(buffer, 50);
  char** tab = backtrace_symbols(buffer, s);
  for (int i = 1; i < s; ++i) printf("%s\n", tab[i]);
  printf("\n");
  free(tab);
}
#endif
