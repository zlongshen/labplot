/***************************************************************************
    File                 : CartesianPlot.cpp
    Project              : LabPlot/SciDAVis
    Description          : A plot containing decoration elements.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011-2013 by Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses) 
                           
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

#include "CartesianPlot.h"
#include "CartesianCoordinateSystem.h"
#include "Axis.h"
#include "XYCurve.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/AbstractPlotPrivate.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include <math.h>

#include <QDebug>
#include <QMenu>
#include <QToolBar>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QIcon>
#else
#include <KIcon>
#include <KAction>
#include <KLocale>
#endif

#define SCALE_MIN CartesianCoordinateSystem::Scale::LIMIT_MIN
#define SCALE_MAX CartesianCoordinateSystem::Scale::LIMIT_MAX

/**
 * \class CartesianPlot
 * \brief A xy-plot.
 *
 * 
 */

class CartesianPlotPrivate:public AbstractPlotPrivate{
    public:
		CartesianPlotPrivate(CartesianPlot *owner);
		CartesianPlot* const q;

		virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
		virtual void mouseMoveEvent (QGraphicsSceneMouseEvent*);
		virtual void wheelEvent( QGraphicsSceneWheelEvent*);
		virtual void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );

		virtual void retransform();
		void retransformScales();
		float round(float value, int precision);
		void checkXRange();
		void checkYRange();

		float xMin, xMax, yMin, yMax;
		float xMinPrev, xMaxPrev, yMinPrev, yMaxPrev;
		bool autoScaleX, autoScaleY;
		float autoScaleOffsetFactor;
		CartesianPlot::Scale xScale, yScale;
		bool suppressRetransform;
		bool m_printing;
};

CartesianPlot::CartesianPlot(const QString &name):AbstractPlot(name, new CartesianPlotPrivate(this)),
	m_legend(0), m_zoomFactor(1.2) {
	init();
}

CartesianPlot::CartesianPlot(const QString &name, CartesianPlotPrivate *dd):AbstractPlot(name, dd),
	m_legend(0), m_zoomFactor(1.2) {
	init();
}

CartesianPlot::~CartesianPlot(){
	delete m_coordinateSystem;
	delete addNewMenu;

	//don't need to delete objects added with addChild()

	//no need to delete the d-pointer here - it inherits from QGraphicsItem 
	//and is deleted during the cleanup in QGraphicsScene		
}

/*!
	initializes all member variables of \c CartesianPlot
*/
void CartesianPlot::init(){
	Q_D(CartesianPlot);

	m_coordinateSystem = new CartesianCoordinateSystem(this);
	d->autoScaleX = true;
	d->autoScaleY = true;
	d->xScale = ScaleLinear;
	d->yScale = ScaleLinear;

	//the following factor determines the size of the offset between the min/max points of the curves
	//and the coordinate system ranges, when doing auto scaling
	//Factor 1 corresponds to the exact match - min/max values of the curves correspond to the start/end values of the ranges.
	d->autoScaleOffsetFactor = 0.05; 

	//TODO: make this factor optional.
	//Provide in the UI the possibility to choose between "exact" or 0% offset, 2%, 5% and 10% for the auto fit option
	
	m_plotArea = new PlotArea(name() + " plot area");
	addChild(m_plotArea);

	//offset between the plot area and the area defining the coordinate system, in scene units.
	d->horizontalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Centimeter);
	d->verticalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Centimeter);
	
	initActions();
	initMenus();

	connect(this, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(childAdded(const AbstractAspect*)));
	connect(this, SIGNAL(aspectRemoved(const AbstractAspect*, const AbstractAspect*, const AbstractAspect*)),
			this, SLOT(childRemoved(const AbstractAspect*, const AbstractAspect*, const AbstractAspect*)));
	graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, true);	
}

