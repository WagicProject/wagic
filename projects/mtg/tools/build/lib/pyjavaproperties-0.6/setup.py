import os
import sys
try:
    import ez_setup
    ez_setup.use_setuptools()
except ImportError:
    pass
from setuptools import setup

# Use a cute trick to include the rest-style docs as the long_description
# therefore having it self-doc'ed and hosted on pypi
f = open(os.path.join(os.path.dirname(__file__), 'README'))
long_description = f.read().strip()
f.close()

setup(
    name='pyjavaproperties',
    version='0.6',
    author='Jesse Noller',
    author_email = 'jnoller@gmail.com',
    description = 'Python replacement for java.util.Properties.',
    long_description = long_description,
    url='http://pypi.python.org/pypi/pyjavaproperties',
    license = 'PSF License',
      classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: Apache Software License',
        'Topic :: Software Development :: Libraries',
        'Topic :: Software Development :: Libraries :: Python Modules',
      ],
    py_modules=['pyjavaproperties'],
    packages=[''],
    package_dir={'': '.'},
    )
