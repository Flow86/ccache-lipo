ccache-lipo: src/main.cpp
	g++ src/main.cpp -o ccache-lipo

build: ccache-lipo

all: build

install: build
	mkdir -vp $(DESTDIR)/usr/lib/ccache-lipo && \
	mkdir -vp $(DESTDIR)/usr/bin/
	cp -v ccache-lipo $(DESTDIR)/usr/bin/ccache-lipo

clean:
	rm -f ccache-lipo

.PHONY: all install build clean
