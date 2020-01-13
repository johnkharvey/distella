all:
	@echo "Targets are:" ; \
	echo ; \
	echo "	windows"; \
	echo "	linux"; \
	echo "	linux-static"; \
	echo "	osx-ub"; \
	echo "	osx-intel";

APP_VERSION="\"3.01a\""
APP_COMPILE="\"March 4, 2013\""

#CFLAGS=-O0 -w -DAPP_VERSION=$(APP_VERSION) -DAPP_COMPILE=$(APP_COMPILE)
CFLAGS=-O0 -g -w -DAPP_VERSION=$(APP_VERSION) -DAPP_COMPILE=$(APP_COMPILE)
SOURCE=source/distella.c

windows:
	$(CC) $(CFLAGS) $(SOURCE) -o DiStella.EXE

linux:
	$(CC) $(CFLAGS) $(SOURCE) -o distella

linux-static:
	$(CC) $(CFLAGS) -static $(SOURCE) -o distella

osx-ub:
	$(CC) $(CFLAGS) -arch i386 -arch x86_64 -arch ppc $(SOURCE) -o distella

osx-intel:
	$(CC) $(CFLAGS) -mmacosx=version-min=10.6 -arch i386 -arch x86_64 $(SOURCE) -o distella
