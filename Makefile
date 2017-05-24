COMPILER = gcc
FILESYSTEM_FILES = vcowfs.c

build: $(FILESYSTEM_FILES)
	$(COMPILER) $(FILESYSTEM_FILES) -o vcowfs `pkg-config fuse --cflags --libs`

clean:
	rm vcowfs
