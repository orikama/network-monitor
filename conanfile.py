from conans import ConanFile

class ConanPackage(ConanFile):
    name = 'network-manager'
    version = '0.1.0'

    generators = 'cmake_find_package'

    requires = []
