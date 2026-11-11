# -*- coding: utf-8 -*-
#
# test_api_surface.py
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

"""
Tests for the namespace of the ``nest`` root module.

``nest`` resolves its namespace lazily: submodules are imported on first access by
``NestModule.__getattr__``, and so are the ``lib.hl_api_*`` modules, through a symbol
map that the module reads from the sources. Static analysers cannot follow that, so
``nest/__init__.py`` spells the same namespace out in an ``if TYPE_CHECKING:`` block.
Nothing reads that block at runtime, which is exactly why it needs a test.
"""

import ast
import importlib.util
import pathlib

import nest
import pytest


def _type_checking_imports():
    """Return the submodules and ``lib`` modules imported in the TYPE_CHECKING block."""

    tree = ast.parse(pathlib.Path(nest.__file__).read_text())
    guard = next(
        node for node in tree.body if isinstance(node, ast.If) and ast.unparse(node.test).endswith("TYPE_CHECKING")
    )

    submodules, api_modules = set(), set()
    for node in ast.walk(guard):
        if not isinstance(node, ast.ImportFrom):
            continue
        if node.module is None:  # `from . import a, b, c`
            submodules |= {alias.name for alias in node.names}
        elif node.module.startswith("lib."):  # `from .lib.hl_api_x import *`
            api_modules.add(node.module.split(".", 1)[1])

    return submodules, api_modules


def test_type_checking_block_lists_every_submodule():
    """The TYPE_CHECKING block must cover the submodules the runtime exposes."""

    declared, _ = _type_checking_imports()
    assert declared == set(nest._submodules())


def test_type_checking_block_lists_every_api_module():
    """The TYPE_CHECKING block must cover the `hl_api` modules the symbol map draws from."""

    _, declared = _type_checking_imports()
    assert declared == {module.rsplit(".", 1)[1] for module in nest._symbols().values()}


def test_symbol_map_matches_the_imported_modules():
    """
    The symbol map is read from the sources without importing them, so it can disagree
    with what the modules export once imported. It must not: a name missing from the
    map does not resolve on `nest`, and a name mapped to the wrong module resolves to
    the wrong object.
    """

    mapped = {}
    for module_name in set(nest._symbols().values()):
        module = importlib.import_module(module_name, "nest")
        for name in module.__all__:
            assert name not in mapped, f"'{name}' is exported by both {mapped[name]} and {module_name}"
            mapped[name] = module_name

    assert nest._symbols() == mapped


def test_all_covers_the_lazily_resolved_namespace():
    """Every name in ``nest.__all__`` must actually resolve on the module."""

    for name in nest.__all__:
        if name in nest._kernel_attr_names:
            # Reading one of these queries the kernel; only check that the descriptor
            # made it onto the module type.
            assert hasattr(type(nest), name), f"'{name}' has no kernel attribute descriptor"
        elif name in nest._submodules():
            # Importing these can need optional third-party packages, so only check
            # that the submodule the name promises is there.
            assert importlib.util.find_spec(f"nest.{name}") is not None, f"'nest.{name}' does not exist"
        else:
            assert hasattr(nest, name), f"'{name}' is advertised in __all__ but does not resolve"


def test_unknown_attribute_raises():
    with pytest.raises(AttributeError):
        nest.thisAttributeDoesNotExist
