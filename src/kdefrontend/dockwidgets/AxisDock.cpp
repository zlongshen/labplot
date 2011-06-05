/***************************************************************************
    File                 : AxisDock.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 20011 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : axes widget class

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
#include "AxisDock.h"
#include "worksheet/Axis.h"
#include "kdefrontend/GuiTools.h"
#include <KMessageBox>
#include <QTimer>


 /*!
  \class AxisDock
  \brief Provides a widget for editing the properties of the axes currently selected in the project explorer.

  \ingroup kdefrontend
*/
 
AxisDock::AxisDock(QWidget* parent):QWidget(parent){

  ui.setupUi(this);

  //TODO
  ui.lPosition->hide();
  ui.cbPosition->hide();
  
  //adjust layouts in the tabs
  QGridLayout* layout;
  for (int i=0; i<ui.tabWidget->count(); ++i){
	layout=static_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
	if (!layout)
	  continue;
	
	layout->setContentsMargins(2,2,2,2);
	layout->setHorizontalSpacing(2);
	layout->setVerticalSpacing(2);
  }

	//**********************************  Slots **********************************************

	//"General"-tab
	connect( ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( ui.chkVisible, SIGNAL(stateChanged(int)), this, SLOT(visibilityChanged(int)) );
	
	connect( ui.cbOrientation, SIGNAL(currentIndexChanged(int)), this, SLOT(orientationChanged(int)) );
	connect( ui.cbPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(positionChanged(int)) );
	connect( ui.lePositionOffset, SIGNAL(returnPressed()), this, SLOT(positionOffsetChanged()) );
	connect( ui.cbScale, SIGNAL(currentIndexChanged(int)), this, SLOT(scaleChanged(int)) );
	
	connect( ui.leStart, SIGNAL(returnPressed()), this, SLOT(startChanged()) );
	connect( ui.leEnd, SIGNAL(returnPressed()), this, SLOT(endChanged()) );
	connect( ui.leZeroOffset, SIGNAL(returnPressed()), this, SLOT(zeroOffsetChanged()) );
	connect( ui.leScalingFactor, SIGNAL(returnPressed()), this, SLOT(scalingFactorChanged()) );
	
	//"Line"-tab
	connect( ui.cbLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(lineStyleChanged(int)) );
	connect( ui.kcbLineColor, SIGNAL(changed (const QColor &)), this, SLOT(lineColorChanged(const QColor&)) );
	connect( ui.sbLineWidth, SIGNAL(valueChanged(double)), this, SLOT(lineWidthChanged(double)) );
	connect( ui.sbLineOpacity, SIGNAL(valueChanged(int)), this, SLOT(lineOpacityChanged(int)) );	
	

	//"Title"-tab
//	connect( labelWidget, SIGNAL(dataChanged(bool)), this, SLOT(slotDataChanged()) );

	//"Major ticks"-tab
	connect( ui.cbMajorTicksDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksDirectionChanged(int)) );
	connect( ui.cbMajorTicksType, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksTypeChanged(int)) );
	connect( ui.sbMajorTicksNumber, SIGNAL(valueChanged(int)), this, SLOT(majorTicksNumberChanged(int)) );
 	connect( ui.leMajorTicksIncrement, SIGNAL(returnPressed()), this, SLOT(majorTicksIncrementChanged()) );
	connect( ui.cbMajorTicksLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(majorTicksLineStyleChanged(int)) );
	connect( ui.kcbMajorTicksColor, SIGNAL(changed (const QColor &)), this, SLOT(majorTicksColorChanged(const QColor&)) );
	connect( ui.sbMajorTicksWidth, SIGNAL(valueChanged(int)), this, SLOT(majorTicksWidthChanged(int)) );
	connect( ui.sbMajorTicksLength, SIGNAL(valueChanged(int)), this, SLOT(majorTicksLengthChanged(int)) );
	connect( ui.sbMajorTicksOpacity, SIGNAL(valueChanged(int)), this, SLOT(majorTicksOpacityChanged(int)) );

	//"Minor ticks"-tab
	connect( ui.cbMinorTicksDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksDirectionChanged(int)) );
	connect( ui.cbMinorTicksType, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksTypeChanged(int)) );
	connect( ui.sbMinorTicksNumber, SIGNAL(valueChanged(int)), this, SLOT(minorTicksNumberChanged(int)) );
 	connect( ui.leMinorTicksIncrement, SIGNAL(returnPressed()), this, SLOT(minorTicksIncrementChanged()) );
	connect( ui.cbMinorTicksLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(minorTicksLineStyleChanged(int)) );
	connect( ui.kcbMinorTicksColor, SIGNAL(changed (const QColor &)), this, SLOT(minorTicksColorChanged(const QColor&)) );
	connect( ui.sbMinorTicksWidth, SIGNAL(valueChanged(int)), this, SLOT(minorTicksWidthChanged(int)) );
	connect( ui.sbMinorTicksLength, SIGNAL(valueChanged(int)), this, SLOT(minorTicksLengthChanged(int)) );
	connect( ui.sbMinorTicksOpacity, SIGNAL(valueChanged(int)), this, SLOT(minorTicksOpacityChanged(int)) );
	
	//"Extra ticks"-tab
	
	//"Tick labels"-tab
	connect( ui.cbLabelsPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(labelsPositionChanged(int)) );
	connect( ui.sbLabelsOffset, SIGNAL(valueChanged(int)), this, SLOT(labelsOffsetChanged(int)) );
	connect( ui.sbLabelsRotation, SIGNAL(valueChanged(int)), this, SLOT(labelsRotationChanged(int)) );
	connect( ui.kfrLabelsFont, SIGNAL(fontSelected(const QFont& )), this, SLOT(labelsFontChanged(const QFont&)) );
	connect( ui.kcbLabelsFontColor, SIGNAL(changed (const QColor &)), this, SLOT(labelsFontColorChanged(const QColor&)) );
	connect( ui.leLabelsPrefix, SIGNAL(returnPressed()), this, SLOT(labelsPrefixChanged()) );
	connect( ui.leLabelsSuffix, SIGNAL(returnPressed()), this, SLOT(labelsSuffixChanged()) );
	connect( ui.sbLabelsOpacity, SIGNAL(valueChanged(int)), this, SLOT(labelsOpacityChanged(int)) );
	
	/*

	connect( ui.sbLabelsPrecision, SIGNAL(valueChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbLabelsFormat, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(labelFormatChanged(const QString&)) );
	connect( ui.leLabelsDateFormat, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	//"Grid"-tab
	connect( ui.chbMajorGrid, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbMajorGridStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.kcbMajorGridColor, SIGNAL(changed (const QColor &)), this, SLOT(createMajorGridStyles()) );
	connect( ui.sbMajorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMajorGridStyles()) );

	connect( ui.chbMinorGrid, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.cbMinorGridStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.kcbMinorGridColor, SIGNAL(changed (const QColor &)), this, SLOT(createMinorGridStyles()) );
	connect( ui.sbMinorGridWidth, SIGNAL(valueChanged(int)), this, SLOT(createMinorGridStyles()) );
	*/

