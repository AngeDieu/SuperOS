#ifndef CALCULATION_TRIGONOMETRY_HELPER_H
#define CALCULATION_TRIGONOMETRY_HELPER_H

#include <poincare/expression.h>

namespace Calculation {

namespace TrigonometryHelper {

Poincare::Expression ExtractExactAngleFromDirectTrigo(
    const Poincare::Expression input, const Poincare::Expression exactOutput,
    Poincare::Context* context,
    const Poincare::Preferences::ComplexFormat complexFormat,
    const Poincare::Preferences::AngleUnit angleUnit);

}

}  // namespace Calculation

#endif
