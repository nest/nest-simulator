import os
import glob
import re
from nbconvert import PythonExporter 


# Generate a link to the repository and jupyterhub see nblinkpuller https://jupyterhub.github.io/nbgitpuller/link.html for each notebook

link_puller = "https://lab.ebrains.eu/hub/user-redirect/git-pull?repo=https%3A%2F%2Fgithub.com%2Fjessica-mitchell%2Fnest-simulator&urlpath=lab%2Ftree%2Fnest-simulator%2Fpynest%2Fexamples%2Fone_neuron_with_noise.ipynb&branch=ebrains-button"

filepath = "../../pynest/examples/"

md_ebrains_button = "[![EBRAINS Notebook](https://nest-simulator.org/TryItOnEBRAINS.png)](nblink)\n",

for filename in glob.glob(os.path.join(filepath, "*.ipynb")):
    name = os.path.basename(filename)
    links = link_puller.replace('one_neuron_with_noise.ipynb', name)
    button = md_ebrains_button.replace('nblink', links)

#With this function I can create one cell

#def create_new_cell(contents):
#    from IPython.core.getipython import get_ipython
#    shell = get_ipython()
#    shell.set_next_input(contents, replace=False)



## need to store these - then add them to the notebook in markdowO
#    exporter = PythonExporter()
#    print(filename)
#    (source, meta) = exporter.from_filename(filename)
#
#    source = source.replace("# In[ ]:", "")
#    source = source.replace("# \n", "")
#    base = os.path.splitext(name)[0]
#    pyname = base + '.py'
#
#    with open('test-notebook/' + pyname, 'w') as outfile:
#        outfile.write(source)



file_name = "../../pynest/examples/structural_plasticity.ipynb"
name = os.path.basename(file_name)
# need to store these - then add them to the notebook in markdown
exporter = PythonExporter()
(source, meta) = exporter.from_filename(file_name)

source = source.replace("# In[ ]:", "")
source = source.replace("# \n", "")
base = os.path.splitext(name)[0]
pyname = base + '.py'

with open('test-notebook/' + pyname, 'w') as outfile:
    outfile.write(source)


#for filename in glob.glob(os.path.join(filepath, "*.ipynb*")):



#Insert notebook name in link

#Use jinja templating? to fill in correct notebook for each page (does this work in a notebook?)
