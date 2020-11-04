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
from tqdm import tqdm
from pprint import pformat
try:
    from math import comb   # breaks in Python < 3.8
except ImportError:
    from math import factorial as fac

    def comb(n, k):
        return fac(n) / (fac(k) * fac(n - k))

import os
import glob
import json
from itertools import chain, combinations
import logging
from collections import Counter
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
        outdir="userdocs/"
        ):
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
        log.info("creating output directory "+outdir)
        os.mkdir(outdir)
    userdoc_re = re.compile(r'BeginUserDocs:?\s*(?P<tags>([\w -]+(,\s*)?)*)\n+(?P<doc>(.|\n)*)EndUserDocs')
    tagdict = dict()    # map tags to lists of documents
    nfiles_total = 0
    with tqdm(unit="files", total=len(filenames)) as progress:
        for filename in filenames:
            progress.set_postfix(file=os.path.basename(filename)[:15], refresh=False)
            progress.update(1)
            log.warning("extracting user documentation from %s...", filename)
            nfiles_total += 1
            match = None
            with open(os.path.join(basedir, filename), 'r', encoding='utf8') as infile:
                match = userdoc_re.search(infile.read())
            if not match:
                log.warning("No user documentation found in " + filename)
                continue
            outname = os.path.basename(os.path.splitext(filename)[0]) + replace_ext
            tags = [t.strip() for t in match.group('tags').split(',')]
            for tag in tags:
                tagdict.setdefault(tag, list()).append(outname)
            doc = match.group('doc')
            try:
                doc = rewrite_short_description(doc, filename)
            except ValueError as e:
                log.warning("Documentation added unfixed: %s", e)
            try:
                doc = rewrite_see_also(doc, filename, tags)
            except ValueError as e:
                log.warning("Failed to rebuild 'See also' section: %s", e)
            write_rst_files(doc, tags, outdir, outname)

    log.info("%4d tags found:\n%s", len(tagdict), pformat(list(tagdict.keys())))
    nfiles = len(set.union(*[set(x) for x in tagdict.values()]))
    log.info("%4d files in input", nfiles_total)
    log.info("%4d files with documentation", nfiles)
    return tagdict


def rewrite_short_description(doc, filename, short_description="Short description"):
    '''
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
    '''

    titles = getTitles(doc)
    if not titles:
        raise ValueError("No sections found in '%s'!" % filename)
    name = os.path.splitext(os.path.basename(filename))[0]
    for title, nexttitle in zip(titles, titles[1:]+[None]):
        if title.group(1) != short_description:
            continue
        secstart = title.end()
        secend = len(doc) + 1  # last section ends at end of document
        if nexttitle:
            secend = nexttitle.start()
        sdesc = doc[secstart:secend].strip().replace('\n', ' ')
        fixed_title = "%s – %s" % (name, sdesc)
        return (
            doc[:title.start()] +
            fixed_title + "\n" + "=" * len(fixed_title) + "\n\n" +
            doc[secend:]
            )
    raise ValueError("No section '%s' found in %s!" % (short_description, filename))


