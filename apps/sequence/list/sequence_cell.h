#ifndef SEQUENCE_SEQUENCE_CELL_H
#define SEQUENCE_SEQUENCE_CELL_H

#include <escher/even_odd_cell.h>
#include <escher/expression_view.h>

#include "vertical_sequence_title_cell.h"

namespace Sequence {

class AbstractSequenceCell : public Escher::EvenOddCell {
 public:
  void setParameterSelected(bool selected);
  virtual Escher::HighlightCell* expressionCell() = 0;
  virtual const Escher::HighlightCell* expressionCell() const = 0;
  VerticalSequenceTitleCell* titleCell() { return &m_sequenceTitleCell; }

 private:
  KDSize minimalSizeForOptimalDisplay() const override;
  int numberOfSubviews() const override { return 2; }
  Escher::View* subviewAtIndex(int index) override;
  void layoutSubviews(bool force = false) override;
  void setEven(bool even) override;

 protected:
  VerticalSequenceTitleCell m_sequenceTitleCell;
  bool m_parameterSelected;
};

class SequenceCell : public AbstractSequenceCell {
 public:
  Escher::EvenOddExpressionCell* expressionCell() override {
    return &m_expressionCell;
  }
  const Escher::EvenOddExpressionCell* expressionCell() const override {
    return &m_expressionCell;
  }
  void updateSubviewsBackgroundAfterChangingState() override;

 private:
  Escher::EvenOddExpressionCell m_expressionCell;
};

}  // namespace Sequence

#endif
