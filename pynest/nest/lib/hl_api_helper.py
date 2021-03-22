# -*- coding: utf-8 -*-
#
# hl_api_helper.py
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
These are helper functions to ease the definition of the high-level
API of the PyNEST wrapper.
"""

import warnings
import json
import functools
import textwrap
import subprocess
import os
import re
import shlex
import sys
import numpy
import pydoc

from string import Template

from ..ll_api import *
from .. import pynestkernel as kernel

__all__ = [
    'broadcast',
    'deprecated',
    'get_parameters',
    'get_parameters_hierarchical_addressing',
    'get_unistring_type',
    'get_wrapped_text',
    'is_coercible_to_sli_array',
    'is_iterable',
    'is_literal',
    'is_sequence_of_connections',
    'is_sequence_of_node_ids',
    'is_string',
    'load_help',
    'model_deprecation_warning',
    'restructure_data',
    'show_deprecation_warning',
    'show_help_with_pager',
    'SuppressedDeprecationWarning',
    'uni_str',
]

# These flags are used to print deprecation warnings only once.
# Only flags for special cases need to be entered here, such as special models
# or function parameters, all flags for deprecated functions will be registered
# by the @deprecated decorator, and therefore does not manually need to be placed here.
_deprecation_warning = {'deprecated_model': {'deprecation_issued': False,
                                             'replacement': 'replacement_mod'},
                        'iaf_psc_alpha_canon': {'deprecation_issued': False,
                                                'replacement': 'iaf_psc_alpha_ps'}}


def format_Warning(message, category, filename, lineno, line=None):
    """Formats deprecation warning."""

    return '%s:%s: %s:%s\n' % (filename, lineno, category.__name__, message)


warnings.formatwarning = format_Warning


def get_wrapped_text(text, width=80):
    """Formats a given multiline string to wrap at a given width, while
    preserving newlines (and removing excessive whitespace).

    Parameters
    ----------
    text : str
        String to format

    Returns
    -------
    str:
        Wrapped string
    """

    lines = text.split("\n")
    lines = [textwrap.fill(" ".join(line.split()), width=width) for line in lines]
    return "\n".join(lines)


def show_deprecation_warning(func_name, alt_func_name=None, text=None):
    """Shows a deprecation warning for a function.

    Parameters
    ----------
    func_name : str
        Name of the deprecated function
    alt_func_name : str, optional
        Name of the function to use instead. Needed if text=None
    text : str, optional
        Text to display instead of standard text
    """
    if func_name in _deprecation_warning:
        if not _deprecation_warning[func_name]['deprecation_issued']:
            if text is None:
                text = ("{0} is deprecated and will be removed in a future version of NEST.\n"
                        "Please use {1} instead!").format(func_name, alt_func_name)
                text = get_wrapped_text(text)

            warnings.warn('\n' + text)   # add LF so text starts on new line
            _deprecation_warning[func_name]['deprecation_issued'] = True


# Since we need to pass extra arguments to the decorator, we need a
# decorator factory. See http://stackoverflow.com/questions/15564512
def deprecated(alt_func_name, text=None):
    """Decorator for deprecated functions.

    Shows a warning and calls the original function.

    Parameters
    ----------
    alt_func_name : str, optional
        Name of the function to use instead, may be empty string
    text : str, optional
        Text to display instead of standard text

    Returns
    -------
    function:
        Decorator function
    """

    def deprecated_decorator(func):
        _deprecation_warning[func.__name__] = {'deprecation_issued': False}

        @functools.wraps(func)
        def new_func(*args, **kwargs):
            show_deprecation_warning(func.__name__, alt_func_name, text=text)
            return func(*args, **kwargs)
        return new_func

    return deprecated_decorator


def get_unistring_type():
    """Returns string type dependent on python version.

    Returns
    -------
    str or basestring:
        Depending on Python version

    """
    import sys
    if sys.version_info[0] < 3:
        return basestring
    return str


uni_str = get_unistring_type()


def is_literal(obj):
    """Check whether obj is a "literal": a unicode string or SLI literal

    Parameters
    ----------
    obj : object
        Object to check

    Returns
    -------
    bool:
        True if obj is a "literal"
    """
    return isinstance(obj, (uni_str, kernel.SLILiteral))


def is_string(obj):
    """Check whether obj is a unicode string

    Parameters
    ----------
    obj : object
        Object to check

    Returns
    -------
    bool:
        True if obj is a unicode string
    """
    return isinstance(obj, uni_str)


def is_iterable(seq):
    """Return True if the given object is an iterable, False otherwise.

    Parameters
    ----------
    seq : object
        Object to check

    Returns
    -------
    bool:
        True if object is an iterable
    """

    try:
        iter(seq)
    except TypeError:
        return False

    return True


def is_coercible_to_sli_array(seq):
    """Checks whether a given object is coercible to a SLI array

    Parameters
    ----------
    seq : object
        Object to check

    Returns
    -------
    bool:
        True if object is coercible to a SLI array
    """

    import sys

    if sys.version_info[0] >= 3:
        return isinstance(seq, (tuple, list, range))
    else:
        return isinstance(seq, (tuple, list, xrange))


def is_sequence_of_connections(seq):
    """Checks whether low-level API accepts seq as a sequence of
    connections.

    Parameters
    ----------
    seq : object
        Object to check

    Returns
    -------
    bool:
        True if object is an iterable of dictionaries or
        subscriptables of CONN_LEN
    """

    try:
        cnn = next(iter(seq))
        return isinstance(cnn, dict) or len(cnn) == kernel.CONN_LEN
    except TypeError:
        pass

    return False


def is_sequence_of_node_ids(seq):
    """Checks whether the argument is a potentially valid sequence of
    node IDs (non-negative integers).

    Parameters
    ----------
    seq : object
        Object to check

    Returns
    -------
    bool:
        True if object is a potentially valid sequence of node IDs
    """

    return all(isinstance(n, int) and n >= 0 for n in seq)


def broadcast(item, length, allowed_types, name="item"):
    """Broadcast item to given length.

    Parameters
    ----------
    item : object
        Object to broadcast
    length : int
        Length to broadcast to
    allowed_types : list
        List of allowed types
    name : str, optional
        Name of item

    Returns
    -------
    object:
        The original item broadcasted to sequence form of length

    Raises
    ------
    TypeError


    """

    if isinstance(item, allowed_types):
        return length * (item, )
    elif len(item) == 1:
        return length * item
    elif len(item) != length:
        raise TypeError(
            "'{0}' must be a single value, a list with one element or a list with {1} elements.".format(name, length))
    return item


def __show_help_in_modal_window(obj, help_text):
    """Open modal window with help text

    Parameters
    ----------
    obj : string
        The filename of the help file
    help_text : string
        Full help_text
    """

    help_text = json.dumps(help_text)
    style = "<style>.modal-body p { display: block;unicode-bidi: embed; " \
            "font-family: monospace; white-space: pre; }</style>"
    s = Template("""
       require(
           ["base/js/dialog"],
           function(dialog) {
               dialog.modal({
                   title: '$jstitle',
                   body: $jstext,
                   buttons: {
                       'close': {}
                   }
               });
           }
       );
       """)

    from IPython.display import HTML, Javascript, display
    display(HTML(style))
    display(Javascript(s.substitute(jstitle=obj, jstext=help_text)))


def get_help_fname(obj):
    """Get file name for help object

    Raises FileNotFound if no help is available for ``obj``.

    Parameters
    ----------
    obj : string
        Object to get help filename for

    Returns
    -------
    string:
        File name of the help text for obj
    """

    docdir = sli_func("statusdict/prgdocdir ::")
    help_fname = os.path.join(docdir, 'html', 'models', f'{obj}.rst')

    if os.path.isfile(help_fname):
        return help_fname
    else:
        raise FileNotFoundError(f"Sorry, there is no help for '{obj}'.")


def load_help(obj):
    """Returns documentation of the given object in RST format

    Parameters
    ----------
    obj : string
        Object to display help for

    Returns
    -------
    string:
        The documentation of the object or None if no help is available
    """

    help_fname = get_help_fname(obj)
    with open(help_fname, 'r', encoding='utf-8') as help_file:
        help_text = help_file.read()
    return help_text


def show_help_with_pager(obj):
    """Display help text for the given object in the Python pager

    If called from within a Jupyter notebook, display help in a modal
    window instead of in the pager.

    Parameters
    ----------
    obj : object
        Object to display

    """

    def check_nb():
        try:
            return get_ipython().__class__.__name__.startswith('ZMQ')
        except NameError:
            return False

    help_text = load_help(obj)

    if check_nb():
        __show_help_in_modal_window(obj + '.rst', help_text)
        return

    pydoc.pager(help_text)


def __is_executable(path, candidate):
    """Returns true for executable files."""

    candidate = os.path.join(path, candidate)
    return os.access(candidate, os.X_OK) and os.path.isfile(candidate)


def model_deprecation_warning(model):
    """Checks whether the model is to be removed in a future version of NEST.
    If so, a deprecation warning is issued.

    Parameters
    ----------
    model: str
        Name of model
    """

    if model in _deprecation_warning:
        if not _deprecation_warning[model]['deprecation_issued']:
            text = ("The {0} model is deprecated and will be removed in a future version of NEST, "
                    "use {1} instead.").format(model, _deprecation_warning[model]['replacement'])
            show_deprecation_warning(model, text=text)


def restructure_data(result, keys):
    """
    Restructure list of status dictionaries or list of parameter values to dict with lists or single list or int.

    Parameters
    ----------
    result: list
        list of status dictionaries or list (of lists) of parameter values.
    keys: string or list of strings
        name(s) of properties

    Returns
    -------
    int, list or dict
    """
    if is_literal(keys):
        if len(result) != 1:
            all_keys = sorted({key for result_dict in result for key in result_dict})
            final_result = []

            for result_dict in result:
                if keys in result_dict.keys():
                    final_result.append(result_dict[keys])
                elif keys in all_keys:
                    final_result.append(None)
        else:
            final_result = result[0][keys]

    elif is_iterable(keys):
        final_result = ({key: [val[i] for val in result]
                         for i, key in enumerate(keys)} if len(result) != 1
                        else {key: val[i] for val in result
                              for i, key in enumerate(keys)})

    elif keys is None:
        if len(result) != 1:
            all_keys = sorted({key for result_dict in result for key in result_dict})
            final_result = {}

            for key in all_keys:
                final_result[key] = []
                for result_dict in result:
                    if key in result_dict.keys():
                        final_result[key].append(result_dict[key])
                    else:
                        final_result[key].append(None)
        else:
            final_result = {key: result_dict[key] for result_dict in result for key in result[0]}
    return final_result


def get_parameters(nc, param):
    """
    Get parameters from nodes.

    Used by NodeCollections `get()` function.

    Parameters
    ----------
    nc: NodeCollection
        nodes to get values from
    param: string or list of strings
        string or list of string naming model properties.

    Returns
    -------
    int, list:
        param is a string so the value(s) is returned
    dict:
        param is a list of string so a dictionary is returned
    """
    # param is single literal
    if is_literal(param):
        cmd = '/{} get'.format(param)
        sps(nc._datum)
        try:
            sr(cmd)
            result = spp()
        except kernel.NESTError:
            result = nc.get()[param]  # If the NodeCollection is a composite.
    # param is array of strings
    elif is_iterable(param):
        result = {param_name: nc.get(param_name) for param_name in param}
    else:
        raise TypeError("Params should be either a string or an iterable")

    return result


def get_parameters_hierarchical_addressing(nc, params):
    """
    Get parameters from nodes, hierarchical case.

    Used by NodeCollections `get()` function.

    Parameters
    ----------
    nc: NodeCollection
        nodes to get values from
    params: tuple
        first value in the tuple should be a string, second can be a string or a list of string.
        The first value corresponds to the path into the hierarchical structure
        while the second value corresponds to the name(s) of the desired
        properties.

    Returns
    -------
    int, list:
        params[-1] is a string so the value(s) is returned
    dict:
        params[-1] is a list of string so a dictionary is returned
    """

    # Right now, NEST only allows get(arg0, arg1) for hierarchical
    # addressing, where arg0 must be a string and arg1 can be string
    # or list of strings.
    if is_literal(params[0]):
        value_list = nc.get(params[0])
        if type(value_list) != tuple:
            value_list = (value_list,)
    else:
        raise TypeError('First argument must be a string, specifying path into hierarchical dictionary')

    result = restructure_data(value_list, None)

    if is_literal(params[-1]):
        result = result[params[-1]]
    else:
        result = {key: result[key] for key in params[-1]}
    return result


class SuppressedDeprecationWarning(object):
    """
    Context manager turning off deprecation warnings for given methods.

    Think thoroughly before use. This context should only be used as a way to
    make sure examples do not display deprecation warnings, that is, used in
    functions called from examples, and not as a way to make tedious
    deprecation warnings dissapear.
    """

    def __init__(self, no_dep_funcs):
        """
        Parameters
        ----------
        no_dep_funcs: Function name (string) or iterable of function names
                      for which to suppress deprecation warnings
        """

        self._no_dep_funcs = (no_dep_funcs if not is_string(no_dep_funcs)
                              else (no_dep_funcs, ))
        self._deprecation_status = {}
        sr('verbosity')  # Use sli-version as we cannon import from info because of cirular inclusion problem
        self._verbosity_level = spp()

    def __enter__(self):

        for func_name in self._no_dep_funcs:
            self._deprecation_status[func_name] = _deprecation_warning[func_name]  # noqa
            _deprecation_warning[func_name]['deprecation_issued'] = True

            # Suppress only if verbosity level is deprecated or lower
            if self._verbosity_level <= sli_func('M_DEPRECATED'):
                # Use sli-version as we cannon import from info because of cirular inclusion problem
                sr("{} setverbosity".format(sli_func('M_WARNING')))

    def __exit__(self, *args):

        # Reset the verbosity level and deprecation warning status
        sr("{} setverbosity".format((self._verbosity_level)))

        for func_name, deprec_dict in self._deprecation_status.items():
            _deprecation_warning[func_name]['deprecation_issued'] = (
                deprec_dict['deprecation_issued'])