//TODO cbMajorTicksDirection is empty when the axes are set
	//QTimer::singleShot(0, this, SLOT(init()));
	init();
}

AxisDock::~AxisDock(){}

void AxisDock::init(){
	m_initializing=true;

// 	//create a LabelWidget in the "Title"-tab
//     QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tabTitle);
// 	labelWidget=new LabelWidget(ui.tabTitle);
//     hboxLayout->addWidget(labelWidget);

	//Validators
	ui.lePositionOffset->setValidator( new QDoubleValidator(ui.lePositionOffset) );
	ui.leStart->setValidator( new QDoubleValidator(ui.leStart) );
	ui.leEnd->setValidator( new QDoubleValidator(ui.leEnd) );
	ui.leZeroOffset->setValidator( new QDoubleValidator(ui.leZeroOffset) );
	ui.leScalingFactor->setValidator( new QDoubleValidator(ui.leScalingFactor) );
	
	ui.leMajorTicksIncrement->setValidator( new QDoubleValidator(ui.leMajorTicksIncrement) );
	ui.leMinorTicksIncrement->setValidator( new QDoubleValidator(ui.leMinorTicksIncrement) );

	//TODO move this stuff to retranslateUI()
	ui.cbScale->addItem( i18n("linear") );
	ui.cbScale->addItem( i18n("log(x)") );
	ui.cbScale->addItem( i18n("log2(x)") );
	ui.cbScale->addItem( i18n("ln(x)") );
	ui.cbScale->addItem( i18n("sqrt(x)") );
	ui.cbScale->addItem( i18n("x^2") );

	ui.cbOrientation->addItem( i18n("horizontal") );
	ui.cbOrientation->addItem( i18n("vertical") );
	
	ui.cbMajorTicksDirection->addItem( i18n("none") );
	ui.cbMajorTicksDirection->addItem( i18n("in") );
	ui.cbMajorTicksDirection->addItem( i18n("out") );
	ui.cbMajorTicksDirection->addItem( i18n("in and out") );

	ui.cbMinorTicksDirection->addItem( i18n("none") );
	ui.cbMinorTicksDirection->addItem( i18n("in") );
	ui.cbMinorTicksDirection->addItem( i18n("out") );
	ui.cbMinorTicksDirection->addItem( i18n("in and out") );
	
	//TODO: Tick labels format

	//Grid
	//TODO: remove this later
	 ui.kcbMajorGridColor->setColor(Qt::black);
	 GuiTools::updatePenStyles(ui.cbMajorGridStyle, QColor(Qt::black));
	 ui.kcbMinorGridColor->setColor(Qt::black);
	 GuiTools::updatePenStyles(ui.cbMinorGridStyle, QColor(Qt::black));
	 
	m_initializing=false;
}


