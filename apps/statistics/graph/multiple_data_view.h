#ifndef STATISTICS_MULTIPLE_DATA_VIEW_H
#define STATISTICS_MULTIPLE_DATA_VIEW_H

#include "data_view.h"
#include "../store.h"
#include <apps/shared/curve_view.h>

namespace Statistics {

class MultipleDataView : public DataView {
public:
  using DataView::DataView;
  virtual Shared::CurveView * dataViewForSeries(int series) = 0;
  KDCoordinate subviewHeight();

  // DataView
  Shared::CurveView * curveViewForSeries(int series) override { return dataViewForSeries(series); };

  // Display
  void reload() override;

  // View
  int numberOfSubviews() const override;
protected:
  void layoutDataSubviews(bool force) override;
  Escher::View * subviewAtIndex(int index) override;
};

}

#endif
