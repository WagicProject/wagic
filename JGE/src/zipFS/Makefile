MAIN=zfs

CC=g++

CFLAGS= -O2 -Wall -DNDEBUG -DUNIX

INCLUDES=

LFLAGS= -lz

SRCS =					\
	zfs.cpp				\
	zfsystem.cpp		\
	ziphdr.cpp			\
	zstream.cpp			\

OBJS = $(SRCS:.cpp=.o)

$(MAIN):  $(OBJS)
	$(CC) -o $(MAIN) $(LFLAGS) $(OBJS)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o $(MAIN)
