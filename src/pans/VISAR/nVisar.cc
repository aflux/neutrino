/*
 *
 *    Copyright (C) 2013 Alessandro Flacco, Tommaso Vinci All Rights Reserved
 * 
 *    This file is part of neutrino.
 *
 *    Neutrino is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Neutrino is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with neutrino.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    Contact Information: 
 *	Alessandro Flacco <alessandro.flacco@polytechnique.edu>
 *	Tommaso Vinci <tommaso.vinci@polytechnique.edu>
 *
 */
#include "nVisar.h"
#include "nVisarZoomer.h"
#include "neutrino.h"

#include <qwt_scale_engine.h>
#include <qwt_curve_fitter.h>
#include <qwt_symbol.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_renderer.h>
 
nVisar::nVisar(neutrino *nparent, QString winname)
: nGenericPan(nparent, winname)
{
	//QWidget: Must construct a QApplication before a QPaintDevice
	my_w.setupUi(this);
 
	// signals

    connect(my_w.tabs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(my_w.tabPhase, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(my_w.tabVelocity, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

	connect(my_w.actionLoadPref, SIGNAL(triggered()), this, SLOT(loadSettings()));
	connect(my_w.actionSavePref, SIGNAL(triggered()), this, SLOT(saveSettings()));

	connect(my_w.actionSaveTxt, SIGNAL(triggered()), this, SLOT(export_txt()));
	connect(my_w.actionSaveTxtMultiple, SIGNAL(triggered()), this, SLOT(export_txt_multiple()));
	connect(my_w.actionSavePDF, SIGNAL(triggered()), this, SLOT(export_pdf()));
	connect(my_w.actionCopy, SIGNAL(triggered()), this, SLOT(export_clipboard()));

	connect(my_w.actionRefresh2, SIGNAL(triggered()), this, SLOT(getCarrier()));
	connect(my_w.actionRefresh, SIGNAL(triggered()), this, SLOT(doWave()));

    connect(nparent, SIGNAL(bufferChanged(nPhysD*)), this, SLOT(bufferChanged(nPhysD*)));
    
	QList<QAction*> actionRects;
	actionRects << my_w.actionRect1 << my_w.actionRect2;
	for (int k=0;k<2;k++){
		fringeLine[k] = new nLine(nparent);
		fringeLine[k]->setParentPan(panName,3);
		fringeLine[k]->changeToolTip(tr("Fringeshift Visar ")+QString::number(k+1));

		fringeRect[k] =  new nRect(nparent);
		fringeRect[k]->setProperty("id", k);

		fringeRect[k]->setParentPan(panName,1);
		fringeRect[k]->changeToolTip(tr("Visar region ")+QString::number(k+1));
		fringeRect[k]->setRect(QRectF(0,0,100,100));
		connect(actionRects[k], SIGNAL(triggered()), fringeRect[k], SLOT(togglePadella()));
	}

//!START SOP stuff
	sopRect =  new nRect(nparent);	
	sopRect->setParentPan(panName,1);
	sopRect->changeToolTip(tr("SOP region"));
	sopRect->setRect(QRectF(0,0,100,100));
	connect(my_w.actionRect3, SIGNAL(triggered()), sopRect, SLOT(togglePadella()));

	my_w.sopPlot->setAxisTitle(QwtPlot::xBottom, tr("Time"));
    my_w.sopPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
    
	my_w.sopPlot->setAxisTitle(QwtPlot::yLeft, tr("Counts [red]"));
	my_w.sopPlot->setAxisTitle(QwtPlot::yRight, tr("Temperature [blue]"));
    
	(qobject_cast<QFrame*> (my_w.sopPlot->canvas()))->setLineWidth(0);
    my_w.sopPlot->enableAxis(QwtPlot::yRight);

	my_w.sopPlot->setAxisAutoScale(QwtPlot::xBottom);
	my_w.sopPlot->setAxisAutoScale(QwtPlot::yLeft);
	my_w.sopPlot->setAxisAutoScale(QwtPlot::yRight);
	
	mouseMarker[3].setLineStyle(QwtPlotMarker::VLine);
	mouseMarker[3].attach(my_w.sopPlot);
//!END SOP stuff
    
	QList<QWidget*> father1, father2;
	father1<< my_w.wvisar1 <<my_w.wvisar2;
	father2<<my_w.setVisar1<<my_w.setVisar2;
	for (int k=0;k<2;k++){
		visar[k].setupUi(father1.at(k));
		father1.at(k)->show();
		setvisar[k].setupUi(father2.at(k));
		father2.at(k)->show();
		
		//hack to save diffrent uis!!!
		foreach (QWidget *obj, father1.at(k)->findChildren<QWidget*>()) {
			obj->setObjectName(obj->objectName()+"-VISAR"+QString::number(k+1));
		}
		foreach (QWidget *obj, father2.at(k)->findChildren<QWidget*>()) {
			obj->setObjectName(obj->objectName()+"-VISAR"+QString::number(k+1));
		}

		visar[k].plotPhaseIntensity->setAxisTitle(QwtPlot::xBottom, tr("Position"));
        visar[k].plotPhaseIntensity->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
		visar[k].plotPhaseIntensity->setAxisTitle(QwtPlot::yLeft, tr("Shift (red)"));
		visar[k].plotPhaseIntensity->setAxisTitle(QwtPlot::yRight, tr("Intensity (blue) - Contrast (gray)"));
		visar[k].plotPhaseIntensity->enableAxis(QwtPlot::yRight);
		(qobject_cast<QFrame*> (visar[k].plotPhaseIntensity->canvas()))->setLineWidth(0);

		for (int i=0;i<2;i++){
			cPhase[i][k].setPen(QPen(Qt::red,1));
			cPhase[i][k].setXAxis(QwtPlot::xBottom);
			cPhase[i][k].setYAxis(QwtPlot::yLeft);
			cIntensity[i][k].setPen(QPen(Qt::blue,1));
			cIntensity[i][k].setXAxis(QwtPlot::xBottom);
			cIntensity[i][k].setYAxis(QwtPlot::yRight);
		}
		for (int i=0;i<2;i++){
			cPhase[i][k].attach(visar[k].plotPhaseIntensity);
			cContrast[i][k].setPen(QPen(Qt::gray,1));
			cContrast[i][k].setYAxis(QwtPlot::yRight);
			cContrast[i][k].setXAxis(QwtPlot::xBottom);
			cContrast[i][k].attach(visar[k].plotPhaseIntensity);
			cIntensity[i][k].attach(visar[k].plotPhaseIntensity);
			mouseMarker[k].setLineStyle(QwtPlotMarker::VLine);
			mouseMarker[k].attach(visar[k].plotPhaseIntensity);
		}
        velocity[k].setPen(QPen(Qt::red,1));
        velocity[k].setXAxis(QwtPlot::xBottom);
        velocity[k].setYAxis(QwtPlot::yLeft);
		velocity[k].attach(my_w.plotVelocity);
        
        reflectivity[k].setPen(QPen(Qt::blue,1));
        reflectivity[k].setXAxis(QwtPlot::xBottom);
        reflectivity[k].setYAxis(QwtPlot::yRight);        
		reflectivity[k].attach(my_w.plotVelocity);
		mouseMarker[2].setLineStyle(QwtPlotMarker::VLine);
		mouseMarker[2].attach(my_w.plotVelocity);

		visar[k].guess->setProperty("id", k);
		visar[k].doWaveButton->setProperty("id", k);
		setvisar[k].physScale->setProperty("id", k);
		
		QPen pen;
		pen=cPhase[0][k].pen();
		pen.setStyle(Qt::DashLine);
		cPhase[0][k].setPen(pen);
		pen.setStyle(Qt::SolidLine);
		cPhase[1][k].setPen(pen);
		pen=cIntensity[0][k].pen();
		pen.setStyle(Qt::DashLine);
		cIntensity[0][k].setPen(pen);
		pen.setStyle(Qt::SolidLine);
		cIntensity[1][k].setPen(pen);

		pen=cContrast[0][k].pen();
		pen.setStyle(Qt::DashLine);
		cContrast[0][k].setPen(pen);
		pen.setStyle(Qt::SolidLine);
		cContrast[1][k].setPen(pen);
	}
	
	my_w.plotVelocity->setAxisTitle(QwtPlot::xBottom, tr("Position (time units)"));
    my_w.plotVelocity->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
	my_w.plotVelocity->setAxisTitle(QwtPlot::yLeft, tr("Velocity (red)"));
	my_w.plotVelocity->setAxisTitle(QwtPlot::yRight, tr("Reflectivity (blue)"));
	my_w.plotVelocity->enableAxis(QwtPlot::yRight);
	(qobject_cast<QFrame*> (my_w.plotVelocity->canvas()))->setLineWidth(0);

	for (int k=0;k<2;k++){
		zoomer[k] = new nVisarZoomer(visar[k].plotPhaseIntensity->canvas());
		zoomer[k]->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
		zoomer[k]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
	}

	zoomer[2] = new nVisarZoomer(my_w.plotVelocity->canvas());
	zoomer[2]->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
	zoomer[2]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

    sopCurve[0].setPen(QPen(Qt::red,1));
    sopCurve[0].setXAxis(QwtPlot::xBottom);
    sopCurve[0].setYAxis(QwtPlot::yLeft);
    sopCurve[0].attach(my_w.sopPlot);

    QwtSymbol *sym1=new QwtSymbol(QwtSymbol::Ellipse);
    sym1->setSize(2,2);
    sym1->setPen(QPen(Qt::blue,1));
    sym1->setColor(Qt::blue);

    sopCurve[1].setXAxis(QwtPlot::xBottom);
    sopCurve[1].setYAxis(QwtPlot::yRight);
    sopCurve[1].attach(my_w.sopPlot);
    sopCurve[1].setSymbol(sym1);
    sopCurve[1].setStyle(QwtPlotCurve::NoCurve);

    
    QwtSymbol *sym2=new QwtSymbol(QwtSymbol::Ellipse);
    sym2->setSize(2,2);
    sym2->setPen(QPen(Qt::magenta,1));
    sym2->setColor(Qt::magenta);
    
    sopCurve[2].setXAxis(QwtPlot::xBottom);
    sopCurve[2].setYAxis(QwtPlot::yRight);
    sopCurve[2].attach(my_w.sopPlot);
    sopCurve[2].setSymbol(sym2);
    sopCurve[2].setStyle(QwtPlotCurve::NoCurve);
    
    sopCurve[3].setXAxis(QwtPlot::yRight);
    sopCurve[3].setYAxis(QwtPlot::xTop);
    sopCurve[3].attach(my_w.sopPlot);
    sopCurve[3].setStyle(QwtPlotCurve::NoCurve);
    
	zoomer[3] = new nVisarZoomer(my_w.sopPlot->canvas());
	zoomer[3]->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
	zoomer[3]->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

	QPen pen;
	pen.setColor(Qt::darkRed);
	pen.setStyle(Qt::DashLine);
	QwtPlotMarker *markV0=new QwtPlotMarker();
	markV0->setLineStyle(QwtPlotMarker::HLine);
	markV0->setLinePen(pen);
	markV0->setYValue(0);
	markV0->attach(my_w.plotVelocity);

	pen.setColor(Qt::darkBlue);
	QwtPlotMarker *markRef1=new QwtPlotMarker();
	markRef1->setLineStyle(QwtPlotMarker::HLine);
	markRef1->setLinePen(pen);
	markRef1->setYValue(1);
	markRef1->setYAxis(QwtPlot::yRight);
	markRef1->attach(my_w.plotVelocity);

	QwtPlotMarker *markX0=new QwtPlotMarker();
	markX0->setLineStyle(QwtPlotMarker::VLine);
	markX0->setXValue(0);
	markX0->setXAxis(QwtPlot::xBottom);
	markX0->attach(my_w.plotVelocity);

	for (int k=0;k<2;k++){
		for (int m=0;m<2;m++){
			QString name="Visar "+QString::number(k+1)+" "+QString::number(m);
            phase[k][m].setName(name.toUtf8().constData());
			phase[k][m].setShortName("phase");
			contrast[k][m].setName(name.toUtf8().constData());
			contrast[k][m].setShortName("contrast");
			intensity[k][m].setName(name.toUtf8().constData());
			intensity[k][m].setShortName("intensity");
		}
	}
    
    decorate();
	connections();
    my_w.tabs->setCurrentIndex(0);

}

void nVisar::loadSettings(QString my_settings) {
	nGenericPan::loadSettings(my_settings);    
	QApplication::processEvents();
	doWave();
	QApplication::processEvents();
	QTimer::singleShot(1000, this, SLOT(tabChanged()));
}

void nVisar::mouseAtMatrix(QPointF p) {
	int k=0;
	double position=0.0;
    if (my_w.tabs->currentIndex()==0) {
        k=my_w.tabPhase->currentIndex();
		position=(direction(k)==0) ? p.y() : p.x();
		mouseMarker[k].setXValue(position);
		mouseMarker[k].plot()->replot();
    } else if (my_w.tabs->currentIndex()==1) {
        k=my_w.tabVelocity->currentIndex();
		position=((direction(k)==0 ? p.y() : p.x() )-setvisar[k].physOrigin->value())*setvisar[k].physScale->value()+setvisar[k].offsetTime->value();
		mouseMarker[2].setXValue(position);
		mouseMarker[2].plot()->replot();
	} else {
		position=((my_w.sopDirection->currentIndex()==0 ? p.y() : p.x() )-my_w.sopOrigin->value())*my_w.sopScale->value()+my_w.sopTimeOffset->value();
		mouseMarker[3].setXValue(position);
		mouseMarker[3].plot()->replot();
	}
	my_w.statusbar->showMessage("Postion : "+QString::number(position));
}

int nVisar::direction(int k) {
	int dir=((int) ((visar[k].angle->value()+360+45)/90.0) )%2;
	return dir;
}

void nVisar::bufferChanged(nPhysD*phys) {
    for (int k=0;k<2;k++){
        fringeRect[k]->hide();
        fringeLine[k]->hide();
        if (phys==getPhysFromCombo(visar[k].shotImage) || 
            phys==getPhysFromCombo(visar[k].refImage) ) {
            fringeRect[k]->show();
            fringeLine[k]->show();
        }
        sopRect->hide();
        if (phys==getPhysFromCombo(my_w.sopShot) || 
            phys==getPhysFromCombo(my_w.sopRef) ) {
            sopRect->show();
        }
    }        
}

void nVisar::tabChanged(int k) {
	
	QApplication::processEvents();

	// QPainter::begin: Paint device returned engine == 0, type: 2
	
    QTabWidget *tabWidget=qobject_cast<QTabWidget *>(sender());
    if (!tabWidget) tabWidget=my_w.tabs;

    if (tabWidget==my_w.tabs) {
		if (k==0) {
            tabWidget=my_w.tabPhase;
		} else if (k==1) {
            tabWidget=my_w.tabVelocity;
		}
	}
    
	if (k<2) {
        int visnum=tabWidget->currentIndex();
		nparent->showPhys(getPhysFromCombo(visar[visnum].shotImage));
        if (tabWidget==my_w.tabVelocity) {
			updatePlot();
		}
	} else if (k==2){
		nparent->showPhys(getPhysFromCombo(my_w.sopShot));
		updatePlotSOP();
    }
}

void nVisar::connections() {
	for (int k=0;k<2;k++){
		connect(fringeRect[k], SIGNAL(sceneChanged()), this, SLOT(getPhase()));
		connect(setvisar[k].offsetShift, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
		connect(setvisar[k].sensitivity, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
		connect(setvisar[k].jumpst, SIGNAL(editingFinished()), this, SLOT(updatePlot()));
		connect(setvisar[k].reflRef, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
		connect(setvisar[k].reflOffset, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
		connect(setvisar[k].jump, SIGNAL(valueChanged(int)), this, SLOT(updatePlot()));
		
		connect(setvisar[k].physScale, SIGNAL(valueChanged(double)), this, SLOT(getPhase()));
		connect(setvisar[k].physOrigin, SIGNAL(valueChanged(int)), this, SLOT(updatePlot()));
		connect(setvisar[k].offsetTime, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
		
		connect(visar[k].guess, SIGNAL(released()), this, SLOT(getCarrier()));
		connect(visar[k].doWaveButton, SIGNAL(released()), this, SLOT(doWave()));

		connect(visar[k].multRef, SIGNAL(editingFinished()), this, SLOT(getPhase()));
		connect(visar[k].offRef, SIGNAL(editingFinished()), this, SLOT(getPhase()));
		connect(visar[k].offShot, SIGNAL(editingFinished()), this, SLOT(getPhase()));
		connect(visar[k].intensityShift, SIGNAL(editingFinished()), this, SLOT(getPhase()));
		
		connect(visar[k].enableVisar, SIGNAL(released()), this, SLOT(updatePlot()));
	}
	connect(sopRect, SIGNAL(sceneChanged()), this, SLOT(updatePlotSOP()));
	connect(my_w.sopRef, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePlotSOP()));
	connect(my_w.sopShot, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePlotSOP()));
	connect(my_w.sopTimeOffset, SIGNAL(valueChanged(double)), this, SLOT(updatePlotSOP()));
	connect(my_w.sopOffset, SIGNAL(valueChanged(double)), this, SLOT(updatePlotSOP()));
	connect(my_w.sopOrigin, SIGNAL(valueChanged(int)), this, SLOT(updatePlotSOP()));
	connect(my_w.sopDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePlotSOP()));
	connect(my_w.sopScale, SIGNAL(valueChanged(double)), this, SLOT(updatePlotSOP()));
	connect(my_w.sopCalibT0, SIGNAL(valueChanged(double)), this, SLOT(updatePlotSOP()));
	connect(my_w.sopCalibA, SIGNAL(valueChanged(double)), this, SLOT(updatePlotSOP()));
	connect(my_w.whichRefl, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePlotSOP()));
	connect(my_w.Tminmax, SIGNAL(editingFinished()), this, SLOT(updatePlotSOP()));
}

void nVisar::disconnections() {
	for (int k=0;k<2;k++){
		disconnect(fringeRect[k], SIGNAL(sceneChanged()), this, SLOT(getPhase()));
		disconnect(setvisar[k].offsetShift, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
		disconnect(setvisar[k].sensitivity, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
		disconnect(setvisar[k].jumpst, SIGNAL(editingFinished()), this, SLOT(updatePlot()));
		disconnect(setvisar[k].reflRef, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
		disconnect(setvisar[k].reflOffset, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));
		disconnect(setvisar[k].jump, SIGNAL(valueChanged(int)), this, SLOT(updatePlot()));

		disconnect(setvisar[k].physScale, SIGNAL(valueChanged(double)), this, SLOT(getPhase()));
		disconnect(setvisar[k].physOrigin, SIGNAL(valueChanged(int)), this, SLOT(updatePlot()));
		disconnect(setvisar[k].offsetTime, SIGNAL(valueChanged(double)), this, SLOT(updatePlot()));

		disconnect(visar[k].guess, SIGNAL(released()), this, SLOT(getCarrier()));
		disconnect(visar[k].doWaveButton, SIGNAL(released()), this, SLOT(doWave()));

		disconnect(visar[k].multRef, SIGNAL(editingFinished()), this, SLOT(getPhase()));
		disconnect(visar[k].offRef, SIGNAL(editingFinished()), this, SLOT(getPhase()));
		disconnect(visar[k].offShot, SIGNAL(editingFinished()), this, SLOT(getPhase()));
		disconnect(visar[k].intensityShift, SIGNAL(editingFinished()), this, SLOT(getPhase()));

		disconnect(visar[k].enableVisar, SIGNAL(released()), this, SLOT(updatePlot()));
	}
	disconnect(sopRect, SIGNAL(sceneChanged()), this, SLOT(updatePlotSOP()));
	disconnect(my_w.sopRef, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePlotSOP()));
	disconnect(my_w.sopShot, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePlotSOP()));
	disconnect(my_w.sopTimeOffset, SIGNAL(valueChanged(double)), this, SLOT(updatePlotSOP()));
	disconnect(my_w.sopOffset, SIGNAL(valueChanged(double)), this, SLOT(updatePlotSOP()));
	disconnect(my_w.sopOrigin, SIGNAL(valueChanged(int)), this, SLOT(updatePlotSOP()));
	disconnect(my_w.sopDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePlotSOP()));
	disconnect(my_w.sopScale, SIGNAL(valueChanged(double)), this, SLOT(updatePlotSOP()));
	disconnect(my_w.sopCalibT0, SIGNAL(valueChanged(double)), this, SLOT(updatePlotSOP()));
	disconnect(my_w.sopCalibA, SIGNAL(valueChanged(double)), this, SLOT(updatePlotSOP()));
	disconnect(my_w.whichRefl, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePlotSOP()));
	disconnect(my_w.Tminmax, SIGNAL(editingFinished()), this, SLOT(updatePlotSOP()));
}

void nVisar::updatePlotSOP() {
	
	disconnections();
	nPhysD *shot=getPhysFromCombo(my_w.sopShot);
	nPhysD *ref=getPhysFromCombo(my_w.sopRef);
	int dir=my_w.sopDirection->currentIndex();
	if (shot) {
        QRect geom2=sopRect->getRect(shot);
		if (ref) geom2=geom2.intersected(QRect(0,0,ref->getW(),ref->getH()));
		if (geom2.isEmpty()) {
			my_w.statusbar->showMessage(tr("Attention: the region is outside the image!"),2000);
			return;
		}
		
		int dx=geom2.width();
		int dy=geom2.height();
		
		QVector<double> sopData(dir==0?dy:dx);
		for (int j=0;j<dy;j++){
			for (int i=0;i<dx; i++) {
				double val=shot->point(i+geom2.x(),j+geom2.y(),0.0);
				if (ref && ref!=shot) val-=ref->point(i+geom2.x(),j+geom2.y(),0.0);
				sopData[dir==0?j:i]+=val;
			}
		}
		
        QVector <QPointF> sopPoints(sopData.size());

		switch (dir) {
			case 0:				
				for (int j=0;j<dy;j++) sopPoints[j]=QPointF((geom2.y()+j-my_w.sopOrigin->value())*my_w.sopScale->value()+my_w.sopTimeOffset->value(),sopData[j]/dx-my_w.sopOffset->value());
				break;
			case 1:
				for (int i=0;i<dx;i++) sopPoints[i]=QPointF((geom2.x()+i-my_w.sopOrigin->value())*my_w.sopScale->value()+my_w.sopTimeOffset->value(),sopData[i]/dy-my_w.sopOffset->value());
				break;
			default:
				break;
		}
		sopCurve[0].setSamples(sopPoints);
		
// TEMPERATURE FROM REFLECTIVITY
        QVector<int> reflList;
        switch (my_w.whichRefl->currentIndex()) {
            case 0:
                reflList << 0;
                break;
            case 1:
                reflList << 1;
                break;
            case 2:
                reflList << 0 << 1;
                break;
            default:
                break;
        }
        
        QVector <QPointF> TempPoints;
        QVector <QPointF> outTempPoints;
        QVector <QPointF> velTempPoints;

        double my_T0=my_w.sopCalibT0->value();
        double my_A=my_w.sopCalibA->value();
        
        QStringList my_minmax=my_w.Tminmax->text().split(" ", QString::SkipEmptyParts);
        double my_min,my_max;
        if (my_minmax.size()==2) {
            my_min=my_minmax[0].toDouble();
            my_max=my_minmax[1].toDouble();
            my_w.sopPlot->setAxisScale(QwtPlot::yRight,my_min,my_max);
        } else {
            my_min = my_max = 0;
            my_w.sopPlot->setAxisAutoScale(QwtPlot::yRight);
        }
        for (int i=0; i<sopPoints.size(); i++) {
            double my_time=sopPoints[i].x();
            double counts=sopPoints[i].y();
            
            double my_reflectivity=0;
            double my_velocity=0;
            
            int numrefl=0;
            for (int k=0;k<reflList.size();k++) {
                if (reflectivity[reflList[k]].dataSize()>0) {
                    for (int j=1;j<(int)reflectivity[reflList[k]].dataSize();j++) {
                        double valj_1=reflectivity[reflList[k]].sample(j-1).x();
                        double valj=reflectivity[reflList[k]].sample(j).x();
                        
                        if (my_time>valj_1 && my_time<=valj ) {
                            double valyj_1=reflectivity[reflList[k]].sample(j-1).y();
                            double valyj=reflectivity[reflList[k]].sample(j).y();

                            double valVj_1=velocity[reflList[k]].sample(j-1).y();
                            double valVj=velocity[reflList[k]].sample(j).y();
                            
                            double refl=valyj_1+(my_time-valj_1)*(valyj-valyj_1)/(valj-valj_1);
                            double vel=valVj_1+(my_time-valj_1)*(valVj-valVj_1)/(valj-valj_1);
                            
                            my_reflectivity+=refl;
                            my_velocity+=vel;
                            
                            numrefl++;
                        }
                    }
                }
            }
            if (numrefl) {
                my_reflectivity/=numrefl;
            }
            my_reflectivity=min(max(my_reflectivity,0.0),1.0);
            double my_temp=my_T0/log(1.0+(1.0-my_reflectivity)*my_A/counts);
            
            if (numrefl) {
                TempPoints<<QPointF(my_time,my_temp);
                outTempPoints<<QPointF(my_time,0.0);
                velTempPoints << QPointF(my_temp,my_velocity);
            } else {
                TempPoints<<QPointF(my_time,0.0);
                outTempPoints<<QPointF(my_time,my_temp);
                velTempPoints << QPointF(0.0,0.0);
            }
        }
		sopCurve[1].setSamples(TempPoints);
		sopCurve[2].setSamples(outTempPoints);
		sopCurve[3].setSamples(velTempPoints);
        
        my_w.sopPlot->setAxisAutoScale(QwtPlot::xBottom);
		my_w.sopPlot->setAxisAutoScale(QwtPlot::yLeft);
        my_w.sopPlot->updateAxes();
		my_w.sopPlot->replot();
		zoomer[3]->setZoomBase();
	}		
	connections();
}

void nVisar::updatePlot() {
	disconnections();
	if (cPhase[0][0].dataSize()>0 || cPhase[0][1].dataSize()>0){
		QVector<double> tjump,njump,rjump;
		foreach (QwtPlotCurve *velJump, velJumps) {
			velJump->detach();
			velJump->setData(NULL);
			velJump->~QwtPlotCurve();
		}
		velJumps.clear();

		for (int k=0;k<2;k++){
			foreach (QwtPlotMarker *mark, marker) {
				mark->detach();
			}
			marker.clear();

            double sensitivity=setvisar[k].sensitivity->value();

            
			if (visar[k].enableVisar->isChecked()) {

                QVector <QPointF> my_vel(cPhase[0][k].dataSize());
                QVector <QPointF> my_refl(cPhase[0][k].dataSize());
				
				double scale=setvisar[k].physScale->value();
				double origin=setvisar[k].physOrigin->value();
				
				tjump.clear();
				njump.clear();
                rjump.clear();
				QStringList jumpt=setvisar[k].jumpst->text().split(";", QString::SkipEmptyParts);
				foreach (QString piece, jumpt) {
					QStringList my_jumps=piece.split(QRegExp("\\s+"), QString::SkipEmptyParts);
                    if (my_jumps.size()>1 && my_jumps.size()<=3) {
                        if (my_jumps.size()>1 && my_jumps.size()<=3) {
                            bool ok1, ok2, ok3=true;
                            double valdt=my_jumps.at(0).toDouble(&ok1);
                            double valdn=my_jumps.at(1).toDouble(&ok2);
                            double valdrefr_index=1.0;
                            if (my_jumps.size()==3) {
                                valdrefr_index=my_jumps.at(2).toDouble(&ok3);
                            }
                            if (sensitivity<0) valdn*=-1.0;
                            if (ok1 && ok2) {
                                tjump << valdt;
                                njump << valdn;
                            } else {
                                my_w.statusbar->showMessage(tr("Skipped unreadable jump '")+piece+QString("' VISAR ")+QString::number(k+1),5000);
                            }
                            if (ok3) {
                                rjump << valdrefr_index;
                            } else {
                                rjump << 1.0;
                            }
                        }
					} else {
						my_w.statusbar->showMessage(tr("Skipped unreadable jump '")+piece+QString("' VISAR ")+QString::number(k+1),5000);
					}
				}
				
				
				foreach (double a, tjump) {
					QwtPlotMarker *mark=new QwtPlotMarker();
					mark->setLineStyle(QwtPlotMarker::VLine);
                    QColor gr_color((Qt::gray));
                    QPen pen(gr_color);
					pen.setStyle(Qt::DashLine);
					mark->setLinePen(pen);
					mark->setXValue(a);
					//		mark->setDepth(-10);
					marker << mark;
				}
				
				foreach(QwtPlotMarker *mark, marker) {
					mark->attach(my_w.plotVelocity);
				}
				
				double offset=setvisar[k].offsetShift->value();
				double offsetTime=setvisar[k].offsetTime->value();
				
                QVector<QVector< QPointF > > velJump_array(abs(setvisar[k].jump->value()));
				
				for (unsigned int j=0;j<cPhase[0][k].dataSize();j++) {
					double time;
					time=(cPhase[0][k].sample(j).x()-origin)*scale+offsetTime;
					double fRef=cPhase[0][k].sample(j).y();
					double fShot=cPhase[1][k].sample(j).y();
					double iRef=cIntensity[0][k].sample(j).y();
					double iShot=cIntensity[1][k].sample(j).y();
					if (getPhysFromCombo(visar[k].shotImage)==getPhysFromCombo(visar[k].refImage)) {
						fRef=0.0;
						iRef=1.0;
					}
					
					int njumps=0;
                    double refr_index=1.0;
					for (int i=0;i<tjump.size();i++) {
						if (time>tjump.at(i)) {
                            njumps+=njump.at(i);
                            refr_index=rjump.at(i);
                        }
					}
					
					double speed=(offset+fShot-fRef+njumps)*sensitivity/refr_index;
					//double refle=setvisar[k].reflRef->value()*iShot/iRef+setvisar[k].reflOffset->value();
                    double Rg=setvisar[k].reflOffset->value();
                    double Rmat=setvisar[k].reflRef->value();
                    double beta=-Rg/pow(1.0-Rg,2);
                    double refle=iShot/iRef * (Rmat-beta) + beta;
                    
					my_vel[j] = QPointF(time,speed);
					my_refl[j] = QPointF(time,refle);
					for (int i=0;i<abs(setvisar[k].jump->value());i++) {
						int jloc=i+1;
						if (sensitivity<0) jloc*=-1;
						if (setvisar[k].jump->value()<0) jloc*=-1;
						velJump_array[i] << QPointF(time,(offset+fShot-fRef+jloc)*sensitivity/refr_index);
					}
				} 
				
				if (setvisar[k].jump->value()!=0) {
					for (int i=0;i<abs(setvisar[k].jump->value());i++) {
						QwtPlotCurve *velJump=new QwtPlotCurve();
						QColor color=Qt::red;
						color.setAlpha(60);
						velJump->setPen(QPen(color,1.0));
						velJump->setXAxis(QwtPlot::xBottom);
						velJump->setYAxis(QwtPlot::yLeft);
						velJump->setSamples(velJump_array[i]);
						velJump->attach(my_w.plotVelocity);
						velJumps << velJump;
					}
				}
				
                velocity[k].setSamples(my_vel);
                reflectivity[k].setSamples(my_refl);

                velocity[k].show();
                reflectivity[k].show();
            } else {

                velocity[k].hide();
                reflectivity[k].hide();
            }
		}			

		int k;
        k=my_w.tabVelocity->currentIndex();
		QPen pen;

		pen=velocity[k].pen();
		pen.setStyle(Qt::SolidLine);
		velocity[k].setPen(pen);
		pen.setStyle(Qt::DashLine);
		velocity[(k+1)%2].setPen(pen);
		
		pen=reflectivity[k].pen();
		pen.setStyle(Qt::SolidLine);
		reflectivity[k].setPen(pen);
		pen.setStyle(Qt::DashLine);
		reflectivity[(k+1)%2].setPen(pen);

		my_w.plotVelocity->setAxisAutoScale(QwtPlot::xBottom);
		my_w.plotVelocity->setAxisAutoScale(QwtPlot::yLeft);
		my_w.plotVelocity->setAxisAutoScale(QwtPlot::yRight);
		my_w.plotVelocity->replot();
		zoomer[2]->setZoomBase();
		my_w.statusbar->showMessage("Plot updated",1000);
        updatePlotSOP();
	}
	connections();
}

void nVisar::getCarrier() {
	if (sender() && sender()->property("id").isValid()) {
		int k=sender()->property("id").toInt();
		getCarrier(k);
	} else {
		for (int k=0;k<2;k++){
			getCarrier(k);
		}
	}
    if (my_w.tabs->currentIndex()==1) {
		my_w.statusbar->showMessage(tr("Carrier (")+QString::number(visar[0].interfringe->value())+tr("px, ")+visar[0].angle->value()+tr("deg) - (")+QString::number(visar[1].interfringe->value())+tr("px, ")+visar[1].angle->value()+tr("deg)"));
	}
}

void nVisar::getCarrier(int k) {
	disconnections();
	QComboBox *combo=NULL;
	if (visar[k].carrierPhys->currentIndex()==0) {
		combo=visar[k].refImage;
	} else {
		combo=visar[k].shotImage;
	}

	nPhysD *phys=getPhysFromCombo(combo); 
	if (phys && fringeRect[k]) {
        QRect geom2=fringeRect[k]->getRect(phys);
		nPhysD datamatrix = phys->sub(geom2.x(),geom2.y(),geom2.width(),geom2.height());

		vec2f vecCarr=phys_guess_carrier(datamatrix, visar[k].guessWeight->value());

		if (vecCarr.first()==0) {
			my_w.statusbar->showMessage("ERROR: Problem finding the carrier try to change the weight", 5000);
		} else {
			visar[k].interfringe->setValue(vecCarr.first());
			visar[k].angle->setValue(vecCarr.second());
            if (my_w.tabPhase->currentIndex()==k) {
				my_w.statusbar->showMessage(tr("Carrier :")+QString::number(vecCarr.first())+tr("px, ")+QString::number(vecCarr.second())+tr("deg"));
			}
		}
	}
	QApplication::processEvents();
	connections();
}

void nVisar::doWave() {
	if (sender() && sender()->property("id").isValid()) {
		int k=sender()->property("id").toInt();
		doWave(k);
	} else {
        for (int k=0;k<2;k++){
			doWave(k);
		}
	}
}


void nVisar::doWave(int k) {

	if (visar[k].enableVisar->isChecked()){ 
        if (getPhysFromCombo(visar[k].refImage) && getPhysFromCombo(visar[k].shotImage )  &&
            getPhysFromCombo(visar[k].refImage)->getW() == getPhysFromCombo(visar[k].shotImage)->getW() &&
            getPhysFromCombo(visar[k].refImage)->getH() == getPhysFromCombo(visar[k].shotImage)->getH()) {
            
            int counter=0;
            QProgressDialog progress("Filter visar "+QString::number(k+1), "Cancel", 0, 11, this);
            progress.setCancelButton(0);
            progress.setWindowModality(Qt::WindowModal);
            progress.show();
            nPhysC physfftRef=getPhysFromCombo(visar[k].refImage)->ft2(PHYS_FORWARD);
            progress.setValue(++counter);
            QApplication::processEvents();
            nPhysC physfftShot=getPhysFromCombo(visar[k].shotImage)->ft2(PHYS_FORWARD);
            progress.setValue(++counter);
            QApplication::processEvents();
            
            size_t dx=physfftRef.getW();
            size_t dy=physfftRef.getH();
            
            
            nPhysImageF<mcomplex> zz_morletRef("Ref"), zz_morletShot("Shot");
            zz_morletRef.resize(dx,dy);
            zz_morletShot.resize(dx,dy);
            
            vector<int> xx(dx), yy(dy);
#pragma omp parallel for
            for (size_t i=0;i<dx;i++) xx[i]=(i+(dx+1)/2)%dx-(dx+1)/2; // swap and center
#pragma omp parallel for
            for (size_t i=0;i<dy;i++) yy[i]=(i+(dy+1)/2)%dy-(dy+1)/2;
            
            for (int m=0;m<2;m++) {
                phase[k][m].resize(dx, dy);
                contrast[k][m].resize(dx, dy);
                intensity[k][m].resize(dx, dy);
            }
            
            progress.setValue(++counter);
            QApplication::processEvents();
#pragma omp parallel for
            for (size_t kk=0; kk<dx*dy; kk++) {
                intensity[k][0].set(kk,getPhysFromCombo(visar[k].refImage)->point(kk));			
                intensity[k][1].set(kk,getPhysFromCombo(visar[k].shotImage)->point(kk));			
            }
            progress.setValue(++counter);
            QApplication::processEvents();

            phys_fast_gaussian_blur(intensity[k][0], visar[k].resolution->value());
            phys_fast_gaussian_blur(intensity[k][1], visar[k].resolution->value());
            
            progress.setValue(++counter);
            QApplication::processEvents();
            double cr = cos((visar[k].angle->value()) * _phys_deg); 
            double sr = sin((visar[k].angle->value()) * _phys_deg);
            double thick_norm=visar[k].resolution->value()*M_PI/sqrt(pow(sr*dx,2)+pow(cr*dy,2));
            double damp_norm=M_PI;
            
            double lambda_norm=visar[k].interfringe->value()/sqrt(pow(cr*dx,2)+pow(sr*dy,2));
#pragma omp parallel for collapse(2)
            for (size_t x=0;x<dx;x++) {
                for (size_t y=0;y<dy;y++) {
                    double xr = xx[x]*cr - yy[y]*sr; //rotate
                    double yr = xx[x]*sr + yy[y]*cr;
                    
                    double e_x = -pow(damp_norm*(xr*lambda_norm-1.0), 2);
                    double e_y = -pow(yr*thick_norm, 2);
                    
                    double gauss = exp(e_x)*exp(e_y)-exp(-pow(damp_norm, 2));
                    
                    zz_morletRef.Timg_matrix[y][x]=physfftRef.Timg_matrix[y][x]*gauss;
                    zz_morletShot.Timg_matrix[y][x]=physfftShot.Timg_matrix[y][x]*gauss;
                    
                }
            }
            
            progress.setValue(++counter);
            QApplication::processEvents();
            
            physfftRef = zz_morletRef.ft2(PHYS_BACKWARD);
            progress.setValue(++counter);
            QApplication::processEvents();
            physfftShot = zz_morletShot.ft2(PHYS_BACKWARD);
            
            progress.setValue(++counter);
            QApplication::processEvents();
            
#pragma omp parallel for
            for (size_t kk=0; kk<dx*dy; kk++) {
                phase[k][0].Timg_buffer[kk] = -physfftRef.Timg_buffer[kk].arg()/(2*M_PI);
                contrast[k][0].Timg_buffer[kk] = 2.0*physfftRef.Timg_buffer[kk].mod()/(dx*dy);
                intensity[k][0].Timg_buffer[kk] -= contrast[k][0].point(kk)*cos(2*M_PI*phase[k][0].point(kk));
                
                phase[k][1].Timg_buffer[kk] = -physfftShot.Timg_buffer[kk].arg()/(2*M_PI);
                contrast[k][1].Timg_buffer[kk] = 2.0*physfftShot.Timg_buffer[kk].mod()/(dx*dy);
                intensity[k][1].Timg_buffer[kk] -= contrast[k][1].point(kk)*cos(2*M_PI*phase[k][1].point(kk));
            }
            progress.setValue(++counter);
            QApplication::processEvents();
            
            getPhase(k);

            progress.setValue(++counter);
            QApplication::processEvents();

            updatePlot();
            progress.setValue(++counter);
            QApplication::processEvents();
        } else {
            statusBar()->showMessage("size mismatch",5000);
        }
    }
}

void nVisar::getPhase() {
	if (sender() && sender()->property("id").isValid()) {
		int k=sender()->property("id").toInt();
		getPhase(k);
	} else {
		for (int k=0;k<2;k++){
			getPhase(k);
		}
	}
	updatePlot();
}

void nVisar::getPhase(int k) {
	if (visar[k].enableVisar->isChecked()) {
		disconnections();
		if (visar[k].refImage->currentIndex()!=-1 && visar[k].shotImage->currentIndex()!=-1) {
			QList<nPhysD*> imageList;
			imageList << getPhysFromCombo(visar[k].refImage) << getPhysFromCombo(visar[k].shotImage);
			for (int m=0;m<2;m++) {
                QRect geom2=fringeRect[k]->getRect(imageList[m]);
				QPolygonF shiftData, intensityData, contrastData;
				int intensityShift=visar[k].intensityShift->value();
				double offsetIntensity=(m==0?visar[k].offRef->value():visar[k].offShot->value());
				if (direction(k)==0) { //fringes are vertical
					for (int j=geom2.top(); j<geom2.bottom();j++) {
						shiftData  << QPointF(j,phase[k][m].point(geom2.center().x(),j,0));
						if (m==0) { //reference
							contrastData  << QPointF(j,contrast[k][m].point(geom2.center().x(),j-intensityShift,0));
						} else { //shot
							contrastData  << QPointF(j,contrast[k][m].point(geom2.center().x(),j-intensityShift,0));
						}
						double intensityTmp=0.0;
						for (int i=geom2.left(); i<geom2.right();i++) {
							if (m==0) { //reference
								intensityTmp+=intensity[k][m].point(i,j-intensityShift,0);
							} else { //shot
								intensityTmp+=intensity[k][m].point(i,j,0);
							}

						}
						double my_val=intensityTmp/geom2.width()-offsetIntensity;
						if (m==0) my_val *= visar[k].multRef->value();
						intensityData << QPointF(j,my_val);
					}
				} else { //fringes are horizontal
					for (int j=geom2.left(); j<geom2.right();j++) {
						shiftData  << QPointF(j,phase[k][m].point(j,geom2.center().y(),0));
						if (m==0) { //reference
							contrastData  << QPointF(j,contrast[k][m].point(j-intensityShift,geom2.center().y(),0));
						} else { //shot
							contrastData  << QPointF(j,contrast[k][m].point(j,geom2.center().y(),0));
						}

						double intensityTmp=0.0;
						for (int i=geom2.top(); i<geom2.bottom();i++) {
							if (m==0) { //reference
								intensityTmp+=intensity[k][m].point(j-intensityShift,i,0);
							} else { //shot
								intensityTmp+=intensity[k][m].point(j,i,0);
							}
						}
						double my_val=intensityTmp/geom2.height()-offsetIntensity;
						if (m==0) my_val *= visar[k].multRef->value();
						intensityData << QPointF(j,my_val);
					}
				}
				
				double buffer,bufferold,dummy=0.0;
				double offsetShift=0;
				if (setvisar[k].physScale->value() > 0) {
					bufferold=shiftData.first().y();
					for (int j=1;j<shiftData.size();j++){
						buffer=shiftData[j].y();
						if (fabs(buffer-bufferold)>0.5) dummy+=SIGN(bufferold-buffer);
						bufferold=buffer;
						shiftData[j].ry()+=dummy;
					}
					offsetShift=shiftData.first().y();
				} else {
					bufferold=shiftData.last().y();
					for (int j=shiftData.size()-2;j>=0;j--){
						buffer=shiftData[j].y();
						if (fabs(buffer-bufferold)>0.5) dummy+=SIGN(bufferold-buffer);
						bufferold=buffer;
						shiftData[j].ry()+=dummy;
					}
					offsetShift=shiftData.last().y();
				}
				setvisar[k].offset->setTitle("Offset "+QString::number(offsetShift));
				for (int j=0;j<shiftData.size();j++){
					shiftData[j].ry() -= offsetShift;
				}
				
				QPolygonF myLine;
				for (int i=0;i<shiftData.size();i++){
					if (direction(k)==0) {		//fringes are vertical
						myLine << QPointF(geom2.x()+geom2.width()/2.0+shiftData[i].y()*visar[k].interfringe->value(),shiftData[i].x());
					} else {
						myLine << QPointF(shiftData[i].x(),geom2.y()+geom2.height()/2.0-shiftData[i].y()*visar[k].interfringe->value());
					}
				}
				fringeLine[k]->setPoints(myLine);
				
				cPhase[m][k].setSamples(shiftData);
				cIntensity[m][k].setSamples(intensityData);
				cContrast[m][k].setSamples(contrastData);
			}
			visar[k].plotPhaseIntensity->setAxisAutoScale(QwtPlot::xBottom);
			visar[k].plotPhaseIntensity->setAxisAutoScale(QwtPlot::yLeft);
			visar[k].plotPhaseIntensity->setAxisAutoScale(QwtPlot::yRight);
			visar[k].plotPhaseIntensity->replot();
			zoomer[k]->setZoomBase();
			visar[k].plotPhaseIntensity->update();
		}
		connections();
	}
}

void
nVisar::export_txt_multiple() {
	QString fnametmp=QFileDialog::getSaveFileName(this,tr("Save VISARs and SOP"),property("fileTxt").toString(),tr("Text files (*.txt *.csv);;Any files (*)"));
	if (!fnametmp.isEmpty()) {
		setProperty("fileTxt", fnametmp);
		QFile t(fnametmp);
		t.open(QIODevice::WriteOnly| QIODevice::Text);
		QTextStream out(&t);
		for (int k=0;k<2;k++){
			out << export_one(k);
			out << endl << endl;
		}
		out << export_sop();
		t.close();
		statusBar()->showMessage(tr("Export in file:")+fnametmp,5000);
	}
}

void
nVisar::export_txt() {
	QString title=tr("Export ");
    switch (my_w.tabs->currentIndex()) {
		case 0:
            title=tr("VISAR")+QString(" ")+QString::number(my_w.tabPhase->currentIndex()+1);
			break;
		case 1:
            title=tr("VISAR")+QString(" ")+QString::number(my_w.tabVelocity->currentIndex()+1);
			break;
		case 2:
			title=tr("SOP");
			break;
	}
	QString fnametmp=QFileDialog::getSaveFileName(this,tr("Save ")+title,property("fileTxt").toString(),tr("Text files (*.txt *.csv);;Any files (*)"));
	if (!fnametmp.isEmpty()) {
		setProperty("fileTxt", fnametmp);
		QFile t(fnametmp);
		t.open(QIODevice::WriteOnly| QIODevice::Text);
		QTextStream out(&t);
        switch (my_w.tabs->currentIndex()) {
			case 0:
                out << export_one(my_w.tabPhase->currentIndex());
				break;
			case 1:
                out << export_one(my_w.tabVelocity->currentIndex());
				break;
			case 2:
				out << export_sop();
				break;
			default:
				break;
		}
		t.close();
		statusBar()->showMessage(tr("Export in file:")+fnametmp,5000);
	}
}

void
nVisar::export_clipboard() {
	QClipboard *clipboard = QApplication::clipboard();
    switch (my_w.tabs->currentIndex()) {
		case 0:
            clipboard->setText(export_one(my_w.tabPhase->currentIndex()));
            my_w.statusbar->showMessage(tr("Points copied to clipboard ")+my_w.tabPhase->tabText(my_w.tabPhase->currentIndex()));
			break;
		case 1:
			clipboard->setText(export_one(0)+"\n\n"+export_one(1));
			my_w.statusbar->showMessage(tr("Points copied to clipboard both visars"));
			break;
		case 2:
			clipboard->setText(export_sop());
			my_w.statusbar->showMessage(tr("Points copied to clipboard SOP"));
			break;
		default:
			break;
	}
}

QString
nVisar::export_sop() {
	QString out;
	out += QString("#SOP Origin       : %L1\n").arg(my_w.sopOrigin->value());
	out += QString("#SOP Offset       : %L1\n").arg(my_w.sopTimeOffset->value());
	out += QString("#SOP Time scale   : %L1\n").arg(my_w.sopScale->value());
	out += QString("#SOP Direction    : %L1\n").arg(my_w.sopDirection->currentIndex()==0 ? "Vertical" : "Horizontal");
	out += QString("#Time\tCounts\n");
	
    out += export_plot (my_w.sopPlot);
	
	return out;
}


QString
nVisar::export_plot(QwtPlot* my_plot) {
	QString out;	
	QList<QwtPlotCurve *> listCurve;
	const QwtPlotItemList& itmList = my_plot->itemList();
    for ( QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); ++it ) {
        if ( (*it)->rtti() == QwtPlotItem::Rtti_PlotCurve ) {
            listCurve << (QwtPlotCurve*)(*it);
        }
    }

    if (listCurve.size()>0) {
		for (unsigned int j=0;j<listCurve.at(0)->dataSize();j++) {
			out += QString("%L1\t").arg(listCurve.at(0)->sample(j).x(),10,'E',3);
			for (int i=0; i<listCurve.size();i++) {
				out += QString("%L1\t").arg(listCurve.at(i)->sample(j).y(),10,'E',3);
			}
			out += "\n";
		}
	}
	
	return out;
}

QString
nVisar::export_one(int k) {
	QString out;
    if (k<2) {
        if (visar[k].enableVisar->isChecked()) {
            out += QString("#VISAR %L1 Offset shift       : %L2\n").arg(QString::number(k+1)).arg(setvisar[k].offsetShift->value());
            out += QString("#VISAR %L1 Sensitivity        : %L2\n").arg(QString::number(k+1)).arg(setvisar[k].sensitivity->value());
            out += QString("#VISAR %L1 Reflectivity       : %L2 %L3\n").arg(QString::number(k+1)).arg(setvisar[k].reflOffset->value()).arg(setvisar[k].reflRef->value());
            out += QString("#VISAR %L1 Jumps              : %L2\n").arg(QString::number(k+1)).arg(setvisar[k].jumpst->text());
            out += QString("#Time\tVelocity\tReflectivity\tPixel\tRefShift\tShotShift\tRefInt\tShotInt\tRefContrast\tShotContrast\n");
            for (unsigned int j=0;j<cPhase[0][k].dataSize();j++) {
                out += QString("%L1\t%L2\t%L3\t%L4\t%L5\t%L6\t%L7\t%L8\t%L9\t%L10\n")
                .arg(velocity[k].sample(j).x(),10,'E',3)
                .arg(velocity[k].sample(j).y(),10,'E',3)
                .arg(reflectivity[k].sample(j).y(),10,'E',3)
                .arg((int)cPhase[0][k].sample(j).x())
                .arg(cPhase[0][k].sample(j).y(),10,'E',3)
                .arg(cPhase[1][k].sample(j).y(),10,'E',3)
                .arg(cIntensity[0][k].sample(j).y(),10,'E',3)
                .arg(cIntensity[1][k].sample(j).y(),10,'E',3)
                .arg(cContrast[0][k].sample(j).y(),10,'E',3)
                .arg(cContrast[1][k].sample(j).y(),10,'E',3);
            }
        }
    }
    return out;
}

void
nVisar::export_pdf() {
	QString fnametmp = QFileDialog::getSaveFileName(this,tr("Save Drawing"),property("fileExport").toString(),"Vector files (*.pdf,*.svg)");
	if (!fnametmp.isEmpty()) {
		setProperty("fileExport", fnametmp);
		QwtPlotRenderer renderer;
		renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);
//		renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames, true);
        
        double DPI=85;
        
        QSizeF my_size(150, 100);
        
        switch (my_w.tabs->currentIndex()) {
			case 0:
                renderer.renderDocument(visar[my_w.tabPhase->currentIndex()].plotPhaseIntensity, fnametmp, QFileInfo(fnametmp).suffix(), my_size, DPI);
				break;
			case 1:
				renderer.renderDocument(my_w.plotVelocity, fnametmp, QFileInfo(fnametmp).suffix(), my_size, DPI);
				break;
			case 2:
				renderer.renderDocument(my_w.sopPlot, fnametmp, QFileInfo(fnametmp).suffix(), my_size, DPI);
				break;
			default:
				break;
		}

	}

}


