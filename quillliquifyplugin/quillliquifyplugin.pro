TEMPLATE        = lib
QMAKE_LFLAGS   += -Wl,--as-needed
QMAKE_CXXFLAGS += -Werror -Wall -W
CONFIG         += plugin quillimagefilter

HEADERS += quillliquifyplugin.h
SOURCES += quillliquifyplugin.cpp

target.path = $$[QT_INSTALL_PLUGINS]/quillimagefilters
INSTALLS += target