/*!
  sets the axes. The properties of the axes in the list \c list can be edited in this widget.
*/
void AxisDock::setAxes(QList<Axis*> list){
  m_initializing=true;
  m_axesList=list;
  Axis* axis=list.first();
  
  //if there are more then one axis in the list, disable the tab "general"
  if (list.size()==1){
	ui.lName->setEnabled(true);
	ui.leName->setEnabled(true);
	ui.lComment->setEnabled(true);
	ui.leComment->setEnabled(true);
	ui.leName->setText(axis->name());
	ui.leComment->setText(axis->comment());
  }else{
	ui.lName->setEnabled(false);
	ui.leName->setEnabled(false);
	ui.lComment->setEnabled(false);
	ui.leComment->setEnabled(false);
	ui.leName->setText("");
	ui.leComment->setText("");	
  }

  //show the properties of the first axis
  
 	//"General"-tab
  ui.chkVisible->setChecked(axis->isVisible());
  ui.cbOrientation->setCurrentIndex( axis->orientation() );
// TODO	ui.cbPosition->setCurrentIndex( axis->position() );
  ui.lePositionOffset->setText( QString::number(axis->offset()) );
  ui.cbScale->setCurrentIndex( axis->scale() );
  ui.leStart->setText( QString::number(axis->start()) );
  ui.leEnd->setText( QString::number(axis->end()) );
  ui.leZeroOffset->setText( QString::number(axis->zeroOffset()) );
  ui.leScalingFactor->setText( QString::number(axis->scalingFactor()) );

 
  //   "Title"-tab

  //Line-tab
  ui.cbLineStyle->setCurrentIndex( axis->linePen().style() );
  ui.kcbLineColor->setColor( axis->linePen().color() );
  ui.sbLineWidth->setValue( axis->linePen().width() );
  ui.sbLineOpacity->setValue( axis->lineOpacity()*100 );
  GuiTools::updatePenStyles(ui.cbLineStyle, axis->linePen().color() );
  
	//"Major ticks"-tab
 	ui.cbMajorTicksDirection->setCurrentIndex( axis->majorTicksDirection() );
	ui.cbMajorTicksType->setCurrentIndex( axis->majorTicksType() );
	this->majorTicksTypeChanged(axis->majorTicksType());
	ui.sbMajorTicksNumber->setValue(axis->majorTicksNumber());
	ui.leMajorTicksIncrement->setText(QString::number(axis->majorTicksIncrement()));
	ui.cbMajorTicksLineStyle->setCurrentIndex( axis->majorTicksPen().style() );
	ui.kcbMajorTicksColor->setColor( axis->majorTicksPen().color() );
	ui.sbMajorTicksWidth->setValue( axis->majorTicksPen().width() );
	ui.sbMajorTicksLength->setValue( axis->majorTicksLength() );
	ui.sbMajorTicksOpacity->setValue( axis->majorTicksOpacity()*100 );
	GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, axis->majorTicksPen().color() );

	//"Minor ticks"-tab
 	ui.cbMinorTicksDirection->setCurrentIndex( axis->minorTicksDirection() );
	ui.cbMinorTicksType->setCurrentIndex( axis->minorTicksType() );
	this->minorTicksTypeChanged(axis->minorTicksType());
	ui.sbMinorTicksNumber->setValue(axis->minorTicksNumber());
	ui.leMinorTicksIncrement->setText(QString::number(axis->minorTicksIncrement()));
	ui.cbMinorTicksLineStyle->setCurrentIndex( axis->minorTicksPen().style() );
	ui.kcbMinorTicksColor->setColor( axis->minorTicksPen().color() );
	ui.sbMinorTicksWidth->setValue( axis->minorTicksPen().width() );
	ui.sbMinorTicksLength->setValue( axis->minorTicksLength() );
	ui.sbMinorTicksOpacity->setValue( axis->minorTicksOpacity()*100 );
	GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, axis->minorTicksPen().color() );
	
	//"Extra ticks"-tab
	
 	//"Tick labels"-tab
	ui.cbLabelsPosition->setCurrentIndex( axis->labelsPosition() );
	ui.sbLabelsRotation->setValue( axis->labelsRotationAngle() );
	ui.kfrLabelsFont->setFont( axis->labelsFont() );
	ui.kcbLabelsFontColor->setColor( axis->labelsColor() );
	ui.leLabelsPrefix->setText( axis->labelsPrefix() );
	ui.leLabelsSuffix->setText( axis->labelsSuffix() );
	ui.sbLabelsOpacity->setValue( axis->labelsOpacity()*100 );
	
	// 	//*******************   "Grid"-tab  ************************************
