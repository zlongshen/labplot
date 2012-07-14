/***************************************************************************
    File                 : ProjectDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
    Description          : widget for worksheet properties
                           
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef PROJECTDOCK_H
#define PROJECTDOCK_H

#include "ui_projectdock.h"
class Project;

//class Worksheet;

class ProjectDock: public QWidget{
	Q_OBJECT
	
public:
	ProjectDock(QWidget *parent);
	void setProject(Project* project);
	
private:
	Ui::ProjectDock ui;
	Project *m_project;
	bool m_initializing;

private slots:
	void retranslateUi();

	void titleChanged(const QString&);
	void authorChanged(const QString&);
	void commentChanged();
  
	void loadConfig(KConfig&);
	void saveConfig(KConfig&);
};

#endif // PROJECTDOCK_H