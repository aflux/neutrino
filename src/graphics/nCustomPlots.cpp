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

#include "nCustomPlots.h"
#include "neutrino.h"

nCustomPlot::nCustomPlot(QWidget* parent):
    QCustomPlot(parent)
{
    connect(this, SIGNAL(axisClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(my_axisClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)));
    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

}

void nCustomPlot::my_axisClick(QCPAxis*ax,QCPAxis::SelectablePart,QMouseEvent*) {
    DEBUG("Here");
    axisRect()->setRangeDragAxes(ax,ax);
    axisRect()->setRangeDrag(ax->orientation());
    axisRect()->setRangeZoomAxes(ax,ax);
    axisRect()->setRangeZoom(ax->orientation());
}



//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////                       /////////////////
//////////////         VISAR         /////////////////
//////////////                       /////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////


nVisarPlot::nVisarPlot(QWidget* parent):
    nCustomPlot(parent),
    yAxis3(axisRect(0)->addAxis(QCPAxis::atRight,0)),
    mouseMarker(this)
{
    yAxis2->setVisible(true);

    yAxis->setLabelColor(Qt::red);
    yAxis->setTickLabelColor(Qt::red);

    yAxis2->setLabelColor(Qt::blue);
    yAxis2->setTickLabelColor(Qt::blue);

    yAxis3->setLabelColor(Qt::darkCyan);
    yAxis3->setTickLabelColor(Qt::darkCyan);

    QList<QCPAxis*> all_axis({xAxis,xAxis2,yAxis,yAxis2,yAxis3});

    foreach(QCPAxis* axis, all_axis) {
        axis->setLabelPadding(-1);
    }

    QSettings settings("neutrino","");
    settings.beginGroup("Preferences");
    QVariant fontString=settings.value("defaultFont");
    if (fontString.isValid()) {
        QFont fontTmp;
        if (fontTmp.fromString(fontString.toString())) {
            foreach(QCPAxis* axis, all_axis) {
                axis->setTickLabelFont(fontTmp);
                axis->setLabelFont(fontTmp);
            }
        }
    }
}

void nVisarPlot::setMousePosition(double position) {
    mouseMarker.start->setCoords(position, QCPRange::minRange);
    mouseMarker.end->setCoords(position, QCPRange::maxRange);
    replot();
}
