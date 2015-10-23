import os
import sys

excludes=["autom4te.cache", "debian", "lib", "libltdl"]

def all_includes(path):
    result = {}
    dirs = [d for d in next(os.walk(path))[1] if d[0] != '.' and not d in excludes ]
    for d in dirs:
        for root, dirs, files in os.walk(path + "/" + d):
            tmp = [f for f in files if f.endswith(".h") or f.endswith(".hpp")]
            if len(tmp) > 0:
                result[d] = tmp

    return result

def get_includes_from(file):
    includes =
    with open(file, 'r') as f:
        for line in f:



if __name__ == '__main__':
    print all_includes(sys.argv[1])
