/*
Copyright (C) 2015 Ruslan Kabatsayev <b7.10110111@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ODBG_REGISTER_VIEW_H_20151230
#define ODBG_REGISTER_VIEW_H_20151230

#include <QScrollArea>
#include <QString>
#include <QLabel>
#include <array>
#include <deque>
#include <QPersistentModelIndex>
#include <functional>
#include "RegisterViewModelBase.h"

class QSettings;

namespace ODbgRegisterView {

class RegisterGroup;
class FieldWidget;
class ValueField;

class ODBRegView : public QScrollArea
{
	Q_OBJECT

	RegisterViewModelBase::Model* model_=nullptr;
	enum class RegisterGroupType
	{
		GPR,
		rIP,
		EFL,
		ExpandedEFL,
		Segment,
		FPUData,
		FPUWords,
		FPULastOp,
		Debug,
		MMX,
		SSEData,
		AVXData,
		MXCSR
	};
	std::vector<RegisterGroupType> regGroupTypes;
	QList<QAction*> menuItems;

	RegisterGroup* makeGroup(RegisterGroupType type);
public:
	ODBRegView(QSettings const& settings, QWidget* parent=nullptr);
	void setModel(RegisterViewModelBase::Model* model);
	QList<ValueField*> valueFields() const;
	QList<FieldWidget*> fields() const;
	void showMenu(QPoint const& position,QList<QAction*>const& additionalItems={}) const;
	void saveState(QSettings& settings) const;
private:
	ValueField* selectedField() const;
	void updateFieldsPalette();
	void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

	QList<RegisterGroup*> groups;
private Q_SLOTS:
	void fieldSelected();
	void modelReset();
	void modelUpdated();
};

class Canvas : public QWidget
{
public:
	Canvas(QWidget* parent=nullptr);
protected:
    void mousePressEvent(QMouseEvent* event) override;
};

class FieldWidget : public QLabel
{
	Q_OBJECT

	void init(int fieldWidth);
protected:
	QPersistentModelIndex index;
	virtual QString text() const;
	ODBRegView* regView() const;
	RegisterGroup* group() const;
public:
	FieldWidget(int fieldWidth,QModelIndex const& index,QWidget* parent=nullptr);
	FieldWidget(int fieldWidth,QString const& fixedText,QWidget* parent=nullptr);
	FieldWidget(QString const& fixedText,QWidget* parent=nullptr);
public Q_SLOTS:
	virtual void update();
};

class ValueField : public FieldWidget
{
	Q_OBJECT

	bool selected_=false;
	bool hovered_=false;
	std::function<QString(QString)> valueFormatter;
	QList<QAction*> menuItems;

	void init();
	bool changed() const;
	QColor fgColorForChangedField() const;
protected:
	QString text() const override;
	void enterEvent(QEvent*) override;
	void leaveEvent(QEvent*) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;

	ValueField* bestNeighbor(std::function<bool(QPoint const& neighborPos,
												ValueField const*curResult,
												QPoint const& selfPos)>const& firstIsBetter) const;
public:
	ValueField(int fieldWidth,
			   QModelIndex const& index,
			   QWidget* parent=nullptr,
			   std::function<QString(QString const&)> const& valueFormatter=[](QString const&s){return s;}
			   );
	ValueField* up() const;
	ValueField* down() const;
	ValueField* left() const;
	ValueField* right() const;

	bool isSelected() const;
	void defaultAction();
	void showMenu(QPoint const& position);
public Q_SLOTS:
	void update() override;
	void select();
	void unselect();
	void updatePalette();
Q_SIGNALS:
	void selected();
};

class SIMDValueManager : public QObject
{
	Q_OBJECT
	QPersistentModelIndex regIndex;
	int lineInGroup;
	QList<ValueField*> elements;
	QList<QAction*> menuItems;
	NumberDisplayMode intMode;
	enum MenuItemNumbers
	{
		VIEW_AS_BYTES,
		VIEW_AS_WORDS,
		VIEW_AS_DWORDS,
		VIEW_AS_QWORDS,

		VIEW_AS_FLOAT32,
		VIEW_AS_FLOAT64,

		VIEW_INT_AS_HEX,
		VIEW_INT_AS_SIGNED,
		VIEW_INT_AS_UNSIGNED,

		MENU_ITEMS_COUNT
	};

	using Model=RegisterViewModelBase::Model;
	Model* model() const;
	RegisterGroup* group() const;
	Model::ElementSize currentSize() const;
	NumberDisplayMode currentFormat() const;
	void setupMenu();
	void updateMenu();
	void fillGroupMenu();
public:
	SIMDValueManager(int lineInGroup,
					 QModelIndex const& nameIndex,
					 RegisterGroup* parent=nullptr);
public Q_SLOTS:
	void displayFormatChanged();
private Q_SLOTS:
	void showAsInt(int size);
	void showAsFloat(int size);
	void setIntFormat(int format);
};

class RegisterGroup : public QWidget
{
	Q_OBJECT

	QList<QAction*> menuItems;
	QString name;

	int lineAfterLastField() const;
	ODBRegView* regView() const;
public:
	RegisterGroup(QString const& name, QWidget* parent=nullptr);
	QList<FieldWidget*> fields() const;
	QList<ValueField*> valueFields() const;
	void setIndices(QList<QModelIndex> const& indices);
	void insert(int line, int column, FieldWidget* widget);
	void appendNameValueComment(QModelIndex const& nameIndex,QString const& tooltip="",bool insertComment=true);
	void showMenu(QPoint const& position,QList<QAction*>const& additionalItems={}) const;
protected:
	void mousePressEvent(QMouseEvent* event) override;
public Q_SLOTS:
	void adjustWidth();

	friend SIMDValueManager;
};

}

#endif
