import re
from pprint import pprint
import os

def UserDocExtractor(
        filenames,
        basedir="..",
        replace_ext='.rst',
        outdir = "from_cpp/"
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
    userdoc_re = re.compile(r'BeginUserDocs:?\s*(?P<tags>(\w+(,\s*)?)*)\n+(?P<doc>(.|\n)*)EndUserDocs')
    tagdict = dict()    # map tags to lists of documents
    for filename in filenames:
        match = None
        with open(os.path.join(basedir, filename)) as infile:
            match = userdoc_re.search(infile.read())
        if not match:
            print("WARNING: No user documentation found in " + filename)
            continue
        if not os.path.exists(outdir):
            print("INFO: creating output directory "+outdir)
            os.mkdir(outdir)
        outname = os.path.basename(filename) + replace_ext
        with open(os.path.join(outdir, outname), "w") as outfile:
            outfile.write(match.group('doc'))
            print("INFO: extracted user documentation from " + filename)
        tags = [t.strip() for t in match.group('tags').split(',')]
        for tag in tags:
            tagdict.setdefault(tag, list()).append(outname)
    return tagdict

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
    if not basetags: return tags

    # items having all given basetags
    baseitems = set.intersection(*[set(items) for tag,items in tags.items() if tag in basetags])
    tree = dict()
    subtags = [t for t in tags.keys() if not t in basetags]
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

def rst_index(hierarchy, underlines = '=-~'):
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
    output = list()
    for tags, items in sorted(hierarchy.items()):
        if isinstance(tags, str):
            title = tags
        else:
            title = " & ".join(tags)
        if title:
            if title != title.upper():
                title = title.title()  # title-case any tag that is not an acronym
            output.append(title)
            output.append(underlines[0]*len(title))
            output.append("")
        if isinstance(items, dict):
            output.append(rst_index(items, underlines[1:]))
        else:
            for item in items:
                output.append("* %s" % item)
            output.append("")
    return "\n".join(output)


from itertools import chain, combinations

def main():
    models_with_documentation = (
        "models/multimeter.h",
        "models/spike_detector.h",
        "models/weight_recorder.h",
        "nestkernel/recording_backend_ascii.h",
        "nestkernel/recording_backend_memory.h",
        "nestkernel/recording_backend_screen.h",
        "nestkernel/recording_backend_sionlib.h",
    )

    outdir = "from_cpp"
    tags = UserDocExtractor(models_with_documentation, outdir=outdir)

    pprint(tags)
    print("%4d tags in" % len(tags))
    print("%4d files" % len(set.union(*[set(x) for x in tags.values()])))

    taglist = list(tags.keys())
    taglist.remove('')
    indexcount = 0
    for current_tags in chain(*[combinations(taglist, L) for L in range(len(taglist)-1)]):
        current_tags = sorted(current_tags)
        indexname = "index_%s.rst" % ("_".join(current_tags))

        hier = make_hierarchy(tags.copy(), *current_tags)
        if not any(hier.values()):
            #print("index %s is empyt!" % str(current_tags))
            continue
        print("generating index for %s..." % (current_tags,))
        indextext = rst_index(hier)
        with open(os.path.join(outdir, indexname), 'w') as outfile:
            outfile.write(indextext)
        indexcount += 1
    print("%4d index files generated" % indexcount)

if __name__ == '__main__':
    main()
