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
from docutils.parsers.rst import Directive
import json


class ListExamplesDirective(Directive):
    # Create a directive that requires one argument
    # (.. helloworld:: argument), which is the model name

    has_content = False
    required_arguments = 1
    option_spec = {}

    def run(self):
        my_arg = self.arguments[0]

        # TODO: Use code that generates this file in this extension
        # The script that generated this json file should be included
        # as a function in this extension somehow, removing the need
        # for using file. The script is still WIP
        with open("_ext/model_match_examples.json") as json_file:
            examples_data = json.load(json_file)

        content = []
        # bullet list equivalent to HTML ul
        bullet_list = nodes.bullet_list()
        for key, values in examples_data.items():
            if key == my_arg:
                for value in values:
                    link_text = value.split("/")[-1]
                    list_item = nodes.list_item()
                    newnode = nodes.reference(value, link_text)
                    newnode["refdocname"] = "auto_examples/"
                    newnode["refuri"] = value + ".html"
                    para = nodes.paragraph()
                    para.append(newnode)
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
