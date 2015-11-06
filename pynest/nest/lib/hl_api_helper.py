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

# These variables MUST be set by __init__.py right after importing.
# There is no safety net, whatsoever.
sps = spp = sr = pcd = kernel = None

import warnings

# Monkeypatch warnings.showwarning() to just print the warning without
# the code line it was emitted by.
def _warning(msg, cat=UserWarning, fname='', lineno=-1):
    print('{0}:{1}: {2}: {3}'.format(fname, lineno, cat.__name__, msg))
warnings.showwarning = _warning

import inspect
import functools
import textwrap

# These flags are used to print deprecation warnings only once. The
# corresponding functions will be removed in the 2.6 release of NEST.

_deprecation_warning = {'BackwardCompatibilityConnect': True}

def show_deprecation_warning(func_name, alt_func_name=None, text=None):        
    if _deprecation_warning[func_name]:
        if alt_func_name is None:
            alt_func_name = 'Connect'
        if text is None:
            text = textwrap.dedent(
                       """\
                       {0} is deprecated and will be removed in a future version of NEST.
                       Please use {1} instead!
                       For details, see http://www.nest-simulator.org/connection_management\
                       """.format(func_name, alt_func_name)
                   )

        warnings.warn('\n' + text)   # add LF so text starts on new line
        _deprecation_warning[func_name] = False

# Since we need to pass extra arguments to the decorator, we need a
# decorator factory. See http://stackoverflow.com/questions/15564512
def deprecated(alt_func_name, text=None):
    """
    Decorator for deprecated functions. Shows a warning and calls the
    original function.
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
    import sys
    if sys.version_info[0] < 3:
        return basestring
    return str

uni_str = get_unistring_type()


def is_literal(obj):
    """
    Check whether obj is a "literal": a unicode string or SLI literal
    """
    return isinstance(obj, (uni_str, kernel.SLILiteral))


def is_string(obj):
    """
    Check whether obj is a "literal": a unicode string or SLI literal
    """
    return isinstance(obj, uni_str)


__debug = False

def get_debug():
    """
    Return the current value of the debug flag of the high-level API.
    """

    global __debug
    return __debug


def set_debug(dbg=True):
    """
    Set the debug flag of the high-level API.
    """

    global __debug
    __debug = dbg


def stack_checker(f):
    """
    Decorator to add stack checks to functions using PyNEST's
    low-level API. This decorator works only on functions. See
    check_stack() for the generic version for functions and
    classes.
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
    """
    Convenience wrapper for applying the stack_checker decorator to
    all class methods of the given class, or to a given function. If
    the object cannot be decorated, it is returned unchanged.
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
    """
    Return True if the given object is an iterable, False otherwise
    """

    try:
        iter(seq)
    except TypeError:
        return False

    return True


def is_coercible_to_sli_array(seq):
    """
    Checks whether `seq` is coercible to a SLI array
    """

    import sys

    if sys.version_info[0] >= 3:
        return isinstance(seq, (tuple, list, range))
    else:
        return isinstance(seq, (tuple, list, xrange))


def is_sequence_of_connections(seq):
    """
    Low-level API accepts an iterable of dictionaries or
    subscriptables of CONN_LEN
    """

    try:
        cnn = next(iter(seq))
        return isinstance(cnn, dict) or len(cnn) == kernel.CONN_LEN
    except TypeError:
        pass

    return False


def is_sequence_of_gids(seq):
    """
    Checks whether the argument is a potentially valid sequence of
    GIDs (non-negative integers)
    """

    return all(isinstance(n, int) and n >= 0 for n in seq)


def broadcast(item, length, allowed_types, name="item"):

    if isinstance(item, allowed_types):
        return length * (item,)
    elif len(item) == 1:
        return length * item
    elif len(item) != length:
        raise TypeError("'%s' must be a single value, a list with "
                        + "one element or a list with %i elements."
                        % (name, length))

    return item
