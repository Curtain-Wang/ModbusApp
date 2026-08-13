QT       += core gui serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    headfile.cpp \
    main.cpp \
    mainwindow.cpp \
    mylineedit.cpp \
    tform1.cpp \
    tform7.cpp \
    tformcali.cpp \
    tformcdtest.cpp \
    tformconfig1.cpp \
    tformdatarecord.cpp \
    tformdownload.cpp \
    tformsernum.cpp \
    voltcurchart.cpp

HEADERS += \
    headfile.h \
    mainwindow.h \
    mylineedit.h \
    tform1.h \
    tform7.h \
    tformcali.h \
    tformcdtest.h \
    tformconfig1.h \
    tformdatarecord.h \
    tformdownload.h \
    tformsernum.h \
    voltcurchart.h

FORMS += \
    mainwindow.ui \
    tform1.ui \
    tform7.ui \
    tformcali.ui \
    tformcdtest.ui \
    tformconfig1.ui \
    tformdatarecord.ui \
    tformdownload.ui \
    tformsernum.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
