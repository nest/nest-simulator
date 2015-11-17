##Helping tools for the NEST documentation

###Static documentation in .md files

In future static documentation will be created here in github markdown.  


###Pynest Examples

####generator/generate_example2html.py
Extract documentation and code from the PYNEST examples (in pynest/examples/) and combines everything into HTML files.
**Usage:**
```
cd generator/
python generate_example2html.py
```
The examples needs to fit the documentation guidelines (see trac.717 for details).

Target folder: doc/extras/userdoc/build/examples

####generator/generate_example2ipynb.py
Extract documentation and code from the PYNEST examples (in pynest/examples/) and combines everything into ipython notebook files.

**Usage:**
```
cd generator/
python generate_example2ipynb.py
```
Target folder: doc/extras/userdoc/build/examples/examples

After running the tool go to doc/extras/userdoc/build/examples/examples and type:
```
ipython notebook
```
More about ipython notebooks: http://ipython.org/notebook.html

####generator/generate_example2png.sh
Generate the pictures for the python examples in a simple loop.
Please note: needs a fully [installed NEST](http://www.nest-simulator.org/installation/) in an X11 enviornment!
It is possible to plot without X11. Then the py files needs a little modification. See here:
http://matplotlib.org/faq/howto_faq.html#matplotlib-in-a-web-application-server

**Usage:**
```
cd generator/
sh generate_example2png.sh
```
The tool is asking for the path to your NEST installation.

Target folder: doc/extras/userdoc/build/examples/examples


###NEST SLI command index

####generator/generate_help2jsonhtml.py

The tool does two things:

1. Extract the whole documentation from the code files (".sli", ".cpp", ".cc", ".h") and write it to HTML.
2. Extract the whole documentation from the code files (".sli", ".cpp", ".cc", ".h") and write it to JSON. In addition, there is a small HTML site to display these JSON files. Go to doc/extras/userdoc/build/json and open index.html.
Please note: With Firefox it works out of the box. Chrome users have to start a simple http server. (cd into the doc/extras/userdoc/build/json folder and enter: python -m SimpleHTTPServer (python3 -m http.server)).

**Usage:**
```
cd generator/
python generate_help2jsonhtml.py
```

Target folders: doc/extras/userdoc/build/reference and doc/extras/userdoc/build/json
