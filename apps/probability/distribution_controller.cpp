#include "distribution_controller.h"
#include <assert.h>
#include <new>
#include "app.h"
#include "distribution/binomial_distribution.h"
#include "distribution/chi_squared_distribution.h"
#include "distribution/exponential_distribution.h"
#include "distribution/geometric_distribution.h"
#include "distribution/normal_distribution.h"
#include "distribution/poisson_distribution.h"
#include "distribution/student_distribution.h"
#include "distribution/uniform_distribution.h"
#include "images/binomial_icon.h"
#include "images/chi_squared_icon.h"
#include "images/exponential_icon.h"
#include "images/geometric_icon.h"
#include "images/normal_icon.h"
#include "images/poisson_icon.h"
#include "images/student_icon.h"
#include "images/uniform_icon.h"
#include "images/focused_binomial_icon.h"
#include "images/focused_chi_squared_icon.h"
#include "images/focused_exponential_icon.h"
#include "images/focused_geometric_icon.h"
#include "images/focused_normal_icon.h"
#include "images/focused_poisson_icon.h"
#include "images/focused_student_icon.h"
#include "images/focused_uniform_icon.h"

namespace Probability {

DistributionController::ContentView::ContentView(SelectableTableView * selectableTableView) :
  m_titleView(KDFont::SmallFont, I18n::Message::ChooseDistribution, 0.5f, 0.5f, Palette::GreyDark, Palette::WallScreen),
  m_selectableTableView(selectableTableView)
{
}

int DistributionController::ContentView::numberOfSubviews() const {
  return 2;
}

View * DistributionController::ContentView::subviewAtIndex(int index) {
  assert(index >= 0 && index < 2);
  if (index == 0) {
    return &m_titleView;
  }
  return m_selectableTableView;
}

void DistributionController::ContentView::layoutSubviews(bool force) {
  KDCoordinate titleHeight = KDFont::SmallFont->glyphSize().height()+k_titleMargin;
  m_titleView.setFrame(KDRect(0, 0, bounds().width(), titleHeight), force);
  m_selectableTableView->setFrame(KDRect(0, titleHeight, bounds().width(),  bounds().height()-titleHeight), force);
}

static I18n::Message sMessages[] = {
  I18n::Message::Binomial,
  I18n::Message::Uniforme,
  I18n::Message::Exponential,
  I18n::Message::Normal,
  I18n::Message::ChiSquared,
  I18n::Message::Student,
  I18n::Message::Geometric,
  I18n::Message::Poisson
};

DistributionController::DistributionController(Responder * parentResponder, Distribution * distribution, ParametersController * parametersController) :
  ViewController(parentResponder),
  m_selectableTableView(this),
  m_contentView(&m_selectableTableView),
  m_distribution(distribution),
  m_parametersController(parametersController)
{
  assert(m_distribution != nullptr);
  m_messages = sMessages;
  m_selectableTableView.setTopMargin(Metric::CommonTopMargin-ContentView::k_titleMargin);
}

View * DistributionController::view() {
  return &m_contentView;
}

void Probability::DistributionController::viewWillAppear() {
  selectRow((int)m_distribution->type());
}

void Probability::DistributionController::didBecomeFirstResponder() {
  App::app()->snapshot()->setActivePage(App::Snapshot::Page::Distribution);
  if (selectedRow() == -1) {
    selectCellAtLocation(0, 0);
  } else {
    selectCellAtLocation(selectedColumn(), selectedRow());
  }
  Container::activeApp()->setFirstResponder(&m_selectableTableView);
}

bool Probability::DistributionController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE || event == Ion::Events::Right) {
    StackViewController * stack = (StackViewController *)parentResponder();
    setDistributionAccordingToIndex(selectedRow());
    stack->push(m_parametersController, KDColorWhite, Palette::PurpleBright, Palette::PurpleBright);
    return true;
  }
  return false;
}

int Probability::DistributionController::numberOfRows() const {
  return k_totalNumberOfModels;
};

HighlightCell * Probability::DistributionController::reusableCell(int index) {
  assert(index >= 0);
  assert(index < k_totalNumberOfModels);
  return &m_cells[index];
}

int Probability::DistributionController::reusableCellCount() const {
  return k_totalNumberOfModels;
}

void Probability::DistributionController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  Cell * myCell = (Cell *)cell;
  myCell->setLabel(m_messages[index]);
  const Image * images[k_totalNumberOfModels] = {
    ImageStore::BinomialIcon,
    ImageStore::UniformIcon,
    ImageStore::ExponentialIcon,
    ImageStore::NormalIcon,
    ImageStore::ChiSquaredIcon,
    ImageStore::StudentIcon,
    ImageStore::GeometricIcon,
    ImageStore::PoissonIcon
  };
  const Image * focusedImages[k_totalNumberOfModels] = {
    ImageStore::FocusedBinomialIcon,
    ImageStore::FocusedUniformIcon,
    ImageStore::FocusedExponentialIcon,
    ImageStore::FocusedNormalIcon,
    ImageStore::FocusedChiSquaredIcon,
    ImageStore::FocusedStudentIcon,
    ImageStore::FocusedGeometricIcon,
    ImageStore::FocusedPoissonIcon
  };
  myCell->setImage(images[index], focusedImages[index]);
  myCell->reloadCell();
}

KDCoordinate Probability::DistributionController::cellHeight() {
  return Metric::ParameterCellHeight;
}

void Probability::DistributionController::setDistributionAccordingToIndex(int index) {
  if ((int)m_distribution->type() == index) {
    return;
  }
  m_distribution->~Distribution();
  switch (index) {
    case 0:
      new(m_distribution) BinomialDistribution();
      break;
    case 1:
      new(m_distribution) UniformDistribution();
      break;
    case 2:
      new(m_distribution) ExponentialDistribution();
      break;
    case 3:
      new(m_distribution) NormalDistribution();
      break;
    case 4:
      new(m_distribution) ChiSquaredDistribution();
      break;
    case 5:
      new(m_distribution) StudentDistribution();
      break;
    case 6:
      new(m_distribution) GeometricDistribution();
      break;
    case 7:
      new(m_distribution) PoissonDistribution();
      break;

    default:
     return;
  }
  m_parametersController->reinitCalculation();
}

}
