#ifndef REGRESSION_PREDICTION_PARAMETER_CONTROLLER_H
#define REGRESSION_PREDICTION_PARAMETER_CONTROLLER_H

#include <escher.h>
#include "store.h"
#include "../shared/curve_view_cursor.h"
#include "go_to_parameter_controller.h"

namespace Regression {

class PredictionParameterController : public ViewController, public SimpleListViewDataSource {
public:
  PredictionParameterController(Responder * parentResponder, Store * store, Shared::CurveViewCursor * cursor);
  View * view() override;
  const char * title() const override;
  bool handleEvent(Ion::Events::Event event) override;
  void didBecomeFirstResponder() override;
  int numberOfRows() override;
  KDCoordinate cellHeight() override;
  HighlightCell * reusableCell(int index) override;
  int reusableCellCount() override;
  void willDisplayCellForIndex(HighlightCell * cell, int index) override;
private:
  constexpr static int k_totalNumberOfCells = 2;
  PointerTableCellWithChevron m_cells[2];
  SelectableTableView m_selectableTableView;
  GoToParameterController m_goToParameterController;
};

}

#endif
