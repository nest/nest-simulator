from setuptools import setup
from setuptools.command.install import install
import os, sys, subprocess

class install_command(install):
    def initialize_options(self):
        install.initialize_options(self)

    def finalize_options(self):
        install.finalize_options(self)

    def run(self):
        env = os.environ.copy()
        subprocess.check_call(f"alias python='{sys.executable}'; cmake data; make install", env=env, shell=True)
        install.run(self)

setup(
    name='nest-simulator',
    version='3.0',
    description='Python bindings for NEST',
    author='The NEST Initiative',
    author_email="hi@gmail.com",
    url='https://www.nest-simulator.org',
    license='GPLv2+',
    packages=['nest', 'nest.tests', 'nest.tests.test_sp',
              'nest.tests.test_spatial', 'nest.lib'],
    package_dir={"": "pynest"},
    install_requires=['numpy', 'scipy'],
    extras_require={
        'test': ['nose', 'matplotlib']
    },
    cmdclass={
        'install':     install_command,
    },
    classifiers=[
        'Development Status :: 6 - Mature',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Intended Audience :: Science/Research',
        'Topic :: Scientific/Engineering',
        'Topic :: Scientific/Engineering :: Artificial Intelligence',
    ],
    python_requires='>=3.8, <4',
    keywords=(
        'nest,'
        + 'simulator,'
        + 'neuroscience,'
        + 'neural,'
        + 'neuron,'
        + 'network,'
        + 'ai,'
        + 'spike,'
        + 'spiking'
    ),
    project_urls={
        'Homepage': 'https://www.nest-simulator.org/',
        'Bug Reports': 'https://github.com/nest/nest-simulator/issues',
        'Source': 'https://github.com/nest/nest-simulator',
        'Documentation': 'https://nest-simulator.readthedocs.io/'
    },
)
