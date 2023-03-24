#ifndef REGRESSION_STORE_PARAMETER_CONTROLLER_H
#define REGRESSION_STORE_PARAMETER_CONTROLLER_H

#include <apps/shared/store_parameter_controller.h>
#include <escher/message_table_cell_with_chevron_and_message.h>

#include "../store.h"

namespace Regression {

class StoreController;

class StoreParameterController : public Shared::StoreParameterController {
 public:
  StoreParameterController(Escher::Responder* parentResponder,
                           Shared::StoreColumnHelper* storeColumnHelper);
  bool handleEvent(Ion::Events::Event event) override;
  int numberOfRows() const override {
    return Shared::StoreParameterController::numberOfRows() + 1;
  }
  int typeAtIndex(int index) const override;
  Escher::HighlightCell* reusableCell(int index, int type) override;
  void willDisplayCellForIndex(Escher::HighlightCell* cell, int index) override;

 private:
  constexpr static int k_changeRegressionCellType =
      Shared::StoreParameterController::k_numberOfCells;
  constexpr static int k_changeRegressionCellIndex = 2;

  Escher::MessageTableCellWithChevronAndMessage m_changeRegressionCell;
};

}  // namespace Regression

#endif