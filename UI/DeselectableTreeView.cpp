#include "DeselectableTreeView.h"

void DeselectableTreeView::mousePressEvent(QMouseEvent *event)
{
	if (selectionModel() == nullptr)
		return;

	QModelIndex item = indexAt(event->pos());
	bool selected = selectionModel()->isSelected(indexAt(event->pos()));
	QTreeView::mousePressEvent(event);
	if ((item.row() == -1 && item.column() == -1) && !selected)
	{
		clearSelection();
		emit(ResetNbFolder());
	}
}