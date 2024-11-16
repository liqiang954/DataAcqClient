QT       += core gui opengl serialbus serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    config.cpp \
    config_modbusdialog.cpp \
    const.cpp \
    loghandler.cpp \
    main.cpp \
    mainwindow.cpp \
    modbusmaster.cpp \
    qcustomplot.cpp \
    threadsafe_queue.cpp

HEADERS += \
    config.h \
    config_modbusdialog.h \
    const.h \
    loghandler.h \
    mainwindow.h \
    modbusmaster.h \
    qcustomplot.h \
    threadsafe_queue.h

FORMS += \
    config_modbusdialog.ui \
    mainwindow.ui

#...
DEFINES += QCUSTOMPLOT_USE_OPENGL  		#使用opengl
#INCLUDEPATH += $$PWD/freeglut/include	#依赖freeglut库以及头文件
LIBS += -L -llibfreeglut -lopengl32     #依赖opengl库文件
#LIBS += -L$$PWD/freeglut/lib/x64  -lfreeglut

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    style.qrc
