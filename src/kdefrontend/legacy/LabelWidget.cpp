/***************************************************************************
    File                 : LabelWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : label settings widget

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
#include "LabelWidget.h"
#include "../elements/Label.h"
#include <KDebug>

/*!
	\class LabelWidget
 	\brief Widget for editing the properties of a Label object.

 	\ingroup kdefrontend
 */
LabelWidget::LabelWidget(QWidget *parent): QWidget(parent){

	ui.setupUi(this);
	ui.teLabel->setTextColor(Qt::black);
	ui.kcbFillingColor->setColor(Qt::black);
	ui.kcbTextColor->setColor(Qt::black);

	//Populate the comboboxes
	QFont symbol("Symbol", 12, QFont::Bold);
 	ui.cbSmallGreekLetters->setFont(symbol);
	for(int i=97;i<122;i++)
		ui.cbSmallGreekLetters->addItem(QChar(i));

	ui.cbBigGreekLetters->setFont(symbol);
	for(int i=68;i<90;i++)
		ui.cbBigGreekLetters->addItem(QChar(i));

	ui.cbSymbolLetters->setFont(symbol);
 	ui.cbSymbolLetters->addItem(QChar(34));
	ui.cbSymbolLetters->addItem(QChar(36));
	ui.cbSymbolLetters->addItem(QChar(39));
	ui.cbSymbolLetters->addItem(QChar(64));
	for(int i=160;i<255;i++)
		ui.cbSymbolLetters->addItem(QChar(i));

	//Validators
	ui.lePositionX->setValidator( new QDoubleValidator(ui.lePositionX) );
	ui.lePositionY->setValidator( new QDoubleValidator(ui.lePositionY) );
	ui.leRotation->setValidator( new QDoubleValidator(ui.leRotation) );

	//Icons
	ui.bFontBold->setIcon( KIcon("format-text-bold") );
	ui.bFontItalic->setIcon( KIcon("format-text-italic") );
	ui.bFontUnderline->setIcon( KIcon("format-text-underline") );
	ui.bFontSuperscript->setIcon( KIcon("format-text-superscript") );
	ui.bFontSubscript->setIcon( KIcon("format-text-subscript") );

	//Slots
	//Alignment
	connect( ui.cbPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(positionChanged(int)) );
	connect( ui.lePositionX, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.lePositionY, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );
	connect( ui.leRotation, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()) );

	//Background
	connect( ui.rbFilling0, SIGNAL(toggled(bool)), this, SLOT( fillingChanged(bool)) );
	connect( ui.rbFilling1, SIGNAL(toggled(bool)), this, SLOT( fillingChanged(bool)) );
	connect( ui.kcbFillingColor, SIGNAL(changed(const QColor& )), this, SLOT(slotDataChanged()) );
	connect( ui.chbBox, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );
	connect( ui.chbShadow, SIGNAL(stateChanged(int)), this, SLOT(slotDataChanged()) );

	//Text
 	connect( ui.kcbTextColor, SIGNAL(changed(const QColor& )), this, SLOT(textColorChanged(const QColor& )) );
 	connect( ui.kfontRequester, SIGNAL(fontSelected(const QFont& )), this, SLOT(fontChanged(const QFont& )) );

	connect( ui.chbTex, SIGNAL(stateChanged(int)), this, SLOT(useTexChanged(int)) );
	connect( ui.bFontBold, SIGNAL(toggled(bool)), this, SLOT( fontBoldToggled(bool)) );
	connect( ui.bFontItalic, SIGNAL(toggled(bool)), this, SLOT( fontItalicToggled(bool)) );
	connect( ui.bFontUnderline, SIGNAL(toggled(bool)), this, SLOT( fontUnderlineToggled(bool)) );
	connect( ui.bFontSuperscript, SIGNAL(toggled(bool)), this, SLOT( fontSuperscriptToggled(bool)) );
	connect( ui.bFontSubscript, SIGNAL(toggled(bool)), this, SLOT( fontSubscriptToggled(bool)) );

	connect( ui.cbSmallGreekLetters, SIGNAL(activated(const QString &)), this, SLOT(insertSymbol(const QString &)));
	connect( ui.cbBigGreekLetters, SIGNAL(activated(const QString &)), this, SLOT(insertSymbol(const QString &)));
	connect( ui.cbSymbolLetters, SIGNAL(activated(const QString &)), this, SLOT(insertSymbol(const QString &)));

	connect( ui.teLabel, SIGNAL(textChanged()), this, SLOT(slotDataChanged()) );
}

LabelWidget::~LabelWidget() {}

/*!
	displays the Label object \c label in the widget.
*/
void LabelWidget::setLabel(const Label* label) {
	//TODO add some "asserts"!!!
	ui.cbPosition->setCurrentIndex( label->positionType() );
	ui.lePositionX->setText( QString::number(label->position().x()) );
	ui.lePositionY->setText( QString::number(label->position().y()) );
	ui.leRotation->setText( QString::number(label->rotation()) );

	if ( label->hasFilling() == false )
		ui.rbFilling0->setChecked(true);
	else
		ui.rbFilling1->setChecked(true);

	ui.kcbFillingColor->setColor(label->fillingColor());
	ui.chbBox->setChecked( label->hasBox() );
	ui.chbShadow->setChecked( label->hasShadow() );

	ui.kfontRequester->setFont(label->textFont() );
	ui.kcbTextColor->setColor(label->textColor());
	if (label->isTex() ){
		ui.chbTex->setChecked( true );
		ui.teLabel->setPlainText( label->text() );
	}else{
		ui.chbTex->setChecked( false );
		ui.teLabel->setHtml( label->text() );
	}

	kDebug()<<"Label data shown."<<endl;
}

