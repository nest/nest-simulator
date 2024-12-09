# -*- coding: utf-8 -*-
#
# model_tag_setup.py
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
from pathlib import Path
from pprint import pformat

logging.basicConfig(level=logging.INFO)
log = logging.getLogger(__name__)

# The following function is used in two other functions, in two separate Sphinx events


def extract_model_text():
    """
    Function to extract user documentation from header files.

    This function searches for documentation blocks in header files located in
    two specified directories: "../../models" and "../../nestkernel". The documentation
    blocks are identified by markers "BeginUserDocs" and "EndUserDocs".

    Yields
    ------

    tuple: A tuple containing the match object and the file path for each file
           where documentation is found.

    Note
    ----
    The documentation block format is expected to be:

    BeginUserDocs: [tags]
    Documentation text
    EndUserDocs
    """
    model_paths = Path("../../models").glob("*.h")
    nestkernel_paths = Path("../../nestkernel").glob("*.h")
    file_paths = list(model_paths) + list(nestkernel_paths)

    userdoc_re = re.compile(
        r"""
    BeginUserDocs:\s*            # Match 'BeginUserDocs:' followed by any whitespace
    (?P<tags>(?:[\w -]+(?:,\s*)?)+) # Match tags (terms, spaces, commas, hyphens)
    \n\n                          # Match two newlines
    (?P<doc>.*?)                 # Capture the document text non-greedily
    (?=EndUserDocs)              # Positive lookahead for 'EndUserDocs'
    """,
        re.VERBOSE | re.DOTALL,
    )

    for file_path in file_paths:
        with open(file_path, "r", encoding="utf8") as file:
            match = userdoc_re.search(file.read())
        if not match:
            log.info("No user documentation found in " + str(file_path))
            continue
        yield match, file_path


# The following block of functions are called at Sphinx core event config-inited


def create_rst_files(app, config):
    """
    Generates reStructuredText (RST) files from header files containing user documentation.

    This function creates an output directory if it does not exist and processes header
    files located in predefined directories. It extracts user documentation blocks from
    the files, optionally modifies the documentation, and writes the resulting text to
    RST files.

    Parameters
    ----------
    app : Sphinx application object
        The Sphinx application instance, used to access the application's configuration.
    config : Sphinx config object
        The configuration object for the Sphinx application.

    """

    outdir = "models/"
    if not os.path.exists(outdir):
        log.info("creating output directory " + outdir)
        os.mkdir(outdir)
    outnames = []
    for match, file_path in extract_model_text():
        doc = match.group("doc")
        filename = file_path.name
        outname = filename.replace(".h", ".rst")
        try:
            doc = rewrite_short_description(doc, filename)
        except ValueError as e:
            log.warning("Documentation added unfixed: %s", e)
        write_rst_files(doc, outdir, outname)


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
    name = Path(filename).stem
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


def write_rst_files(doc, outdir, outname):
    """
    Write raw rst to a file and generate a wrapper with index
    """
    with open(os.path.join(outdir, outname), "w") as outfile:
        outfile.write(doc)


# The following block of functions are called at Sphinx core event
# env-before-read-docs


def get_model_tags(app, env, docname):
    """
    Prepares the environment dictionaries by loading models, tags, and their combinations
    from files, and writes this data to a JSON file for client-side use.

    This function ensures that two dictionaries (`model_dict` and `tag_dict`) are
    initialized in the environment if they don't already exist. It then populates `model_dict`
    with model names extracted from the specified directory and uses these models to populate
    `tag_dict` with tags.

    Parameters
    ----------
    app
        Sphinx application object
    env
        The build environment object of Sphinx, which stores shared data between the builders
    docname : str
        The name of the document being processed.

    Note
    ----

    Writes to `static/data/filter_model.json` which is used client-side.
    """

    # Initialize necessary dictionaries if not already present
    if not hasattr(env, "tag_dict"):
        env.tag_dict = {}

    if not hasattr(env, "model_dict"):
        env.model_dict = {}

    # Extract models and tags, and find tag-to-model relationships
    env.model_dict = prepare_model_dict()
    env.tag_dict = find_models_in_tag_combinations(env.model_dict)

    json_output = Path("static/data/filter_model.json")
    json_output.parent.mkdir(exist_ok=True, parents=True)
    # Write the JSON output directly to a file used for dynamically loading data client-side
    with open(json_output, "w+") as json_file:
        json.dump(env.tag_dict, json_file, indent=2)