def rewrite_see_also(doc, filename, tags, see_also="See also"):
    '''
    Replace the content of a section named `see_also` in the document `doc`
    with links to indices of all its tags.

    The original content of the section -if not empty- will discarded and
    logged as a warning.

    Parameters
    ----------

    doc : str
      restructured text with all sections

    filename : str, path
      name that is inserted in the replaced title (and used for useful error
      messages).

    tags : iterable (list or dict)
      all tags the given document is linked to. These are used to construct the
      links in the `see_also` section.

    see_also : str
      title of the section that is to be rewritten to the document title

    Returns
    -------

    str
        original parameter doc with see_also section replaced
    '''

    titles = getTitles(doc)
    if not titles:
        raise ValueError("No sections found in '%s'!" % filename)

    def rightcase(text):
        '''
        Make text title-case except for acronyms, where an acronym is
        identified simply by being all upper-case.

        This function operates on the whole string, so a text with mixed
        acronyms and non-acronyms will not be recognized and everything will be
        title-cased, including the embedded acronyms.

        Parameters
        ----------

        text : str
          text that needs to be changed to the right casing.

        Returns
        -------

        str
          original text with poentially different characters being
          upper-/lower-case.
        '''
        if text != text.upper():
            return text.title()  # title-case any tag that is not an acronym
        return text   # return acronyms unmodified

    for title, nexttitle in zip(titles, titles[1:]+[None]):
        if title.group(1) != see_also:
            continue
        secstart = title.end()
        secend = len(doc) + 1  # last section ends at end of document
        if nexttitle:
            secend = nexttitle.start()
        original = doc[secstart:secend].strip().replace('\n', ' ')
        if original:
            log.warning("dropping manual 'see also' list in %s user docs: '%s'", filename, original)
        return (
            doc[:secstart] +
            "\n" + ", ".join([":doc:`{taglabel} <index_{tag}>`".format(tag=tag, taglabel=rightcase(tag))
                             for tag in tags]) + "\n\n" +
            doc[secend:]
            )
    raise ValueError("No section '%s' found in %s!" % (see_also, filename))


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
    tags : dict
       flat dictionary of tag to entry

    basetags : iterable
       iterable of a subset of tags.keys(), if no basetags are given the
       original tags list is returned unmodified.

    Returns
    -------

    dict
       A hierarchical dictionary of (dict or set) with items in the
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


def rst_index(hierarchy, current_tags=[], underlines='=-~', top=True):
    """
    Create an index page from a given hierarchical dict of documents.

    The given `hierarchy` is pretty-printed and returned as a string.

    Parameters
    ----------
    hierarchy : dict
       dictionary or dict-of-dict returned from `make_hierarchy()`

    current_tags : list
       applied filters for the current index (parameters given to
       `make_hierarchy()`. Defaults to `[]`, which doesn't display any filters.

    underlines : iterable
       list of characters to use for underlining deeper levels of the generated
       index.

    top : bool
       optional argument keeping track of recursive calls. Calls from within
       `rst_index` itself will always call with `top=False`.

    Returns
    -------

    str
       formatted pretty index.
    """
    def mktitle(t, ul, link=None):
        text = t
        if t != t.upper():
            text = t.title()  # title-case any tag that is not an acronym
        title = ':doc:`{text} <{filename}>`'.format(
            text=text,
            filename=link or "index_"+t)
        text = title+'\n'+ul*len(title)+'\n'
        return text

    def mkitem(t):
        return "* :doc:`%s`" % os.path.splitext(t)[0]

    output = list()
    if top:
        page_title = "Model Directory"
        if len(hierarchy.keys()) == 1:
            page_title += ": " + ", ".join(current_tags)
        output.append(page_title)
        output.append(underlines[0]*len(page_title)+"\n")
        if len(hierarchy.keys()) != 1:
            underlines = underlines[1:]

    for tags, items in sorted(hierarchy.items()):
        if isinstance(tags, str):
            title = tags
        else:
            title = " & ".join(tags)
        if title and not len(hierarchy) == 1:   # not print title if already selected by current_tags
            output.append(mktitle(title, underlines[0]))
        if isinstance(items, dict):
            output.append(rst_index(items, current_tags, underlines[1:], top=False))
        else:
            for item in sorted(items):
                output.append(mkitem(item))
            output.append("")
    return "\n".join(output)


def reverse_dict(tags):
    """
    Return the reversed dict-of-list

    Given a dictionary `keys:values`, this function creates the inverted
    dictionary `value:[key, key2, ...]` with one entry per value of the given
    dict. Since many keys can have the same value, the reversed dict must have
    list-of-keys as values.

    Parameters
    ----------

    tags : dict
       Values must be hashable to be used as keys for the result.

    Returns
    -------

    dict
       Mapping the original values to lists of original keys.
    """
    revdict = dict()
    for tag, items in tags.items():
        for item in items:
            revdict.setdefault(item, list()).append(tag)
    return revdict


