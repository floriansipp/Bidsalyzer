#ifndef DESELECTABLETREEVIEW_H
#define DESELECTABLETREEVIEW_H

#include "QTreeView"
#include "QMouseEvent"
#include "QDebug"

class DeselectableTreeView : public QTreeView
{
	Q_OBJECT

public:
	DeselectableTreeView(QWidget *parent) : QTreeView(parent) {}
	virtual ~DeselectableTreeView() {}

private:
	virtual void mousePressEvent(QMouseEvent *event);

signals:
	void ResetNbFolder();
};
#endif // DESELECTABLETREEVIEW_H