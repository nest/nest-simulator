# -*- coding: utf-8 -*-
#
# hl_api_server_mpi.py
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

import inspect
import time
from copy import deepcopy

import nest

from .hl_api_server_helpers import do_exec, get_or_error, nestify

__all__ = [
    "do_call",
    "set_mpi_comm",
]

mpi_comm = None


@get_or_error
def api_client(call_name, args, kwargs):
    """API Client to call function in NEST."""

    call = getattr(nest, call_name)

    if callable(call):
        if "inspect" in kwargs:
            response = {"data": getattr(inspect, kwargs["inspect"])(call)}
        else:
            response = do_call(call_name, args, kwargs)
    else:
        response = call

    return nest.serialize_data(response)


def combine(call_name, response):
    """Combine responses from different MPI processes.

    In a distributed scenario, each MPI process creates its own share
    of the response from the data available locally. To present a
    coherent view on the reponse data for the caller, this data has to
    be combined.

    If this function is run serially (i.e., without MPI), it just
    returns the response data from the only process immediately.

    The type of the returned result can vary depending on the call
    that produced it.

    The combination of results is based on a cascade of heuristics
    based on the call that was issued and individual repsonse data:
      * if all responses are None, the combined response will also just
        be None
      * for some specific calls, the responses are known to be the
        same from the master and all workers. In this case, the
        combined response is just the master response
      * if the response list contains only a single actual response and
        None otherwise, the combined response will be that one actual
        response
      * for calls to GetStatus on recording devices, the combined
        response will be a merged dictionary in the sense that all
        fields that contain a single value in the individual responsed
        are kept as a single values, while lists will be appended in
        order of appearance; dictionaries in the response are
        recursively treated in the same way
      * for calls to GetStatus on neurons, the combined response is just
        the single dictionary returned by the process on which the
        neuron is actually allocated
      * if the response contains one list per process, the combined
        response will be those lists concatenated and flattened.

    """

    if mpi_comm is None:
        return response[0]

    if all(v is None for v in response):
        return None

    # return the master response if all responses are known to be the same
    if call_name in ("exec", "Create", "GetDefaults", "GetKernelStatus", "SetKernelStatus", "SetStatus"):
        return response[0]

    # return a single response if there is only one which is not None
    filtered_response = list(filter(lambda x: x is not None, response))
    if len(filtered_response) == 1:
        return filtered_response[0]

    # return a single merged dictionary if there are many of them
    if all(type(v[0]) is dict for v in response):
        return merge_dicts(response)

    # return a flattened list if the response only consists of lists
    if all(type(v) is list for v in response):
        return [item for lst in response for item in lst]

    log("combine()", f"ERROR: cannot combine response={response}")
    msg = "Cannot combine data because of unknown reason"
    raise Exception(msg)  # pylint: disable=W0719


def do_call(call_name, args=[], kwargs={}):
    """Call a PYNEST function or execute a script within the server.

    If the server is run serially (i.e., without MPI), this function
    will do one of two things: If call_name is "exec", it will execute
    the script given in args via do_exec(). If call_name is the name
    of a PyNEST API function, it will call that function and pass args
    and kwargs to it.

    If the server is run with MPI, this function will first communicate
    the call type ("exec" or API call) and the args and kwargs to all
    worker processes. Only then will it execute the call in the same
    way as described above for the serial case. After the call, all
    worker responses are collected, combined and returned.

    Please note that this function must only be called on the master
    process (i.e., the task with rank 0) in a distributed scenario.

    """

    if mpi_comm is not None:
        assert mpi_comm.Get_rank() == 0

    if mpi_comm is not None:
        log(call_name, "sending call bcast")
        mpi_comm.bcast(call_name, root=0)
        data = (args, kwargs)
        log(call_name, f"sending data bcast, data={data}")
        mpi_comm.bcast(data, root=0)

    if call_name == "exec":
        master_response = do_exec(kwargs)
    else:
        call, args, kwargs = nestify(call_name, args, kwargs)
        log(call_name, f"local call, args={args}, kwargs={kwargs}")
        master_response = call(*args, **kwargs)

    response = [master_response]
    if mpi_comm is not None:
        log(call_name, "waiting for response gather")
        response = mpi_comm.gather(response[0], root=0)
        log(call_name, f"received response gather, data={response}")

    return combine(call_name, response)


def log(call_name, msg):
    msg = f"==> MASTER 0/{time.time():.7f} ({call_name}): {msg}"
    print(msg, flush=True)


def merge_dicts(response):
    """Merge status dictionaries of recorders

    This function runs through a zipped list and performs the
    following steps:
      * sum up all n_events fields
      * if recording to memory: merge the event dictionaries by joining
        all contained arrays
      * if recording to ascii: join filenames arrays
      * take all other values directly from the device on the first
        process

    """

    result = []

    for device_dicts in zip(*response):
        # TODO: either stip fields like thread, vp, thread_local_id,
        # and local or make them lists that contain the values from
        # all dicts.

        element_type = device_dicts[0]["element_type"]

        if element_type not in ("neuron", "recorder", "stimulator"):
            msg = f'Cannot combine data of element with type "{element_type}".'
            raise Exception(msg)  # pylint: disable=W0719

        if element_type == "neuron":
            tmp = list(filter(lambda status: status["local"], device_dicts))
            assert len(tmp) == 1
            result.append(tmp[0])

        if element_type == "recorder":
            tmp = deepcopy(device_dicts[0])
            tmp["n_events"] = 0

            for device_dict in device_dicts:
                tmp["n_events"] += device_dict["n_events"]

            record_to = tmp["record_to"]
            if record_to not in ("ascii", "memory"):
                msg = f'Cannot combine data when recording to "{record_to}".'
                raise Exception(msg)  # pylint: disable=W0719

            if record_to == "memory":
                event_keys = tmp["events"].keys()
                for key in event_keys:
                    tmp["events"][key] = []
                for device_dict in device_dicts:
                    for key in event_keys:
                        tmp["events"][key].extend(device_dict["events"][key])

            if record_to == "ascii":
                tmp["filenames"] = []
                for device_dict in device_dicts:
                    tmp["filenames"].extend(device_dict["filenames"])

            result.append(tmp)

        if element_type == "stimulator":
            result.append(device_dicts[0])

    return result


def set_mpi_comm(comm):
    global mpi_comm
    mpi_comm = comm
