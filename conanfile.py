#!/usr/bin/env python
# -*- coding: utf-8 -*-
from conans import ConanFile


class UVWConan(ConanFile):
    name = "uvw"
    description = "Header-only, event based, tiny and easy to use libuv wrapper in modern C++"
    homepage = "https://github.com/skypjack/uvw"
    url = homepage
    license = "MIT"
    topics = ("conan", "uvw", "libuv", "header-only", "wrapper", "event-loop")
    author = "Michele Caini <michele.caini@gmail.com>"
    exports = "LICENSE"
    exports_sources = "src/*"
    no_copy_source = True
    requires = "libuv/1.44.1@bincrafters/stable"

    def package(self):
        self.copy(pattern="LICENSE", dst="licenses")
        self.copy(pattern="*.hpp", dst="include", src="src")

    def package_id(self):
        self.info.header_only()
