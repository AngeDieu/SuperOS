#ifndef ESCHER_LAYOUT_FIELD_DELEGATE_H
#define ESCHER_LAYOUT_FIELD_DELEGATE_H

#include <escher/toolbox.h>
#include <escher/input_event_handler_delegate.h>
#include <ion/events.h>

class LayoutField;

class LayoutFieldDelegate : public InputEventHandlerDelegate {
public:
  virtual bool layoutFieldShouldFinishEditing(LayoutField * layoutField, Ion::Events::Event event) = 0;
  virtual bool layoutFieldDidReceiveEvent(LayoutField * layoutField, Ion::Events::Event event) = 0;
  virtual bool layoutFieldDidFinishEditing(LayoutField * layoutField, Poincare::Layout layoutR, Ion::Events::Event event) { return false; }
  virtual bool layoutFieldDidAbortEditing(LayoutField * layoutField) { return false; }
  virtual void layoutFieldDidChangeSize(LayoutField * layoutField) {}
};

#endif
