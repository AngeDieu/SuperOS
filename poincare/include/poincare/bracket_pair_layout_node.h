#ifndef POINCARE_BRACKET_PAIR_LAYOUT_NODE_H
#define POINCARE_BRACKET_PAIR_LAYOUT_NODE_H

#include <poincare/layout_cursor.h>
#include <poincare/layout_node.h>
#include <poincare/layout_reference.h>

namespace Poincare {

class BracketPairLayoutRef;

class BracketPairLayoutNode : public LayoutNode {
  friend class BracketPairLayoutRef;
public:
  using LayoutNode::LayoutNode;

  static void RenderWithChildSize(KDSize childSize, KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor);

  // LayoutNode
  void moveCursorLeft(LayoutCursor * cursor, bool * shouldRecomputeLayout) override;
  void moveCursorRight(LayoutCursor * cursor, bool * shouldRecomputeLayout) override;
  void deleteBeforeCursor(LayoutCursor * cursor) override;
  int writeTextInBuffer(char * buffer, int bufferSize, PrintFloat::Mode floatDisplayMode, int numberOfSignificantDigits) const override;
  bool shouldCollapseSiblingsOnRight() const override{ return true; }
  void didCollapseSiblings(LayoutCursor * cursor) override;

  // TreeNode
  size_t size() const override { return sizeof(BracketPairLayoutNode); }
  int numberOfChildren() const override { return 1; }
#if TREE_LOG
  const char * description() const override { return "BracketPairLayout"; }
#endif

protected:
  // LayoutNode
  void computeBaseline() override;
  void computeSize() override;
  KDPoint positionOfChild(LayoutNode * child) override;
  LayoutNode * childLayout() {
    assert(numberOfChildren() == 1);
    return childAtIndex(0);
  }

private:
  constexpr static KDCoordinate k_externWidthMargin = 2;
  constexpr static KDCoordinate k_widthMargin = 5;
  constexpr static KDCoordinate k_verticalExternMargin = 0;
  constexpr static KDCoordinate k_bracketWidth = 5;
  constexpr static KDCoordinate k_lineThickness = 1;
  constexpr static KDCoordinate k_verticalMargin = 1;
  KDCoordinate externWidthMargin() const { return k_externWidthMargin; }
  virtual KDCoordinate widthMargin() const { return k_widthMargin; }
  virtual KDCoordinate verticalExternMargin() const { return k_verticalExternMargin; }
  virtual bool renderTopBar() const { return true; }
  virtual bool renderBottomBar() const { return true; }
  void render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) override;
};

}

#endif

