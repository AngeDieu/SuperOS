#include <escher/palette.h>
#include <escher/unequal_view.h>

namespace Escher {

constexpr uint8_t unequalMask[UnequalView::k_size][UnequalView::k_size] = {
    {0xff, 0xf6, 0x7e, 0x23, 0x06, 0x23, 0x7e, 0xf6, 0xff},
    {0xf5, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3a, 0xf5},
    {0x7d, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x7f},
    {0x23, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x22},
    {0x06, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x06},
    {0x23, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x21},
    {0x7c, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7e},
    {0xf5, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3a, 0xf5},
    {0xff, 0xf6, 0x7e, 0x23, 0x06, 0x23, 0x7e, 0xf6, 0xff},
};

void UnequalView::drawRect(KDContext* ctx, KDRect rect) const {
  if (!m_state) {
    return;
  }
  /* Draw the symbol aligned on the right of the view and vertically centered.
   */
  KDRect frame(bounds().width() - k_size, bounds().height() / 2 - k_size / 2,
               k_size, k_size);
  KDColor buffer[k_size * k_size];
  ctx->blendRectWithMask(frame, Palette::GrayDark, &unequalMask[0][0], buffer);
}

}  // namespace Escher