ifdef DEBUG
	CFLAGS = -g
else
	CFLAGS = -O3
endif

OBJS = pixelflut

all: $(OBJS)

clean:
	rm -f $(OBJS)
