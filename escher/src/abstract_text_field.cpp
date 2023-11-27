#include <assert.h>
#include <escher/abstract_text_field.h>
#include <escher/clipboard.h>
#include <escher/container.h>
#include <escher/text_input_helpers.h>
#include <ion/events.h>
#include <ion/keyboard/layout_events.h>
#include <ion/unicode/utf8_decoder.h>
#include <ion/unicode/utf8_helper.h>
#include <poincare/aliases_list.h>
#include <poincare/parametered_expression.h>
#include <poincare/serialization_helper.h>
#include <poincare/symbol_abstract.h>
#include <poincare/xnt_helpers.h>

#include <algorithm>

namespace Escher {

static char s_sharedTextBuffer[AbstractTextField::MaxBufferSize()];

/* AbstractTextField::ContentView */

AbstractTextField::ContentView::ContentView(char *textBuffer,
                                            size_t textBufferSize,
                                            KDGlyph::Format format)
    : TextInput::ContentView(format),
      m_textBuffer(textBuffer),
      m_textBufferSize(textBufferSize),
      m_isEditing(false),
      m_isStalled(false) {
  if (textBuffer == nullptr) {
    m_textBuffer = s_sharedTextBuffer;
  }
  assert(m_textBufferSize <= k_maxBufferSize);
}

void AbstractTextField::ContentView::setBackgroundColor(
    KDColor backgroundColor) {
  m_format.style.backgroundColor = backgroundColor;
  markWholeFrameAsDirty();
}

void AbstractTextField::ContentView::setTextColor(KDColor textColor) {
  m_format.style.glyphColor = textColor;
  markWholeFrameAsDirty();
}

void AbstractTextField::ContentView::drawRect(KDContext *ctx,
                                              KDRect rect) const {
  KDColor backgroundColor = m_format.style.backgroundColor;
  if (m_isEditing) {
    backgroundColor = KDColorWhite;
  }
  ctx->fillRect(bounds(), backgroundColor);
  KDGlyph::Style glyphStyle{.glyphColor = m_format.style.glyphColor,
                            .backgroundColor = backgroundColor,
                            .font = m_format.style.font};
  if (selectionIsEmpty()) {
    ctx->drawString(text(), glyphFrameAtPosition(text(), text()).origin(),
                    glyphStyle);
  } else {
    assert(m_isEditing);
    int selectionOffset = selectionLeft() - text();
    const char *textToDraw = text();
    // Draw the non selected text on the left of the selection
    ctx->drawString(textToDraw, glyphFrameAtPosition(text(), text()).origin(),
                    glyphStyle, selectionOffset);
    int selectionLength = selectionRight() - selectionLeft();
    textToDraw += selectionOffset;
    // Draw the selected text
    ctx->drawString(text() + selectionOffset,
                    glyphFrameAtPosition(text(), textToDraw).origin(),
                    {.glyphColor = m_format.style.glyphColor,
                     .backgroundColor = Palette::Select,
                     .font = m_format.style.font},
                    selectionLength);
    textToDraw += selectionLength;
    // Draw the non selected text on the right of the selection
    ctx->drawString(text() + selectionOffset + selectionLength,
                    glyphFrameAtPosition(text(), textToDraw).origin(),
                    glyphStyle);
  }
}

const char *AbstractTextField::ContentView::text() const {
  return const_cast<const char *>(m_textBuffer);
}

void AbstractTextField::ContentView::setText(const char *text) {
  size_t textRealLength = strlen(text);
  if (textRealLength > m_textBufferSize - 1) {
    /* The text was too long to be copied
     * TODO Maybe add a warning for the user? */
    m_textBuffer[0] = 0;
    return;
  }
  // Copy the text
  strlcpy(m_textBuffer, text, m_textBufferSize);
  /* Replace System parentheses (used to keep layout tree structure) by normal
   * parentheses */
  Poincare::SerializationHelper::
      ReplaceSystemParenthesesAndBracesByUserParentheses(m_textBuffer);
  markWholeFrameAsDirty();
}

void AbstractTextField::ContentView::setEditing(bool isEditing) {
  if (m_isEditing == isEditing) {
    return;
  }
  if (!isEditing) {
    m_isStalled = false;
  }
  resetSelection();
  m_isEditing = isEditing;
  const char *bufferStart = text();
  const char *bufferEnd = textEnd();
  if (m_cursorLocation < bufferStart || m_cursorLocation > bufferEnd) {
    m_cursorLocation = bufferEnd;
  }
  markWholeFrameAsDirty();
  layoutSubviews();
}

void AbstractTextField::ContentView::stallOrStopEditing() {
  if (!m_isStalled) {
    setEditing(false);
  }
}

void AbstractTextField::ContentView::reinitTextBuffer() {
  /* We first need to clear the buffer, otherwise setCursorLocation might do
   * various operations on a buffer with maybe non-initialized content, such as
   * stringSize, etc. Those operation might be perilous on non-UTF8 content. */
  m_textBuffer[0] = 0;
  setCursorLocation(text());
}

bool AbstractTextField::ContentView::insertTextAtLocation(const char *text,
                                                          char *location,
                                                          int textLen) {
  assert(m_isEditing);

  char *buffer = const_cast<char *>(this->text());
  size_t editedLength = textLength();

  size_t textLength = textLen < 0 ? strlen(text) : (size_t)textLen;
  // TODO when paste fails because of a too big message, create a pop-up
  if (editedLength + textLength >= m_textBufferSize || textLength == 0) {
    return false;
  }

  memmove(location + textLength, location,
          (buffer + editedLength + 1) - location);

  /* Caution: One byte will be overridden by the null-terminating char of
   * strlcpy */
  size_t copySize =
      std::min(textLength + 1,
               static_cast<size_t>((buffer + m_textBufferSize) - location));
  char *overridenByteLocation = location + copySize - 1;
  char overridenByte = *overridenByteLocation;
  strlcpy(location, text, copySize);
  *overridenByteLocation = overridenByte;

  reloadRectFromPosition(m_format.horizontalAlignment == 0.0f ? location
                                                              : buffer);
  return true;
}

KDSize AbstractTextField::ContentView::minimalSizeForOptimalDisplay() const {
  KDSize stringSize = KDFont::Font(m_format.style.font)->stringSize(text());
  assert(stringSize.height() == KDFont::GlyphHeight(m_format.style.font));
  if (m_isEditing) {
    return KDSize(stringSize.width() + TextCursorView::k_width,
                  stringSize.height());
  }
  return stringSize;
}

bool AbstractTextField::ContentView::removePreviousGlyph() {
  assert(m_isEditing);

  char *buffer = const_cast<char *>(text());

  if (m_format.horizontalAlignment > 0.0f) {
    /* Reload the view. If we do it later, the text beins supposedly shorter, we
     * will not clean the first char. */
    reloadRectFromPosition(buffer);
  }
  // Remove the glyph if possible
  int removedLength = UTF8Helper::RemovePreviousGlyph(
      buffer, const_cast<char *>(cursorLocation()));
  if (removedLength == 0) {
    assert(cursorLocation() == buffer);
    return false;
  }

  // Set the cursor location and reload the view
  assert(cursorLocation() - removedLength >= buffer);
  setCursorLocation(cursorLocation() - removedLength);
  if (m_format.horizontalAlignment == 0.0f) {
    reloadRectFromPosition(cursorLocation());
  }
  layoutSubviews();
  return true;
}

bool AbstractTextField::ContentView::removeEndOfLine() {
  assert(m_isEditing);
  char *buffer = const_cast<char *>(text());
  size_t lengthToCursor = (size_t)(cursorLocation() - buffer);
  if (textLength() == lengthToCursor) {
    return false;
  }
  reloadRectFromPosition(m_format.horizontalAlignment == 0.0f ? cursorLocation()
                                                              : buffer);
  *(const_cast<char *>(cursorLocation())) = 0;
  layoutSubviews();
  return true;
}

void AbstractTextField::ContentView::willModifyTextBuffer() {
  assert(m_isEditing);
  /* This method should be called when the buffer is modified outside the
   * content view, for instance from the textfield directly. */
  reloadRectFromPosition(text());
}

void AbstractTextField::ContentView::didModifyTextBuffer() {
  /* This method should be called when the buffer is modified outside the
   * content view, for instance from the textfield directly. */
  layoutSubviews();
}

size_t AbstractTextField::dumpContent(char *buffer, size_t bufferSize,
                                      int *cursorOffset) {
  size_t size = textLength() + 1;
  if (size > bufferSize) {
    buffer[0] = 0;
    *cursorOffset = -1;
  } else {
    memcpy(buffer, text(), size);
    *cursorOffset = cursorLocation() - text();
  }
  return 0;
}

size_t AbstractTextField::ContentView::deleteSelection() {
  assert(!selectionIsEmpty());
  assert(m_isEditing);
  size_t removedLength = selectionRight() - selectionLeft();
  char *buffer = const_cast<char *>(text());
  strlcpy(const_cast<char *>(selectionLeft()), selectionRight(),
          m_textBufferSize - (selectionLeft() - buffer));
  // We cannot call resetSelection() because m_selectionEnd is invalid.
  m_selectionStart = nullptr;
  return removedLength;
}

KDRect AbstractTextField::ContentView::cursorRect() const {
  return m_isEditing ? TextInput::ContentView::cursorRect() : KDRectZero;
}

KDRect AbstractTextField::ContentView::glyphFrameAtPosition(
    const char *buffer, const char *position) const {
  assert(buffer != nullptr && position != nullptr);
  assert(position >= buffer);
  KDSize glyphSize = KDFont::GlyphSize(m_format.style.font);
  KDCoordinate cursorWidth = TextCursorView::k_width;
  KDCoordinate horizontalOffset =
      m_format.horizontalAlignment == 0.0f
          ? 0.0f
          : m_format.horizontalAlignment *
                (bounds().width() -
                 KDFont::Font(m_format.style.font)->stringSize(buffer).width() -
                 cursorWidth);
  return KDRect(
      horizontalOffset + KDFont::Font(m_format.style.font)
                             ->stringSizeUntil(buffer, position)
                             .width(),
      m_format.verticalAlignment * (bounds().height() - glyphSize.height()),
      glyphSize);
}

/* AbstractTextField */

AbstractTextField::AbstractTextField(Responder *parentResponder,
                                     View *contentView,
                                     TextFieldDelegate *delegate)
    : TextInput(parentResponder, contentView), m_delegate(delegate) {}

void AbstractTextField::setBackgroundColor(KDColor backgroundColor) {
  ScrollView::setBackgroundColor(backgroundColor);
  contentView()->setBackgroundColor(backgroundColor);
}

void AbstractTextField::setText(const char *text) {
  if (contentView()->isStalled()) {
    return;
  }
  contentView()->setText(text);
  resetScroll();
  /* Set the cursor location here and not in ContentView::setText so that
   * TextInput::willSetCursorLocation is called. */
  setCursorLocation(this->text() + strlen(text));
}

void AbstractTextField::removePreviousGlyphIfRepetition(
    bool defaultXNTHasChanged) {
  if (!defaultXNTHasChanged && Ion::Events::repetitionFactor() > 0 &&
      isEditing() && contentView()->selectionIsEmpty()) {
    // Since XNT is cycling on simple glyphs, remove the last inserted one
    bool success = removePreviousGlyph();
    assert(success);
    (void)success;  // Silence compilation warnings
    /* TODO: Handle cycling with non-default layouts and Fix issues with
     * repetition over a syntax error dismissal */
  }
}

// TODO : Handle cycling with non-default layouts.
size_t AbstractTextField::insertXNTChars(CodePoint defaultXNTCodePoint,
                                         char *buffer, size_t bufferLength) {
  assert(isEditable());
  if (!isEditing()) {
    setEditing(true);
    reinitTextBuffer();
    if (m_delegate) {
      m_delegate->textFieldDidStartEditing(this);
    }
  }
  UTF8Decoder decoder(text(), cursorLocation());
  bool defaultXNTHasChanged = false;
  if (Poincare::FindXNTSymbol(decoder, &defaultXNTHasChanged,
                              &defaultXNTCodePoint)) {
    const char *parameterText;
    size_t parameterLength = bufferLength + 1;
    if (Poincare::ParameteredExpression::ParameterText(
            decoder.stringPosition(), &parameterText, &parameterLength) &&
        bufferLength >= parameterLength) {
      memcpy(buffer, parameterText, parameterLength);
      return parameterLength;
    }
  }
  removePreviousGlyphIfRepetition(defaultXNTHasChanged);
  return UTF8Decoder::CodePointToChars(defaultXNTCodePoint, buffer,
                                       bufferLength);
}

bool AbstractTextField::addXNTCodePoint(CodePoint xnt) {
  if (!isEditable()) {
    return false;
  }
  constexpr int bufferSize = Poincare::SymbolAbstractNode::k_maxNameSize;
  char buffer[bufferSize];
  size_t length = insertXNTChars(xnt, buffer, bufferSize - 1);

  assert(length < bufferSize);
  buffer[length] = 0;
  return handleEventWithText(buffer, false, true);
}

void AbstractTextField::willResignFirstResponder() {
  contentView()->stallOrStopEditing();
  TextInput::willResignFirstResponder();
}

bool AbstractTextField::handleEvent(Ion::Events::Event event) {
  assert(!contentView()->isStalled());
  bool textDidChange;
  bool didHandleEvent = privateHandleEvent(event, &textDidChange);
  if (didHandleEvent && textDidChange && m_delegate) {
    m_delegate->textFieldDidHandleEvent(this);
  }
  return didHandleEvent;
}

bool AbstractTextField::privateHandleEvent(Ion::Events::Event event,
                                           bool *textDidChange) {
  assert(!contentView()->isStalled());
  size_t previousTextLength = strlen(text());
  *textDidChange = false;

  // Handle move and selection
  if (handleMoveEvent(event) || handleSelectEvent(event)) {
    return true;
  }

  // Notify delegate
  if (m_delegate) {
    if (m_delegate->textFieldDidReceiveEvent(this, event)) {
      return true;
    }
    if (isEditing() && m_delegate->textFieldShouldFinishEditing(this, event)) {
      setEditing(false);
      if (m_delegate->textFieldDidFinishEditing(this, event)) {
        resetScroll();
      } else {
        setEditing(true);
      }
      *textDidChange = previousTextLength != strlen(text());
      return true;
    }
  }

  /* If move event was not caught neither by handleMoveEvent nor by
   * layoutFieldShouldFinishEditing, we handle it here to avoid bubbling the
   * event up. */
  if (event.isMoveEvent() && isEditing()) {
    return true;
  }

  // Handle sto
  if (event == Ion::Events::Sto && handleStoreEvent()) {
    return true;
  }

  // Handle copy
  if (event == Ion::Events::Copy) {
    storeInClipboard();
    return true;
  }

  if (!isEditable()) {
    return false;
  }
  // From now on, we only consider editable fields

  // Handle boxes
  if (privateHandleBoxEvent(event)) {
    return true;
  }

  // Handle cut
  if (event == Ion::Events::Cut) {
    if (storeInClipboard()) {
      if (!selectionIsEmpty()) {
        deleteSelection();
      } else {
        removeWholeText();
      }
      *textDidChange = true;
    }
    return true;
  }

  // Handle paste
  if (event == Ion::Events::Paste) {
    bool didHandleEvent = privateHandleEventWithText(
        Clipboard::SharedClipboard()->storedText(), false, true);
    *textDidChange = didHandleEvent;
    return didHandleEvent;
  }

  // Enter edition
  if ((event == Ion::Events::OK || event == Ion::Events::EXE) && !isEditing()) {
    const char *previousText = text();
    setEditing(true);
    if (m_delegate) {
      m_delegate->textFieldDidStartEditing(this);
    }
    setText(previousText);
    *textDidChange = true;
    return true;
  }

  // Handle back
  if (event == Ion::Events::Back && isEditing()) {
    setEditing(false);
    if (m_delegate) {
      m_delegate->textFieldDidAbortEditing(this);
    }
    resetScroll();
    *textDidChange = previousTextLength != strlen(text());
    return true;
  }

  // Handle backspace
  if (event == Ion::Events::Backspace && isEditing()) {
    if (selectionIsEmpty()) {
      return (*textDidChange = removePreviousGlyph());
    }
    deleteSelection();
    *textDidChange = true;
    return true;
  }

  // Handle clear
  if (event == Ion::Events::Clear && isEditing()) {
    if (!selectionIsEmpty()) {
      deleteSelection();
    } else if (!removeEndOfLine()) {
      removeWholeText();
    }
    *textDidChange = true;
    return true;
  }

  // Handle special events with text
  char buffer[Ion::Events::EventData::k_maxDataSize] = {0};
  size_t eventTextLength =
      Ion::Events::copyText(static_cast<uint8_t>(event), buffer,
                            Ion::Events::EventData::k_maxDataSize);
  if (eventTextLength > 0) {
    bool didHandleEvent = privateHandleEventWithText(buffer);
    *textDidChange = didHandleEvent;
    return didHandleEvent;
  }

  return false;
}

void AbstractTextField::scrollToCursor() {
  if (!isEditing()) {
    return;
  }
  return TextInput::scrollToCursor();
}

bool AbstractTextField::handleMoveEvent(Ion::Events::Event event) {
  if (!isEditing()) {
    return false;
  }
  if (event == Ion::Events::Left || event == Ion::Events::Right) {
    if (!selectionIsEmpty()) {
      resetSelection();
      return true;
    }
    if (event == Ion::Events::Left && cursorLocation() > text()) {
      return TextInput::moveCursorLeft();
    }
    if (event == Ion::Events::Right && cursorLocation() < textEnd()) {
      return TextInput::moveCursorRight();
    }
  }
  return false;
}

bool AbstractTextField::handleSelectEvent(Ion::Events::Event event) {
  if (!isEditing()) {
    return false;
  }
  if (event.isSelectionEvent()) {
    selectLeftRight(
        (event == Ion::Events::ShiftLeft || event == Ion::Events::ShiftUp)
            ? OMG::Direction::Left()
            : OMG::Direction::Right(),
        event == Ion::Events::ShiftUp || event == Ion::Events::ShiftDown);
    return true;
  }
  return false;
}

bool AbstractTextField::privateHandleEventWithText(
    const char *eventText, bool indentation, bool forceCursorRightOfText) {
  assert(isEditable());
  if (!isEditing()) {
    setEditing(true);
    reinitTextBuffer();
    if (m_delegate) {
      m_delegate->textFieldDidStartEditing(this);
    }
  }

  assert(isEditing());

  // Delete the selected text if needed
  if (!selectionIsEmpty()) {
    deleteSelection();
  }

  if (eventText[0] != 0) {
    // Remove the Empty code points
    constexpr int bufferSize = AbstractTextField::MaxBufferSize();
    char buffer[bufferSize];
    {
      CodePoint c[] = {UCodePointEmpty, '\n'};
      bool complete = UTF8Helper::CopyAndRemoveCodePoints(buffer, bufferSize,
                                                          eventText, c, 2);
      /* If the text is too long to be stored in buffer, we do not insert any
       * text. This behaviour is consistent with 'insertTextAtLocation'
       * behaviour. */
      if (!complete) {
        return false;
      }
    }
    /* Replace System parentheses (used to keep layout tree structure) by normal
     * parentheses */
    Poincare::SerializationHelper::
        ReplaceSystemParenthesesAndBracesByUserParentheses(buffer);

    if (insertTextAtLocation(buffer, const_cast<char *>(cursorLocation()))) {
      /* The cursor position depends on the text as we sometimes want to
       * position the cursor at the end of the text and sometimes after the
       * first parenthesis. */
      const char *nextCursorLocation = cursorLocation();
      if (forceCursorRightOfText) {
        nextCursorLocation += strlen(buffer);
      } else {
        nextCursorLocation += TextInputHelpers::CursorIndexInCommand(eventText);
      }
      setCursorLocation(nextCursorLocation);
    }
  }
  return true;
}

bool AbstractTextField::handleEventWithText(const char *eventText,
                                            bool indentation,
                                            bool forceCursorRightOfText) {
  /* TODO: this method should not exist, we should only have
   * privateHandleEventWithText and always call handleEvent. */
  if (privateHandleEventWithText(eventText, indentation,
                                 forceCursorRightOfText)) {
    if (m_delegate) {
      m_delegate->textFieldDidHandleEvent(this);
    }
    return true;
  }
  return false;
}

void AbstractTextField::removeWholeText() {
  assert(isEditable());
  resetSelection();
  reinitTextBuffer();
  markWholeFrameAsDirty();
  layoutSubviews();
  resetScroll();
}

bool AbstractTextField::storeInClipboard() const {
  if (!isEditing()) {
    Clipboard::SharedClipboard()->store(text());
    return true;
  } else if (!selectionIsEmpty()) {
    const char *start = nonEditableContentView()->selectionLeft();
    Clipboard::SharedClipboard()->store(
        start, nonEditableContentView()->selectionRight() - start);
    return true;
  }
  return false;
}

bool AbstractTextField::handleStoreEvent() {
  if (!isEditing() && m_delegate && m_delegate->textFieldIsStorable(this)) {
    App::app()->storeValue(text());
  } else if (isEditing() && !selectionIsEmpty()) {
    const char *start = nonEditableContentView()->selectionLeft();
    static_assert(TextField::MaxBufferSize() ==
                  Escher::Clipboard::k_bufferSize);
    char buffer[Escher::Clipboard::k_bufferSize];
    strlcpy(buffer, start,
            nonEditableContentView()->selectionRight() - start + 1);
    App::app()->storeValue(buffer);
  } else {
    App::app()->storeValue();
  }
  return true;
}

}  // namespace Escher
