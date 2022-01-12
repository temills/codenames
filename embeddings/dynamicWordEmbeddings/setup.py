from distutils.core import setup, Extension
from Cython.Build import cythonize
import numpy
sources = ['fast_inference.pyx', "dpwec/pwe.cpp", "dpwec/pweinference.cpp", "dpwec/pwelearn.cpp"]
include_path = [ numpy.get_include()]

setup(ext_modules = cythonize(Extension(
               "fast_inference",                                # the extension name
               sources=sources,                                  # the Cython source and
               include_dirs = include_path,
               language="c++",                        # generate and compile C++ code
                extra_compile_args=['-O3']

)))

