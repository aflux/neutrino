include (../neutrino.pri)

TEMPLATE = lib

OBJECTS_DIR = ../build
MOC_DIR = ../build
RCC_DIR = ../build
UI_DIR = ../build


## padellume
FORMS += $$UIs_DIR/nOpenRAW.ui
HEADERS += $$PANS_DIR/nOpenRAW.h
SOURCES += $$PANS_DIR/nOpenRAW.cc


SOURCES += $$GRAPH_DIR/nLine.cc $$GRAPH_DIR/nRect.cc $$GRAPH_DIR/nEllipse.cc $$GRAPH_DIR/nPoint.cc
HEADERS += $$GRAPH_DIR/nLine.h  $$GRAPH_DIR/nRect.h  $$GRAPH_DIR/nEllipse.h  $$GRAPH_DIR/nPoint.h
HEADERS += $$GRAPH_DIR/nMouse.h  $$GRAPH_DIR/nTics.h
SOURCES += $$GRAPH_DIR/nMouse.cc $$GRAPH_DIR/nTics.cc


FORMS += $$UIs_DIR/nMouseInfo.ui
HEADERS += $$PANS_DIR/nMouseInfo.h 
SOURCES += $$PANS_DIR/nMouseInfo.cc

HEADERS += $$PANS_DIR/winlist/nWinList.h  $$PANS_DIR/winlist/nTreeWidget.h
SOURCES += $$PANS_DIR/winlist/nWinList.cc $$PANS_DIR/winlist/nTreeWidget.cc
FORMS += $$UIs_DIR/nWinList.ui


## focal spot
HEADERS += $$PANS_DIR/nFocalSpot.h
SOURCES += $$PANS_DIR/nFocalSpot.cc
FORMS += $$UIs_DIR/nFocalSpot.ui

## lineout
HEADERS += $$PANS_DIR/nLineout.h  $$PANS_DIR/nLineoutBoth.h
SOURCES += $$PANS_DIR/nLineout.cc $$PANS_DIR/nLineoutBoth.cc
FORMS += $$UIs_DIR/nLineout.ui $$UIs_DIR/nLineoutBoth.ui 

## operators +-*/
HEADERS += $$PANS_DIR/nOperator.h
SOURCES += $$PANS_DIR/nOperator.cc
FORMS += $$UIs_DIR/nOperator.ui

## cutoff mask
HEADERS += $$PANS_DIR/nCutoffMask.h
SOURCES += $$PANS_DIR/nCutoffMask.cc
FORMS += $$UIs_DIR/nCutoffMask.ui

## rotation
HEADERS += $$PANS_DIR/nRotate.h
SOURCES += $$PANS_DIR/nRotate.cc
FORMS += $$UIs_DIR/nRotate.ui

## Affine
HEADERS += $$PANS_DIR/nAffine.h
SOURCES += $$PANS_DIR/nAffine.cc
FORMS += $$UIs_DIR/nAffine.ui

## Blur
HEADERS += $$PANS_DIR/nBlur.h
SOURCES += $$PANS_DIR/nBlur.cc
FORMS += $$UIs_DIR/nBlur.ui

#HDF stuff 
neutrino-HDF {
	FORMS += $$UIs_DIR/nHDF5.ui
	SOURCES += $$PANS_DIR/nHDF5.cc
	HEADERS += $$PANS_DIR/nHDF5.h 
} 

#Box Lineout
FORMS += $$UIs_DIR/nBoxLineout.ui
SOURCES += $$PANS_DIR/nBoxLineout.cc
HEADERS += $$PANS_DIR/nBoxLineout.h 

#Find peaks
FORMS += $$UIs_DIR/nFindPeaks.ui
SOURCES += $$PANS_DIR/nFindPeaks.cc
HEADERS += $$PANS_DIR/nFindPeaks.h 

#VISAR
FORMS += $$UIs_DIR/nVISAR1.ui $$UIs_DIR/nVISAR2.ui  $$UIs_DIR/nVISAR3.ui 
SOURCES += $$PANS_DIR/VISAR/nVisar.cc $$PANS_DIR/VISAR/nVisarZoomer.cc
HEADERS += $$PANS_DIR/VISAR/nVisar.h  $$PANS_DIR/VISAR/nVisarZoomer.h 

# Wavelet
FORMS += $$UIs_DIR/nWavelet.ui
SOURCES += $$PANS_DIR/nWavelet.cc
HEADERS += $$PANS_DIR/nWavelet.h 

# Spectral Analysis
FORMS += $$UIs_DIR/nSpectralAnalysis.ui
SOURCES += $$PANS_DIR/nSpectralAnalysis.cc
HEADERS += $$PANS_DIR/nSpectralAnalysis.h

# Inversions
FORMS += $$UIs_DIR/nIntegralInversion.ui
SOURCES += $$PANS_DIR/nIntegralInversion.cc
HEADERS += $$PANS_DIR/nIntegralInversion.h 

# Monitor directory
FORMS += $$UIs_DIR/nMonitor.ui
SOURCES += $$PANS_DIR/nMonitor.cc
HEADERS += $$PANS_DIR/nMonitor.h 

# RegionPath
FORMS += $$UIs_DIR/nRegionPath.ui
SOURCES += $$PANS_DIR/nRegionPath.cc
HEADERS += $$PANS_DIR/nRegionPath.h 

# Preferences Panel
FORMS += $$UIs_DIR/nPreferences.ui
SOURCES += $$PANS_DIR/nPreferences.cc
HEADERS += $$PANS_DIR/nPreferences.h 

# Preferences Panel
FORMS += $$UIs_DIR/nCompareLines.ui
SOURCES += $$PANS_DIR/nCompareLines.cc
HEADERS += $$PANS_DIR/nCompareLines.h 
 
# Shortcuts
FORMS += $$UIs_DIR/nShortcuts.ui
SOURCES += $$PANS_DIR/nShortcuts.cc
HEADERS += $$PANS_DIR/nShortcuts.h 

#Find peaks
FORMS += $$UIs_DIR/nGhost.ui
SOURCES += $$PANS_DIR/nGhost.cc
HEADERS += $$PANS_DIR/nGhost.h 


