INCDIR = -I/usr/include/freetype2
LIBS = -lfreetype

CFLAGS += $(INCDIR)

all : neofont

neofont : neofont.o
	$(CC) -o neofont $(LIBS) $<

clean :
	@rm -f neofont *.o *.asc *.gbk

update : neofont fallback.ttf gkai00mp.ttf mono.ttf
	@./neofont && mv *.asc *.gbk ../../Res/graphics/

