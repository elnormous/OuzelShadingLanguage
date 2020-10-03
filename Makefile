.PHONY: all
all: osl test

.PHONY: osl
osl:
	$(MAKE) -C src debug=$(debug)

.PHONY: test
test:
	$(MAKE) -C test debug=$(debug)

.PHONY: clean
clean:
	$(MAKE) -C src clean
	$(MAKE) -C test clean