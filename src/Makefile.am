AUTOMAKE_OPTIONS=foreign
CFLAGS = -g -Wall
CPPFLAGS = -g -pthread -Wall -Wno-unused-function
CXXFLAGS = -g -Wall -Wno-unused-function
DEFS = -D_CENTERSVR_BUILD #-D_FCM_CLIENT_BUILD
source=
depfile=
depmode=
AM_CPPFLAGS = -I/usr/local/include -I../include
AM_LDFLAGS = -L/usr/local/lib -L/usr/lib/x86_64-linux-gnu -g -pthread -lcurl #-liconv
bin_PROGRAMS = nvrserver
nvrserver_LDADD =
nvrserver_SOURCES = \
	main.cpp \
	jmutex.cpp \
	markup.cpp \
	web_client.cpp \
	server_manager.cpp