/*!
	initializes all children of \c CartesianPlot and 
	setups a default plot of type \c type with a plot title.
*/
void CartesianPlot::initDefault(Type type){
	Q_D(CartesianPlot);
	
	switch (type) {
		case FourAxes:
		{
			d->xMin = 0;
			d->xMax = 1;
			d->yMin = 0;
			d->yMax = 1;

			//Axes
			Axis *axis = new Axis("x axis 1", Axis::AxisHorizontal);
			addChild(axis);
			axis->setPosition(Axis::AxisBottom);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			QPen pen = axis->majorGridPen();
			pen.setStyle(Qt::SolidLine);
			axis->setMajorGridPen(pen);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::DotLine);
			axis->setMinorGridPen(pen);

			axis = new Axis("x axis 2", Axis::AxisHorizontal);
			addChild(axis);
			axis->setPosition(Axis::AxisTop);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			axis->setLabelsPosition(Axis::NoLabels);
			axis->title()->setText(QString());

			axis = new Axis("y axis 1", Axis::AxisVertical);
			addChild(axis);
			axis->setPosition(Axis::AxisLeft);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			pen = axis->majorGridPen();
			pen.setStyle(Qt::SolidLine);
			axis->setMajorGridPen(pen);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::DotLine);
			axis->setMinorGridPen(pen);

			axis = new Axis("y axis 2", Axis::AxisVertical);
			addChild(axis);
			axis->setPosition(Axis::AxisRight);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setOffset(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			axis->setLabelsPosition(Axis::NoLabels);
			axis->title()->setText(QString());
			
			break;
		}
		case TwoAxes:
		{
			d->xMin = 0;
			d->xMax = 1;
			d->yMin = 0;
			d->yMax = 1;

			Axis *axis = new Axis("x axis 1", Axis::AxisHorizontal);
			addChild(axis);
			axis->setPosition(Axis::AxisBottom);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);

			axis = new Axis("y axis 1", Axis::AxisVertical);
			addChild(axis);
			axis->setPosition(Axis::AxisLeft);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			
			break;
		}
		case TwoAxesCentered:
		{
			d->xMin = -0.5;
			d->xMax = 0.5;
			d->yMin = -0.5;
			d->yMax = 0.5;

			d->horizontalPadding = 0;
			d->verticalPadding = 0;
			
			Axis *axis = new Axis("x axis 1", Axis::AxisHorizontal);
			addChild(axis);
			axis->setPosition(Axis::AxisCentered);
			axis->setStart(-0.5);
			axis->setEnd(0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(10);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->title()->setText(QString());

			axis = new Axis("y axis 1", Axis::AxisVertical);
			addChild(axis);
			axis->setPosition(Axis::AxisCentered);
			axis->setStart(-0.5);
			axis->setEnd(0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(10);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->title()->setText(QString());
			
			break;
		}			
		case TwoAxesCenteredZero:
		{
			d->xMin = -0.5;
			d->xMax = 0.5;
			d->yMin = -0.5;
			d->yMax = 0.5;

			Axis *axis = new Axis("x axis 1", Axis::AxisHorizontal);
			addChild(axis);
			axis->setPosition(Axis::AxisCustom);
			axis->setOffset(0);
			axis->setStart(-0.5);
			axis->setEnd(0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->title()->setText(QString());

			axis = new Axis("y axis 1", Axis::AxisVertical);
			addChild(axis);
			axis->setPosition(Axis::AxisCustom);
			axis->setOffset(0);
			axis->setStart(-0.5);
			axis->setEnd(0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->title()->setText(QString());
			
			break;
		}
	}

	d->xMinPrev = d->xMin;
	d->xMaxPrev = d->xMax;
	d->yMinPrev = d->yMin;
	d->yMaxPrev = d->yMax;

	//Plot title
 	m_title = new TextLabel(this->name());
	addChild(m_title);
	m_title->setHidden(true);
	m_title->graphicsItem()->setParentItem(m_plotArea->graphicsItem()); //set the parent before doing any positioning
	TextLabel::PositionWrapper position;
	position.horizontalPosition = TextLabel::hPositionCenter;
	position.verticalPosition = TextLabel::vPositionTop;
	m_title->setPosition(position);
	m_title->setHorizontalAlignment(TextLabel::hAlignCenter);
	m_title->setVerticalAlignment(TextLabel::vAlignBottom);
	
	//Geometry, specify the plot rect in scene coordinates.
	//TODO: Use default settings for left, top, width, height and for min/max for the coordinate system
	float x = Worksheet::convertToSceneUnits(2, Worksheet::Centimeter);
	float y = Worksheet::convertToSceneUnits(2, Worksheet::Centimeter);
	float w = Worksheet::convertToSceneUnits(10, Worksheet::Centimeter);
	float h = Worksheet::convertToSceneUnits(10, Worksheet::Centimeter);
	
	//all plot children are initialized -> set the geometry of the plot in scene coordinates.
	d->rect = QRectF(x,y,w,h);
	d->retransform();
}

void CartesianPlot::initActions(){
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	addCurveAction = new QAction(tr("xy-curve"), this);
	addLegendAction = new QAction(tr("legend"), this);
	addHorizontalAxisAction = new QAction(tr("horizontal axis"), this);
	addVerticalAxisAction = new QAction(tr("vertical axis"), this);

	scaleAutoAction = new QAction(i18n("auto scale"), this);
	scaleAutoXAction = new QAction(i18n("auto scale X"), this);
	scaleAutoYAction = new QAction(i18n("auto scale Y"), this);
	zoomInAction = new QAction(i18n("zoom in"), this);
	zoomOutAction = new QAction(i18n("zoom out"), this);
	zoomInXAction = new QAction(i18n("zoom in X"), this);
	zoomOutXAction = new QAction(i18n("zoom out X"), this);
	zoomInYAction = new QAction(i18n("zoom in Y"), this);
	zoomOutYAction = new QAction(i18n("zoom out Y"), this);
    shiftLeftXAction = new QAction(i18n("shift left X"), this);
	shiftRightXAction = new QAction(i18n("shift right X"), this);
	shiftUpYAction = new QAction(i18n("shift up Y"), this);
	shiftDownYAction = new QAction(i18n("shift down Y"), this);
#else
	addCurveAction = new KAction(KIcon("xy-curve"), i18n("xy-curve"), this);
	addLegendAction = new KAction(KIcon("text-field"), i18n("legend"), this);
	addHorizontalAxisAction = new KAction(KIcon("axis-horizontal"), i18n("horizontal axis"), this);
	addVerticalAxisAction = new KAction(KIcon("axis-vertical"), i18n("vertical axis"), this);
	
	scaleAutoAction = new KAction(KIcon("auto-scale-all"), i18n("auto scale"), this);
	scaleAutoXAction = new KAction(KIcon("auto-scale-x"), i18n("auto scale X"), this);
	scaleAutoYAction = new KAction(KIcon("auto-scale-y"), i18n("auto scale Y"), this);
	zoomInAction = new KAction(KIcon("zoom-in"), i18n("zoom in"), this);
	zoomOutAction = new KAction(KIcon("zoom-out"), i18n("zoom out"), this);
	zoomInXAction = new KAction(KIcon("x-zoom-in"), i18n("zoom in X"), this);
	zoomOutXAction = new KAction(KIcon("x-zoom-out"), i18n("zoom out X"), this);
	zoomInYAction = new KAction(KIcon("y-zoom-in"), i18n("zoom in Y"), this);
	zoomOutYAction = new KAction(KIcon("y-zoom-out"), i18n("zoom out Y"), this);
    shiftLeftXAction = new KAction(KIcon("shift-left-x"), i18n("shift left X"), this);
	shiftRightXAction = new KAction(KIcon("shift-right-x"), i18n("shift right X"), this);
	shiftUpYAction = new KAction(KIcon("shift-up-y"), i18n("shift up Y"), this);
	shiftDownYAction = new KAction(KIcon("shift-down-y"), i18n("shift down Y"), this);
#endif
	connect(addCurveAction, SIGNAL(triggered()), SLOT(addCurve()));
	connect(addLegendAction, SIGNAL(triggered()), SLOT(addLegend()));
	connect(addHorizontalAxisAction, SIGNAL(triggered()), SLOT(addAxis()));
	connect(addVerticalAxisAction, SIGNAL(triggered()), SLOT(addAxis()));
	
	//zoom actions
	connect(scaleAutoAction, SIGNAL(triggered()), SLOT(scaleAuto()));
	connect(scaleAutoXAction, SIGNAL(triggered()), SLOT(scaleAutoX()));
	connect(scaleAutoYAction, SIGNAL(triggered()), SLOT(scaleAutoY()));
	connect(zoomInAction, SIGNAL(triggered()), SLOT(zoomIn()));
	connect(zoomOutAction, SIGNAL(triggered()), SLOT(zoomOut()));
	connect(zoomInXAction, SIGNAL(triggered()), SLOT(zoomInX()));
	connect(zoomOutXAction, SIGNAL(triggered()), SLOT(zoomOutX()));
	connect(zoomInYAction, SIGNAL(triggered()), SLOT(zoomInY()));
	connect(zoomOutYAction, SIGNAL(triggered()), SLOT(zoomOutY()));
	connect(shiftLeftXAction, SIGNAL(triggered()), SLOT(shiftLeftX()));
	connect(shiftRightXAction, SIGNAL(triggered()), SLOT(shiftRightX()));
	connect(shiftUpYAction, SIGNAL(triggered()), SLOT(shiftUpY()));
	connect(shiftDownYAction, SIGNAL(triggered()), SLOT(shiftDownY()));
	
	visibilityAction = new QAction(tr("visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, SIGNAL(triggered()), this, SLOT(visibilityChanged()));	
}

void CartesianPlot::initMenus(){
	addNewMenu = new QMenu(tr("Add new"));
	addNewMenu->addAction(addCurveAction);
	addNewMenu->addAction(addLegendAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addHorizontalAxisAction);
	addNewMenu->addAction(addVerticalAxisAction);
	
	zoomMenu = new QMenu(tr("Zoom"));
	zoomMenu->addAction(scaleAutoAction);
	zoomMenu->addAction(scaleAutoXAction);
	zoomMenu->addAction(scaleAutoYAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(zoomInAction);
	zoomMenu->addAction(zoomOutAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(zoomInXAction);
	zoomMenu->addAction(zoomOutXAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(zoomInYAction);
	zoomMenu->addAction(zoomOutYAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(shiftLeftXAction);
	zoomMenu->addAction(shiftRightXAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(shiftUpYAction);
	zoomMenu->addAction(shiftDownYAction);
}

QMenu* CartesianPlot::createContextMenu(){
	QMenu* menu = AbstractWorksheetElement::createContextMenu();

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE	
	QAction* firstAction = menu->actions().first();
#else
	QAction* firstAction = menu->actions().at(1);
#endif

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	menu->insertMenu(firstAction, addNewMenu);
	menu->insertMenu(firstAction, zoomMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

void CartesianPlot::fillToolBar(QToolBar* toolBar) const{
	toolBar->addAction(addCurveAction);
	toolBar->addAction(addLegendAction);
	toolBar->addSeparator();
	toolBar->addAction(addHorizontalAxisAction);
	toolBar->addAction(addVerticalAxisAction);
	
	toolBar->addAction(scaleAutoAction);
	toolBar->addAction(scaleAutoXAction);
	toolBar->addAction(scaleAutoYAction);
	toolBar->addAction(zoomInAction);
	toolBar->addAction(zoomOutAction);
	toolBar->addAction(zoomInXAction);
	toolBar->addAction(zoomOutXAction);
	toolBar->addAction(zoomInYAction);
	toolBar->addAction(zoomOutYAction);
	toolBar->addAction(shiftLeftXAction);
	toolBar->addAction(shiftRightXAction);
	toolBar->addAction(shiftUpYAction);
	toolBar->addAction(shiftDownYAction);	
}
/*!
	Returns an icon to be used in the project explorer.
*/
QIcon CartesianPlot::icon() const{
	QIcon ico;
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	ico.addPixmap(QPixmap(":/graph.xpm"));
#else
	ico = KIcon("office-chart-line");
#endif
	return ico;
}

//##############################################################################
//################################  getter methods  ############################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, autoScaleX, autoScaleX)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, xMin, xMin)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, xMax, xMax)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::Scale, xScale, xScale)

BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, autoScaleY, autoScaleY)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, yMin, yMin)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, yMax, yMax)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::Scale, yScale, yScale)

/*!
	return the actual bounding rectangular of the plot (plot's rectangular minus padding)
	in plot's coordinates
 */
//TODO: return here a private variable only, update this variable on rect and padding changes.
QRectF CartesianPlot::plotRect() {
	Q_D(const CartesianPlot);
	QRectF rect = d->mapFromScene(d->rect).boundingRect();
	rect.setX(rect.x() + d->horizontalPadding);
	rect.setY(rect.y() + d->verticalPadding);
	rect.setWidth(rect.width() - d->horizontalPadding);
	rect.setHeight(rect.height()-d->verticalPadding);
	return rect;
}

//##############################################################################
//######################  setter methods and undo commands  ####################
//##############################################################################
/*!
	set the rectangular, defined in scene coordinates
 */
STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetRect, QRectF, rect, retransform)
void CartesianPlot::setRect(const QRectF& rect){
	Q_D(CartesianPlot);
	if (rect != d->rect)
		exec(new CartesianPlotSetRectCmd(d, rect, tr("%1: set geometry rect")));
}

//TODO: provide an undo-aware version
// STD_SETTER_CMD_IMPL_F(CartesianPlot, SetAutoScaleX, bool, autoScaleX, retransformScales)
void CartesianPlot::setAutoScaleX(bool autoScaleX){
	Q_D(CartesianPlot);
	if (autoScaleX != d->autoScaleX){
// 		exec(new CartesianPlotSetAutoScaleXCmd(d, autoScaleX, tr("%1: change auto scale x")));
		d->autoScaleX = autoScaleX;
		if (autoScaleX)
			this->scaleAutoX();
	}
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXMin, float, xMin, retransformScales)
void CartesianPlot::setXMin(float xMin){
	Q_D(CartesianPlot);
	if (xMin != d->xMin)
		exec(new CartesianPlotSetXMinCmd(d, xMin, tr("%1: set min x")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXMax, float, xMax, retransformScales);
void CartesianPlot::setXMax(float xMax){
	Q_D(CartesianPlot);
	if (xMax != d->xMax)
		exec(new CartesianPlotSetXMaxCmd(d, xMax, tr("%1: set max x")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXScale, CartesianPlot::Scale, xScale, retransformScales);
void CartesianPlot::setXScale(Scale scale){
	Q_D(CartesianPlot);
	if (scale != d->xScale)
		exec(new CartesianPlotSetXScaleCmd(d, scale, tr("%1: set x scale")));
}

//TODO: provide an undo-aware version
// STD_SETTER_CMD_IMPL_F(CartesianPlot, SetAutoScaleY, bool, autoScaleY, retransformScales)
void CartesianPlot::setAutoScaleY(bool autoScaleY){
	Q_D(CartesianPlot);
	if (autoScaleY != d->autoScaleY){
// 		exec(new CartesianPlotSetAutoScaleYCmd(d, autoScaleY, tr("%1: change auto scale y")));
		d->autoScaleY = autoScaleY;
		if (autoScaleY)
			this->scaleAutoY();
	}
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYMin, float, yMin, retransformScales);
void CartesianPlot::setYMin(float yMin){
	Q_D(CartesianPlot);
	if (yMin != d->yMin)
		exec(new CartesianPlotSetYMinCmd(d, yMin, tr("%1: set min y")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYMax, float, yMax, retransformScales);
void CartesianPlot::setYMax(float yMax){
	Q_D(CartesianPlot);
	if (yMax != d->yMax)
		exec(new CartesianPlotSetYMaxCmd(d, yMax, tr("%1: set max y")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYScale, CartesianPlot::Scale, yScale, retransformScales);
void CartesianPlot::setYScale(Scale scale){
	Q_D(CartesianPlot);
	if (scale != d->yScale)
		exec(new CartesianPlotSetYScaleCmd(d, scale, tr("%1: set y scale")));
}

//################################################################
//########################## Slots ###############################
//################################################################
void CartesianPlot::addAxis(){
	if (QObject::sender() == addHorizontalAxisAction)
		addChild(new Axis("x-axis", Axis::AxisHorizontal));
	else
		addChild(new Axis("y-axis", Axis::AxisVertical));
}

void CartesianPlot::addCurve(){
	XYCurve* curve = new XYCurve("xy-curve");
	this->addChild(curve);
	connect(curve, SIGNAL(xDataChanged()), this, SLOT(xDataChanged()));
	connect(curve, SIGNAL(yDataChanged()), this, SLOT(yDataChanged()));

	//update the legend on changes of the name, line and symbol styles
	connect(curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(lineTypeChanged(XYCurve::LineType)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(linePenChanged(const QPen&)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(symbolsTypeIdChanged(QString)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(symbolsPenChanged(const QPen&)), this, SLOT(updateLegend()));
}

void CartesianPlot::addLegend(){
	m_legend = new CartesianPlotLegend(this, "legend");
	this->addChild(m_legend);

	//only one legend is allowed -> disable the action
	addLegendAction->setEnabled(false);
}

void CartesianPlot::childAdded(const AbstractAspect* child) {
	const XYCurve* curve = qobject_cast<const XYCurve*>(child);
	if (curve)
		updateLegend();
}

void CartesianPlot::childRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child) {
	Q_UNUSED(parent);
	Q_UNUSED(before);
	if (m_legend == child) {
		addLegendAction->setEnabled(true);
		m_legend = 0;
	} else {
		const XYCurve* curve = qobject_cast<const XYCurve*>(child);
		if (curve)
			updateLegend();
	}
}

void CartesianPlot::updateLegend() {
	if (m_legend)
		m_legend->retransform();
}

/*!
	called when in one of the curves the x-data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::xDataChanged(){
	qDebug()<<"CartesianPlot::xDataChanged";
	Q_D(CartesianPlot);
	if (d->autoScaleX)
		this->scaleAutoX();
}

/*!
	called when in one of the curves the x-data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::yDataChanged(){
	Q_D(CartesianPlot);
	if (d->autoScaleY)
		this->scaleAutoY();
}

void CartesianPlot::scaleAutoX(){
	Q_D(CartesianPlot);
	
	//loop over all xy-curves and determine the maximum x-value
	double min = INFINITY;
	double max = -INFINITY;
	QList<XYCurve*> children = this->children<XYCurve>();
	foreach(XYCurve* curve, children) {
		if (!curve->xColumn())
			continue;

		if (curve->xColumn()->minimum() != INFINITY){
			if (curve->xColumn()->minimum() < min)
				min = curve->xColumn()->minimum();
		}
		
		if (curve->xColumn()->maximum() != -INFINITY){
			if (curve->xColumn()->maximum() > max)
				max = curve->xColumn()->maximum();
		}
	}

	bool update = false;
	if (min != d->xMin && min != INFINITY){
		d->xMin = min;
		update = true;
	}
	
	if (max != d->xMax && max != -INFINITY){
		d->xMax = max;
		update = true;
	}

	if(update){
		if (d->xMax == d->xMin){
			//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
			if (d->xMax!=0){
				d->xMax = d->xMax*1.1;
				d->xMin = d->xMin*0.9;
			}else{
				d->xMax = 0.1;
				d->xMin = -0.1;
			}
		}else{
			float offset = (d->xMax - d->xMin)*d->autoScaleOffsetFactor;
			d->xMin -= offset;
			d->xMax += offset;
		}
		d->retransformScales();
	}
}

void CartesianPlot::scaleAutoY(){
	qDebug()<<"CartesianPlot::scaleAutoY()";
	Q_D(CartesianPlot);

	//loop over all xy-curves and determine the maximum y-value
	double min = INFINITY;
	double max = -INFINITY;
	QList<XYCurve*> children = this->children<XYCurve>();
	foreach(XYCurve* curve, children) {
		if (!curve->yColumn())
			continue;
		
		if (curve->yColumn()->minimum() != INFINITY){
			if (curve->yColumn()->minimum() < min)
				min = curve->yColumn()->minimum();
		}
		
		if (curve->yColumn()->maximum() != -INFINITY){
			if (curve->yColumn()->maximum() > max)
				max = curve->yColumn()->maximum();
		}
	}

	bool update = false;
	if (min != d->yMin && min != INFINITY){
		d->yMin = min;
		update = true;
	}
	
	if (max != d->yMax && max != -INFINITY){
		d->yMax = max;
		update = true;
	}
	
	if(update){
		if (d->yMax == d->yMin){
			//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
			if (d->yMax!=0){
				d->yMax = d->yMax*1.1;
				d->yMin = d->yMin*0.9;
			}else{
				d->yMax = 0.1;
				d->yMin = -0.1;
			}
		}else{
			float offset = (d->yMax - d->yMin)*d->autoScaleOffsetFactor;
			d->yMin -= offset;
			d->yMax += offset;
		}
		d->retransformScales();
	}
}

void CartesianPlot::scaleAuto(){
	Q_D(CartesianPlot);

	//loop over all xy-curves and determine the maximum x-value
	double xMin = INFINITY;
	double xMax = -INFINITY;
	double yMin = INFINITY;
	double yMax = -INFINITY;
	QList<XYCurve*> children = this->children<XYCurve>();
	foreach(XYCurve* curve, children) {
		if (!curve->xColumn())
			continue;

		if (curve->xColumn()->minimum() != INFINITY){
			if (curve->xColumn()->minimum() < xMin)
				xMin = curve->xColumn()->minimum();
		}
		
		if (curve->xColumn()->maximum() != -INFINITY){
			if (curve->xColumn()->maximum() > xMax)
				xMax = curve->xColumn()->maximum();
		}

		if (!curve->yColumn())
			continue;
		
		if (curve->yColumn()->minimum() != INFINITY){
			if (curve->yColumn()->minimum() < yMin)
				yMin = curve->yColumn()->minimum();
		}
		
		if (curve->yColumn()->maximum() != -INFINITY){
			if (curve->yColumn()->maximum() > yMax)
				yMax = curve->yColumn()->maximum();
		}		
	}

	bool updateX = false;
	bool updateY = false;
	if (xMin != d->xMin && xMin != INFINITY){
		d->xMin = xMin;
		updateX = true;
	}

	if (xMax != d->xMax && xMax != -INFINITY){
		d->xMax = xMax;
		updateX = true;
	}

	if (yMin != d->yMin && yMin != INFINITY){
		d->yMin = yMin;
		updateY = true;
	}

	if (yMax != d->yMax && yMax != -INFINITY){
		d->yMax = yMax;
		updateY = true;
	}

	if(updateX || updateY){
		if (updateX){
			if (d->xMax == d->xMin) {
				//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
				if (d->xMax!=0){
					d->xMax = d->xMax*1.1;
					d->xMin = d->xMin*0.9;
				}else{
					d->xMax = 0.1;
					d->xMin = -0.1;
				}
			} else {
				float offset = (d->xMax - d->xMin)*d->autoScaleOffsetFactor;
				d->xMin -= offset;
				d->xMax += offset;
			}
		}
		if (updateY){
			if (d->yMax == d->yMin) {
				//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
				if (d->yMax!=0){
					d->yMax = d->yMax*1.1;
					d->yMin = d->yMin*0.9;
				}else{
					d->yMax = 0.1;
					d->yMin = -0.1;
				}
			} else {
				float offset = (d->yMax - d->yMin)*d->autoScaleOffsetFactor;
				d->yMin -= offset;
				d->yMax += offset;
			}
		}
		d->retransformScales();
	}
}

void CartesianPlot::zoomIn(){
	Q_D(CartesianPlot);
	float oldRange = (d->xMax-d->xMin);
	float newRange = (d->xMax-d->xMin)/m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;

	oldRange = (d->yMax-d->yMin);
	newRange = (d->yMax-d->yMin)/m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;

	d->retransformScales();
}

void CartesianPlot::zoomOut(){
	Q_D(CartesianPlot);
	float oldRange = (d->xMax-d->xMin);
	float newRange = (d->xMax-d->xMin)*m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;

	oldRange = (d->yMax-d->yMin);
	newRange = (d->yMax-d->yMin)*m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;

	d->retransformScales();
}

void CartesianPlot::zoomInX(){
	Q_D(CartesianPlot);
	float oldRange = (d->xMax-d->xMin);
	float newRange = (d->xMax-d->xMin)/m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomOutX(){
	Q_D(CartesianPlot);
	float oldRange = (d->xMax-d->xMin);
	float newRange = (d->xMax-d->xMin)*m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomInY(){
	Q_D(CartesianPlot);
	float oldRange = (d->yMax-d->yMin);
	float newRange = (d->yMax-d->yMin)/m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomOutY(){
	Q_D(CartesianPlot);
	float oldRange = (d->yMax-d->yMin);
	float newRange = (d->yMax-d->yMin)*m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::shiftLeftX(){
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax -= offsetX;
	d->xMin -= offsetX;
	d->retransformScales();
}

void CartesianPlot::shiftRightX(){
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax += offsetX;
	d->xMin += offsetX;
	d->retransformScales();
}

void CartesianPlot::shiftUpY(){
	Q_D(CartesianPlot);
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax += offsetY;
	d->yMin += offsetY;
	d->retransformScales();
}

void CartesianPlot::shiftDownY(){
	Q_D(CartesianPlot);
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax -= offsetY;
	d->yMin -= offsetY;
	d->retransformScales();
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void CartesianPlot::visibilityChanged(){
	Q_D(CartesianPlot);
	this->setVisible(!d->isVisible());
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
CartesianPlotPrivate::CartesianPlotPrivate(CartesianPlot *owner) 
	: AbstractPlotPrivate(owner), q(owner), suppressRetransform(false), m_printing(false) {
}

/*!
	updates the position of plot rectangular in scene coordinates to \c r and recalculates the scales.
	The size of the plot corresponds to the size of the plot area, the area which is filled with the background color etc.
	and which can pose the parent item for several sub-items (like TextLabel).
	Note, the size of the area used to define the coordinate system doesn't need to be equal to this plot area.
	Also, the size (=bounding box) of CartesianPlot can be greater than the size of the plot area.
 */
void CartesianPlotPrivate::retransform(){
	if (suppressRetransform)
		return;

	qDebug()<<"CartesianPlotPrivate::retransform";
	prepareGeometryChange();
	setPos( rect.x()+rect.width()/2, rect.y()+rect.height()/2);
	
	retransformScales();

	//plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	q->plotArea()->setRect(rect);

	//call retransform() for the title and the legend (if available)
	//when a predefined position relative to the (Left, Centered etc.) is used, 
	//the actual position needs to be updated on plot's geometry changes.
	q->title()->retransform();
	if (q->m_legend)
		q->m_legend->retransform();

	q->retransform();
}

void CartesianPlotPrivate::retransformScales(){
	CartesianPlot* plot = dynamic_cast<CartesianPlot*>(q);
	CartesianCoordinateSystem* cSystem = dynamic_cast<CartesianCoordinateSystem*>(plot->coordinateSystem());
	QList<CartesianCoordinateSystem::Scale*> scales;
	
	//perform the mapping from the scene coordinates to the plot's coordinates here.
	QRectF itemRect = mapRectFromScene( rect );

	//create x-scales
	//TODO: for negative values of xMin and yMin use a value smaller that xMax/yMax and not 0.1
	if (xScale == CartesianPlot::ScaleLinear){
		scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.x()+horizontalPadding,
																	itemRect.x()+itemRect.width()-horizontalPadding,
																	xMin, xMax);
	}else if (xScale == CartesianPlot::ScaleLog10){
		if (xMin<=0) xMin=0.1;
		scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.x()+horizontalPadding,
																	itemRect.x()+itemRect.width()-horizontalPadding,
																	xMin, xMax, 10.0);
	}else if (xScale == CartesianPlot::ScaleLog2){
		if (xMin<=0) xMin=0.1;
		scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.x()+horizontalPadding,
																	itemRect.x()+itemRect.width()-horizontalPadding,
																	xMin, xMax, 2.0);
	}else{ //CartesianPlot::ScaleLn
		if (xMin<=0) xMin=0.1;
		scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.x()+horizontalPadding,
																	itemRect.x()+itemRect.width()-horizontalPadding,
																	xMin, xMax, 2.71828);
	}

	cSystem ->setXScales(scales);

	//create y-scales
	scales.clear();
	if (yScale == CartesianPlot::ScaleLinear){
		scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.y()+itemRect.height()-verticalPadding,
																	itemRect.y()+verticalPadding, 
																	yMin, yMax);
	}else if (yScale == CartesianPlot::ScaleLog10 || yScale == CartesianPlot::ScaleLog2 || yScale == CartesianPlot::ScaleLn){
		float base;
		if (yScale == CartesianPlot::ScaleLog10)
			base = 10.0;
		else if (yScale == CartesianPlot::ScaleLog2)
			base = 2.0;
		else
			base = 2.71828;
		
		checkXRange();
		checkYRange();
		scales << CartesianCoordinateSystem::Scale::createLogScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																	itemRect.y()+itemRect.height()-verticalPadding,
																	itemRect.y()+verticalPadding, 
																	yMin, yMax, base);
	}
	
	cSystem ->setYScales(scales);

	//calculate the changes in x and y and save the current values for xMin, xMax, yMin, yMax
	float deltaXMin = 0;
	float deltaXMax = 0;
	float deltaYMin = 0;
	float deltaYMax = 0;

	if (xMin!=xMinPrev) {
		deltaXMin = xMin - xMinPrev;
		emit plot->xMinChanged(xMin);
	}

	if (xMax!=xMaxPrev) {
		deltaXMax = xMax - xMaxPrev;
		emit plot->xMaxChanged(xMax);
	}

	if (yMin!=yMinPrev) {
		deltaYMin = yMin - yMinPrev;
		emit plot->yMinChanged(yMin);
	}

	if (yMax!=yMaxPrev) {
		deltaYMax = yMax - yMaxPrev;
		emit plot->yMaxChanged(yMax);
	}

	xMinPrev = xMin;
	xMaxPrev = xMax;
	yMinPrev = yMin;
	yMaxPrev = yMax;

	//adjust auto-scale axes
	QList<Axis*> childElements = q->children<Axis>();
	foreach(Axis* axis, childElements){
		if (!axis->autoScale())
			continue;
			
		if (axis->orientation() == Axis::AxisHorizontal){
			if (deltaXMax!=0)
				axis->setEnd(xMax, false);
			if (deltaXMin!=0)
				axis->setStart(xMin, false);

// 			if (axis->position() == Axis::AxisCustom && deltaYMin != 0){
// 				axis->setOffset(axis->offset() + deltaYMin, false);
// 			}
		}else{
			if (deltaYMax!=0)
				axis->setEnd(yMax, false);
			if (deltaYMin!=0)
				axis->setStart(yMin, false);

// 			if (axis->position() == Axis::AxisCustom && deltaXMin != 0){
// 				axis->setOffset(axis->offset() + deltaXMin, false);
// 			}
		}
	}
	
	// call retransform() on the parent to trigger the update of all axes and curves
	q->retransform();
}

/*!
 * don't allow any negative values on for the x range when log or sqrt scalings are used
 */
void CartesianPlotPrivate::checkXRange() {
	double min = 0.01;

	if (xMin <= 0.0) {
		(min < xMax*min) ? xMin = min : xMin = xMax*min;
		emit q->xMinChanged(xMin);
	}else if (xMax <= 0.0) {
		(-min > xMin*min) ? xMax = -min : xMax = xMin*min;
		emit q->xMaxChanged(xMax);
	}
}

/*!
 * don't allow any negative values on for the y range when log or sqrt scalings are used
 */
void CartesianPlotPrivate::checkYRange() {
	double min = 0.01;

	if (yMin <= 0.0) {
		(min < yMax*min) ? yMin = min : yMin = yMax*min;
		emit q->yMinChanged(yMin);
	}else if (yMax <= 0.0) {
		(-min > yMin*min) ? yMax = -min : yMax = yMin*min;
		emit q->yMaxChanged(yMax);
	}
}

float CartesianPlotPrivate::round(float value, int precision){
	return int(value*pow(10, precision) + (value<0 ? -0.5 : 0.5))/pow(10, precision);
}

/*!
 * Reimplemented from QGraphicsItem.
 */
QVariant CartesianPlotPrivate::itemChange(GraphicsItemChange change, const QVariant &value){
	if (change == QGraphicsItem::ItemPositionChange) {
		const QPointF& itemPos = value.toPointF();//item's center point in parent's coordinates;
		float x = itemPos.x();
		float y = itemPos.y();
		
		//calculate the new rect and forward the changes to the frontend
		QRectF newRect;
		float w = rect.width();
		float h = rect.height();
		newRect.setX(x-w/2);
		newRect.setY(y-h/2);
		newRect.setWidth(w);
		newRect.setHeight(h);
		emit q->rectChanged(newRect);
     }
	return QGraphicsItem::itemChange(change, value);
}

void CartesianPlotPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	//TODO: implement the navigation in plot on mouse move events, 
	//calculate the position changes and call shift*()-functions

	QGraphicsItem::mouseMoveEvent(event) ;
}

void CartesianPlotPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	const QPointF& itemPos = pos();//item's center point in parent's coordinates;
	float x = itemPos.x();
	float y = itemPos.y();
	
	//calculate the new rect and set it
	QRectF newRect;
	float w = rect.width();
	float h = rect.height();
	newRect.setX(x-w/2);
	newRect.setY(y-h/2);
	newRect.setWidth(w);
	newRect.setHeight(h);
	
	suppressRetransform = true;
	q->setRect(newRect);
	suppressRetransform = false;	

	QGraphicsItem::mouseReleaseEvent(event);
}

void CartesianPlotPrivate::wheelEvent(QGraphicsSceneWheelEvent* event) {
	//determine first, which axes are selected and zoom only in the corresponding direction.
	//zoom the entire plot if no axes selected.
	bool zoomX = false;
	bool zoomY = false;
	QList<Axis*> axes = q->children<Axis>();
	foreach(Axis* axis, axes){
		if (!axis->graphicsItem()->isSelected())
			continue;
			
		if (axis->orientation() == Axis::AxisHorizontal)
			zoomX  = true;
		else
			zoomY = true;
	}

	if (event->delta() > 0) {
		if (!zoomX && !zoomY) {
			//no special axis selected -> zoom in everything
			q->zoomIn();
		} else {
			if (zoomX) q->zoomInX();
			if (zoomY) q->zoomInY();
		}
	} else {
		if (!zoomX && !zoomY) {
			//no special axis selected -> zoom in everything
			q->zoomOut();
		} else {
			if (zoomX) q->zoomOutX();
			if (zoomY) q->zoomOutY();
		}
	}
}

void CartesianPlotPrivate::hoverMoveEvent ( QGraphicsSceneHoverEvent * event ){
	QPointF point = event->pos();
	if (q->plotRect().contains(point)){
		CartesianCoordinateSystem* cSystem = dynamic_cast<CartesianCoordinateSystem*>(q->coordinateSystem());
		QPointF logicalPoint = cSystem->mapSceneToLogical(point);
		QString info = "x=" + QString::number(logicalPoint.x()) + ", y=" + QString::number(logicalPoint.y());
		q->info(info);
	}

	QGraphicsItem::hoverMoveEvent(event) ;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void CartesianPlot::save(QXmlStreamWriter* writer) const{
	Q_D(const CartesianPlot);

	writer->writeStartElement( "cartesianPlot" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//geometry
	writer->writeStartElement( "geometry" );
	writer->writeAttribute( "x", QString::number(d->rect.x()) );
	writer->writeAttribute( "y", QString::number(d->rect.y()) );
	writer->writeAttribute( "width", QString::number(d->rect.width()) );
	writer->writeAttribute( "height", QString::number(d->rect.height()) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();
	
	//coordinate system and padding
// 	m_coordinateSystem->save(writer); //TODO save scales
	writer->writeStartElement( "coordinateSystem" );
	writer->writeAttribute( "autoScaleX", QString::number(d->autoScaleX) );
	writer->writeAttribute( "autoScaleY", QString::number(d->autoScaleY) );
	writer->writeAttribute( "xMin", QString::number(d->xMin) );
	writer->writeAttribute( "xMax", QString::number(d->xMax) );
	writer->writeAttribute( "yMin", QString::number(d->yMin) );
	writer->writeAttribute( "yMax", QString::number(d->yMax) );
	writer->writeAttribute( "xScale", QString::number(d->xScale) );
	writer->writeAttribute( "yScale", QString::number(d->yScale) );
	writer->writeAttribute( "horizontalPadding", QString::number(d->horizontalPadding) );
	writer->writeAttribute( "verticalPadding", QString::number(d->verticalPadding) );
	writer->writeEndElement();
	
    //serialize all children (plot area, title text label, axes and curves)
    QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
    foreach(AbstractWorksheetElement *elem, childElements)
        elem->save(writer);

    writer->writeEndElement(); // close "cartesianPlot" section
}


//! Load from XML
bool CartesianPlot::load(XmlStreamReader* reader){
	Q_D(CartesianPlot);

    if(!reader->isStartElement() || reader->name() != "cartesianPlot"){
        reader->raiseError(tr("no cartesianPlot element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = tr("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "cartesianPlot")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader))
				return false;
		}else if(reader->name() == "geometry"){
            attribs = reader->attributes();
	
            str = attribs.value("x").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'x'"));
            else
                d->rect.setX( str.toDouble() );

			str = attribs.value("y").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'y'"));
            else
                d->rect.setY( str.toDouble() );
			
			str = attribs.value("width").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'width'"));
            else
                d->rect.setWidth( str.toDouble() );
			
			str = attribs.value("height").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'height'"));
            else
                d->rect.setHeight( str.toDouble() );

			str = attribs.value("visible").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'visible'"));
            else
                d->setVisible(str.toInt());			
		}else if(reader->name() == "coordinateSystem"){
// 			m_coordinateSystem->load(reader); //TODO read scales
            attribs = reader->attributes();

			str = attribs.value("autoScaleX").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'autoScaleX'"));
            else
                d->autoScaleX = bool(str.toInt());

			str = attribs.value("autoScaleY").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'autoScaleY'"));
            else
                d->autoScaleY = bool(str.toInt());

            str = attribs.value("xMin").toString();
            if(str.isEmpty()) {
                reader->raiseWarning(attributeWarning.arg("'xMin'"));
			} else {
                d->xMin = str.toDouble();
				d->xMinPrev = d->xMin;
			}

            str = attribs.value("xMax").toString();
            if(str.isEmpty()) {
                reader->raiseWarning(attributeWarning.arg("'xMax'"));
			} else {
                d->xMax = str.toDouble();
				d->xMaxPrev = d->xMax;
			}

            str = attribs.value("yMin").toString();
            if(str.isEmpty()) {
                reader->raiseWarning(attributeWarning.arg("'yMin'"));
			} else {
                d->yMin = str.toDouble();
				d->yMinPrev = d->yMin;
			}

            str = attribs.value("yMax").toString();
            if(str.isEmpty()) {
                reader->raiseWarning(attributeWarning.arg("'yMax'"));
			} else {
                d->yMax = str.toDouble();
				d->yMaxPrev = d->yMax;
			}

			str = attribs.value("xScale").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'xScale'"));
            else
                d->xScale = CartesianPlot::Scale(str.toInt());

			str = attribs.value("yScale").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'yScale'"));
            else
                d->yScale = CartesianPlot::Scale(str.toInt());

            str = attribs.value("horizontalPadding").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'horizontalPadding'"));
            else
                d->horizontalPadding = str.toDouble();
			
            str = attribs.value("verticalPadding").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'verticalPadding'"));
            else
                d->verticalPadding = str.toDouble();	
        }else if(reader->name() == "textLabel"){
            m_title = new TextLabel("");
            if (!m_title->load(reader)){
                delete m_title;
				m_title=0;
                return false;
            }else{
                addChild(m_title);
            }
		}else if(reader->name() == "plotArea"){
			m_plotArea->load(reader);
		}else if(reader->name() == "axis"){
            Axis* axis = new Axis("");
            if (!axis->load(reader)){
                delete axis;
                return false;
            }else{
                addChild(axis);
            }
		}else if(reader->name() == "xyCurve"){
            XYCurve* curve = new XYCurve("");
            if (!curve->load(reader)){
                delete curve;
                return false;
            }else{
                addChild(curve);
            }
		}else if(reader->name() == "cartesianPlotLegend"){
            m_legend = new CartesianPlotLegend(this, "");
            if (!m_legend->load(reader)){
                delete m_legend;
                return false;
            }else{
                addChild(m_legend);
				addLegendAction->setEnabled(false);	//only one legend is allowed -> disable the action
            }
        }else{ // unknown element
            reader->raiseWarning(tr("unknown cartesianPlot element '%1'").arg(reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

	d->retransform();
	if (m_title) {
		m_title->setHidden(true);
		m_title->graphicsItem()->setParentItem(m_plotArea->graphicsItem());
	}

    return true;
}
