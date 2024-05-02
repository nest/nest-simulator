# -*- coding: utf-8 -*-
#
# extract_userdocs_cpp.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

import glob
import json
import logging
import os
import re
from collections import Counter
from itertools import chain, combinations
from math import comb
from pprint import pformat

from tqdm import tqdm

logging.basicConfig(level=logging.INFO)
log = logging.getLogger(__name__)


def UserDocExtractor(filenames, basedir="../..", replace_ext=".rst", outdir="models/"):
    """
    Extract all user documentation from given files.

    This method searches for "BeginUserDocs" and "EndUserDocs" keywords and
    extracts all text inbetween as user-level documentation. The keyword
    "BeginUserDocs" may optionally be followed by a colon ":" and a comma
    separated list of tags till the end of the line. Note that this allows tags
    to contain spaces, i.e. you do not need to introduce underscores or hyphens
    for multi-word tags.

    Example
    -------

    /* BeginUserDocs: example, user documentation generator

    [...]

    EndUserDocs */

    This will extract "[...]" as documentation for the file and tag it with
    'example' and 'user documentation generator'.

    The extracted documentation is written to a file in `basedir` named after
    the sourcefile with ".rst" replacing the original extension.

    Parameters
    ----------

    filenames : iterable
       Any iterable with input file names (relative to `basedir`).

    basedir : str, path
       Directory to which input `filenames` are relative.

    replace_ext : str
       Replacement for the extension of the original filename when writing to `outdir`.

    outdir : str, path
       Directory where output files are created.

    Returns
    -------

    dict
       mapping tags to lists of documentation filenames (relative to `outdir`).
    """
    if not os.path.exists(outdir):
        log.info("creating output directory " + outdir)
        os.mkdir(outdir)
    userdoc_re = re.compile(r"BeginUserDocs:?\s*(?P<tags>([\w -]+(,\s*)?)*)\n+(?P<doc>(.|\n)*)EndUserDocs")
    tagdict = dict()  # map tags to lists of documents
    nfiles_total = 0
    outnames = []
    with tqdm(unit="files", total=len(filenames)) as progress:
        for filename in filenames:
            progress.set_postfix(file=os.path.basename(filename)[:15], refresh=False)
            progress.update(1)
            log.info("extracting user documentation from %s...", filename)
            nfiles_total += 1
            match = None
            with open(os.path.join(basedir, filename), "r", encoding="utf8") as infile:
                match = userdoc_re.search(infile.read())
            if not match:
                log.info("No user documentation found in " + filename)
                continue
            outname = os.path.basename(os.path.splitext(filename)[0]) + replace_ext
            tags = [t.strip() for t in match.group("tags").split(",")]
            for tag in tags:
                tagdict.setdefault(tag, list()).append(outname)
            doc = match.group("doc")
            try:
                doc = rewrite_short_description(doc, filename)
            except ValueError as e:
                log.warning("Documentation added unfixed: %s", e)
            write_rst_files(doc, tags, outdir, outname)

            outnames.append(outname)

    with open(os.path.join(outdir, "toc_tree.json"), "w") as tocfile:
        json.dump(outnames, tocfile)

    log.info("%4d tags found:\n%s", len(tagdict), pformat(list(tagdict.keys())))
    nfiles = len(set.union(*[set(x) for x in tagdict.values()]))
    log.info("%4d files in input", nfiles_total)
    log.info("%4d files with documentation", nfiles)

    return


def rewrite_short_description(doc, filename, short_description="Short description"):
    """
    Modify a given text by replacing the first section named as given in
    `short_description` by the filename and content of that section.
    Parameters
    ----------
    doc : str
      restructured text with all sections
    filename : str, path
      name that is inserted in the replaced title (and used for useful error
      messages).
    short_description : str
      title of the section that is to be rewritten to the document title
    Returns
    -------
    str
        original parameter doc with short_description section replaced
    """

    titles = getTitles(doc)
    if not titles:
        raise ValueError("No sections found in '%s'!" % filename)
    name = os.path.splitext(os.path.basename(filename))[0]
    for title, nexttitle in zip(titles, titles[1:] + [None]):
        if title.group(1) != short_description:
            continue
        secstart = title.end()
        secend = len(doc) + 1  # last section ends at end of document
        if nexttitle:
            secend = nexttitle.start()
        sdesc = doc[secstart:secend].strip().replace("\n", " ")
        fixed_title = "%s – %s" % (name, sdesc)
        return doc[: title.start()] + fixed_title + "\n" + "=" * len(fixed_title) + "\n\n" + doc[secend:]
    raise ValueError("No section '%s' found in %s!" % (short_description, filename))


def getTitles(text):
    """
    extract all sections from the given RST file

    Parameters
    ----------

    text : str
      restructuredtext user documentation

    Returns
    -------

    list
      elements are the section title re.match objects
    """
    titlechar = r"\+"
    title_re = re.compile(r"^(?P<title>.+)\n(?P<underline>" + titlechar + r"+)$", re.MULTILINE)
    titles = []
    # extract all titles
    for match in title_re.finditer(text):
        log.debug("MATCH from %s to %s: %s", match.start(), match.end(), pformat(match.groupdict()))
        if len(match.group("title")) != len(match.group("underline")):
            log.warning(
                "Length of section title '%s' (%d) does not match length of underline (%d)",
                match.group("title"),
                len(match.group("title")),
                len(match.group("underline")),
            )
        titles.append(match)
    return titles


def write_rst_files(doc, tags, outdir, outname):
    """
    Write raw rst to a file and generate a wrapper with index
    """
    with open(os.path.join(outdir, outname), "w") as outfile:
        outfile.write(doc)


def relative_glob(*pattern, basedir=os.curdir, **kwargs):
    tobase = os.path.relpath(basedir, os.curdir)
    # prefix all patterns with basedir and expand
    names = chain.from_iterable(glob.glob(os.path.join(tobase, pat), **kwargs) for pat in pattern)
    # remove prefix from all expanded names
    return [name[len(tobase) + 1 :] for name in names]


def config_inited_handler(app, config):
    models_rst_dir = "models/"
    basedir = "../.."
    replace_ext = ".rst"
    listoffiles = relative_glob("models/*.h", "nestkernel/*.h", basedir="../..")
    UserDocExtractor(listoffiles, basedir, replace_ext, models_rst_dir)


def setup(app):
    app.connect("config-inited", config_inited_handler)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
