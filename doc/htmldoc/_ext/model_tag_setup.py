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

logging.basicConfig(level=logging.INFO)
log = logging.getLogger(__name__)


def find_models_in_tag_combinations(models_dict):
    """
    Processes a dictionary mapping models to tags to create a list of tag-model combinations.

    This function reverses a dictionary that maps models to a list of tags, creating a new
    mapping from tags to models. It then creates a structured list of dictionaries, each
    containing information about a tag and the associated models.

    Parameters
    ----------

    models_dict : dict
        A dictionary where keys are model identifiers and values are lists of tags.

    Returns
    -------

    result_list : list
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


def extract_tags_from_files(directory_path):
    """
    Iterates through a list of file paths, opens each file, and extracts tags from the line that contains 'BeginUserDocs'.
    Tags are expected to be comma-separated right after 'BeginUserDocs'.

    Parameters
    ----------

    directory_path : list
        A list of strings, where each string is a path to a file.

    Returns
    -------

    models_dict : dict
        A dictionary with filenames as keys and lists of tags as values.
    """
    models_dict = {}

    file_paths = glob.glob(os.path.join(directory_path, "*.h"))

    userdoc_re = re.compile(r"BeginUserDocs:?\s*(?P<tags>([\w -]+(,\s*)?)*)\n+(?P<doc>(.|\n)*)EndUserDocs")
    for file_path in file_paths:
        with open(file_path, "r", encoding="utf8") as file:
            match = userdoc_re.search(file.read())
        if not match:
            log.info("No user documentation found in " + file_path)
            continue

        filename = os.path.basename(file_path)
        formatted_path = filename.replace(".h", ".html")

        # Initialize with no tags for the file
        models_dict[formatted_path] = []

        tags = [t.strip() for t in match.group("tags").split(",")]
        # Strip whitespace from each tag, replace spaces with underscores, and filter out empty strings
        tags = [tag.strip().replace(" ", "_") for tag in tags if tag.strip()]
        models_dict[formatted_path] = tags

    return models_dict


def get_models(app, env, docname):
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

    directory_path = "../../models/"

    # Initialize necessary dictionaries if not already present
    if not hasattr(env, "tag_dict"):
        env.tag_dict = {}

    if not hasattr(env, "model_dict"):
        env.model_dict = {}

    # Extract models and tags, and find tag-to-model relationships
    env.model_dict = extract_tags_from_files(directory_path)
    env.tag_dict = find_models_in_tag_combinations(models_dict)

    # Write the JSON output directly to a file used for dynamically loading data client-side
    with open("static/data/filter_model.json", "w") as json_file:
        json.dump(env.tag_dict, json_file, indent=2)


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

    This function connects other functions to run before reading documents and when reading
    source files.
    """
    app.connect("env-before-read-docs", get_models)
    app.connect("source-read", template_renderer)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
