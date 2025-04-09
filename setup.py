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

        build_args = ["--config", config, "--", "-j2"]
        env = os.environ.copy()

        subprocess.check_call(
            ["cmake", here] + cmake_args,
            cwd=self.build_temp,
            env=env,
        )

        if not self.dry_run:
            subprocess.check_call(
                ["cmake", "--build", "."] + build_args,
                cwd=self.build_temp,
                env=env,
            )

        self._generate_stubs(extdir, ext.name)

    def _generate_stubs(self, extdir: str, modname: str) -> None:
        """
        Generate .pyi stubs using pybind11-stubgen.

        :param extdir: Directory containing the built module.
        :param modname: Module name, e.g. "_qgis_headless".
        """
        stubgen_cmd = [
            "pybind11-stubgen",
            modname,
            "--output-dir",
            extdir,
        ]
        if self.dry_run:
            stubgen_cmd.append("--dry-run")

        env = os.environ.copy()
        if "PYTHONPATH" in env:
            env["PYTHONPATH"] = os.pathsep.join((env["PYTHONPATH"], extdir))
        else:
            env["PYTHONPATH"] = extdir
        subprocess.check_call(stubgen_cmd, env=env)

        if self.dry_run:
            return

        ruff_check_cmd = [
            "ruff",
            "check",
            "--fix",
            "--unsafe-fixes",
            os.path.join(extdir, f"{modname}.pyi"),
        ]
        subprocess.check_call(ruff_check_cmd)

        ruff_format_cmd = [
            "ruff",
            "format",
            os.path.join(extdir, f"{modname}.pyi"),
        ]
        subprocess.check_call(ruff_format_cmd)


setup(
    ext_modules=[CMakeExtension("_qgis_headless")],
    cmdclass={"build_ext": CMakeBuild},
)
