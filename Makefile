CC = clang

ifdef DEBUG
	CFLAGS = -g
else
	CFLAGS = -O3
endif

LDLIBS_png := `pkg-config --libs libpng`
CFLAGS_png := `pkg-config --cflags libpng`

LDLIBS += $(LDLIBS_png)
CFLAGS += $(CFLAGS_png)

OBJS = pixelflut.o pf_png.o
EXES = pixelflut

all: $(EXES)

pixelflut: $(OBJS)

clean:
	rm -f $(OBJS) $(EXES)
