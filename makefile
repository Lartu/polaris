# +------------------+
# | Polaris Makefile |
# +------------------+

#PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
PREFIX := /usr/local
endif

all:
	mkdir -p build
	c++ src/polaris.cpp -std=c++98 -pedantic -o build/polaris

clean:
	rm -rf build

install:
	install ./build/polaris $(DESTDIR)$(PREFIX)/bin/
	install -m 775 ./build/polaris $(DESTDIR)$(PREFIX)/bin/