import re
from pprint import pprint
import os

def UserDocExtractor(modelfiles, basedir="..", nameext='.h'):
    rst_dir = "from_cpp/"

    userdoc_re = re.compile(r'BeginUserDocs:?\s*(?P<tags>(\w+(,\s*)?)*)\n+(?P<doc>(.|\n)*)EndUserDocs')
    tagdict = dict()    # map tags to lists of documents
    for model in [os.path.splitext(m)[0] for m in modelfiles]:
        with open(os.path.join(basedir, model + nameext)) as f:
            match = userdoc_re.search(f.read())
            if match:
                if not os.path.exists(rst_dir):
                    print("INFO: creating output directory "+rst_dir)
                    os.mkdir(rst_dir)
                rst_fname = os.path.basename(model) + ".rst"
                with open(os.path.join(rst_dir, rst_fname), "w") as rst_file:
                    rst_file.write(match.group('doc'))
                    print("INFO: Wrote model documentation for model " + model)
                tags = [t.strip() for t in match.group('tags').split(',')]
                for tag in tags:
                    tagdict.setdefault(tag, list()).append(rst_fname)
            else:
                print("WARNING: No documentation found for model " + model)
    return tagdict

if __name__ == '__main__':
    models_with_documentation = (
        "models/multimeter",
        "models/spike_detector",
        "models/weight_recorder",
        "nestkernel/recording_backend_ascii",
        "nestkernel/recording_backend_memory",
        "nestkernel/recording_backend_screen",
        "nestkernel/recording_backend_sionlib",
    )

    tags = UserDocExtractor(models_with_documentation)
    pprint(tags)
