#-------------------------------------------------
#
# Project created by QtCreator 2019-06-17T03:26:07
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = ALLHUI
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += SHARED_BUILT_AS_STATIC


SOURCES += main.cpp \
    Vector.cpp \
    solver.cpp \
    Set.cpp \
    Problem.cpp \
    Compact.cpp

HEADERS += \
    Vector.h \
    Solver.h \
    SHARED_EXPORT.h \
    Set.h \
    Problem.h \
    IVector.h \
    ISolver.h \
    ISet.h \
    IProblem.h \
    ILog.h \
    ICompact.h \
    IBrocker.h \
    error.h \
    Compact.h

QMAKE_CXXFLAGS += -std=gnu++0x

LIBS += "$$PWD/ILog.dll"









