import os
import re
import sys

excludes=["autom4te.cache", "debian", "lib", "libltdl"]

class IncludeInfo():
    filename = ""
    name = ""
    spiky = False
    origin = "a_unknown"

    def __init__(self, filename, name, spiky, all_headers):
        self.filename = filename
        self.name = name
        self.spiky = spiky
        self.set_origin(all_headers)

    def is_header_include(self):
        return ( self.name.split('.')[0] == self.filename.split('.')[0] or
                 self.name.split('.')[0] == self.filename.split('_impl.')[0] )

    def is_cpp_include(self):
        return not self.name.endswith('.h') and not self.name.endswith('.hpp') and self.spiky

    def is_c_include(self):
        return self.name.endswith('.h') and self.spiky

    def is_project_include(self):
        return not self.spiky and (self.name.endswith('.h') or self.name.endswith('.hpp'))

    def set_origin(self, includes):
        for k,v in includes.iteritems():
            if self.name in v:
                self.origin = k
                break

    def cmp_value(self):
        v = 8 if self.is_header_include() else 0
        v += 4 if self.is_c_include() else 0
        v += 2 if self.is_cpp_include() else 0
        v += 1 if self.is_project_include() else 0
        return v

    def __cmp__(self, other):
        s = self.cmp_value()
        o = other.cmp_value()
        val = o - s
        if val == 0:
            val = cmp( self.origin, other.origin )
            if val == 0:
                return cmp(self.name, other.name)
            else:
                return val
        else:
            return val

    def to_string(self):
        l_guard = '<' if self.spiky else '"'
        r_guard = '>' if self.spiky else '"'
        return '#include ' + l_guard + self.name + r_guard #+ " // from " + self.origin

def all_includes(path):
    result = {}
    dirs = [d for d in next(os.walk(path))[1] if d[0] != '.' and not d in excludes ]
    for d in dirs:
        for root, dirs, files in os.walk(path + "/" + d):
            tmp = [f for f in files if f.endswith(".h") or f.endswith(".hpp")]
            if len(tmp) > 0:
                result[d] = tmp

    return result

def create_include_info(line, filename, all_headers):
    match = re.search('^#include ([<"])(.*)([>"])', line)
    name = match.group(2)
    spiky = match.group(1) == '<'
    return IncludeInfo(filename, name, spiky, all_headers)

def get_includes_from(file, all_headers):
    includes = []
    with open(file, 'r') as f:
        for line in f:
            if line.startswith('#include'):
                includes += [create_include_info(line, os.path.basename(file), all_headers)]
    return includes

def is_include_order_ok(includes):
    return includes == sorted(includes)

def print_includes(includes):
    s_incs = sorted(includes)

    is_c = False
    is_cpp = False
    origin = ""

    for i in s_incs:
        if not i.is_header_include():
            if not is_c and i.is_c_include():
                is_c = True
                is_cpp = False
                origin = ""
                print("\n// C includes:")

            if not is_cpp and i.is_cpp_include():
                is_c = False
                is_cpp = True
                origin = ""
                print("\n// C++ includes:")

            if i.is_project_include() and origin != i.origin:
                is_c = False
                is_cpp = False
                origin = i.origin
                if i.origin == "a_unknown":
                    print("\n// Generated includes:")
                else:
                    print("\n// Includes from " + i.origin + ":")

        print(i.to_string())

def process_source(path, f, all_header, print_all=False, ok_silent=True):
    includes = get_includes_from(path + "/" + f, all_header)
    order_ok = is_include_order_ok(includes)
    if order_ok and not ok_silent:
        print("Includes for " + f + " are OK!")
    if not order_ok or print_all:
        print("\n##############################")
        print("Suggested includes for " + f + ":")
        print("##############################\n")
        print_includes(includes)
        print("\n##############################")

def process_all_sources(path, print_all=False, ok_silent=True):
    all_header = all_includes(path)

    dirs = [d for d in next(os.walk(path))[1] if d[0] != '.' and not d in excludes ]
    for d in dirs:
        for root, dirs, files in os.walk(path + "/" + d):
            for f in files:
                if re.search("\.h$|\.hpp$|\.c$|\.cc|\.cpp$",f):
                    # valid source file
                    process_source(root, f, all_header, print_all, ok_silent)


if __name__ == '__main__':
    process_all_sources( sys.argv[1], print_all=True )

