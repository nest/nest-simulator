# -*- coding: utf-8 -*-
#
# hl_api_exceptions.py
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


class NESTMappedException(type):
    """Metaclass for exception namespace that dynamically creates exception classes.

    If a class (self) of this (meta)-type has an unknown attribute requested, __getattr__ defined
    below gets called, creating a class with that name (the error name) and with an __init__ taking
    commandname and errormessage (as created in the source) which is a closure on the parent and
    errorname as well, with a parent of default type (self.default_parent) or
    self.parents[errorname] if defined. """

    def __getattr__(cls, errorname):
        """Creates a class of type "errorname" which is a child of cls.default_parent or
        cls.parents[errorname] if one is defined.

        This __getattr__ function also stores the class permanently as an attribute of cls for
        re-use where cls is actually the class that triggered the getattr (the class that
        NESTMappedException is a metaclass of). """

        # Dynamic class construction, first check if we know its parent
        if errorname in cls.parents:
            parent = getattr(cls, cls.parents[errorname])
        else:  # otherwise, get the default (SLIException)
            parent = cls.default_parent

        # and now dynamically construct the new class
        # not NESTMappedException, since that would mean the metaclass would let the new class inherit
        # this __getattr__, allowing unintended dynamic construction of attributes
        newclass = type(
            cls.__name__ + '.' + errorname,
            (parent,),
            {
                '__init__': cls.init(parent, errorname),
                '__doc__':
                """Dynamically created exception {} from {}.

                Created for the namespace: {}.
                Parent exception: {}.
                """.format(errorname, cls.source, cls.__name__, parent.__name__)
            }
        )

        # Cache for reuse: __getattr__ should now not get called if requested again
        setattr(cls, errorname, newclass)

        # And now we return the exception
        return newclass


class NESTErrors(metaclass=NESTMappedException):
    """Namespace for NEST exceptions, including dynamically created classes from SLI.

    Dynamic exception creation is through __getattr__ defined in the metaclass NESTMappedException.
    """

    class NESTError(Exception):
        """Base exception class for all NEST exceptions.
        """

        def __init__(self, message):
            """Initializer for NESTError base class.

            Parameters:
            -----------
            message: str
                full error message to report.
            """

            Exception.__init__(self, message)
            self.message = message

    class SLIException(NESTError):
        """Base class for all exceptions coming from sli.
        """

        def __init__(self, commandname, errormessage, errorname='SLIException'):
            """Initialize function.

            Parameters:
            -----------
            errorname: error name from SLI.
            commandname: command name from SLI.
            errormessage: message from SLI.
            """
            message = "{} in SLI function {}{}".format(errorname, commandname, errormessage)
            NESTErrors.NESTError.__init__(self, message)

            self.errorname = errorname
            self.commandname = commandname
            self.errormessage = errormessage

    class PyNESTError(NESTError):
        """Exceptions produced from Python/Cython code.
        """
        pass

    @staticmethod
    def init(parent, errorname):
        """ Static class method to construct init's for SLIException children.

        Construct our new init with closure on errorname (as a default value) and parent.
        The default value allows the __init__ to be chained and set by the leaf child.
        This also moves the paramerization of __init__ away from the class construction logic
        and next to the SLIException init.

        Parameters:
        ----------
        parent: the ancestor of the class needed to properly walk up the MRO (not possible with super() or
            super(type,...) because of the dynamic creation of the function
             (used as a closure on the constructed __init__).
        errorname: the class name for information purposes
          internally (used as a closure on the constructed __init__).
        """

        def __init__(self, commandname, errormessage, errorname=errorname, *args, **kwargs):
            # recursively init the parent class: all of this is only needed to properly set errorname
            parent.__init__(self, commandname, errormessage, *args, errorname=errorname, **kwargs)

        docstring = \
            """Initialization function.

            Parameters:
            -----------
            commandname: sli command name.
            errormessage: sli error message.
            errorname: set by default ("{}") or passed in by child (shouldn't be explicitly set
                        when creating an instance)
            *args, **kwargs: passed through to base class.

            self will be a descendant of {}.
            """.format(errorname, parent.__name__)

        try:
            __init__.__doc__ = docstring
        except AttributeError:
            __init__.__func__.__doc__ = docstring

        return __init__

    # source: the dynamically created exceptions come from SLI
    # default_parent: the dynamically created exceptions are descended from SLIExcepton
    # parents: unless they happen to be mapped in this list to another exception descended from SLIException
    #          these should be updated when new exceptions in sli are created that aren't directly descended
    #          from SLIException (but nothing bad will happen, it's just that otherwise they'll be directly
    #          descended from SLIException instead of an intermediate exception; they'll still be constructed
    #          and useable)
    source = "SLI"
    default_parent = SLIException
    parents = {
        'TypeMismatch': 'InterpreterError',
        'SystemSignal': 'InterpreterError',
        'RangeCheck': 'InterpreterError',
        'ArgumentType': 'InterpreterError',
        'BadParameterValue': 'SLIException',
        'DictError': 'InterpreterError',
        'UndefinedName': 'DictError',
        'EntryTypeMismatch': 'DictError',
        'StackUnderflow': 'InterpreterError',
        'IOError': 'SLIException',
        'UnaccessedDictionaryEntry': 'DictError',
        'UnknownModelName': 'KernelException',
        'NewModelNameExists': 'KernelException',
        'UnknownModelID': 'KernelException',
        'ModelInUse': 'KernelException',
        'UnknownSynapseType': 'KernelException',
        'UnknownNode': 'KernelException',
        'NoThreadSiblingsAvailable': 'KernelException',
        'LocalNodeExpected': 'KernelException',
        'NodeWithProxiesExpected': 'KernelException',
        'UnknownReceptorType': 'KernelException',
        'IncompatibleReceptorType': 'KernelException',
        'UnknownPort': 'KernelException',
        'IllegalConnection': 'KernelException',
        'InexistentConnection': 'KernelException',
        'UnknownThread': 'KernelException',
        'BadDelay': 'KernelException',
        'UnexpectedEvent': 'KernelException',
        'UnsupportedEvent': 'KernelException',
        'BadProperty': 'KernelException',
        'BadParameter': 'KernelException',
        'DimensionMismatch': 'KernelException',
        'DistributionError': 'KernelException',
        'InvalidDefaultResolution': 'KernelException',
        'InvalidTimeInModel': 'KernelException',
        'StepMultipleRequired': 'KernelException',
        'TimeMultipleRequired': 'KernelException',
        'GSLSolverFailure': 'KernelException',
        'NumericalInstability': 'KernelException',
        'KeyError': 'KernelException',
        'MUSICPortUnconnected': 'KernelException',
        'MUSICPortHasNoWidth': 'KernelException',
        'MUSICPortAlreadyPublished': 'KernelException',
        'MUSICSimulationHasRun': 'KernelException',
        'MUSICChannelUnknown': 'KernelException',
        'MUSICPortUnknown': 'KernelException',
        'MUSICChannelAlreadyMapped': 'KernelException'
    }


# So we don't break any code that currently catches a nest.NESTError
NESTError = NESTErrors.NESTError
