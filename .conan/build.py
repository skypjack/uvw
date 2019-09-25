#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import platform
from cpt.packager import ConanMultiPackager


def get_version():
    with open("CMakeLists.txt") as cmake:
        content = cmake.read()
        match = re.search(r'project\(uvw VERSION (.*)\)', content)
        if match:
            return match.group(1)
        tag_version = os.getenv("GITHUB_REF")
        package_version = tag_version.replace("refs/tags/v", "")
        return package_version

def get_username():
    return os.getenv("CONAN_USERNAME", "skypjack")


def get_reference():
    version = get_version()
    username = get_username()
    return "uvw/{}@{}/stable".format(version, username)


def get_upload():
    username = get_username()
    url = "https://api.bintray.com/conan/{}/conan".format(username)
    default_upload = url if os.getenv("GITHUB_REF") else False
    return os.getenv("CONAN_UPLOAD", default_upload)


def upload_when_stable():
    return os.getenv("CONAN_UPLOAD_ONLY_WHEN_STABLE", "1").lower() not in ["0", "false", "no"]


if __name__ == "__main__":
    test_folder = os.path.join(".conan", "test_package")
    builder = ConanMultiPackager(reference=get_reference(),
                                 username=get_username(),
                                 upload=get_upload(),
                                 test_folder=test_folder,
                                 stable_branch_pattern=r'v?\d+\.\d+\.\d+.*',
                                 upload_only_when_stable=upload_when_stable())
    if platform.system() == "Linux":
        builder.add(settings={"compiler": "gcc", "compiler.version": "8",
                            "arch": "x86_64", "build_type": "Release"},
                    options={}, env_vars={}, build_requires={})
    builder.run()
