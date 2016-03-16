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
#include <QWidget>

#include "nPlug.h"
#include "nGenericPan.h"

// you should include here the relevant (if any) ui_??.h
//#include "ui_skel.h"

#ifndef __skel_plugin
#define __skel_plugin

class neutrino;

// This object does the real work, here you write a nGenericPan as if it were in the main tree
class mySkelGUI : public nGenericPan {
Q_OBJECT
public:
	mySkelGUI(neutrino *, QString);

	Ui::skel my_w;

public slots:

	// here the GUI slots

private:

	// here your private stuff

};


// the skel object is in charge of reconstructing connections (runtime) with neutrino. It is in charge
// of the real object instantiation.
class skel : public QObject, nPlug {
Q_OBJECT
Q_INTERFACES(nPlug)

public:
	skel();

	~skel()
	{ std::cerr<<"~skel"<<  std::endl; }
	
	QString name()
	{ return QString("My skel plugin"); }

	bool instantiate(neutrino *); // where the construction is performed

	bool unload(); // where we dismantle everything when politely asked to
	{ }
	
	nGenericPan *my_GP;

public slots:
	void pan_closed(QObject *);

signals:
	void plugin_died(QObject *);

private:
	neutrino *nparent;

};



#endif