/*!
	enables ( \c b=true ) or disables ( \c b=true ) rotation.
	Needed for/in ??? //TODO
*/
void LabelWidget::setLabelRotationEnabled(const bool b){
	ui.lRotation->setEnabled(b);
	ui.leRotation->setEnabled(b);
	ui.lDegree->setEnabled(b);
}

/*!
	saves the Label object \c label.
*/
void LabelWidget::saveLabel(Label* label) const{
	label->setPositionType( Label::LabelPosition(ui.cbPosition->currentIndex()) );
	label->setPosition( QPointF(ui.lePositionX->text().toFloat(), ui.lePositionY->text().toFloat()) );
	label->setRotation( ui.leRotation->text().toFloat() );

	label->setFilling( ui.rbFilling1->isChecked() );
	label->setFillingColor( ui.kcbFillingColor->color() );
	label->enableBox( ui.chbBox->isChecked() );
	label->enableShadow( ui.chbShadow->isChecked() );

	label->setTextFont( ui.kfontRequester->font() );
	label->setTextColor( ui.kcbTextColor->color() );
	label->enableTex( ui.chbTex->isChecked() );
	if ( label->isTex() )
		label->setText( ui.teLabel->toPlainText() );
	else
		label->setText( ui.teLabel->toHtml() );

	kDebug()<<"Label data saved."<<endl;
}


//***********************************************************************************
//**********************************    SLOTS   **************************************
//***********************************************************************************

/*!
	called if the current position of the title is changed in the combobox.
	Enables/disables the lineedits for x- and y-coordinates if the "custom"-item is selected/deselected
*/
void LabelWidget::positionChanged(int index){
	if (index == ui.cbPosition->count()-1 ){
		ui.lePositionX->setEnabled(true);
		ui.lePositionY->setEnabled(true);
	}else{
		ui.lePositionX->setEnabled(false);
		ui.lePositionY->setEnabled(false);
	}
	emit dataChanged(true);
}

/*!

*/
void LabelWidget::fillingChanged(bool){
	if (ui.rbFilling0->isChecked())
		ui.kcbFillingColor->setEnabled(false);
	else
		ui.kcbFillingColor->setEnabled(true);

	emit dataChanged(true);
}

void LabelWidget::fillingColorClicked(){
	emit dataChanged(true);
}

void LabelWidget::fontChanged(const QFont& font){
 	ui.teLabel->selectAll();
	ui.teLabel->setCurrentFont(font);

	//TODO Clear selection!!!
// 	ui.teLabel->textCursor().clearSelection();

	ui.teLabel->setFocus();
	emit dataChanged(true);
}

void LabelWidget::textColorChanged(const QColor& color){
 	ui.teLabel->selectAll();
	ui.teLabel->setTextColor(color);

	//TODO Clear selection!!!
// 	ui.teLabel->textCursor().clearSelection();

	ui.teLabel->setFocus();
	emit dataChanged(true);
}

void LabelWidget::useTexChanged(int state){
	QFont font;
	if (state==Qt::Checked){	//use Tex-syntax
		//remove bold-, italic- etc. properties in texteditwidget
		font.setBold(false);
		font.setItalic(false);
		font.setUnderline(false);

		ui.frameFontOptions->setEnabled(false);
	}else{
		font=ui.kfontRequester->font();
		font.setBold(ui.bFontBold->isChecked());
		font.setItalic(ui.bFontItalic->isChecked());
		font.setUnderline(ui.bFontUnderline->isChecked());

		ui.frameFontOptions->setEnabled(true);
	}
	ui.teLabel->setCurrentFont(font);
	emit dataChanged(true);
}

void LabelWidget::fontBoldToggled(bool checked){
	QFont font=ui.teLabel->font();
	font.setBold(checked);
	ui.teLabel->setCurrentFont(font);
	ui.teLabel->setFocus();
}


void LabelWidget::fontItalicToggled(bool checked){
	if (checked)
		ui.teLabel->setFontItalic(true);
	else
		ui.teLabel->setFontItalic(false);

	ui.teLabel->setFocus();
}

void LabelWidget::fontUnderlineToggled(bool checked){
	if (checked)
		ui.teLabel->setFontUnderline(true);
	else
		ui.teLabel->setFontUnderline(false);

	ui.teLabel->setFocus();
}

void LabelWidget::fontSuperscriptToggled(bool checked){
	QTextCharFormat format;
	if (checked)
		format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
	else
		format.setVerticalAlignment(QTextCharFormat::AlignNormal);

	ui.teLabel->mergeCurrentCharFormat(format);
	ui.teLabel->setFocus();
}


void LabelWidget::fontSubscriptToggled(bool checked){
	QTextCharFormat format;
	if (checked)
		format.setVerticalAlignment(QTextCharFormat::AlignSubScript);
	else
		format.setVerticalAlignment(QTextCharFormat::AlignNormal);

	ui.teLabel->mergeCurrentCharFormat(format);
	ui.teLabel->setFocus();
}

void LabelWidget::insertSymbol(const QString& string) {
	QString fontFamily=ui.teLabel->fontFamily();
	ui.teLabel->setFontFamily("Symbol");
	ui.teLabel->insertPlainText(string);
	ui.teLabel->setFocus();
	ui.teLabel->setFontFamily(fontFamily);
	ui.teLabel->setFocus();
	emit dataChanged(true);
}

void LabelWidget::setText(const QString& s){
	ui.teLabel->clear();
	ui.teLabel->insertPlainText(s);
}

void LabelWidget::slotDataChanged(){
	emit dataChanged(true);
}