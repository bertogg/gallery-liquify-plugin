TEMPLATE        = lib
QMAKE_LFLAGS   += -Wl,--as-needed
QMAKE_CXXFLAGS += -Werror -Wall -W
CONFIG         += plugin meegotouch gallerycore quillimagefilter

HEADERS += galleryliquifyplugin.h
SOURCES += galleryliquifyplugin.cpp

target.path = /usr/lib/gallery
INSTALLS += target
