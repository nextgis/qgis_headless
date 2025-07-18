import os
import subprocess
import sys
from os import makedirs, path

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

here = path.abspath(path.dirname(__file__))


class CMakeExtension(Extension):
    def __init__(self, name):
        super().__init__(name, sources=[])


class CMakeBuild(build_ext):
    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)

    def build_cmake(self, ext):
        if not os.path.isdir(self.build_temp):
            makedirs(self.build_temp)

        extdir = path.abspath(path.dirname(self.get_ext_fullpath(ext.name)))

        config = "Debug" if self.debug else "Release"
        cmake_args = [
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DCMAKE_BUILD_TYPE={config}",
        ]
        extra_cmake_args = os.environ.get("CMAKE_ARGS", "")
        if extra_cmake_args:
            cmake_args += extra_cmake_args.split()

        env = os.environ.copy()
        subprocess.check_call(
            ["cmake", here] + cmake_args,
            cwd=self.build_temp,
            env=env,
        )

        if self.dry_run:
            return

        build_args = ["--config", config, "--", "-j2"]
        subprocess.check_call(
            ["cmake", "--build", "."] + build_args,
            cwd=self.build_temp,
            env=env,
        )


setup(
    ext_modules=[CMakeExtension("_qgis_headless")],
    cmdclass=dict(build_ext=CMakeBuild),
)
