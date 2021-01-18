import nest

# import os, contextlib
# import ctypes

# from contextlib import contextmanager
# import ctypes
# import io
# import os, sys
# import tempfile

# libc = ctypes.CDLL(None)
# c_stdout = ctypes.c_void_p.in_dll(libc, '__stdoutp')

# @contextmanager
# def stdout_redirector(stream):
#     # The original fd stdout points to. Usually 1 on POSIX systems.
#     original_stdout_fd = sys.stdout.fileno()

#     def _redirect_stdout(to_fd):
#         """Redirect stdout to the given file descriptor."""
#         # Flush the C-level buffer stdout
#         libc.fflush(c_stdout)
#         # Flush and close sys.stdout - also closes the file descriptor (fd)
#         sys.stdout.close()
#         # Make original_stdout_fd point to the same file as to_fd
#         os.dup2(to_fd, original_stdout_fd)
#         # Create a new sys.stdout that points to the redirected fd
#         sys.stdout = io.TextIOWrapper(os.fdopen(original_stdout_fd, 'wb'))

#     # Save a copy of the original stdout fd in saved_stdout_fd
#     saved_stdout_fd = os.dup(original_stdout_fd)
#     try:
#         # Create a temporary file and redirect stdout to it
#         tfile = tempfile.TemporaryFile(mode='w+b')
#         _redirect_stdout(tfile.fileno())
#         # Yield to caller, then redirect stdout back to the saved fd
#         yield
#         _redirect_stdout(saved_stdout_fd)
#         # Copy contents of temporary file to the given stream
#         tfile.flush()
#         tfile.seek(0, io.SEEK_SET)
#         stream.write(tfile.read())
#     finally:
#         tfile.close()
#         os.close(saved_stdout_fd)

# with stdout_redirector(io.BytesIO()):
#     import nest

# with open(os.devnull, "w") as f, contextlib.redirect_stdout(f):
#     import nest

def createReducedNestModel(ctree):
    n_neat = nest.Create('iaf_neat')

    for node in ctree:
        p_ind = -1 if ctree.isRoot(node) else node.parent_node.index

        nest.AddCompartment(n_neat, node.index, p_ind,
                            {'C_m': node.ca*1e3,
                             'g_c': node.g_c,
                             'g_L': node.currents['L'][0],
                             'E_L': node.currents['L'][1]
                            }
                           )

    return n_neat