// 	ui.chbMajorGrid->setChecked( axis->hasMajorGrid() );
// 	ui.cbMajorGridStyle->setCurrentIndex( axis->majorGridStyle()-1 );
// 	ui.kcbMajorGridColor->setColor( axis->majorGridColor() );
// 	ui.sbMajorGridWidth->setValue( axis->majorGridWidth() );
// 
// 	ui.chbMinorGrid->setChecked( axis->hasMinorGrid() );
// 	ui.cbMinorGridStyle->setCurrentIndex( axis->minorGridStyle()-1 );
// 	ui.kcbMinorGridColor->setColor( axis->minorGridColor() );
// 	ui.sbMinorGridWidth->setValue( axis->minorGridWidth() );

  m_initializing = false;
}

//**********************************************************
//****************** SLOTS *******************************
//**********************************************************

//"General"-tab
void AxisDock::nameChanged(){
  if (m_initializing)
	return;
  
  m_axesList.first()->setName(ui.leName->text());
}

void AxisDock::commentChanged(){
  if (m_initializing)
	return;
  
  m_axesList.first()->setComment(ui.leComment->text());
}

void AxisDock::visibilityChanged(int state){
  if (m_initializing)
	return;
  
  bool b;
  if (state==Qt::Checked)
	b=true;
  else
	b=false;

  foreach(Axis* axis, m_axesList){
	axis->setVisible(b);
  }
}

