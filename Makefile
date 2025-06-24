PDFLaTeX := pdflatex
spec_dir := spec
spec_src := spec.tex
spec_pdf := $(spec_dir)/spec.pdf

$(spec_dir):
	mkdir $(spec_dir)

$(spec_pdf): $(spec_dir)
	$(PDFLaTeX) -output-directory=$(spec_dir) -jobname=spec $(spec_src)

all: $(spec_dir) $(spec_pdf)

clean:
	rm -rf $(spec_dir)

.PHONY: all clean
