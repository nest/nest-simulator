try :
    import os, distutils.sysconfig as sysc
    if os.path.isfile(sysc.get_python_inc() + "/Python.h") :
        print "yes"
    else :
        print "no"
except ImportError, Exception:
    print "no"