/*!
	called if the orientation (horizontal or vertical) of the current axis is changed.
*/

void AxisDock::orientationChanged(int index){
   Axis::AxisOrientation orientation = (Axis::AxisOrientation)index;
  int oldIndex = ui.cbPosition->currentIndex();
  int oldLabelsIndex = ui.cbLabelsPosition->currentIndex();
  
  ui.cbPosition->clear();
  ui.cbLabelsPosition->clear();
  ui.cbLabelsPosition->addItem(i18n("no labels"));
  if (orientation == Axis::AxisHorizontal){
	ui.cbPosition->addItem( i18n("bottom") );
	ui.cbPosition->addItem( i18n("top") );
	ui.cbLabelsPosition->addItem( i18n("top") );
	ui.cbLabelsPosition->addItem( i18n("bottom") );	
  }else{//vertical
	ui.cbPosition->addItem( i18n("left") );
	ui.cbPosition->addItem( i18n("right") );
	ui.cbLabelsPosition->addItem( i18n("right") );
	ui.cbLabelsPosition->addItem( i18n("left") );
  }
  ui.cbPosition->addItem( i18n("custom") );	  
  ui.cbPosition->setCurrentIndex(oldIndex);
  ui.cbLabelsPosition->setCurrentIndex(oldLabelsIndex);
  
    if (m_initializing)
	  return;
	
  foreach(Axis* axis, m_axesList){
	axis->setOrientation(orientation);
  }
}

//TODO
void AxisDock::positionChanged(int index){
//   if (m_initializing)
// 	return;
// 
// 	double offset;
// 	if (index==1){//under or left
// 	  ofset=0;
// 	  ui.lPositionOffset->hide();
// 	  ui.lePositionOffset->hide();
// 	  
// 	Axis::AxisPosition position;
//   	if (index==2){ //custom position
// 		ui.lPositionOffset->show();
// 		ui.lePositionOffset->show();
// 		position = Axis::AxisCustom;
// 	}else{
// 		ui.lPositionOffset->hide();
// 		ui.lePositionOffset->hide();
// 		
// 		if ( m_axesList->first()->orientation() == Axis::AxisHorizontal)
// 		  postion = Axis::AxisPosition(index);
// 		else
// 		  postion = Axis::AxisPosition(index+2);
// 	}
// 
//   foreach(Axis* axis, m_axesList){
// 	axis->setPosition(position);
//   }
}

void AxisDock::positionOffsetChanged(){
  if (m_initializing)
	return;
  
  double offset = ui.lePositionOffset->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setOffset(offset);
  }
}

void AxisDock::scaleChanged(int index){
  if (m_initializing)
	return;

  Axis::AxisScale scale = (Axis::AxisScale)index;
  foreach(Axis* axis, m_axesList){
	axis->setScale(scale);
  }
}

void AxisDock::startChanged(){
  if (m_initializing)
	return;
  
  double value = ui.leStart->text().toDouble();

  //check first, whether the value for the lower limit is valid for the log- and square root scaling. If not, set the default values.
  Axis::AxisScale scale = Axis::AxisScale(ui.cbScale->currentIndex());
  if (scale==Axis::ScaleLog10|| scale==Axis::ScaleLog2|| scale==Axis::ScaleLn){
	  if(value <= 0){
		  KMessageBox::sorry(this,
										  i18n("The axes lower limit has a non-positive value. Default minimal value will be used."),
										  i18n("Wrong lower limit value") );
		  ui.leStart->setText( "0.01" );
		  value=0.01;
	  }
  }else if (scale==Axis::ScaleSqrt){
	  if(value < 0){
		  KMessageBox::sorry(this,
										  i18n("The axes lower limit has a negative value. Default minimal value will be used."),
										  i18n("Wrong lower limit value") );
		  ui.leStart->setText( "0" );
		  value=0;
	  }
  }

  foreach(Axis* axis, m_axesList){
	axis->setStart(value);
  }
}

