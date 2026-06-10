Documentation notes
===================

Build the Sphinx HTML documentation from the repository root with:

    python -m pip install -r docs/requirements.txt
    make -C docs html

The generated HTML entry point is:

    docs/_build/html/index.html

Generated documentation under docs/_build is ignored by git and should be
rebuilt locally when needed.

Manual page
-----------

The manual page source is:

    docs/man/wlcf.1

View it from the repository root with:

    man ./docs/man/wlcf.1

If man2html is installed, generate an HTML copy with:

    cd docs/man
    ./makehtml.sh

General references
------------------

Sphinx documentation:

    https://www.sphinx-doc.org/en/master/tutorial/index.html
    https://docs.readthedocs.io/en/stable/intro/getting-started-with-sphinx.html
    https://sphinx-themes.org/

Examples from related communities:

    https://github.com/lsst
    https://github.com/LSSTDESC
