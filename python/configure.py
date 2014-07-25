#!/usr/bin/env python
import sipconfig
import subprocess
import os

from distutils import sysconfig

def commandOutput(cmd, arguments):
    return subprocess.check_output([cmd] + arguments).strip().decode('utf-8')

def getEnv(name, default):
    return os.environ.get(name) or default

class Config:
    def __init__(self, qmakePath):
        self.__qmakePath = qmakePath

        qtVersion = self.qmakeVariable('QT_VERSION')
        self.__hasQt4 = qtVersion.startswith('4.')
        self.__hasQt5 = qtVersion.startswith('5.')

        if self.__hasQt5:
            from PyQt5.QtCore import PYQT_CONFIGURATION
            self.__sipFlags = PYQT_CONFIGURATION['sip_flags']
        elif self.__hasQt4:
            try:
                from PyQt4.QtCore import PYQT_CONFIGURATION
                self.__sipFlags = PYQT_CONFIGURATION['sip_flags']
            except ImportError:
                from PyQt4 import pyqtconfig
                config = pyqtconfig.Configuration()
                self.__sipFlags = config.pyqt_sip_flags
        else:
            raise Exception('Qt version 4 or 5 is required!')

    def qmakeOutput(self, *arguments):
        return commandOutput(self.__qmakePath, list(arguments))

    def qmakeVariable(self, name):
        return self.qmakeOutput('-query', name)

    def sipFlags(self):
        return self.__sipFlags

    def hasQt4(self):
        return self.__hasQt4

    def hasQt5(self):
        return self.__hasQt5

def main():
    qmakePath = getEnv('QMAKE', 'qmake')
    sipPath = getEnv('SIP', 'sip')

    sipConfig = sipconfig.Configuration()
    config = Config(qmakePath)

    projectPath = getEnv('PROJECT_PATH', '../..')
    libraryPath = getEnv('LIBRARY_PATH', projectPath + '/fakevim')
    sipFilePath = getEnv('SIP_FILE_PATH', projectPath + '/python/fakevim.sip')
    pyQtIncludePath = getEnv('PYQT_INCLUDE_PATH',
            '/usr/share/sip/PyQt' + (config.hasQt5() and '5' or '4'))

    commandOutput(sipPath, config.sipFlags().split(' ') + [
        '-I', pyQtIncludePath,
        '-b', 'fakevim_python.pro',
        '-o', '-c', '.',
        sipFilePath
        ])

    with open('fakevim_python.pro', 'a') as pro:
        pro.write(
        '''\
        TEMPLATE = lib
        CONFIG += release plugin
        QT += widgets

        TARGET = $${{target}}
        HEADERS = $${{headers}}
        SOURCES = $${{sources}}

        INCLUDEPATH += "{sipInclude}" "{pythonInclude}" "{projectInclude}"
        LIBS += -Wl,-rpath,"{libraryPath}" -L"{libraryPath}" -lfakevim "{pythonLibrary}"
        DEFINES += FAKEVIM_PYQT_MAJOR_VERSION={qtVersion}
        '''.format(
            pythonInclude = sysconfig.get_python_inc(),
            sipInclude = sipConfig.sip_inc_dir,
            projectInclude = projectPath,
            libraryPath = libraryPath,
            pythonLibrary = sysconfig.get_config_var('LIBDIR') + "/" + sysconfig.get_config_var('LDLIBRARY'),
            qtVersion = config.hasQt5() and 5 or 4
            )
        )

if __name__ == "__main__":
    main()