void AxisDock::endChanged(){
  if (m_initializing)
	return;

  double value = ui.leEnd->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setEnd(value);
  }
}

void AxisDock::zeroOffsetChanged(){
  if (m_initializing)
	return;
  
  double offset = ui.leZeroOffset->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setZeroOffset(offset);
  }
}

void AxisDock::scalingFactorChanged(){
  if (m_initializing)
	return;
  
  double scalingFactor = ui.leScalingFactor->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setScalingFactor(scalingFactor);
  }
}

// "Line"-tab
void AxisDock::lineStyleChanged(int index){
   if (m_initializing)
	return;

  Qt::PenStyle penStyle=Qt::PenStyle(index);
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->linePen();
	pen.setStyle(penStyle);
	axis->setLinePen(pen);
  }
}

void AxisDock::lineColorChanged(const QColor& color){
  if (m_initializing)
	return;

  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->linePen();
	pen.setColor(color);
	axis->setLinePen(pen);
  }  

  m_initializing=true;
  GuiTools::updatePenStyles(ui.cbLineStyle, color);
  m_initializing=false;
}

void AxisDock::lineWidthChanged(double  value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->linePen();
	pen.setWidth(value);
	axis->setLinePen(pen);
  }  
}

void AxisDock::lineOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(Axis* axis, m_axesList)
	axis->setLineOpacity(opacity);
}

//"Major ticks" tab
void AxisDock::majorTicksDirectionChanged(int index){
  if (m_initializing)
	return;

  Axis::TicksDirection direction = Axis::TicksDirection(index);
  if (direction == Axis::noTicks)
	this->setMajorTicksWidgetsEnabled(false);
  else
	this->setMajorTicksWidgetsEnabled(true);
  
  foreach(Axis* axis, m_axesList)
	axis->setMajorTicksDirection(direction);
}

void AxisDock::setMajorTicksWidgetsEnabled(bool b){
	ui.lMajorTicksType->setEnabled(b);
	ui.cbMajorTicksType->setEnabled(b);
	ui.lMajorTicksType->setEnabled(b);  
	ui.cbMajorTicksType->setEnabled(b);  
	ui.lMajorTicksNumber->setEnabled(b);
	ui.sbMajorTicksNumber->setEnabled(b);
	ui.lMajorTicksIncrement->setEnabled(b);
	ui.leMajorTicksIncrement->setEnabled(b);
	ui.lMajorTicksLineStyle->setEnabled(b);
	ui.cbMajorTicksLineStyle->setEnabled(b);
	ui.lMajorTicksColor->setEnabled(b);
	ui.kcbMajorTicksColor->setEnabled(b);
	ui.lMajorTicksWidth->setEnabled(b);
	ui.sbMajorTicksWidth->setEnabled(b);
	ui.lMajorTicksLength->setEnabled(b);
	ui.sbMajorTicksLength->setEnabled(b);
	ui.lMajorTicksOpacity->setEnabled(b);
	ui.sbMajorTicksOpacity->setEnabled(b);
	ui.lMajorTicksOpacityPercent->setEnabled(b);
}

/*!
	called if the current style of the ticks (Number or Increment) is changed.
	Shows/hides the corresponding widgets.
*/
void AxisDock::majorTicksTypeChanged(int index){
  Axis::TicksType type = Axis::TicksType(index);
  if ( type == Axis::TicksTotalNumber){
	  ui.lMajorTicksNumber->show();
	  ui.sbMajorTicksNumber->show();
	  ui.lMajorTicksIncrement->hide();
	  ui.leMajorTicksIncrement->hide();	  
  }else{
	  ui.lMajorTicksNumber->hide();
	  ui.sbMajorTicksNumber->hide();
	  ui.lMajorTicksIncrement->show();
	  ui.leMajorTicksIncrement->show();	  
  }
  
  if (m_initializing)
	return;
		
  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksType(type);
  }
}

