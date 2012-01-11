/***************************************************************************
    File                 : Set.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : generic set class
                           
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
#include "Set.h"

Set::Set(){
	m_shown=true;
}

Set::Set(const Set::SetType type){
	this->setType(type);
}

void Set::setType(const Set::SetType type){
	if (type==SET2D){
		list_ranges<<Range()<<Range();
		list_numbers<<100;
	}else if (type==SET3D){
		list_ranges<<Range()<<Range()<<Range();
		list_numbers<<100<<100;
	}
	m_type=type;
	m_shown = true;
}

Set::SetType Set::type() const{
	return m_type;
}

// Set::Set(QString name, int number)
// 	: m_name(name), m_number(number){
//
// 	m_label = new Label();
// //	source = src;
// //	type = t;
// //	style = st;
// //	if(style == 0)
// 	m_style = new Style();
// //	symbol = sy;
// //	if(symbol == 0)
//
// // 	m_symbol = new Symbol();
// 	m_shown = true;
//
// //	readas = 0;
// //	fitfunction = QString("a*x+b");
// }


/*void Graph::saveGraph(QTextStream *t) {
	*t<<name<<endl;
	// OLD : *t<<label<<endl;
	label->save(t);
	*t<<shown<<endl;
	*t<<(int)source<<endl;
	*t<<readas<<endl;
	av.save(t);
	*t<<fitfunction<<endl;
}

// open Graph specfic things
void Graph::openGraph(QTextStream *t,int version) {
	if (version > 2) {
		// name + label
		t->readLine();
		name = t->readLine();
		if(version > 21)
			label->open(t,version,false);
		else {
			QString title = t->readLine();
			label->setTitle(title);
		}

		if(version>14) {
			int tmp;
			*t>>tmp;
			shown = (bool) tmp;
			if(version>17) {
				*t>>tmp;
				source = (LSource) tmp;
				if(version>18)
					*t>>readas;
			}
			av.open(t, version);
			if(version>18) {
				t->readLine();
				fitfunction = t->readLine();
				//kDebug()<<"FIT FUNCTION : "<<fitfunction<<endl;
			}
		}
	}
}

QDomElement Graph::saveGraphXML(QDomDocument doc, int gtype) {
	kDebug()<<"Graph::saveGraphXML()"<<endl;
	QDomElement graphtag = doc.createElement( "Graph" );
	graphtag.setAttribute("type",QString::number(gtype));

	QDomElement tag = doc.createElement( "Name" );
    	graphtag.appendChild( tag );
  	QDomText t = doc.createTextNode( name );
    	tag.appendChild( t );
	tag = doc.createElement( "Number" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(number) );
    	tag.appendChild( t );
	tag = doc.createElement( "PlotType" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(type) );
    	tag.appendChild( t );

	tag = label->saveXML(doc);
    	graphtag.appendChild( tag );

	tag = doc.createElement( "Shown" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(shown) );
    	tag.appendChild( t );
	tag = doc.createElement( "Source" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(source) );
    	tag.appendChild( t );
	tag = doc.createElement( "ReadAs" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( QString::number(readas) );
    	tag.appendChild( t );
	tag = doc.createElement( "FitFunction" );
    	graphtag.appendChild( tag );
  	t = doc.createTextNode( fitfunction );
    	tag.appendChild( t );

	tag = av.saveXML(doc);
    	graphtag.appendChild( tag );
	tag = style->saveXML(doc);
    	graphtag.appendChild( tag );
	tag = symbol->saveXML(doc);
    	graphtag.appendChild( tag );

	kDebug()<<"	calling saveXML()"<<endl;
	saveXML(doc,graphtag);
	kDebug()<<"	done"<<endl;

	return graphtag;
}

void Graph::openGraphXML(QDomElement e) {
	if(e.tagName() == "Name" )
		name = e.text();
	if(e.tagName() == "Label" )
		label->openXML(e.firstChild());
	else if(e.tagName() == "Number" )
		number = e.text().toInt();
	else if(e.tagName() == "PlotType" )
		type = (PType) e.text().toInt();
	else if(e.tagName() == "Shown" )
		shown = (bool) e.text().toInt();
	else if(e.tagName() == "Source" )
		source = (LSource) e.text().toInt();
	else if(e.tagName() == "ReadAs" )
		readas = e.text().toInt();
	else if(e.tagName() == "FitFunction" )
		fitfunction = e.text();
	else if(e.tagName() == "Annotate" )
		av.openXML(e.firstChild());
	else if(e.tagName() == "Style" )
		style->openXML(e.firstChild());
	else if(e.tagName() == "Symbol" )
		symbol->openXML(e.firstChild());
}
*/
void Set::drawStyle(QPainter *p, int x, int y) {
/*	if(style->Type() != NOLINESTYLE)
		p->drawLine(x,y,x+30,y);
	symbol->draw(p,x+15,y);
*/
}