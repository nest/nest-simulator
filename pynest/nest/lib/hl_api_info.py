# -*- coding: utf-8 -*-
#
# hl_api_info.py
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
Functions to get information on NEST.
"""

import sys
import os
import textwrap
import webbrowser

from ..ll_api import *
from .hl_api_helper import *
from .hl_api_types import to_json
import nest

__all__ = [
    'authors',
    'get_argv',
    'GetStatus',
    'get_verbosity',
    'help',
    'helpdesk',
    'message',
    'SetStatus',
    'set_verbosity',
    'sysinfo',
]


@check_stack
def sysinfo():
    """Print information on the platform on which NEST was compiled.

    """

    sr("sysinfo")


@check_stack
def authors():
    """Print the authors of NEST.

    """

    sr("authors")


@check_stack
def helpdesk():
    """Open the NEST documentation index in a browser.

    This command opens the NEST documentation index page using the
    system's default browser.

    Please note that the help pages will only be available if you ran
    ``make html`` prior to installing NEST. For more details, see
    :ref:`documentation_workflow`.

    """

    docdir = sli_func("statusdict/prgdocdir ::")
    help_fname = os.path.join(docdir, 'html', 'index.html')

    if not os.path.isfile(help_fname):
        msg = "Sorry, the help index cannot be opened. "
        msg += "Did you run 'make html' before running 'make install'?"
        raise FileNotFoundError(msg)

    webbrowser.open_new(f"file://{help_fname}")


@check_stack
def help(obj=None, return_text=False):
    """Display the help page for the given object in a pager.

    If ``return_text`` is omitted or explicitly given as ``False``,
    this command opens the help text for ``object`` in the default
    pager using the ``pydoc`` module.

    If ``return_text`` is ``True``, the help text is returned as a
    string in reStructuredText format instead of displaying it.

    Parameters
    ----------
    obj : object, optional
        Object to display help for
    return_text : bool, optional
        Option for returning the help text

    Returns
    -------
    None or str
        The help text of the object if `return_text` is `True`.

    """

    if obj is not None:
        try:
            if return_text:
                return load_help(obj)
            else:
                show_help_with_pager(obj)
        except FileNotFoundError:
            print(textwrap.dedent(f"""
                Sorry, there is no help for model '{obj}'.
                Use the Python help() function to obtain help on PyNEST functions."""))
    else:
        print(nest.__doc__)


@check_stack
def get_argv():
    """Return argv as seen by NEST.

    This is similar to Python :code:`sys.argv` but might have changed after
    MPI initialization.

    Returns
    -------
    tuple
        Argv, as seen by NEST

    """

    sr('statusdict')
    statusdict = spp()
    return statusdict['argv']


@check_stack
def message(level, sender, text):
    """Print a message using message system of NEST.

    Parameters
    ----------
    level :
        Level
    sender :
        Message sender
    text : str
        Text to be sent in the message

    """

    sps(level)
    sps(sender)
    sps(text)
    sr('message')


@check_stack
def get_verbosity():
    """Return verbosity level of NEST's messages.

    - M_ALL=0,  display all messages
    - M_INFO=10, display information messages and above
    - M_DEPRECATED=18, display deprecation warnings and above
    - M_WARNING=20, display warning messages and above
    - M_ERROR=30, display error messages and above
    - M_FATAL=40, display failure messages and above

    Returns
    -------
    int:
        The current verbosity level
    """

    sr('verbosity')
    return spp()


@check_stack
def set_verbosity(level):
    """Change verbosity level for NEST's messages.

    - M_ALL=0,  display all messages
    - M_INFO=10, display information messages and above
    - M_DEPRECATED=18, display deprecation warnings and above
    - M_WARNING=20, display warning messages and above
    - M_ERROR=30, display error messages and above
    - M_FATAL=40, display failure messages and above

    Parameters
    ----------
    level : str
        Can be one of 'M_FATAL', 'M_ERROR', 'M_WARNING', 'M_DEPRECATED',
        'M_INFO' or 'M_ALL'.
    """

    sr("{} setverbosity".format(level))


@check_stack
def SetStatus(nodes, params, val=None):
    """Set parameters of nodes or connections.

    Parameters of nodes or connections, given in `nodes`, is set as specified
    by `params`. If `val` is given, `params` has to be a `string` with the
    name of an attribute, which is set to `val` on the nodes/connections. `val`
    can be a single value or a list of the same size as nodes.

    Parameters
    ----------
    nodes : NodeCollection or SynapseCollection
        Either a `NodeCollection` representing nodes, or a `SynapseCollection`
        of connection handles as returned by
        :py:func:`.GetConnections()`.
    params : str or dict or list
        Dictionary of parameters (either lists or single values) or list of dictionaries of parameters
        of same length as `nodes`. If `val` is given, this has to be a string giving
        the name of a model property.
    val : int, list, optional
        If given, params has to be the name of a model property.

    Raises
    ------
    TypeError
        If `nodes` is not a NodeCollection of nodes, a SynapseCollection of synapses, or if the
        number of parameters don't match the number of nodes or
        synapses.

    See Also
    -------
    :py:func:`GetStatus`,
    :py:meth:`NodeCollection.get()<nest.lib.hl_api_types.NodeCollection.get>`,
    :py:meth:`NodeCollection.set()<nest.lib.hl_api_types.NodeCollection.set>`

    """

    if not isinstance(nodes, (nest.NodeCollection, nest.SynapseCollection)):
        raise TypeError("'nodes' must be NodeCollection or a SynapseCollection.")

    # This was added to ensure that the function is a nop (instead of,
    # for instance, raising an exception) when applied to an empty
    # list, which is an artifact of the API operating on lists, rather
    # than relying on language idioms, such as comprehensions
    if len(nodes) == 0:
        return

    params_is_dict = isinstance(params, dict)
    set_status_nodes = isinstance(nodes, nest.NodeCollection)
    if set_status_nodes:
        local_nodes = [nodes.local] if len(nodes) == 1 else nodes.local
        set_status_nodes = set_status_nodes and all(local_nodes)

    if (params_is_dict and set_status_nodes):

        node_params = nodes[0].get()
        contains_list = [is_iterable(vals) and key in node_params and not is_iterable(node_params[key]) for
                         key, vals in params.items()]

        if any(contains_list):
            temp_param = [{} for _ in range(len(nodes))]

            for key, vals in params.items():
                if not is_iterable(vals):
                    for temp_dict in temp_param:
                        temp_dict[key] = vals
                else:
                    for i, temp_dict in enumerate(temp_param):
                        temp_dict[key] = vals[i]
            params = temp_param

    if val is not None and is_literal(params):
        if is_iterable(val) and not isinstance(val, (uni_str, dict)):
            params = [{params: x} for x in val]
        else:
            params = {params: val}

    if isinstance(params, (list, tuple)) and len(nodes) != len(params):
        raise TypeError("status dict must be a dict, or a list of dicts of length {}".format(len(nodes)))

    if isinstance(nodes, nest.SynapseCollection):
        params = broadcast(params, len(nodes), (dict,), "params")

        sps(nodes)
        sps(params)

        sr('2 arraystore')
        sr('Transpose { arrayload pop SetStatus } forall')
    else:
        sli_func('SetStatus', nodes, params)


@check_stack
def GetStatus(nodes, keys=None, output=''):
    """Return the parameter dictionaries of nodes or connections.

    If `keys` is given, a list of values is returned instead. `keys` may
    also be a list, in which case the returned list contains lists of
    values.

    Parameters
    ----------
    nodes : NodeCollection or SynapseCollection
        Either a `NodeCollection` representing nodes, or a `SynapseCollection` of
        connection handles as returned by :py:func:`.GetConnections()`.
    keys : str or list, optional
        string or a list of strings naming model properties.
        `GetStatus` then returns a single value or a list of values
        belonging to the keys given.
    output : str, optional
        Whether the returned data should be in a selected format
        (``output='json'``).

    Returns
    -------
    list of dicts :
        All parameters in a dict for each node or connection.
    list of values :
        If `keys` is a string, the value of the corresponding parameter for each node or connection is returned.
    list of lists of values :
        If `keys` is a list of strings, a list of values of the corresponding parameters for each node or connection
        is returned.
    str :
        If `output` is `json`, the above formats are converted to JSON format before they are returned.

    Raises
    ------
    TypeError
        If `nodes` or `keys` are on the wrong form.

    See Also
    --------
    :py:func:`SetStatus`,
    :py:meth:`NodeCollection.set()<nest.lib.hl_api_types.NodeCollection.set>`,
    :py:meth:`NodeCollection.get()<nest.lib.hl_api_types.NodeCollection.get>`

    Examples
    --------
    *For nodes:*

    >>>    nest.GetStatus(nodes)
           ({'archiver_length': 0,
             'beta_Ca': 0.001,
             ...
             'global_id': 1,
             ...
             'vp': 0},
            ...
            {'archiver_length': 0,
             'beta_Ca': 0.001,
             ...
             'global_id': 3,
             ...
             'vp': 0})

    >>>    nest.GetStatus(nodes, 'V_m')
           (-70.0, -70.0, -70.0)

    >>>    nest.GetStatus(nodes, ['V_m', 'C_m'])
           ((-70.0, 250.0), (-70.0, 250.0), (-70.0, 250.0))

    >>>    nest.GetStatus(nodes, ['V_m', 'C_m'], output='json')
           '[[-70.0, 250.0], [-70.0, 250.0], [-70.0, 250.0]]'

    *For connections:*

    >>>    nest.GetStatus(conns)
           ({'delay': 1.0,
             ...
             'source': 1,
             ...
             'weight': 1.0},
            ...
            {'delay': 1.0,
             ...
             'source': 3,
             ...
             'weight': 1.0})

    >>>    nest.GetStatus(conns, 'weight')
           (1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0)

    >>>    nest.GetStatus(conns, ['source', 'delay'])
           ((1, 1.0),
            ...
            (3, 1.0))

    >>>    nest.GetStatus(conns, ['source', 'delay'], output='json')
           '[[1, 1.0], [1, 1.0], [1, 1.0], [2, 1.0], [2, 1.0], [2, 1.0],
           [3, 1.0], [3, 1.0], [3, 1.0]]'
    """

    if not (isinstance(nodes, nest.NodeCollection) or isinstance(nodes, nest.SynapseCollection)):
        raise TypeError("The first input (nodes) must be NodeCollection or a SynapseCollection with connection handles")

    if len(nodes) == 0:
        return '[]' if output == 'json' else ()

    if keys is None:
        cmd = 'GetStatus'
    elif is_literal(keys):
        cmd = 'GetStatus {{ /{0} get }} Map'.format(keys)
    elif is_iterable(keys):
        keys_str = " ".join("/{0}".format(x) for x in keys)
        cmd = 'GetStatus {{ [ [ {0} ] ] get }} Map'.format(keys_str)
    else:
        raise TypeError("keys should be either a string or an iterable")

    sps(nodes)

    sr(cmd)

    result = spp()

    if isinstance(result, dict):
        # We have taken GetStatus on a layer object, or another NodeCollection with metadata, which returns a
        # dictionary from C++, so we need to turn it into a tuple for consistency.
        result = (result,)

    if output == 'json':
        result = to_json(result)

    return result
