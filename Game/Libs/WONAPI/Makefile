OBJS = \
	WONAuth/Auth.o \
	WONCommon/Common.o \
	WONCrypt/Crypt.o \
	WONDir/Dir.o \
	WONMisc/Misc.o \
	WONMisc/gqueryreporting.o\
	WONMisc/nonport.o\
	WONServer/Server.o \
	WONSocket/Socket.o \
	WONRouting/Routing.o \
	WONAPI.o \
	WONStatus.o

SRCS = \
	WONAuth/Auth.cpp \
	WONCommon/Common.cpp \
	WONCrypt/Crypt.cpp \
	WONDir/Dir.cpp \
	WONDB/DB.cpp \
	WONMisc/Misc.cpp \
	WONMisc/gqueryreporting.c \
	WONMisc/nonport.c \
	WONServer/Server.cpp \
	WONSocket/Socket.cpp \
	WONRouting/Routing.cpp \
	WONAPI.cpp \
	WONStatus.cpp

CFLAGS = -g -D_LINUX -I.
CPPFLAGS = $(CFLAGS)

all: libWON.a

clean:
	find . -name '*.o' -type f -exec rm {} \;
	rm libWON.a

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

libWON.a: $(OBJS)
	$(AR) $(ARFLAGS)s libWON.a $(OBJS)


