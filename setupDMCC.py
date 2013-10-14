#
# SetupDMCC.py - setup the DMCC library
#

from distutils.core import setup, Extension

setup(
    ext_modules = [
        Extension("DMCC", sources=["DMCC-py.c","DMCC.c"]),
        ],
    )

