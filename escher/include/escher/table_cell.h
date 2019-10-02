#ifndef ESCHER_TABLE_CELL_H
#define ESCHER_TABLE_CELL_H

#include <escher/highlight_cell.h>
#include <escher/metric.h>

class TableCell : public HighlightCell {
public:
  enum class Layout {
    Vertical,
    Horizontal
  };
  TableCell(Layout layout = Layout::Horizontal);
  virtual View * labelView() const;
  virtual View * accessoryView() const;
  virtual View * subAccessoryView() const;
  void drawRect(KDContext * ctx, KDRect rect) const override;
  constexpr static KDCoordinate k_labelMargin = 10;
  constexpr static KDCoordinate k_accessoryMargin = 10;
protected:
  int numberOfSubviews() const override;
  View * subviewAtIndex(int index) override;
  void layoutSubviews(bool force = false) override;
  constexpr static KDCoordinate k_separatorThickness = Metric::CellSeparatorThickness;
private:
  constexpr static KDCoordinate k_accessoryBottomMargin = 3;
  Layout m_layout;
};

#endif

