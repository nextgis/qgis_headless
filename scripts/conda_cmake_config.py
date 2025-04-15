#!/usr/bin/env python

import json
import os
import sys
from pathlib import Path


def is_conda_environment() -> bool:
    return bool(os.environ.get("CONDA_PREFIX"))


def get_conda_paths() -> dict:
    prefix = Path(os.environ["CONDA_PREFIX"])
    return {
        "cmake": str(prefix / "Library" / "bin" / "cmake.exe"),
        "bin": str(prefix / "Library" / "bin"),
        "include": str(prefix / "Library" / "include"),
        "lib": str(prefix / "Library" / "lib"),
        "lib_qgis_core": str(prefix / "Library" / "lib" / "qgis_core.lib"),
        "python": sys.executable,
    }


def convert_path(path: str) -> str:
    return path.replace("/", "\\").replace("\\", "/")


def mode_ide(ide: str):
    if ide not in {"vscode"}:
        sys.exit(f"Unsupported IDE: {ide}")

    paths = get_conda_paths()
    vscode_dir = Path(".vscode")
    vscode_dir.mkdir(exist_ok=True)
    settings_path = vscode_dir / "settings.json"

    settings = {}
    if settings_path.exists():
        with open(settings_path, encoding="utf-8") as file:
            settings = json.load(file)

    settings["cmake.cmakePath"] = convert_path(paths["cmake"])
    settings["cmake.additionalCompilerSearchDirs"] = [convert_path(paths["bin"])]
    settings["cmake.configureSettings"] = {
        "CMAKE_INCLUDE_PATH": convert_path(paths["include"]),
        "CMAKE_LIBRARY_PATH": convert_path(paths["lib"]),
        "QGIS_CORE_INCLUDE_DIRS": convert_path(paths["include"]),
        "QGIS_CORE_LIBRARIES": convert_path(paths["lib_qgis_core"]),
        "PYTHON_EXECUTABLE": convert_path(paths["python"]),
        "CMAKE_C_COMPILER": f"{convert_path(paths['bin'])}/clang.exe",
        "CMAKE_CXX_COMPILER": f"{convert_path(paths['bin'])}/clang++.exe",
        "CMAKE_C_COMPILER_LAUNCHER": "ccache",
        "CMAKE_CXX_COMPILER_LAUNCHER": "ccache",
        "CMAKE_LIBRARY_OUTPUT_DIRECTORY": "${workspaceFolder}",
    }

    with open(settings_path, "w", encoding="utf-8") as file:
        json.dump(settings, file, indent=4)


def mode_env(shell: str):
    if shell not in {"bash", "cmd", "ps"}:
        sys.exit(f"Unsupported shell: {shell}")

    paths = get_conda_paths()
    args = {
        "CMAKE_INCLUDE_PATH": convert_path(paths["include"]),
        "CMAKE_LIBRARY_PATH": convert_path(paths["lib"]),
        "QGIS_CORE_INCLUDE_DIRS": convert_path(paths["include"]),
        "QGIS_CORE_LIBRARIES": convert_path(paths["lib_qgis_core"]),
        "PYTHON_EXECUTABLE": convert_path(paths["python"]),
        "CMAKE_C_COMPILER": f"{convert_path(paths['bin'])}/clang.exe",
        "CMAKE_CXX_COMPILER": f"{convert_path(paths['bin'])}/clang++.exe",
        "CMAKE_C_COMPILER_LAUNCHER": "ccache",
        "CMAKE_CXX_COMPILER_LAUNCHER": "ccache",
    }

    joined_args = " ".join(f"-D{k}={v}" for k, v in args.items()) + " -GNinja"

    if shell == "bash":
        print(f'export CMAKE_ARGS="{joined_args}"')
    elif shell == "ps":
        print(f'$env:CMAKE_ARGS = "{joined_args}"')
    elif shell == "cmd":
        print(f"set CMAKE_ARGS={joined_args}")


if __name__ == "__main__":
    if not is_conda_environment():
        sys.exit("Error: Not in a conda environment")

    if len(sys.argv) < 3:
        sys.exit(f"Usage: {sys.argv[0]} <mode> <arg>")

    mode, arg = sys.argv[1], sys.argv[2]

    if mode == "ide":
        mode_ide(arg)
    elif mode == "env":
        mode_env(arg)
    else:
        sys.exit(f"Unknown mode: {mode}")
