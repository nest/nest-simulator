from .ll_api import sr, stack_checker, sps, spp


class KernelAttribute:
    """
    Descriptor that dispatches attribute access to the nest kernel.
    """
    def __init__(self, typehint, description, readonly=False, default=None, localonly=False):
        self._readonly = readonly
        self._localonly = localonly
        self._default = default

        readonly = readonly and "**read only**"
        localonly = localonly and "**local only**"

        self.__doc__ = (
            description
            + ("." if default is None else f", defaults to ``{default}``.")
            + ("\n\n" if readonly or localonly else "")
            + ", ".join(c for c in (readonly, localonly) if c)
            + f"\n\n:type: {typehint}"
        )

    def __set_name__(self, cls, name):
        self._name = name
        self._full_status = name == "kernel_status"

    @stack_checker
    def __get__(self, instance, cls=None):
        if instance is None:
            return self

        sr('GetKernelStatus')
        status_root = spp()

        if self._full_status:
            return status_root
        else:
            return status_root[self._name]

    @stack_checker
    def __set__(self, instance, value):
        if self._readonly:
            msg = f"`{self._name}` is a read only kernel attribute."
            raise AttributeError(msg)
        sps({self._name: value})
        sr('SetKernelStatus')