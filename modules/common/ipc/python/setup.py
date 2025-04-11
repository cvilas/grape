from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "grape_ipc",
        ["py_bindings.cpp"],  
        include_dirs=["../include"],
        extra_compile_args=["-std=c++17"], 
    ),
]

setup(
    name="grape_ipc",
    version="0.1",
    author="Vilas Chitrakaran",
    author_email="cvilas@gmail.com",
    description="Python bindings for grape::ipc",
    long_description="",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.6",
)