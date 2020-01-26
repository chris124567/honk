CFLAGS := -std=c99
CFLAGS += -DSERVER
CFLAGS += -D_XOPEN_SOURCE=600
#warnings
CFLAGS += -Wall -pedantic -Wextra -Wshadow -Wnull-dereference -Wduplicated-branches -Wundef -Wstrict-prototypes -Wno-unused-parameter -Wstrict-aliasing  -Wstack-protector -Wpadded
CFLAGS += -DDEBUG
#speed
CFLAGS += -Ofast -fuse-linker-plugin -flto -fno-fat-lto-objects -g0
CFLAGS += -fmerge-all-constants -ffunction-sections -fdata-sections -fomit-frame-pointer -Wl,--gc-sections -s -Wl,--build-id=none -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-gnu-unique
#slow build - questionable optimizations
CFLAGS += -fgraphite-identity -floop-interchange -ftree-loop-distribution -floop-strip-mine -floop-block -ftree-vectorize -fipa-pta -fivopts -ftree-loop-im -fgcse-sm -fgcse-las -fgcse-after-reload -fweb -frename-registers -ftree-loop-linear -ftree-vectorize -fno-math-errno -ffinite-math-only -fno-signed-zeros -ffast-math
#security
CFLAGS += -D_FORTIFY_SOURCE=2 -Wformat=2 -Wstrict-overflow=5
CFLAGS += -Wa,--noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -Wl,-z,defs --param ssp-buffer-size=4 -fstack-clash-protection -mmitigate-rop -fstrict-overflow
CFLAGS += -fstack-protector-all
#linker
CFLAGS += -Wl,--fuse-ld=ld.gold -Wl,--no-eh-frame-hdr -Wl,--disable-new-dtags -Wl,--hash-style=sysv -Wl,--no-demangle -Wl,--strip-all -Wl,--discard-all -Wl,--discard-locals -Wl,--no-omagic -Wl,-z,nodelete -Wl,-z,nodlopen -Wl,-z,nodump -Wl,-z,noexecstack -Wl,-z,noseparate-code  -Wl,--check-sections -fno-ident
CFLAGS += -lpthread

CFLAGS2 := -std=c99
CFLAGS2 += -D_XOPEN_SOURCE=600
#warnings
CFLAGS2 += -Wall -pedantic -Wextra -Wshadow -Wnull-dereference -Wduplicated-branches -Wundef -Wstrict-prototypes -Wno-unused-parameter -Wstrict-aliasing  -Wstack-protector -Wpadded
#CFLAGS2 += -DDEBUG
#speed
CFLAGS2 += -Ofast -fuse-linker-plugin -flto -fno-fat-lto-objects -g0
CFLAGS2 += -fmerge-all-constants -ffunction-sections -fdata-sections -fomit-frame-pointer -Wl,--gc-sections -s -Wl,--build-id=none -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-gnu-unique
#slow build - questionable optimizations
CFLAGS2 += -fgraphite-identity -floop-interchange -ftree-loop-distribution -floop-strip-mine -floop-block -ftree-vectorize -fipa-pta -fivopts -ftree-loop-im -fgcse-sm -fgcse-las -fgcse-after-reload -fweb -frename-registers -ftree-loop-linear -ftree-vectorize -fno-math-errno -ffinite-math-only -fno-signed-zeros -ffast-math
#security
CFLAGS2 += -D_FORTIFY_SOURCE=2 -Wformat=2 -Wstrict-overflow=5
CFLAGS2 += -Wa,--noexecstack -Wl,-z,noexecheap -Wl,-z,relro -Wl,-z,now -Wl,-z,defs --param ssp-buffer-size=4 -fstack-clash-protection -mmitigate-rop -fstrict-overflow
#breaks debug builds by randomly changing the vlaue of a variable, possibly some bug in gcc?
CFLAGS2 += -fstack-protector-all
#linker
CFLAGS2 +=  -static -Wl,--fuse-ld=ld.gold -Wl,--no-eh-frame-hdr -Wl,--disable-new-dtags -Wl,--hash-style=sysv -Wl,--no-demangle -Wl,--as-needed -Wl,--strip-all -Wl,--discard-all -Wl,--discard-locals -Wl,--no-omagic -Wl,-z,nodelete -Wl,-z,nodlopen -Wl,-z,nodump -Wl,-z,noexecstack -Wl,-z,noseparate-code  -Wl,--check-sections -Wl,--no-dynamic-linker -fno-ident

CC := gcc
CC2 := diet gcc
SRC := server
SRC2 := honk
FILES := rand.c util.c serialization.c protocol.c
FILES2 := rand.c util.c serialization.c protocol.c scanner.c

all:
	${CC} ${CFLAGS} ${FILES} ${SRC}.c -o ${SRC}
	${CC2} ${CFLAGS2} ${FILES2} ${SRC2}.c -o ${SRC2}

	strip ${SRC} -R .note -R .note.gnu.gold-version -R .comment
	strip ${SRC2} -R .note -R .note.gnu.gold-version -R .comment

clean:
	rm ${SRC}
