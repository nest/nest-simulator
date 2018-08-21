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
import inspect
import json
import functools
import textwrap
import subprocess
import os
import re
import sys

from string import Template

# These variables MUST be set by __init__.py right after importing.
# There is no safety net, whatsoever.
sps = spp = sr = kernel = None

# These flags are used to print deprecation warnings only once. The
# corresponding functions will be removed in the 2.6 release of NEST.
_deprecation_warning = {'BackwardCompatibilityConnect': True}


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
    lines = [textwrap.fill(" ".join(l.split()), width=width) for l in lines]
    return "\n".join(lines)


def show_deprecation_warning(func_name, alt_func_name=None, text=None):
    """Shows a deprecation warning for a function.

    Parameters
    ----------
    func_name : str
        Name of the deprecated function
    alt_func_name : str, optional
        Name of the function to use instead
    text : str, optional
        Text to display instead of standard text
    """
    if _deprecation_warning[func_name]:
        if alt_func_name is None:
            alt_func_name = 'Connect'
        if text is None:
            text = "{0} is deprecated and will be removed in a future \
            version of NEST.\nPlease use {1} instead!\n\
            For details, see\
            http://www.nest-simulator.org/connection_management\
            ".format(func_name, alt_func_name)
            text = get_wrapped_text(text)

        warnings.warn('\n' + text)   # add LF so text starts on new line
        _deprecation_warning[func_name] = False


# Since we need to pass extra arguments to the decorator, we need a
# decorator factory. See http://stackoverflow.com/questions/15564512
def deprecated(alt_func_name, text=None):
    """Decorator for deprecated functions.

    Shows a warning and calls the original function.

    Parameters
    ----------
    alt_func_name : str, optional
        Name of the function to use instead
    text : str, optional
        Text to display instead of standard text

    Returns
    -------
    function:
        Decorator function
    """

    def deprecated_decorator(func):
        _deprecation_warning[func.__name__] = True

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

__debug = False


def get_debug():
    """Return the current value of the debug flag of the high-level API.

    Returns
    -------
    bool:
        current value of the debug flag
    """

    global __debug
    return __debug


def set_debug(dbg=True):
    """Set the debug flag of the high-level API.

    Parameters
    ----------
    dbg : bool, optional
        Value to set the debug flag to
    """

    global __debug
    __debug = dbg


def stack_checker(f):
    """Decorator to add stack checks to functions using PyNEST's
    low-level API.

    This decorator works only on functions. See
    check_stack() for the generic version for functions and
    classes.

    Parameters
    ----------
    f : function
        Function to decorate

    Returns
    -------
    function:
        Decorated function

    Raises
    ------
    kernel.NESTError
    """

    @functools.wraps(f)
    def stack_checker_func(*args, **kwargs):
        if not get_debug():
            return f(*args, **kwargs)
        else:
            sr('count')
            stackload_before = spp()
            result = f(*args, **kwargs)
            sr('count')
            num_leftover_elements = spp() - stackload_before
            if num_leftover_elements != 0:
                eargs = (f.__name__, num_leftover_elements)
                etext = "Function '%s' left %i elements on the stack."
                raise kernel.NESTError(etext % eargs)
            return result

    return stack_checker_func


def check_stack(thing):
    """Convenience wrapper for applying the stack_checker decorator to
    all class methods of the given class, or to a given function.

    If the object cannot be decorated, it is returned unchanged.

    Parameters
    ----------
    thing : function or class
        Description

    Returns
    -------
    function or class
        Decorated function or class

    Raises
    ------
    ValueError
    """

    if inspect.isfunction(thing):
        return stack_checker(thing)
    elif inspect.isclass(thing):
        for name, mtd in inspect.getmembers(thing, predicate=inspect.ismethod):
            if name.startswith("test_"):
                setattr(thing, name, stack_checker(mtd))
        return thing
    else:
        raise ValueError("unable to decorate {0}".format(thing))


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


