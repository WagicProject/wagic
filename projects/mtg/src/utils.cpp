#include "../include/debug.h"
#include "../include/utils.h"


int lowercase(string sBuffer) {
  std::transform( sBuffer.begin(), sBuffer.end(), sBuffer.begin(),
		  ::tolower );
  return 1;
}





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



int readfile_to_ints(const char * filename, int * out_buffer){
  std::ifstream fichier(filename);
  std::string s;
  unsigned int count = 0;
  if(fichier){
    while(std::getline(fichier,s)){
      int value = atoi(s.c_str());
      if (value){
	out_buffer[count] = value;
	++count;
      }
    }

  }
  fichier.close();
  return count;

}

int fileExists(const char * filename){
 std::ifstream fichier(filename);
if(fichier){
  fichier.close();
  return 1;
}

char alternateFilename[512];
sprintf(alternateFilename, "Res/%s",filename);
 std::ifstream fichier2(alternateFilename);
if(fichier2){
  fichier2.close();
  return 1;
}
return 0;
}