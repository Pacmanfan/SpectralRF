#-------------------------------------------------
#
# Project created by QtCreator 2020-05-24T22:59:51
#
#-------------------------------------------------

QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SpectralRF
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
LIBS += -lboost_system -lfftw3f -lSoapySDR -lpthread

CONFIG += c++11

SOURCES += \
    fft_fftw.cpp \
    fft_hist.cpp \
    ftmarker.cpp \
    ftmarker_gui.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    sweeper.cpp \
    wgt_fft.cpp \
    wgt_util.cpp \
    wgt_waterfall.cpp \
    wgt_marker_editor.cpp \
    wgt_marker_table.cpp \
    wgt_sig_base.cpp \
    radiosettings.cpp \
    sdr_device.cpp \
    sdr_device_manager.cpp \
    sigtuner_gui.cpp \
    sweepfile.cpp \
    signaldetector.cpp


HEADERS += \
    fft_fftw.h \
    fft_hist.h \
    ftmarker.h \
    ftmarker_gui.h \
    mainwindow.h \
    qcustomplot.h \
    sweeper.h \
    wgt_fft.h \
    wgt_sig_base.h \
    wgt_util.h \
    wgt_waterfall.h \
    wgt_marker_editor.h \
    wgt_marker_table.h \
    radiosettings.h \
    sdr_device.h \
    sdr_device_manager.h \
    sigtuner_gui.h \
    sinetable.h \
    sweepfile.h \
    signaldetector.h


FORMS += \
    mainwindow.ui \
    wgt_fft.ui \
    wgt_waterfall.ui \
    wgt_marker_editor.ui \
    wgt_marker_table.ui \
    radiosettings.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Notes.txt \
    src/Notes.txt \
    README.md

RESOURCES += \
    images.qrc
