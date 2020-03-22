# Makefile for DiStella

#==================
# Global Variables
#==================
APP_VERSION="\"3.02-SNAPSHOT\""
APP_COMPILE="\"March 22, 2020\""

#CFLAGS=-O0 -w -DAPP_VERSION=$(APP_VERSION) -DAPP_COMPILE=$(APP_COMPILE)
CFLAGS=-O0 -g -w -DAPP_VERSION=$(APP_VERSION) -DAPP_COMPILE=$(APP_COMPILE)
SOURCE=source/distella.c

#====================
# Auto-detector code
#====================
OUTFILE	:=
ifeq ($(OS),Windows_NT)
	OUTFILE=DiStella.EXE
else
	OUTFILE=distella
endif

#===============
# Build Targets
#===============
.PHONY:	default all

default:	auto

all:
	@echo "Targets are:" ; \
	echo ; \
	echo "	windows"; \
	echo "	linux"; \
	echo "	linux-static"; \
	echo "	osx-ub"; \
	echo "	osx-intel";
	echo "  osx-intel-legacy";


auto:
	$(CC) $(CFLAGS) $(SOURCE) -o $(OUTFILE)

#================
# Direct targets
#================
windows:
	$(CC) $(CFLAGS) $(SOURCE) -o DiStella.EXE

linux:
	$(CC) $(CFLAGS) $(SOURCE) -o distella

linux-static:
	$(CC) $(CFLAGS) -static $(SOURCE) -o distella

osx-ub:
	$(CC) $(CFLAGS) -arch i386 -arch x86_64 -arch ppc $(SOURCE) -o distella

osx-intel:
	$(CC) $(CFLAGS) -arch x86_64 $(SOURCE) -o distella

osx-intel-legacy:
	$(CC) $(CFLAGS) -mmacosx=version-min=10.6 -arch i386 -arch x86_64 $(SOURCE) -o distellaa