def CreateTagIndices(tags, outdir="userdocs/"):
    """
    This function generates all combinations of tags and creates an index page
    for each combination using `rst_index`.

    Parameters
    ----------

    tags : dict
       dictionary of tags

    outdir : str, path
       path to the intended output directory (handed to `rst_index`.

    Returns
    -------

    list
        list of names of generated files.
    """
    taglist = list(tags.keys())
    maxtaglen = max([len(t) for t in tags])
    for tag, count in sorted([(tag, len(lst)) for tag, lst in tags.items()], key=lambda x: x[1]):
        log.info("    %%%ds tag in %%d files" % maxtaglen, tag, count)
    if "" in taglist:
        taglist.remove('')
    indexfiles = list()
    depth = min(4, len(taglist))    # how many levels of indices to create at most
    nindices = sum([comb(len(taglist), L) for L in range(depth-1)])
    log.info("indices down to level %d → %d possible keyword combinations", depth, nindices)
    for current_tags in tqdm(chain(*[combinations(taglist, L) for L in range(depth-1)]), unit="idx",
                             desc="keyword indices", total=nindices):
        current_tags = sorted(current_tags)
        indexname = "index%s.rst" % "".join(["_"+x for x in current_tags])

        hier = make_hierarchy(tags.copy(), *current_tags)
        if not any(hier.values()):
            log.debug("index %s is empyt!", str(current_tags))
            continue
        nfiles = len(set.union(*chain([set(subtag) for subtag in hier.values()])))
        if nfiles < 2:
            log.warning("skipping index for %s, as it links only to %d distinct file(s)", set(hier.keys()), nfiles)
            continue
        log.debug("generating index for %s...", str(current_tags))
        indextext = rst_index(hier, current_tags)
        with open(os.path.join(outdir, indexname), 'w') as outfile:
            outfile.write(indextext)
        indexfiles.append(indexname)
    log.info("%4d non-empty index files generated", len(indexfiles))
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


def getTitles(text):
    '''
    extract all sections from the given RST file

    Parameters
    ----------

    text : str
      restructuredtext user documentation

    Returns
    -------

    list
      elements are the section title re.match objects
    '''
    titlechar = r'\+'
    title_re = re.compile(r'^(?P<title>.+)\n(?P<underline>'+titlechar+r'+)$', re.MULTILINE)
    titles = []
    # extract all titles
    for match in title_re.finditer(text):
        log.debug("MATCH from %s to %s: %s", match.start(), match.end(), pformat(match.groupdict()))
        if len(match.group('title')) != len(match.group('underline')):
            log.warning("Length of section title '%s' (%d) does not match length of underline (%d)",
                        match.group('title'),
                        len(match.group('title')),
                        len(match.group('underline')))
        titles.append(match)
    return titles


def getSections(text, titles=None):
    '''
    Extract sections between titles

    Parameters
    ----------

    text : str
      Full documentation text

    titles : list (optional)
      Iterable with the ordered title re.match objects.  If not given, titles
      will be generated with a call to `getTitles()`.


    Returns
    -------

    list
      tuples of each title re.match object and the text of the following section.
    '''
    if titles is None:
        titles = getTitles(text)
    sections = list()
    for title, following in zip(titles, titles[1:]+[None]):
        secstart = title.end()
        secend = None   # None = end of string
        if following:
            secend = following.start()
        if title.group('title') in sections:
            log.warning('Duplicate title in user documentation of %s', filename)
        sections.append((title.group('title'), text[secstart:secend].strip()))
    return sections


def ExtractUserDocs(listoffiles, basedir='..', outdir='userdocs/'):
    """
    Extract and build all user documentation and build tag indices.

    Writes extracted information to JSON files in outdir. In particular the
    list of seen tags mapped to files they appear in, and the indices generated
    from all combinations of tags.

    Parameters are the same as for `UserDocExtractor` and are handed to it
    unmodified.

    Returns
    -------

    None
    """
    data = JsonWriter(outdir)
    # Gather all information and write RSTs
    tags = UserDocExtractor(listoffiles, basedir=basedir, outdir=outdir)
    data.write(tags, "tags")

    indexfiles = CreateTagIndices(tags, outdir=outdir)
    data.write(indexfiles, "indexfiles")


if __name__ == '__main__':
    ExtractUserDocs(
        relative_glob("models/*.h", "nestkernel/*.h", basedir='..'),
        outdir="userdocs/"
    )
