BUILDDIR = _build

RST2PDF_OPTS = -s style.json,friendly -b1

all: setup pdf

.PHONY: setup
setup:
	mkdir -p ${BUILDDIR}

.PHONY: setup pdf
pdf:
	/usr/bin/rst2pdf ${RST2PDF_OPTS} report.rst -o ${BUILDDIR}/report.pdf

.PHONY: clean
clean:
	rm -rf ${BUILDDIR}

