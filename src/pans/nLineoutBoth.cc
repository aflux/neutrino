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
#include "nLineoutBoth.h"
#include "neutrino.h"

nLineoutBoth::nLineoutBoth(neutrino *parent, QString win_name)
: nGenericPan(parent, win_name)
{
	my_w.setupUi(this);

    my_w.statusBar->addPermanentWidget(my_w.autoScale, 0);
    my_w.statusBar->addPermanentWidget(my_w.lockClick, 0);
    my_w.statusBar->addPermanentWidget(my_w.lockColors, 0);

    connect(parent, SIGNAL(bufferChanged(nPhysD*)), this, SLOT(updateLastPoint(void)));

    connect(my_w.autoScale, SIGNAL(released()), this, SLOT(updateLastPoint(void)));
    connect(parent, SIGNAL(nZoom(double)), this, SLOT(updateLastPoint(void)));

    connect(my_w.lockClick,SIGNAL(released()), this, SLOT(setBehaviour()));
    setBehaviour();

    my_w.plot->addGraph(my_w.plot->xAxis, my_w.plot->yAxis2);
    my_w.plot->graph(0)->setPen(QPen(Qt::red));
    my_w.plot->addGraph(my_w.plot->yAxis, my_w.plot->xAxis2);
    my_w.plot->graph(1)->setPen(QPen(Qt::blue));

    my_cursor[0]=new QCPItemLine(my_w.plot);
    my_cursor[1]=new QCPItemLine(my_w.plot);

    my_w.plot->xAxis2->setVisible(true);
    my_w.plot->yAxis2->setVisible(true);

    my_w.plot->yAxis->setRangeReversed(true);


    my_w.plot->xAxis->setTickLabelFont(nparent->my_w.my_view->font());
    my_w.plot->yAxis->setTickLabelFont(nparent->my_w.my_view->font());
    my_w.plot->xAxis2->setTickLabelFont(nparent->my_w.my_view->font());
    my_w.plot->yAxis2->setTickLabelFont(nparent->my_w.my_view->font());

    my_w.plot->xAxis->setLabel(tr("X (red)"));
    my_w.plot->xAxis->setLabelColor(Qt::red);
    my_w.plot->xAxis->setTickLabelColor(Qt::red);
    my_w.plot->yAxis2->setLabel(tr("X value (blue)"));
    my_w.plot->yAxis2->setLabelColor(Qt::red);
    my_w.plot->yAxis2->setTickLabelColor(Qt::red);
    my_w.plot->yAxis->setLabel(tr("Y (blue)"));
    my_w.plot->yAxis->setLabelColor(Qt::blue);
    my_w.plot->yAxis->setTickLabelColor(Qt::blue);
    my_w.plot->xAxis2->setLabel(tr("Y value (blue)"));
    my_w.plot->xAxis2->setLabelColor(Qt::blue);
    my_w.plot->xAxis2->setTickLabelColor(Qt::blue);


    my_w.plot->xAxis->setLabelFont(nparent->my_w.my_view->font());
    my_w.plot->yAxis->setLabelFont(nparent->my_w.my_view->font());
    my_w.plot->xAxis2->setLabelFont(nparent->my_w.my_view->font());
    my_w.plot->yAxis2->setLabelFont(nparent->my_w.my_view->font());

//    my_w.plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables | QCP::iMultiSelect | QCP::iSelectItems | QCP::iSelectOther);

	decorate();
	updateLastPoint();
    
}

void nLineoutBoth::setBehaviour() {
    if (my_w.lockClick->isChecked()) {
        disconnect(nparent->my_w.my_view, SIGNAL(mouseposition(QPointF)), this, SLOT(updatePlot(QPointF)));
        connect(nparent->my_w.my_view, SIGNAL(mousePressEvent_sig(QPointF)), this, SLOT(updatePlot(QPointF)));
    } else {
        disconnect(nparent->my_w.my_view, SIGNAL(mousePressEvent_sig(QPointF)), this, SLOT(updatePlot(QPointF)));
        connect(nparent->my_w.my_view, SIGNAL(mouseposition(QPointF)), this, SLOT(updatePlot(QPointF)));
    }
}

// mouse movement
void nLineoutBoth::updatePlot(QPointF p) {

    if (currentBuffer != NULL) {
        //get bounds from schermo
        QPointF orig = nparent->my_w.my_view->mapToScene(QPoint(0,0));
        QPointF corner = nparent->my_w.my_view->mapToScene(QPoint(nparent->my_w.my_view->width(), nparent->my_w.my_view->height()));

        int b_o[2], b_c[2], b_p[2];

        b_o[0] = int(orig.x()); b_o[1] = int(orig.y());
        b_c[0] = int(corner.x()); b_c[1] = int(corner.y());
        b_p[0] = int(p.x()); b_p[1] = int(p.y());


        for (int k=0;k<2;k++) {
			
            double vmin=0, vmax=0;
            const double *dvec = currentBuffer->to_dvector((phys_direction)k, b_p[(k+1)%2]);
            size_t lat_skip = max(b_o[k], 0);

            size_t z_size = min(b_c[k]-lat_skip, currentBuffer->getSizeByIndex((phys_direction)k)-lat_skip);
            phys_get_vec_brightness(dvec+lat_skip, z_size, vmin, vmax);

            QVector<double> x(z_size);
            for (unsigned int i=0;i<z_size;i++){
                x[i]=i+lat_skip;
            }
            QVector<double> y(z_size);
            if (k==0) {
                for (unsigned int i=0;i<z_size;i++){
                    y[i]=currentBuffer->point(i+lat_skip,b_p[(k+1)%2]);
                }
                my_cursor[k]->start->setCoords( p.x(), QCPRange::minRange);
                my_cursor[k]->end->setCoords( p.x(), QCPRange::maxRange);
            } else {
                for (unsigned int i=0;i<z_size;i++){
                    y[i]=currentBuffer->point(b_p[(k+1)%2],i+lat_skip);
                }
                my_cursor[k]->start->setCoords(QCPRange::minRange, p.y());
                my_cursor[k]->end->setCoords(QCPRange::maxRange, p.y());
            }
            my_w.plot->graph(k)->setData(x,y);

            my_w.plot->graph(k)->keyAxis()->setRange(lat_skip, lat_skip+z_size);

            if(my_w.autoScale->isChecked()) {
                QVector<double>::iterator minY = std::min_element(y.begin(), y.end());
                QVector<double>::iterator maxY = std::max_element(y.begin(), y.end());
                my_w.plot->graph(k)->valueAxis()->setRange(*minY,*maxY);
            } else {
                if(my_w.lockColors->isChecked()) {
                    vec2f rang=currentBuffer->property["display_range"];
                    my_w.plot->graph(k)->valueAxis()->setRange(rang.x(),rang.y());
                } else {
                    my_w.plot->graph(k)->valueAxis()->setRange(currentBuffer->get_min(), currentBuffer->get_max());
                }
            }
        }


        statusBar()->showMessage(tr("Point (")+QString::number(p.x())+","+QString::number(p.y())+")="+QString::number(currentBuffer->point(p.x(),p.y())));
        my_w.plot->replot();
    }
}

void nLineoutBoth::updateLastPoint() {
    if (!my_w.lockClick->isChecked()) {
        updatePlot(nparent->my_mouse.pos());
    }
}




