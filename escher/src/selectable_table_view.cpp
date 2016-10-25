#include <escher/selectable_table_view.h>

SelectableTableView::SelectableTableView(Responder * parentResponder, TableViewDataSource * dataSource, 
  KDCoordinate topMargin, KDCoordinate rightMargin, KDCoordinate bottomMargin, KDCoordinate leftMargin) :
  TableView(dataSource, topMargin, rightMargin, bottomMargin, leftMargin),
  Responder(parentResponder),
  m_selectedCellX(0),
  m_selectedCellY(-1)
{
}

int SelectableTableView::selectedRow() {
  return m_selectedCellY;
}

int SelectableTableView::selectedColumn() {
  return m_selectedCellX;
}

void SelectableTableView::didBecomeFirstResponder() {
  reloadData();
}

void SelectableTableView::deselectTable() {
  TableViewCell * previousCell = cellAtLocation(m_selectedCellX, m_selectedCellY);
  previousCell->setHighlighted(false);
  m_selectedCellX = 0;
  m_selectedCellY = -1;
}

bool SelectableTableView::setSelectedCellAtLocation(int i, int j) {
  if (i < 0 || i >= dataSource()->numberOfColumns()) {
    return false;
  }
  if (j < 0 || j >= dataSource()->numberOfRows()) {
    return false;
  }

  if (m_selectedCellY >= 0) {
    TableViewCell * previousCell = cellAtLocation(m_selectedCellX, m_selectedCellY);
    previousCell->setHighlighted(false);
  }
  m_selectedCellX = i;
  m_selectedCellY = j;
  if (m_selectedCellY >= 0) {
    scrollToCell(i, j);
    TableViewCell * cell = cellAtLocation(i, j);
    cell->setHighlighted(true);
  }
  return true;
}

bool SelectableTableView::handleEvent(Ion::Events::Event event) {
  switch (event) {
    case Ion::Events::Event::DOWN_ARROW:
      return setSelectedCellAtLocation(m_selectedCellX, m_selectedCellY+1);
    case Ion::Events::Event::UP_ARROW:
      return setSelectedCellAtLocation(m_selectedCellX, m_selectedCellY-1);
    case Ion::Events::Event::LEFT_ARROW:
      return setSelectedCellAtLocation(m_selectedCellX-1, m_selectedCellY);
    case Ion::Events::Event::RIGHT_ARROW:
      return setSelectedCellAtLocation(m_selectedCellX+1, m_selectedCellY);
    default:
      return false;
  }
}
