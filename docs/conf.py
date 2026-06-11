# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = "3ptWL-mod"
copyright = "2026, 3ptWL-mod contributors"
author = "3ptWL-mod contributors"

# The full version, including alpha/beta/rc tags
release = "1.0.0"


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = []

# Add any paths that contain templates here, relative to this directory.
templates_path = []

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of built-in themes.
#
#html_theme = 'alabaster'
#html_theme = 'sphinx_book_theme'
#html_theme = 'pydata_sphinx_theme'
#html_theme = 'press'
#html_theme = 'python_docs_theme'

html_theme = "sphinx_rtd_theme"
html_title = "3ptWL-mod documentation"
html_show_sourcelink = True
html_context = {
    "display_github": True,
    "github_user": "sadirs",
    "github_repo": "3ptWL-mod",
    "github_version": "main",
    "conf_py_path": "/docs/",
}
#html_theme = 'cloud'

man_pages = [
    (
        "index",
        "wlcf",
        "3ptWL-mod command-line reference",
        [author],
        1,
    )
]

latex_documents = [
    (
        "index",
        "3ptWL-mod.tex",
        "3ptWL-mod Documentation",
        author,
        "manual",
    )
]
