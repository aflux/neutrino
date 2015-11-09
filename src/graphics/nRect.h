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
#include <QtGui>
#include <QGraphicsObject>
#include "ui_nObject.h"
#include "nPhysImageF.h"

#ifndef __nRect
#define __nRect

class neutrino;
class QwtPlot;
class QwtPlotCurve;
class QwtPlotMarker;

class nRect : public QGraphicsObject {
	Q_OBJECT
public:
	
	nRect(neutrino *);
	~nRect();
	
	neutrino *parent(){
		return (neutrino *) QGraphicsObject::parent();
	};
	
	enum { Type = QGraphicsItem::UserType + 2 };
	int type() const { return Type;}
	
	void mousePressEvent ( QGraphicsSceneMouseEvent * );
	void mouseReleaseEvent ( QGraphicsSceneMouseEvent * );
	void mouseMoveEvent ( QGraphicsSceneMouseEvent * );
	void keyPressEvent ( QKeyEvent *);
	void keyReleaseEvent ( QKeyEvent *);
	void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * );
	void focusInEvent(QFocusEvent * event);
	void focusOutEvent(QFocusEvent * event);
	
	void moveBy(QPointF);
	
	qreal nWidth, nSizeHolder;
	QColor nColor, holderColor;
	
	// pure virtuals in QGraphicsObjec
	QRectF boundingRect() const;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
	
	QList<QGraphicsRectItem*> ref;
	QList<int> moveRef;

	QPointF click_pos;
	
	void changeP(int,QPointF,bool);
	
	double zoom;
	// roba da padelle
	QMainWindow my_pad;
	Ui::nObject my_w;
	
	QPainterPath path() const;
	QPainterPath shape() const;
	
	//	QList<QwtPlotMarker*> marker;
	void selectThis(bool);
	
	QwtPlot *my_qwt;
	QwtPlotCurve *lineOut;
	QList<QwtPlotMarker*> marker;
	
public slots:
	
	void togglePadella();
	
	void itemChanged();

	void interactive();
	
	void setRect(QRectF);
	QRect getRect(nPhysD* image =NULL);
	QRectF getRectF();
	QString getRectString();

    void bufferChanged(nPhysD*);

	void zoomChanged(double);
	void showMessage(QString);
	void changePointPad(int);
	void sizeHolder(double);
	void setWidthF(double);
	void setOrder(double);
	void changeToolTip(QString);
	void changeColor();
	void changeColor(QColor);
	void changeColorHolder();
	void changeColorHolder(QColor);
	void tableUpdated(QTableWidgetItem *);
	
	void expandX();
	void expandY();
	void intersection();
	void submatrix();
	
	void changeWidth();
	void changeHeight();
	
	void updateSize();
	
	void movePoints(QPointF);
	
	void appendPoint();
	void addPoint(int);
	
	void addPointAfterClick(QPointF);

	void setParentPan(QString,int);
	
	//SETTINGS
	void loadSettings();
	void saveSettings();
	void loadSettings(QSettings *);
	void saveSettings(QSettings *);
	
	
signals:
	void sceneChanged();
	void key_pressed(int);
};

#endif
