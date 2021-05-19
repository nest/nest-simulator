import re
import os
import sys
from docutils import nodes
from docutils.parsers.rst import Directive, directives


class HoverXTooltipDirective(Directive):
    """Directive to add a tooltip.

    Attributes:
        option_spec (dict): Specification of objects in directive.
        optional_arguemtns (int): Number of optional arguements.
        required_arguments (int): Number of required arguements.
    """

    required_arguments = 0
    optional_arguemtns = 0
    option_spec = {'term': directives.unchanged, 'desc': directives.unchanged}

    def run(self):
        """Generates tooltip by defining text and tooltip content
        explicitly.

        Returns:
            list: Returns list of nodes.
        """

        # Retrieve terms from the directive in rst file.
        options = self.options
        term = options["term"]
        desc = options["desc"]

        # the tag in which the term and description is defined.
        span_tag = "<span class='hoverxtool' data-toggle='tooltip' " \
            "data-placement='top' title='{desc}'>" \
            "{term}</span>".format(desc=desc, term=term)

        # the docutils object that holds the tag as html code.
        hover_raw_node = nodes.raw(text=span_tag, format='html')

        # the jquery function to enable hovering.
        jquery_tag = "<script type='text/javascript'>" \
            "$('span[data-toggle=tooltip]').tooltip({" \
            "animated: 'fade'," \
            "delay: {show: 100, hide: 1500}," \
            "placement: 'top', }); </script>"
        js_raw_node = nodes.raw(text=jquery_tag, format='html')

        return [hover_raw_node, js_raw_node]


def hxt_role(pattern):
    """Generates tooltip by defining text and tooltip content explicitly.

    Args:
        pattern (str): placeholder for term and description.

    Returns:
        function: returns its own function.
    """
    def role(name, rawtext, text, lineno, inliner, options={}, content=[]):
        #node = nodes.reference(rawtext, text, refuri=url, **options)

        # example: iaf <hxt:integrate and fire>
        raw_params = pattern % (text,)

        # define regex pattern termplates to extract the term and description.
        term_template = "(.*?) <hxt:"
        desc_template = "<hxt:(.*?)>"

        try:
            # extract the term and description. This must be inside
            # a try block because we can't predict user input.
            term = re.search(term_template, raw_params).group(1)
            desc = re.search(desc_template, raw_params).group(1)

            # the tag in which the term and description is defined.
            span_tag = "<span class='hoverxtool hoverxtool_highlighter' " \
                "data-toggle='tooltip' " \
                "data-placement='top' title='{desc}'>" \
                "{term}</span>".format(desc=desc, term=term)

            # the docutils object that holds the tag as html code.
            hover_raw_node = nodes.raw(text=span_tag, format='html')

            # the jquery function to enable hovering.
            jquery_tag = "<script type='text/javascript'>" \
                "$('span[data-toggle=tooltip]').tooltip({" \
                "animated: 'fade'," \
                "delay: {show: 100, hide: 1500}," \
                "placement: 'top', }); </script>"
            js_raw_node = nodes.raw(text=jquery_tag, format='html')

            return [hover_raw_node, js_raw_node], []

        except Exception as e:
            print("Something went wrong while parsing the hxt pattern: ({e})"
                  .format(e))
            sys.exit(-1)

    return role


def hxt_role_ref(pattern):
    """Generates tooltip based on glossary.rst file.

    Args:
        pattern (str): placeholder for term from rst file where it was defined.

    Returns:
        function: returns its function pointer.
    """
    def role(name, rawtext, text, lineno, inliner, options={}, content=[]):

        term = pattern % (text,)
        desc = get_desc_from_glossary(term)

        # link to the glossary term.

        # use this for local builds.
        # refuri = (f'{os.getcwd()}/userdoc/html/glossary.html#term-{term}')
        refuri = (f'glossary.html#term-{term}')

        # the tag in which the term and description is defined.
        ref_tag = "<a class='reference external' " \
                  "href='{refuri}'>" \
                  "<span class='hoverxtool hoverxtool_highlighter' " \
                  "data-toggle='tooltip' " \
                  "data-placement='top' title='{desc}'>" \
                  "{term}</span>" \
                  "</a>" \
                  .format(refuri=refuri, desc=desc, term=term)

        # the docutils object that holds the tag as html code.
        ref_node = nodes.raw(text=ref_tag, format='html')

        # the jquery function to enable hovering.
        # Note this should be added once!
        jquery_tag = "<script type='text/javascript'>" \
                     "$('span[data-toggle=tooltip]').tooltip({" \
                     "animated: 'fade'," \
                     "delay: {show: 100, hide: 1500}," \
                     "placement: 'top', }); </script>"
        js_raw_node = nodes.raw(text=jquery_tag, format='html')

        return [ref_node, js_raw_node], []
    return role


class Memoize:
    """Caches funtion output based on the parameters."""

    def __init__(self, fn):
        self.fn = fn
        self.memo = {}

    def __call__(self, *args):
        """Checks if memo dict already has args.
        If it is not available, call function and store args/return values,
        otherwise return the function return values directly.

        Args:
            *str: Function arguements.

        Returns:
            TYPE: Function return values.
        """
        if args not in self.memo:
            self.memo[args] = self.fn(*args)
        return self.memo[args]


@Memoize
def get_desc_from_glossary(term):
    """Parses glossary.rst file.

    Args:
        term (str): the term in the glossary list.

    Returns:
        str: the description of the term in the glossary list.
    """

    try:
        with open('./userdoc/glossary.rst') as f:
            file_content = f.read()

        # generate a list of lines from file content.
        raw_file_content = list(filter(None, file_content
                                .split('Glossary')[1].splitlines(True)))

        glossary_dict = {}  # dictionary that holds terms and descriptions.
        for idx, line in enumerate(raw_file_content):
            # detect a term based on value of first column.
            if len(line) > 1 and line[1] is not ' ':
                # the key is the term in the dictionary.
                key = line.strip('\n')
                # the value is the description (which is on the next line).
                val = raw_file_content[idx+1].strip('\n')

                glossary_dict[key[1:]] = val

        return glossary_dict[term]
    except Exception as e:
        return f'Description Unavailable: {e}'


def setup(app):
    """Adds the necessary routines to Sphinx.

    Args:
        app (TYPE): Description

    Returns:
        TYPE: Description
    """
    # add external css/js files
    app.add_js_file('js/jquery-2.0.3.min.js')
    app.add_js_file('js/bootstrap.min.js')
    app.add_css_file('css/bootstrap.min.css')

    # add custom css file
    app.add_css_file('css/hoverxtooltip.css')

    # add the HoverXTooltipDirective
    app.add_directive("hxt_directive", HoverXTooltipDirective)

    # add the role that generates explicit text and tooltip.
    app.add_role('hxt', hxt_role('%s'))

    # add the role that generates tooltips based on glossary.rst.
    app.add_role('hxt_ref', hxt_role_ref('%s'))

    return {
        'version': '0.1',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
