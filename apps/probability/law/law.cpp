#include "law.h"
#include <poincare/solver.h>
#include <cmath>
#include <float.h>

namespace Probability {

double Law::cumulativeDistributiveFunctionAtAbscissa(double x) const {
  if (!isContinuous()) {
    int end = std::round(x);
    double result = 0.0;
    for (int k = 0; k <=end; k++) {
      result += evaluateAtDiscreteAbscissa(k);
      /* Avoid too long loop */
      if (k > k_maxNumberOfOperations) {
        break;
      }
      if (result >= k_maxProbability) {
        result = 1.0;
        break;
      }

    }
    return result;
  }
  return 0.0;
}

double Law::rightIntegralFromAbscissa(double x) const {
  if (isContinuous()) {
    return 1.0 - cumulativeDistributiveFunctionAtAbscissa(x);
  }
  return 1.0 - cumulativeDistributiveFunctionAtAbscissa(x-1.0);
}

double Law::finiteIntegralBetweenAbscissas(double a, double b) const {
  if (b < a) {
    return 0.0;
  }
  if (isContinuous()) {
    return cumulativeDistributiveFunctionAtAbscissa(b) - cumulativeDistributiveFunctionAtAbscissa(a);
  }
  int start = std::round(a);
  int end = std::round(b);
  double result = 0.0;
  for (int k = start; k <=end; k++) {
    result += evaluateAtDiscreteAbscissa(k);
    /* Avoid too long loop */
    if (k-start > k_maxNumberOfOperations) {
      break;
    }
    if (result >= k_maxProbability) {
      result = 1.0;
      break;
    }
  }
  return result;
}

double Law::cumulativeDistributiveInverseForProbability(double * probability) {
  if (*probability >= 1.0) {
    return INFINITY;
  }
  if (isContinuous()) {
    return 0.0;
  }
  if (*probability <= 0.0) {
    return 0.0;
  }
  double p = 0.0;
  int k = 0;
  double delta = 0.0;
  do {
    delta = std::fabs(*probability-p);
    p += evaluateAtDiscreteAbscissa(k++);
    if (p >= k_maxProbability && std::fabs(*probability-1.0) <= delta) {
      *probability = 1.0;
      return k-1.0;
    }
  } while (std::fabs(*probability-p) <= delta && k < k_maxNumberOfOperations && p < 1.0);
  p -= evaluateAtDiscreteAbscissa(--k);
  if (k == k_maxNumberOfOperations) {
    *probability = 1.0;
    return INFINITY;
  }
  *probability = p;
  if (std::isnan(*probability)) {
    return NAN;
  }
  return k-1.0;
}

double Law::rightIntegralInverseForProbability(double * probability) {
  if (isContinuous()) {
    double f = 1.0 - *probability;
    return cumulativeDistributiveInverseForProbability(&f);
  }
  if (*probability >= 1.0) {
    return 0.0;
  }
  if (*probability <= 0.0) {
    return INFINITY;
  }
  double p = 0.0;
  int k = 0;
  double delta = 0.0;
  do {
    delta = std::fabs(1.0-*probability-p);
    p += evaluateAtDiscreteAbscissa(k++);
    if (p >= k_maxProbability && std::fabs(1.0-*probability-p) <= delta) {
      *probability = 0.0;
      return k;
    }
  } while (std::fabs(1.0-*probability-p) <= delta && k < k_maxNumberOfOperations);
  if (k == k_maxNumberOfOperations) {
    *probability = 1.0;
    return INFINITY;
  }
  *probability = 1.0 - (p - evaluateAtDiscreteAbscissa(k-1));
  if (std::isnan(*probability)) {
    return NAN;
  }
  return k-1.0;
}

double Law::evaluateAtDiscreteAbscissa(int k) const {
  return 0.0;
}

double Law::cumulativeDistributiveInverseForProbabilityUsingBrentRoots(double * probability) {
  if (*probability >= 1) {
    return INFINITY;
  }
  if (*probability <= 0) {
    return -INFINITY;
  }
  Poincare::Coordinate2D result = Poincare::Solver::BrentMinimum(
      xMin(),
      xMax(),
      [](double x, Poincare::Context * context, Poincare::Preferences::ComplexFormat complexFormat, Poincare::Preferences::AngleUnit angleUnit, const void * context1, const void * context2, const void * context3) {
        const Law * law = reinterpret_cast<const Law *>(context1);
        const double * proba = reinterpret_cast<const double *>(context2);
        return std::fabs(law->cumulativeDistributiveFunctionAtAbscissa(x) - *proba);
      },
      nullptr,
      Poincare::Preferences::sharedPreferences()->complexFormat(),
      Poincare::Preferences::sharedPreferences()->angleUnit(),
      this,
      probability,
      nullptr);
  assert(std::fabs(result.value()) < FLT_EPSILON*10); // TODO FLT_EPSILON is too strict
  return result.abscissa();
}

float Law::yMin() const {
  return -k_displayBottomMarginRatio * yMax();
}

}
