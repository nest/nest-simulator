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
from sphinx.addnodes import pending_xref
from sphinx.application import Sphinx
import json
import re
import os


def ModelMatchExamples():
    # Get list of models and search the examples directory for matches

    filepath_models = "../../models/"
    filepath_examples = "auto_examples/"

    model_files = []
    for filename in os.listdir(filepath_models):
        if filename.endswith(".h"):
            model_files.append(os.path.splitext(filename)[0])

    matches = {}
    for root, dirs, files in os.walk(filepath_examples):
        for filename in files:
            if filename.endswith(".rst"):
                full_path = os.path.join(root, filename)
                if "auto_examples/index" in full_path:
                    continue
                with open(full_path, "r", errors="ignore") as file:
                    content = file.read()
                    for model_file in model_files:
                        if model_file in content:
                            if model_file not in matches:
                                matches[model_file] = []
                            matches[model_file].append(full_path)

    return matches


class ListExamplesDirective(Directive):
    # Provide a list of the examples that contain the given model name.
    # The model name is the required argument in the directive
    # (.. listexamples:: model_name). No options, nor content is expected.

    has_content = False
    required_arguments = 1
    option_spec = {}

    def run(self):
        my_arg = self.arguments[0]

        # bullet list equivalent to HTML <ul>
        bullet_list = nodes.bullet_list()
        examples_list = ModelMatchExamples()

        sorted_examples = {key: sorted(values) for key, values in examples_list.items()}
        for key, values in sorted_examples.items():
            if key == my_arg:
                for value in values:
                    # A leading '/'  is added to the example path to take advantage of the
                    # std:label (ref role) syntax (see also objects.inv doc for intersphinx)
                    # we also need the names to be all lower case
                    value_label = "/" + value.lower()
                    link_text = value.split("/")[-1]
                    # equivalent to HTML <li>
                    list_item = nodes.list_item()
                    link_node = pending_xref(
                        "",
                        reftype="ref",
                        refdomain="std",
                        refexplicit=False,
                        reftarget=value_label,
                        refwarn=True,
                        classes=["xref", "std", "std-ref"],
                    )
                    link_node += nodes.inline(text=link_text)
                    para = nodes.paragraph()
                    para.append(link_node)
                    list_item.append(para)
                    bullet_list += list_item

        return [bullet_list]


def setup(app):
    # Note that the directive names need to be all lower case
    app.add_directive("listexamples", ListExamplesDirective)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
