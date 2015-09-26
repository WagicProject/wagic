SET(SOURCE
  png.c 
  pngerror.c 
  pnggccrd.c 
  pngget.c 
  pngmem.c 
  pngpread.c 
  pngread.c 
  pngrio.c 
  pngrtran.c 
  pngrutil.c 
  pngset.c 
  pngtrans.c 
  pngvcrd.c 
  pngwio.c 
  pngwrite.c 
  pngwtran.c 
  pngwutil.c )
  
add_library(png ${SOURCE})

if(operating_system_psp)
    #  -DCHROME_PNG_WRITE_SUPPORT \
    #  -DPNG_USER_CONFIG \
    set_target_properties(unzip PROPERTIES COMPILE_FLAGS "-D__ANDROID__ -DUNICODE ")
endif() 
