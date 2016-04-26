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
#include "nLineout.h"

nLineout::nLineout(neutrino *parent, QString win_name, enum phys_direction plot_dir)
: nGenericPan(parent, win_name), cut_dir(plot_dir)
{
	my_w.setupUi(this);

    connect(my_w.actionToggleZoom,SIGNAL(triggered()), this, SLOT(updateLastPoint()));

    connect(parent, SIGNAL(bufferChanged(nPhysD*)), this, SLOT(updateLastPoint()));

    connect(my_w.actionLockClick,SIGNAL(triggered()), this, SLOT(setBehaviour()));
    setBehaviour();
    
    my_w.plot->addGraph(my_w.plot->xAxis, my_w.plot->yAxis);
    my_w.plot->graph(0)->setPen(QPen(Qt::black));

    my_cursor=new QCPItemLine(my_w.plot);

    my_w.plot->xAxis->setTickLabelFont(nparent->my_w.my_view->font());
    my_w.plot->yAxis->setTickLabelFont(nparent->my_w.my_view->font());

    decorate();
    updateLastPoint();
}

void nLineout::setBehaviour() {
    if (my_w.actionLockClick->isChecked()) {
        disconnect(nparent->my_w.my_view, SIGNAL(mouseposition(QPointF)), this, SLOT(updatePlot(QPointF)));
        connect(nparent->my_w.my_view, SIGNAL(mousePressEvent_sig(QPointF)), this, SLOT(updatePlot(QPointF)));
    } else {
        disconnect(nparent->my_w.my_view, SIGNAL(mousePressEvent_sig(QPointF)), this, SLOT(updatePlot(QPointF)));
        connect(nparent->my_w.my_view, SIGNAL(mouseposition(QPointF)), this, SLOT(updatePlot(QPointF)));
    }
}


// mouse movemen
void
nLineout::updatePlot(QPointF p) {

    if (currentBuffer != NULL) {
        //get bounds from schermo
        QPointF orig,corner;

        if (my_w.actionToggleZoom->isChecked()) {
            orig = nparent->my_w.my_view->mapToScene(QPoint(0,0));
            corner = nparent->my_w.my_view->mapToScene(QPoint(nparent->my_w.my_view->width(), nparent->my_w.my_view->height()));
        } else {
            orig = QPoint(0,0);
            corner = QPoint(currentBuffer->getW(),currentBuffer->getH());
        }

        int b_o[2], b_c[2], b_p[2];

        b_o[0] = int(orig.x()); b_o[1] = int(orig.y());
        b_c[0] = int(corner.x()); b_c[1] = int(corner.y());
        b_p[0] = int(p.x()); b_p[1] = int(p.y());

        int k=(int)cut_dir;

        size_t lat_skip = max(b_o[k], 0);
        size_t z_size = min(b_c[k]-lat_skip, currentBuffer->getSizeByIndex((phys_direction)k)-lat_skip);

        QVector<double> x(z_size);
        for (unsigned int i=0;i<z_size;i++){
            x[i]=(i+lat_skip-currentBuffer->get_origin((phys_direction)k))*currentBuffer->get_scale((phys_direction)k);
        }
        QVector<double> y(z_size);
        if (k==0) {
            for (unsigned int i=0;i<z_size;i++){
                y[i]=currentBuffer->point(i+lat_skip,b_p[(k+1)%2]);
            }
        } else {
            for (unsigned int i=0;i<z_size;i++){
                y[i]=currentBuffer->point(b_p[(k+1)%2],i+lat_skip);
            }
        }

        my_cursor->start->setCoords(b_p[k], QCPRange::minRange);
        my_cursor->end->setCoords(b_p[k], QCPRange::maxRange);

        my_w.plot->graph(0)->setData(x,y);
        my_w.plot->graph(0)->keyAxis()->setRange(lat_skip, lat_skip+z_size);

        if(!my_w.actionAutoscale->isChecked()) {
            QVector<double>::iterator minY = std::min_element(y.begin(), y.end());
            QVector<double>::iterator maxY = std::max_element(y.begin(), y.end());
            my_w.plot->graph(0)->valueAxis()->setRange(*minY,*maxY);
        } else {
            if(my_w.actionLockColors->isChecked()) {
                vec2f rang=currentBuffer->property["display_range"];
                my_w.plot->graph(0)->valueAxis()->setRange(rang.x(),rang.y());
            } else {
                my_w.plot->graph(0)->valueAxis()->setRange(currentBuffer->get_min(), currentBuffer->get_max());
            }
        }
        statusBar()->showMessage(tr("Point (")+QString::number(p.x())+","+QString::number(p.y())+")="+QString::number(currentBuffer->point(p.x(),p.y())));
        my_w.plot->replot();
    }

}

void nLineout::nZoom(double d) {
    updateLastPoint();
}

void nLineout::updateLastPoint() {
    updatePlot(nparent->my_mouse.pos());
}