void AxisDock::majorTicksNumberChanged(int value){
  if (m_initializing)
	return;

  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksNumber(value);
  }
}

void AxisDock::majorTicksIncrementChanged(){
  if (m_initializing)
	return;

  double value = ui.leMajorTicksIncrement->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksIncrement(value);
  }
}

void AxisDock::majorTicksLineStyleChanged(int index){
   if (m_initializing)
	return;

  Qt::PenStyle penStyle=Qt::PenStyle(index);
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->majorTicksPen();
	pen.setStyle(penStyle);
	axis->setMajorTicksPen(pen);
  }
}

void AxisDock::majorTicksColorChanged(const QColor& color){
  if (m_initializing)
	return;

  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->majorTicksPen();
	pen.setColor(color);
	axis->setMajorTicksPen(pen);
  }  

  m_initializing=true;
  GuiTools::updatePenStyles(ui.cbMajorTicksLineStyle, color);
  m_initializing=false;
}

void AxisDock::majorTicksWidthChanged(int value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->majorTicksPen();
	pen.setWidth(value);
	axis->setMajorTicksPen(pen);
  }  
}

void AxisDock::majorTicksLengthChanged(int value){
  if (m_initializing)
	return;
  
  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksLength(value);
  }  
}

void AxisDock::majorTicksOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(Axis* axis, m_axesList){
	axis->setMajorTicksOpacity(opacity);
  }
}

//"Minor ticks" tab
void AxisDock::minorTicksDirectionChanged(int index){
  if (m_initializing)
	return;

  Axis::TicksDirection direction = Axis::TicksDirection(index);
  if (direction == Axis::noTicks){
	this->setMinorTicksWidgetsEnabled(false);
  }else{
	this->setMinorTicksWidgetsEnabled(true);
  }
  
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksDirection(direction);  
  }
}

void AxisDock::setMinorTicksWidgetsEnabled(bool b){
	ui.lMinorTicksType->setEnabled(b);
	ui.cbMinorTicksType->setEnabled(b);
	ui.lMinorTicksType->setEnabled(b);  
	ui.cbMinorTicksType->setEnabled(b);  
	ui.lMinorTicksNumber->setEnabled(b);
	ui.sbMinorTicksNumber->setEnabled(b);
	ui.lMinorTicksIncrement->setEnabled(b);
	ui.leMinorTicksIncrement->setEnabled(b);
	ui.lMinorTicksLineStyle->setEnabled(b);
	ui.cbMinorTicksLineStyle->setEnabled(b);
	ui.lMinorTicksColor->setEnabled(b);
	ui.kcbMinorTicksColor->setEnabled(b);
	ui.lMinorTicksWidth->setEnabled(b);
	ui.sbMinorTicksWidth->setEnabled(b);
	ui.lMinorTicksLength->setEnabled(b);
	ui.sbMinorTicksLength->setEnabled(b);
	ui.lMinorTicksOpacity->setEnabled(b);
	ui.sbMinorTicksOpacity->setEnabled(b);
	ui.lMinorTicksOpacityPercent->setEnabled(b);
}

void AxisDock::minorTicksTypeChanged(int index){
  Axis::TicksType type = Axis::TicksType(index);
  if ( type == Axis::TicksTotalNumber){
	ui.lMinorTicksNumber->show();
	ui.sbMinorTicksNumber->show();
	ui.lMinorTicksIncrement->hide();
	ui.leMinorTicksIncrement->hide();
  }else{
	  ui.lMinorTicksNumber->hide();
	  ui.sbMinorTicksNumber->hide();
	  ui.lMinorTicksIncrement->show();
	  ui.leMinorTicksIncrement->show();
  }
  
  if (m_initializing)
	return;
		
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksType(type);
  }
}

void AxisDock::minorTicksNumberChanged(int value){
  if (m_initializing)
	return;

  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksNumber(value);
  }  
}

void AxisDock::minorTicksIncrementChanged(){
  if (m_initializing)
	return;

  double value = ui.leMinorTicksIncrement->text().toDouble();
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksIncrement(value);
  }  
}