def is_sequence_of_gids(seq):
    """Checks whether the argument is a potentially valid sequence of
    GIDs (non-negative integers).

    Parameters
    ----------
    seq : object
        Object to check

    Returns
    -------
    bool:
        True if object is a potentially valid sequence of GIDs
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
        raise TypeError("'{0}' must be a single value, a list with " +
                        "one element or a list with {1} elements.".format(
                            name, length))
    return item


def __check_nb():
    """Return true if called from a Jupyter notebook."""
    try:
        return get_ipython().__class__.__name__.startswith('ZMQ')
    except NameError:
        return False


def __show_help_in_modal_window(objname, hlptxt):
    """Open modal window with help text

    Parameters
    ----------
    objname :   str
            filename
    hlptxt  :   str
            Full text
    """

    hlptxt = json.dumps(hlptxt)
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

    display(Javascript(s.substitute(jstitle=objname, jstext=hlptxt)))


def get_help_filepath(hlpobj):
    """Get file path of help object

    Prints message if no help is available for hlpobj.

    Parameters
    ----------
    hlpobj : string
        Object to display help for

    Returns
    -------
    string:
        Filepath of the help object or None if no help available
    """

    helpdir = os.path.join(sli_func("statusdict/prgdocdir ::"), "help")
    objname = hlpobj + '.hlp'
    for dirpath, dirnames, files in os.walk(helpdir):
        for hlp in files:
            if hlp == objname:
                objf = os.path.join(dirpath, objname)
                return objf
    else:
        print("Sorry, there is no help for '" + hlpobj + "'.")
        return None


def load_help(hlpobj):
    """Returns documentation of the object

    Parameters
    ----------
    hlpobj : object
        Object to display help for

    Returns
    -------
    string:
        The documentation of the object or None if no help available
    """

    objf = get_help_filepath(hlpobj)
    if objf:
        with open(objf, 'r') as fhlp:
            hlptxt = fhlp.read()
        return hlptxt
    else:
        return None


def __is_executable(path, candidate):
    """Returns true for executable files."""

    candidate = os.path.join(path, candidate)
    return os.access(candidate, os.X_OK) and os.path.isfile(candidate)


def show_help_with_pager(hlpobj, pager=None):
    """Output of doc in python with pager or print

    Parameters
    ----------
    hlpobj : object
        Object to display
    pager: str, optional
        pager to use, False if you want to display help using print().
    """

    if sys.version_info < (2, 7, 8):
        print("NEST help is only available with Python 2.7.8 or later.\n")
        return

    if 'NEST_INSTALL_DIR' not in os.environ:
        print(
            'NEST help needs to know where NEST is installed.'
            'Please source nest_vars.sh or define NEST_INSTALL_DIR manually.')
        return

    # check that help is available
    objf = get_help_filepath(hlpobj)
    if objf is None:
        return   # message is printed by get_help_filepath()

    if __check_nb():
        # Display help in notebook
        # Load the helptext, check the file exists.
        hlptxt = load_help(hlpobj)
        if hlptxt:
            # Opens modal window only in notebook.
            objname = hlpobj + '.hlp'
            __show_help_in_modal_window(objname, hlptxt)
        return

    if not pager and pager is not None:
        # pager == False: display using print()
        # Note: we cannot use "pager is False" as Numpy has its own False
        hlptxt = load_help(hlpobj)
        if hlptxt:
            print(hlptxt)
        return

    # Help is to be displayed by pager
    # try to find a pager if not explicitly given
    if pager is None:
        pager = sli_func('/page /command GetOption')

        # pager == false if .nestrc does not define one
        if not pager:
            # Search for pager in path. The following is based on
            # https://stackoverflow.com/questions/377017
            for candidate in ['less', 'more', 'cat']:
                if any(__is_executable(path, candidate)
                       for path in os.environ['PATH'].split(os.pathsep)):
                    pager = candidate
                    break

    # check that we have a pager
    if not pager:
        print('NEST help requires a pager program. You can configure '
              'it in the .nestrc file in your home directory.')
        return

    try:
        subprocess.check_call([pager, objf])
    except (OSError, IOError, subprocess.CalledProcessError):
        print('Displaying help with pager "{}" failed. '
              'Please define a working parser in file .nestrc '
              'in your home directory.'.format(pager))


@check_stack
def get_verbosity():
    """Return verbosity level of NEST's messages.

    Returns
    -------
    int:
        The current verbosity level
    """

    # Defined in hl_api_helper to avoid circular inclusion problem with
    # hl_api_info.py
    sr('verbosity')
    return spp()


@check_stack
def set_verbosity(level):
    """Change verbosity level for NEST's messages.

    Parameters
    ----------
    level : str
        Can be one of 'M_FATAL', 'M_ERROR', 'M_WARNING', 'M_DEPRECATED',
        'M_INFO' or 'M_ALL'.
    """

    # Defined in hl_api_helper to avoid circular inclusion problem with
    # hl_api_info.py
    sr("{} setverbosity".format(level))


def model_deprecation_warning(model):
    """Checks whether the model is to be removed in a future verstion of NEST.
    If so, a deprecation warning is issued.

    Parameters
    ----------
    model: str
        Name of model
    """

    deprecated_models = {'aeif_cond_alpha_RK5': 'aeif_cond_alpha'}

    if model in deprecated_models:
        text = "The {0} model is deprecated and will be removed in a \
        future version of NEST, use {1} instead.\
        ".format(model, deprecated_models[model])
        text = get_wrapped_text(text)
        warnings.warn('\n' + text)


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
        self._verbosity_level = get_verbosity()

    def __enter__(self):

        for func_name in self._no_dep_funcs:
            self._deprecation_status[func_name] = _deprecation_warning[func_name]  # noqa
            _deprecation_warning[func_name] = False

            # Suppress only if verbosity level is deprecated or lower
            if self._verbosity_level <= sli_func('M_DEPRECATED'):
                set_verbosity(sli_func('M_WARNING'))

    def __exit__(self, *args):

        # Reset the verbosity level and deprecation warning status
        set_verbosity(self._verbosity_level)

        for func_name, deprec_status in self._deprecation_status.items():
            _deprecation_warning[func_name] = deprec_status