def prepare_model_dict():
    """
    Extracts user documentation tags from header files and organizes them into a dictionary.

    This function iterates through the header files found by `extract_model_text()`, extracts
    the tags from each file, and creates a dictionary where the keys are the filenames (with
    the ".h" extension replaced by ".html") and the values are lists of tags.

    The tags are expected to be comma-separated and will be stripped of whitespace. Tags
    that are empty after stripping are excluded from the list.

    Returns
    -------

    dict
        A dictionary with filenames as keys and lists of tags as values.

    Example
    -------

    If a header file named "example.h" contains the following documentation block:

        BeginUserDocs: neuron, adaptive threshold, integrate-and-fire
        ...
        EndUserDocs

    The resulting dictionary will have an entry:
        {
            "example.html": ["neuron", "adaptive_threshold", "integrate-and-fire"]
        }
    """
    models_dict = {}

    for match, file_path in extract_model_text():
        filename = file_path.name
        formatted_path = filename.replace(".h", ".html")

        # Initialize with no tags for the file
        models_dict[formatted_path] = []

        tags = [t.strip() for t in match.group("tags").split(",")]
        if "NOINDEX" in tags:
            continue
        # Strip whitespace from each tag, replace spaces with underscores, and filter out empty strings
        tags = [tag.strip() for tag in tags if tag.strip()]
        models_dict[formatted_path] = tags

    return models_dict


def find_models_in_tag_combinations(models_dict):
    """
    Processes a dictionary mapping models to tags to create a list of tag-model combinations.

    This function reverses a dictionary that maps models to a list of tags, creating a new
    mapping from tags to models. It then creates a structured list of dictionaries, each
    containing information about a tag and the associated models.

    Parameters
    ----------

    models_dict : dict[str, list[str]]
        A dictionary where keys are model identifiers and values are lists of tags.

    Returns
    -------

    result_list : list[dict]
        A list of dictionaries, each containing the tag, associated models, and the count of models.

    """
    # Reverse the models_dict to map tags to models
    model_to_tags = {}
    for model, tags in models_dict.items():
        for tag in tags:
            if tag not in model_to_tags:
                model_to_tags[tag] = set()
            model_to_tags[tag].add(model)

    result_list = []

    for tag, models in model_to_tags.items():
        if isinstance(models, set):
            models = list(models)  # Convert set to list for JSON serialization

        tag_info = {"tag": tag, "models": models, "count": len(models)}  # Number of models
        result_list.append(tag_info)

    return result_list


# The following function is called at Sphinx core even source read


def template_renderer(app, docname, source):
    """
    Modifies the source content for specified templates by rendering them with model and tag data.

    Checks if the document being processed is one of the specified templates. If it is,
    it retrieves the models and  tags from the environment and
    uses this data to render the template content.

    Parameters
    ----------

    app
        Sphinx application object
    docname : str
        The name of the document being processed, used to determine if the
        current document matches one of the specified templates.
    source : list
        A list containing the source content of the document; modified in-place
        and used to inject the rendered content.
    """
    env = app.builder.env
    template_files = ["models/index", "neurons/index", "synapses/index", "devices/index"]

    # Render the document if it matches one of the specified templates
    if any(docname == template_file for template_file in template_files):
        html_context = {"tag_dict": env.tag_dict, "model_dict": env.model_dict}
        src = source[0]
        rendered = app.builder.templates.render_string(src, html_context)
        source[0] = rendered


def setup(app):
    """
    Configures application hooks for the Sphinx documentation builder.

    This function connects other functions to run during separte Sphinx events
    """
    app.connect("config-inited", create_rst_files)
    app.connect("env-before-read-docs", get_model_tags)
    app.connect("source-read", template_renderer)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
