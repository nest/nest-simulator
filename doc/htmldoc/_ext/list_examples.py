# -*- coding: utf-8 -*-
#
# list_examples.py
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

from docutils import nodes
from docutils.parsers.rst import Directive, Parser

from sphinx.application import Sphinx
from sphinx.util.docutils import SphinxDirective
import os
import glob


class listnode(nodes.General, nodes.Element):
    pass


def ProcessExamples(app, doctree, docname):
    # Create the bullet list of examples, sorted by title
    # given by the model name in the directive argument

    env = app.builder.env
    mydict = {}

    dict_match = ModelMatchExamples()

    for node in doctree.findall(listnode):
        for item in env.all_examples:
            # compare current location with attribute location to get
            # correct model name
            if item["name"] == docname:
                list_of_examples = dict_match[item["model_name"]]
                for value in list_of_examples:
                    mydocname = os.path.splitext(value)[0]
                    mydict[mydocname] = str(env.titles[mydocname].children[0])

            sorted_examples = dict(sorted(mydict.items(), key=lambda x: x[1]))

            bullet_list = nodes.bullet_list()

            for filename, title in sorted_examples.items():
                # Create a reference node that links to the example
                # equivalent to HTML <li>
                list_item = nodes.list_item()
                newnode = nodes.reference("", "")
                newnode["internal"] = True
                newnode["refdocname"] = filename
                newnode["refuri"] = app.builder.get_relative_uri(docname, filename)
                newnode.append(nodes.Text(title))
                para = nodes.paragraph()
                para += newnode
                list_item.append(para)
                bullet_list += list_item

        node.replace_self(bullet_list)


def ModelMatchExamples():
    # Get list of models and search the examples directory for matches

    filepath_models = "../../models/"
    filepath_examples = "auto_examples/"

    model_files = []
    for filename in os.listdir(filepath_models):
        if filename.endswith(".h"):
            model_files.append(os.path.splitext(filename)[0])

    matches = {}
    files = glob.glob(filepath_examples + "**/*.rst", recursive=True)
    for filename in files:
        if "auto_examples/index" in filename:
            continue
        with open(filename, "r", errors="ignore") as file:
            content = file.read()
            for model_file in model_files:
                if model_file in content:
                    if model_file not in matches:
                        matches[model_file] = []
                    matches[model_file].append(filename)

    return matches


class ListExamplesDirective(SphinxDirective):
    # Provide a list of the examples that contain the given model name.
    # The model name is the required argument in the directive
    # (.. listexamples:: model_name). No options, nor content is expected.

    has_content = False
    required_arguments = 1
    option_spec = {}

    def run(self):
        my_arg = self.arguments[0]

        if not hasattr(self.env, "all_examples"):
            self.env.all_examples = []

        # See TODO tutorial in Sphinx for more info.
        # Using environment attribute all_examples to store argument and
        # its location (docname)
        self.env.all_examples.append({"model_name": my_arg, "name": self.env.docname})

        return [listnode("")]


def setup(app):
    # Note that the directive names need to be all lower case
    app.add_directive("listexamples", ListExamplesDirective)
    app.add_node(listnode)
    app.connect("doctree-resolved", ProcessExamples)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
