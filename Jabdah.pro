#-------------------------------------------------
#
# Project created by QtCreator 2013-07-04T13:27:27
#
#-------------------------------------------------

QT += core gui winextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Jabdah
TEMPLATE = app


SOURCES += main.cpp\
        jabdah.cpp \
    about.cpp \
    preferencesdialog.cpp \
    settings.cpp \
    definitions.cpp \
    addfolders.cpp \
	playlist.cpp

HEADERS  += jabdah.h \
    about.h \
    preferencesdialog.h \
    settings.h \
    definitions.h \
    addfolders.h \
	playlist.h

FORMS    += jabdah.ui \
    about.ui \
    preferencesdialog.ui \
    addfolders.ui \
	playlist.ui

unix|win32: LIBS += -L$$PWD/bass/lib/ -lbass
win32: LIBS += -L$$PWD/taglib/ -ltaglib
unix: LIBS += -ltag

INCLUDEPATH += $$PWD/bass/include
DEPENDPATH += $$PWD/bass/include

INCLUDEPATH += $$PWD/taglib/include

OTHER_FILES +=

RESOURCES += \
    images.qrc

RC_FILE = icon.rc