void AxisDock::minorTicksLineStyleChanged(int index){
   if (m_initializing)
	return;

  Qt::PenStyle penStyle=Qt::PenStyle(index);
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->minorTicksPen();
	pen.setStyle(penStyle);
	axis->setMinorTicksPen(pen);
  }
}

void AxisDock::minorTicksColorChanged(const QColor& color){
  if (m_initializing)
	return;

  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->minorTicksPen();
	pen.setColor(color);
	axis->setMinorTicksPen(pen);
  }  

  m_initializing=true;
  GuiTools::updatePenStyles(ui.cbMinorTicksLineStyle, color);
  m_initializing=false;
}

void AxisDock::minorTicksWidthChanged(int value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(Axis* axis, m_axesList){
	pen=axis->minorTicksPen();
	pen.setWidth(value);
	axis->setMinorTicksPen(pen);
  }  
}

void AxisDock::minorTicksLengthChanged(int value){
  if (m_initializing)
	return;
  
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksLength(value);
  }  
}

void AxisDock::minorTicksOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(Axis* axis, m_axesList){
	axis->setMinorTicksOpacity(opacity);
  }
}

//"Tick labels"-tab
void AxisDock::labelsPositionChanged(int index){
  if (m_initializing)
	return;
  
  Axis::LabelsPosition position = Axis::LabelsPosition(index);
  foreach(Axis* axis, m_axesList){
	axis->setLabelsPosition(position);
  }
}

void AxisDock::setLabelsWidgetsEnabled(bool b){
  	ui.lLabelsOffset->setEnabled(b);
	ui.sbLabelsOffset->setEnabled(b);
	ui.lLabelsRotation->setEnabled(b);
	ui.sbLabelsRotation->setEnabled(b);
	ui.lLabelsRotationGrad->setEnabled(b);
	ui.lLabelsFont->setEnabled(b);
	ui.kfrLabelsFont->setEnabled(b);
	ui.lLabelsColor->setEnabled(b);
	ui.kcbLabelsFontColor->setEnabled(b);
	ui.lLabelsPrefix->setEnabled(b);
	ui.leLabelsPrefix->setEnabled(b);
	ui.lLabelsSuffix->setEnabled(b);
	ui.leLabelsSuffix->setEnabled(b);
	ui.lLabelsOpacity->setEnabled(b);
	ui.sbLabelsOpacity->setEnabled(b);
	ui.lLabelsOpacityPercent->setEnabled(b);
}

void AxisDock::labelsOffsetChanged(int value){
  if (m_initializing)
	return;
	
  //TODO
//   foreach(Axis* axis, m_axesList){
// 	axis->setLabelsOffset(value);
//   }  
}

void AxisDock::labelsRotationChanged(int value){
  if (m_initializing)
	return;
	
  foreach(Axis* axis, m_axesList){
	axis->setLabelsRotationAngle(value);
  }
}

void AxisDock::labelsPrefixChanged(){
  if (m_initializing)
	return;
		
  QString prefix = ui.leLabelsPrefix->text();
  foreach(Axis* axis, m_axesList){
	axis->setLabelsPrefix(prefix);
  }
}

void AxisDock::labelsSuffixChanged(){
  if (m_initializing)
	return;
		
  QString suffix = ui.leLabelsSuffix->text();
  foreach(Axis* axis, m_axesList){
	axis->setLabelsSuffix(suffix);
  }
}

void AxisDock::labelsFontChanged(const QFont& font){
  if (m_initializing)
	return;
	
  foreach(Axis* axis, m_axesList){
	axis->setLabelsFont(font);
  }
}

void AxisDock::labelsFontColorChanged(const QColor& color){
  if (m_initializing)
	return;
	
  foreach(Axis* axis, m_axesList){
	axis->setLabelsColor(color);
  }
}

void AxisDock::labelsOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(Axis* axis, m_axesList){
	axis->setLabelsOpacity(opacity);
  }  
}