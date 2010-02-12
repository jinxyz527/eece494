RST2PDF = /usr/bin/rst2pdf
RST2PDF_OPTS = 

PDF = results.pdf
RST = results.rst

$(PDF): $(RST)
	$(RST2PDF) $(RST2PDF_OPTS) $(RST) -o ../$(PDF)

clean:
	rm -r ../$(PDF)

