ifdef DEBUG
	CFLAGS = -g -fopenmp
else
	CFLAGS = -O3 -fopenmp
endif

LDFLAGS = -fopenmp

OBJS = pixelflut

all: $(OBJS)

clean:
	rm -f $(OBJS)
