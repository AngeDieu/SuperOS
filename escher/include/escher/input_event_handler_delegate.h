#ifndef ESCHER_INPUT_EVENT_HANDLER_DELEGATE_H
#define ESCHER_INPUT_EVENT_HANDLER_DELEGATE_H

class InputEventHandler;
class Toolbox;

class InputEventHandlerDelegate {
public:
  virtual Toolbox * toolboxForInputEventHandler(InputEventHandler * textInput) = 0;
};

#endif
