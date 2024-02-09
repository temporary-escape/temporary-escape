# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= python3 -msphinx
SOURCEDIR     = source
BUILDDIR      = build

.ONESHELL:

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile

pyenv:
	python3 -m venv ./venv
	source ./venv/bin/activate
	pip install -r requirements.txt

releases_py: pyenv
	source ./venv/bin/activate
	python3 ./releases.py

genapi_py: pyenv
	source ./venv/bin/activate
	python3 ./genapi.py

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile releases_py genapi_py
	source ./venv/bin/activate
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
	echo "temporaryescape.org" > "$(BUILDDIR)/html/CNAME"
