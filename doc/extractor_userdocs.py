# -*- coding: utf-8 -*-
#
# extractor_userdocs.py
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

import re
from pprint import pformat
import os
import glob
import json
from itertools import chain, combinations
import logging
logging.basicConfig(level=logging.INFO)
log = logging.getLogger()


def relative_glob(*pattern, basedir=os.curdir, **kwargs):
    tobase = os.path.relpath(basedir, os.curdir)
    tohere = os.path.relpath(os.curdir, basedir)
    # prefix all patterns with basedir and expand
    names = chain(*[glob.glob(os.path.join(tobase, pat), **kwargs) for pat in pattern])
    # remove prefix from all expanded names
    return [name[len(tobase)+1:] for name in names]


def UserDocExtractor(
        filenames,
        basedir="..",
        replace_ext='.rst',
        outdir="from_cpp/"
        ):
    """
    Extract all user documentation from given files.

    This method searches for "BeginUserDocs" and "EndUserDocs" keywords and
    extracts all text inbetween as user-level documentation. The keyword
    "BeginUserDocs" may optionally be followed by a colon ":" and a comma
    separated list of tags till the end of the line.

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

    filenames
    : Any iterable with input file names (relative to `basedir`).
    basedir
    : Directory to which input `filenames` are relative.
    replace_ext
    : Replacement for the extension of the original filename when writing to `outdir`.
    outdir
    : Directory where output files are created.

    Returns
    -------
    Dictionary mapping tags to lists of documentation filenames (relative to
    `outdir`).
    """
    if not os.path.exists(outdir):
        log.info("creating output directory "+outdir)
        os.mkdir(outdir)
    userdoc_re = re.compile(r'BeginUserDocs:?\s*(?P<tags>(\w+(,\s*)?)*)\n+(?P<doc>(.|\n)*)EndUserDocs')
    tagdict = dict()    # map tags to lists of documents
    nfiles_total = 0
    for filename in filenames:
        log.info("extracting user documentation from %s...", filename)
        nfiles_total += 1
        match = None
        with open(os.path.join(basedir, filename)) as infile:
            match = userdoc_re.search(infile.read())
        if not match:
            log.warning("No user documentation found in " + filename)
            continue
        outname = os.path.basename(os.path.splitext(filename)[0]) + replace_ext
        tags = [t.strip() for t in match.group('tags').split(',')]
        for tag in tags:
            tagdict.setdefault(tag, list()).append(outname)
        write_rst_files(match.group('doc'), tags, outdir, outname)

    log.info("%4d tags found", len(tagdict))
    log.info("     "+pformat(list(tagdict.keys())))
    nfiles = len(set.union(*[set(x) for x in tagdict.values()]))
    log.info("%4d files in input", nfiles_total)
    log.info("%4d files with documentation", nfiles)
    return tagdict


def write_rst_files(doc, tags, outdir, outname):
    """
    Write raw rst to a file and generate a wrapper with index
    """
    with open(os.path.join(outdir, outname), "w") as outfile:
        outfile.write(doc)


def make_hierarchy(tags, *basetags):
    """
    This method adds a single level of hierachy to the given dictionary.

    First a list of items with given basetags is created (intersection). Then
    this list is subdivided into sections by creating intersections with all
    remaining tags.

    Parameters
    ----------
    tags
    : flat dictionary of tag to entry

    basetags
    : iterable of a subset of tags.keys(), if no basetags are given the
      original tags list is returned unmodified.

    Returns a hierarchical dictionary of (dict or set) with items in the
    intersection of basetag.
    """
    if not basetags:
        return tags

    # items having all given basetags
    baseitems = set.intersection(*[set(items) for tag, items in tags.items() if tag in basetags])
    tree = dict()
    subtags = [t for t in tags.keys() if t not in basetags]
    for subtag in subtags:
        docs = set(tags[subtag]).intersection(baseitems)
        if docs:
            tree[subtag] = docs
    remaining = None
    if tree.values():
        remaining = baseitems.difference(set.union(*tree.values()))
    if remaining:
        tree[''] = remaining
    return {basetags: tree}


def rst_index(hierarchy, underlines='=-~'):
    """
    Create an index page from a given hierarchical dict of documents.

    The given `hierarchy` is pretty-printed and returned as a string.

    Parameters
    ----------
    hierarchy
    : any dict or dict-of-dict returned from `make_hierarchy()`
    underlines
    : list of characters to use for underlining deeper levels of the generated
      index.

    Returns
    -------
    String with pretty index.
    """
    def mktitle(t, ul):
        return t+'\n'+ul*len(t)+'\n'

    def mkitem(t):
        return "* :doc:`%s`" % os.path.splitext(t)[0]

    output = list()
    for tags, items in sorted(hierarchy.items()):
        if isinstance(tags, str):
            title = tags
        else:
            title = " & ".join(tags)
        if title:
            if title != title.upper():
                title = title.title()  # title-case any tag that is not an acronym
            output.append(mktitle(title, underlines[0]))
        if isinstance(items, dict):
            output.append(rst_index(items, underlines[1:]))
        else:
            for item in items:
                output.append(mkitem(item))
            output.append("")
    return "\n".join(output)


def reverse_dict(tags):
    """
    return the reversed dict-of-list
    """
    revdict = dict()
    for tag, items in tags.items():
        for item in items:
            revdict.setdefault(item, list()).append(tag)
    return revdict


def CreateTagIndices(tags, outdir="from_cpp/"):
    taglist = list(tags.keys())
    if "" in taglist:
        taglist.remove('')
    indexfiles = list()
    for current_tags in chain(*[combinations(taglist, L) for L in range(len(taglist)-1)]):
        current_tags = sorted(current_tags)
        indexname = "index%s.rst" % "".join(["_"+x for x in current_tags])

        hier = make_hierarchy(tags.copy(), *current_tags)
        if not any(hier.values()):
            log.debug("index %s is empyt!", str(current_tags))
            continue
        log.debug("generating index for %s...", str(current_tags))
        indextext = rst_index(hier)
        with open(os.path.join(outdir, indexname), 'w') as outfile:
            outfile.write(indextext)
        indexfiles.append(indexname)
    log.info("%4d index files generated", len(indexfiles))
    return indexfiles


class JsonWriter(object):
    """
    Helper class to have a unified data output interface.
    """
    def __init__(self, outdir):
        self.outdir = outdir
        log.info("writing JSON files to %s", self.outdir)

    def write(self, obj, name):
        """
        Store the given object with the given name.
        """
        outname = os.path.join(self.outdir, name + ".json")
        with open(outname, 'w') as outfile:
            json.dump(obj, outfile)
            log.info("data saved as " + outname)


def ExtractUserDocs(listoffiles, basedir='..', outdir='from_cpp'):
    """
    Extract and build all user documentation and build tag indices.
    """
    data = JsonWriter(outdir)
    # Gather all information and write RSTs
    tags = UserDocExtractor(listoffiles, basedir=basedir, outdir=outdir)
    data.write(tags, "tags")

    indexfiles = CreateTagIndices(tags, outdir=outdir)
    data.write(indexfiles, "indexfiles")


if __name__ == '__main__':
    ExtractUserDocs(relative_glob("models/*.h", "nestkernel/*.h", basedir='..'), outdir="from_cpp/")
