#include "plot_curve_view.h"
#include "plot_controller.h"
#include <apps/shared/curve_view.h>
#include <algorithm>
#include <assert.h>

namespace Statistics {

PlotCurveView::PlotCurveView(Shared::CurveViewRange * curveViewRange,
                             Shared::CurveViewCursor * curveViewCursor,
                             Shared::CursorView * cursorView,
                             Store * store,
                             PlotControllerDelegate * plotControllerDelegate) :
    // No banners to display
    Shared::LabeledCurveView(curveViewRange, curveViewCursor, nullptr, cursorView, false),
    m_store(store),
    m_plotControllerDelegate(plotControllerDelegate) {
}

void PlotCurveView::drawSeriesCurve(KDContext * ctx, KDRect rect, int series) const {
  int sortedIndex[Store::k_maxNumberOfPairs];
  m_store->buildSortedIndex(series, sortedIndex);
  int numberOfPairs = m_plotControllerDelegate->totalValues(series, sortedIndex);
  // Draw and connect each points
  KDColor color = Store::colorLightOfSeriesAtIndex(series);
  double previousX, previousY;
  for (size_t i = 0; i < numberOfPairs; i++) {
    double x = m_plotControllerDelegate->valueAtIndex(series, sortedIndex, i);
    double y = m_plotControllerDelegate->resultAtIndex(series, sortedIndex, i);
    Shared::CurveView::drawDot(ctx, rect, x, y, color);
    if (m_plotControllerDelegate->connectPoints() && i > 0) {
      Shared::CurveView::drawSegment(ctx, rect, x, y, previousX, previousY, color);
    }
    previousX = x;
    previousY = y;
  }
  float x, y, u, v;
  if (m_plotControllerDelegate->drawSeriesZScoreLine(series, &x, &y, &u, &v, &color)) {
    Shared::CurveView::drawSegment(ctx, rect, x, y, u, v, color);
  }
}

void PlotCurveView::drawRect(KDContext * ctx, KDRect rect) const {
  ctx->fillRect(rect, KDColorWhite);
  drawGrid(ctx, rect);
  drawAxes(ctx, rect);
  simpleDrawBothAxesLabels(ctx, rect);
  for (int i = 0; i < Store::k_numberOfSeries; i++) {
    if (m_store->seriesIsValid(i)) {
      drawSeriesCurve(ctx, rect, i);
    }
  }
}

void PlotCurveView::appendLabelSuffix(Axis axis, char * labelBuffer, int maxSize, int glyphLength, int maxGlyphLength) {
  m_plotControllerDelegate->appendLabelSuffix(axis, labelBuffer, maxSize, glyphLength, maxGlyphLength);
}

}
