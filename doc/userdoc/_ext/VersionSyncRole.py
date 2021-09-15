import json
from docutils import nodes


def version_role(pattern):
    def role(name, rawtext, text, lineno, inliner, options={}, content=[]):

        package_level = pattern % text
        payload = package_level.split(',')
        package = payload[0]

        if len(payload) > 1:
            level = payload[1]
        else:
            level = "min"

        with open('./userdoc/_ext/versions.json') as fp:
            data = json.load(fp)

        # version = data[package.strip()][level.strip()]
        try:
            version = data[package.strip()][level.strip()]
        except KeyError as e:
            version = f"'{level}' was not found!"

        node = nodes.Text(version)
        return [node], []
    return role


# def version_level_role(pattern, level):
#     def role(name, rawtext, text, lineno, inliner, options={}, content=[]):

#         package = pattern % text

#         with open('./userdoc/_ext/versions.json') as fp:
#             data = json.load(fp)

#         version = data[package.strip()][level.strip()]

#         node = nodes.Text(version)
#         return [node], []
#     return role


def setup(app):
    """Adds the necessary routines to Sphinx.

    Args:
        app (TYPE): Description

    Returns:
        TYPE: Description
    """
    # add the role
    app.add_role('version', version_role('%s'))
    # app.add_role('minversion', version_level_role('%s', 'min_version'))
    # app.add_role('maxversion', version_level_role('%s', 'max_version'))

    return {
        'version': '0.1',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
