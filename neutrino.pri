TARGET = ../Neutrino

CONFIG += qt qwt debug_and_release windows

## -- folder definitions -- used also by files including this one
UIs_DIR = $$PWD/UIs
SRC_DIR = $$PWD/src
PANS_DIR = $$PWD/src/pans
GRAPH_DIR = $$PWD/src/graphics

NPHYS_DIR = $$PWD/nPhysImage

#CONFIG += neutrino-HDF

QT += svg xml network core gui printsupport widgets
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000

QMAKE_LIBDIR_FLAGS = -L../ -L../nPhysImage

VERSION = 1.0.0

# VERSION STUFF
NVERSION=$$system(git describe)

macx {
	DEFINES += __VER=\'\"$${NVERSION}\"\'
} else {
	DEFINES += __VER=\\\"$${NVERSION}\\\"
}

message($${NVERSION})
# nPhysImage compilation
nPhys.target = nPhys
CONFIG(debug, debug|release) {
    nPhys.commands = make -C ../nPhysImage debug
} else {
    nPhys.commands = make -C ../nPhysImage release
}
QMAKE_EXTRA_TARGETS += nPhys
PRE_TARGETDEPS = nPhys


CONFIG(debug, debug|release) {
    DEFINES += __phys_debug=10
    message("DEBUG!")
} else {
    DEFINES += QT_NO_DEBUG_OUTPUT
    message("RELEASE!")
}

#this accounts for a bug in qmake for xcode
macx-xcode {
    DEFINES += __phys_debug=10
}

# base
INCLUDEPATH += ../src
DEPENDPATH += ../src ../resources ../UIs

RESOURCES=../resources/neutrino.qrc

win32:RC_FILE=../resources/neutrino.rc

QMAKE_INFO_PLIST=../resources/neutrino.plist

macx {
	neutrino.path = .
	neutrino.files = neutrino.app
	INSTALLS += neutrino

	ICON = ../resources/icons/icon.icns

	LIBS += -L/opt/local/lib
	INCLUDEPATH += /opt/local/include /opt/local/include/netpbm
				
# workaround for a bug in macdeployqt: http://stackoverflow.com/questions/3454001/macdeployqt-not-copying-plugins
# sudo ln -sf /Developer/Applications/Qt/plugins /usr/plugins

    # this is required from src/osxApp.h
    LIBS += -framework IOKit -framework CoreFoundation
}


unix:!macx {
	INCLUDEPATH += /usr/include/qwt
	LIBS += -lqwt

    neutrino.path = /usr/local/bin
	neutrino.files = neutrino
    INSTALLS += neutrino

# gsl
	LIBS += -L/usr/lib -lgsl -lgslcblas -lm

}        

win32 {
	CONFIG -= neutrino-HDF

	ICON = ../resources/icons/icon.ico
	QWT_ROOT = /c/compile/qwt-6.1.0
	
	LIBS += -L../lib	
		
	DEFINES += QT_NO_DEBUG
	
	# GNU32 subsys
	INCLUDEPATH += /c/compile/GnuWin32/include
	LIBS += -L/c/compile/GnuWin32/bin -L/c/compile/GnuWin32/lib
	
	# qwt
	INCLUDEPATH += /c/compile/qwt-6.1.0/src
	LIBS +=  -lqwt -L/c/compile/qwt-6.1.0/lib

	DEFINES    += QT_DLL QWT_DLL
	
	qtAddLibrary(qwt)
	
} 

# physImage
INCLUDEPATH += ../nPhysImage 
#LIBS += -L../nPhysImage -lnPhysImageF -lnetpbm  -L/usr/lib -lgsl -lgslcblas -lm -ltiff -ljpeg -lm -ldf -lcfitsio 
LIBS += -L../nPhysImage -lnPhysImageF -lnetpbm  -L/usr/lib -lgsl -lgslcblas -lm -ltiff -ljpeg -lm -lfftw3

neutrino-HDF {
    LIBS += -lmfhdf -ldf -lhdf5
	message ( HDF enabled )
	DEFINES += __phys_HDF

	macx {
		LIBS+=-lmfhdf -ldf -ljpeg -lz -lm -lhdf5 -lhdf5_hl
	}

	win32 {
        INCLUDEPATH += /c/compile/HDF_Group/HDFshared/4.2.10/include
        LIBS += -L/c/compile/HDF_Group/HDFshared/4.2.10/lib
        LIBS += -lmfhdfdll -lhdfdll -lhdf5 -l hdf5_hl
        
        LIBS -= -ljpeg -lm -lmfhdf -ldf
        
        INCLUDEPATH += /c/compile/HDF_Group/HDF5/1.8.10/include
        LIBS += -L/c/compile/HDF_Group/HDF5/1.8.10/lib
	}

	unix:!macx {
		INCLUDEPATH += /usr/include/hdf
		LIBS+=-lmfhdf -ldf -ljpeg -lz -lm -lhdf5 -lhdf5_hl
	}
} 

# paths and mouse and tics
INCLUDEPATH += ../src/graphics
DEPENDPATH += ../src/graphics

# pans
INCLUDEPATH += ../src/pans
DEPENDPATH += ../src/pans

# colorbar
INCLUDEPATH += ../src/pans/colorbar
DEPENDPATH += ../src/pans/colorbar

# VISAR
INCLUDEPATH += ../src/pans/VISAR
DEPENDPATH += ../src/pans/VISAR

#winlist pan
INCLUDEPATH += ../src/pans/winlist
DEPENDPATH += ../src/pans/winlist


FORMS += $$UIs_DIR/neutrino.ui $$UIs_DIR/nSbarra.ui
FORMS += $$UIs_DIR/nLine.ui $$UIs_DIR/nObject.ui $$UIs_DIR/nPoint.ui

# external colormaps
SOURCES += $$SRC_DIR/neutrinoPalettes.cc

DEPENDPATH += ../nPhysImage
HEADERS += $$NPHYS_DIR/config.h

HEADERS += $$SRC_DIR/nGenericPan.h  $$SRC_DIR/neutrino.h 
SOURCES += $$SRC_DIR/nGenericPan.cc $$SRC_DIR/neutrino.cc

HEADERS += $$SRC_DIR/nView.h 
SOURCES += $$SRC_DIR/nView.cc

HEADERS += $$SRC_DIR/nPlug.h 
SOURCES += $$SRC_DIR/nPlug.cc

FORMS += $$UIs_DIR/nColorBarWin.ui
HEADERS += $$PANS_DIR/colorbar/nColorBarWin.h  $$PANS_DIR/colorbar/nHistogram.h
SOURCES += $$PANS_DIR/colorbar/nColorBarWin.cc $$PANS_DIR/colorbar/nHistogram.cc

FORMS += $$UIs_DIR/nPhysProperties.ui
HEADERS += $$PANS_DIR/nPhysProperties.h 
SOURCES += $$PANS_DIR/nPhysProperties.cc


# base app
HEADERS += $$SRC_DIR/nApp.h
macx {
	# osx app
	HEADERS += $$SRC_DIR/osxApp.h
}
