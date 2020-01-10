#!/usr/bin/env python3

from setuptools import setup


def readme():
    with open("README.md") as fd:
        return fd.read()

setup(name="flippy",
      version="0.0.1",
      description="asdf",
      author="Carl Fabian Luepke",
      author_email="gitlab@luepke.email",
      url="https://gitlab.com/fluepke/fluepdot",
      long_description=readme(),
      license="AGPLv3",
      packages=["flippy", ],
      entry_points = {'console_scripts': [
          'bitmap2program=tools.bitmap2program:cli',
          'text2program=tools.text2program:cli',
          'flippy=flippy.main:cli'], },
      install_requires=["pyserial", "cobs", "pillow", "crcmod", "bitarray", "cffi"],
      python_requires='>=3',
      cffi_modules=["flippy/framebuffer_print_build.py:BUILDER"],
)
