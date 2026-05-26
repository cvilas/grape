//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/plot/window.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <limits>
#include <print>
#include <random>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "font.h"
#include "grape/exception.h"

namespace {

//-------------------------------------------------------------------------------------------------
constexpr auto toSDLColor(const grape::plot::Color& col) -> SDL_Color {
  return { .r = col.r, .g = col.g, .b = col.b, .a = col.a };
};

//-------------------------------------------------------------------------------------------------
/// Generate visually pleasing random colors. Reference: docs/how_to_color.md
auto generateRandomColor() -> SDL_Color {
  constexpr auto GOLDEN_RATIO_CONJUGATE = 0.618033988749895;

  static std::random_device rd;
  static std::mt19937 mt(rd());
  static std::uniform_real_distribution<double> dist(0., 1.);
  static auto hue = dist(mt);

  hue = std::fmod(hue + GOLDEN_RATIO_CONJUGATE, 1.0);

  // HSV → RGB  (SAT = 0.5, VAL = 0.95 for a pastel palette)
  static constexpr auto SAT = 0.5;
  static constexpr auto VAL = 0.95;
  static constexpr auto HUE_SECTORS = 6.0;
  const auto hue_f = hue * HUE_SECTORS;
  const auto sector = static_cast<int>(hue_f);
  const auto frac = hue_f - sector;
  const auto pv = VAL * (1.0 - SAT);
  const auto qv = VAL * (1.0 - (SAT * frac));
  const auto tv = VAL * (1.0 - (SAT * (1.0 - frac)));
  auto rgb = SDL_Color{};
  rgb.a = SDL_ALPHA_OPAQUE;
  static constexpr auto COLOR_SCALE = 255.0;
  switch (sector % static_cast<int>(HUE_SECTORS)) {
    case 0:
      rgb.r = static_cast<uint8_t>(VAL * COLOR_SCALE);
      rgb.g = static_cast<uint8_t>(tv * COLOR_SCALE);
      rgb.b = static_cast<uint8_t>(pv * COLOR_SCALE);
      break;
    case 1:
      rgb.r = static_cast<uint8_t>(qv * COLOR_SCALE);
      rgb.g = static_cast<uint8_t>(VAL * COLOR_SCALE);
      rgb.b = static_cast<uint8_t>(pv * COLOR_SCALE);
      break;
    case 2:
      rgb.r = static_cast<uint8_t>(pv * COLOR_SCALE);
      rgb.g = static_cast<uint8_t>(VAL * COLOR_SCALE);
      rgb.b = static_cast<uint8_t>(tv * COLOR_SCALE);
      break;
    case 3:
      rgb.r = static_cast<uint8_t>(pv * COLOR_SCALE);
      rgb.g = static_cast<uint8_t>(qv * COLOR_SCALE);
      rgb.b = static_cast<uint8_t>(VAL * COLOR_SCALE);
      break;
    case 4:
      rgb.r = static_cast<uint8_t>(tv * COLOR_SCALE);
      rgb.g = static_cast<uint8_t>(pv * COLOR_SCALE);
      rgb.b = static_cast<uint8_t>(VAL * COLOR_SCALE);
      break;
    default:
      rgb.r = static_cast<uint8_t>(VAL * COLOR_SCALE);
      rgb.g = static_cast<uint8_t>(pv * COLOR_SCALE);
      rgb.b = static_cast<uint8_t>(qv * COLOR_SCALE);
      break;
  }
  return rgb;
}

//-------------------------------------------------------------------------------------------------
/// Compute a "nice" step size for grid lines given a data range and desired number of steps.
/// Uses 1-2-5 [preferred-number](https://en.wikipedia.org/wiki/Preferred_number) snapping per
/// decade. Ref: P. S. Heckbert, "Nice Numbers for Graph Labels," Graphics Gems, 1990.
auto niceStep(double range, int num_steps) -> double {
  if (range <= 0.0 || num_steps <= 0) {
    return 1.0;
  }
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  const auto rough = range / num_steps;
  const auto mag = std::pow(10.0, std::floor(std::log10(rough)));
  const auto norm_ratio = rough / mag;
  auto kk = 10.0;
  if (norm_ratio < 1.5) {
    kk = 1.0;
  } else if (norm_ratio < 3.5) {
    kk = 2.0;
  } else if (norm_ratio < 7.5) {
    kk = 5.0;
  }
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
  return kk * mag;
}

//-------------------------------------------------------------------------------------------------
/// Log an SDL/TTF error and return the original value when a bool-returning call fails.
auto sdlCheck(bool ok, const char* expr) -> bool {
  if (not ok) {
    grape::panic<grape::Exception>(std::format("{}: {}", expr, SDL_GetError()));
  }
  return ok;
}

//-------------------------------------------------------------------------------------------------
/// Log an SDL/TTF error and return the original pointer when a pointer-returning call fails.
template <typename T>
auto sdlCheck(T* ptr, const char* expr) -> T* {
  if (ptr == nullptr) {
    grape::panic<grape::Exception>(std::format("{}: {}", expr, SDL_GetError()));
  }
  return ptr;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SDL_CHECK(call) sdlCheck((call), #call)

using SDLWindowPtr = std::unique_ptr<SDL_Window, void (*)(SDL_Window*)>;
using SDLRendererPtr = std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer*)>;
using SDLTexturePtr = std::unique_ptr<SDL_Texture, void (*)(SDL_Texture*)>;
using TTFEnginePtr = std::unique_ptr<TTF_TextEngine, void (*)(TTF_TextEngine*)>;
using TTFFontPtr = std::unique_ptr<TTF_Font, void (*)(TTF_Font*)>;
using TTFTextPtr = std::unique_ptr<TTF_Text, void (*)(TTF_Text*)>;

}  // namespace

namespace grape::plot {

//=================================================================================================
struct Window::Impl {
  static constexpr std::size_t MAX_POINTS_PER_TRACE = 65'536;

  static constexpr auto BORDER_SZ = 20.F;        //!< padding around the window
  static constexpr auto MARGIN_SZ = 5.F;         //!< Space between things
  static constexpr auto DEFAULT_FONT_PT = 15.F;  //!< default font point size
  static constexpr auto TITLE_FONT_PT = 17.F;    //!< title font point size
  static constexpr auto TICK_FONT_PT = 13.F;     //!< tick font point size
  static constexpr auto TICK_LEN = 6.F;          //!< tick length
  static constexpr auto TICK_LABEL_LEN = 6U;     //!< Max length of y-axis tick labels in chars

  static constexpr auto GRID_EPSILON = 0.01;

  SDLWindowPtr window{ nullptr, SDL_DestroyWindow };
  SDLRendererPtr renderer{ nullptr, SDL_DestroyRenderer };
  TTFFontPtr default_font{ nullptr, TTF_CloseFont };
  TTFFontPtr tick_font{ nullptr, TTF_CloseFont };
  TTFFontPtr title_font{ nullptr, TTF_CloseFont };
  SDL_WindowID window_id = 0;
  bool open = true;

  std::vector<std::unique_ptr<Trace>> traces;
  std::vector<Trace::View> trace_views;  //!< Per-frame snapshot

  // Text-texture cache
  TTFEnginePtr text_engine{ nullptr, TTF_DestroyRendererTextEngine };
  TTFTextPtr title_text{ nullptr, TTF_DestroyText };
  TTFTextPtr x_label_text{ nullptr, TTF_DestroyText };
  TTFTextPtr y_label_text{ nullptr, TTF_DestroyText };

  SDL_FRect plot_area{};  // plot area layout

  // View state
  struct ViewState {
    double x_range_width = 0.0;  // 0 = show all; >0 = show last N x-units
    double vy_min = -1.0;
    double vy_max = 1.0;
    bool y_manual = false;
  };
  ViewState view{};
  double x_view_min = 0.0;  // derived each frame in updateView
  double x_view_max = 1.0;  // derived each frame in updateView

  // Zoom history (undo stack for scroll-wheel zoom-in)
  static constexpr auto MAX_ZOOM_HISTORY = std::size_t{ 64 };
  std::vector<ViewState> zoom_history;

  // Grid step cache (computed once per frame in updateView)
  double x_grid_step = 1.0;
  double y_grid_step = 1.0;
  double x_grid_first = 0.0;
  double y_grid_first = 0.0;

  // Screen-space transform (precomputed in updateView; screen = offset + data * scale)
  float sx_scale = 1.0F;
  float sx_offset = 0.0F;
  float sy_scale = -1.0F;
  float sy_offset = 0.0F;

  // Cached render output size (updated in recalcLayout)
  int render_w = 0;
  int render_h = 0;

  // Legend data
  struct Legend {
    bool enabled = true;
    int name_maxlen = 0;
    SDL_FRect rect{};
    SDL_FPoint norm{ .x = 0.85F, .y = 0.08F };  // default: top-right, as fraction of window size
    bool dragging{};
    SDL_FPoint drag_delta{};
    SDLTexturePtr texture{ nullptr, SDL_DestroyTexture };
    std::vector<TTFTextPtr> texts;
    std::vector<Color> colors;
  } legend{};

  // Y-axis pan state
  bool y_pan_dragging = false;
  float y_last_pan = 0.0F;

  // FPS calculations
  struct FPSCounter {
    bool show = false;
    Uint64 last_ms = 0;
    int frame_count = 0;
    float frame_rate = 0.0F;
  } fps{};

  // Scratch buffers reused across frames
  mutable std::vector<SDL_FPoint> pts_scratch;
  mutable std::vector<SDL_FPoint> step_pts_scratch;
  mutable TTFTextPtr tick_scratch{ nullptr, TTF_DestroyText };

  // Helpers
  void recalcLayout();
  void updateView();

  void drawGrid() const;
  void drawAxes() const;
  void drawTraces() const;
  void drawTicks() const;
  void drawAxisLabels() const;
  void drawLegend();
  void drawTitle() const;

  void rebuildTitleText(const std::string& text);
  void rebuildLabelText(AxisId id, const std::string& text);
  [[nodiscard]] auto rebuildLegend() -> bool;

  void renderTickText(const char* text, const SDL_FPoint& pos, const SDL_Color& col) const;
  void drawFps();
  void drawPointSymbol(PointStyle ps, const SDL_FPoint& pos, const SDL_Color& col) const;

  [[nodiscard]] auto dataToScreenX(double xd) const -> float;
  [[nodiscard]] auto dataToScreenY(double yd) const -> float;
  [[nodiscard]] auto screenToDataY(float sy) const -> double;
  [[nodiscard]] auto isInsidePlot(float mx, float my) const -> bool;
  [[nodiscard]] auto isLegendHit(float mx, float my) const -> bool;

  void onMouseButtonDown(const SDL_MouseButtonEvent& ev);
  void onMouseMotion(const SDL_MouseMotionEvent& ev);
  void onMouseWheel(const SDL_MouseWheelEvent& ev);
};

//-------------------------------------------------------------------------------------------------
void Window::Impl::recalcLayout() {
  int ww = 0;
  int wh = 0;
  SDL_CHECK(SDL_GetWindowSize(window.get(), &ww, &wh));
  static constexpr auto PLOT_X = BORDER_SZ + (TICK_LABEL_LEN * TICK_FONT_PT) + MARGIN_SZ + TICK_LEN;
  static constexpr auto PLOT_Y = BORDER_SZ + TITLE_FONT_PT + MARGIN_SZ;

  plot_area = {
    .x = PLOT_X,
    .y = PLOT_Y,
    .w = static_cast<float>(ww) - PLOT_X - BORDER_SZ,
    .h = static_cast<float>(wh) - PLOT_Y - BORDER_SZ - DEFAULT_FONT_PT - MARGIN_SZ - TICK_FONT_PT -
         MARGIN_SZ - TICK_LEN,
  };
  legend.rect.x = legend.norm.x * static_cast<float>(ww);
  legend.rect.y = legend.norm.y * static_cast<float>(wh);
  SDL_CHECK(SDL_GetRenderOutputSize(renderer.get(), &render_w, &render_h));
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::updateView() {
  auto dx_min = std::numeric_limits<double>::max();
  auto dx_max = std::numeric_limits<double>::lowest();

  for (const auto& tv : trace_views) {
    if (not tv.samples.empty()) {
      dx_min = std::min(dx_min, tv.samples.front().x);
      dx_max = std::max(dx_max, tv.samples.back().x);
    }
  }
  if (dx_min > dx_max) {
    dx_min = 0.0;
    dx_max = 1.0;
  }

  static constexpr auto VIEW_MIN_RANGE = 1e-6;

  // X view
  if (view.x_range_width > 0.0) {
    x_view_max = dx_max;
    x_view_min = dx_max - view.x_range_width;
  } else {
    x_view_min = dx_min;
    x_view_max = dx_max;
  }
  if (x_view_max - x_view_min < VIEW_MIN_RANGE) {
    x_view_min -= 1.0;
    x_view_max += 1.0;
  }

  // Y view (auto-scale from visible points)
  if (not view.y_manual) {
    view.vy_min = std::numeric_limits<double>::max();
    view.vy_max = std::numeric_limits<double>::lowest();
    for (const auto& tv : trace_views) {
      const auto begin = std::ranges::lower_bound(tv.samples, x_view_min, {}, &Sample::x);
      for (auto it = begin; it != tv.samples.end() && it->x <= x_view_max; ++it) {
        view.vy_min = std::min(view.vy_min, it->y);
        view.vy_max = std::max(view.vy_max, it->y);
      }
    }
    if (view.vy_min <= view.vy_max) {
      static constexpr auto Y_VIEW_PAD = 0.05;
      const double pad = (view.vy_max - view.vy_min) * Y_VIEW_PAD;
      view.vy_min -= pad;
      view.vy_max += pad;
    }
  }
  if (view.vy_max - view.vy_min < VIEW_MIN_RANGE) {
    static constexpr auto HALF = 0.5;
    const double mid = (view.vy_min + view.vy_max) * HALF;
    view.vy_min = mid - (VIEW_MIN_RANGE * HALF);
    view.vy_max = mid + (VIEW_MIN_RANGE * HALF);
  }

  // Cache grid step values shared by drawGrid() and drawAxisLabels()
  static constexpr auto X_GRID_DIVS = 6;
  static constexpr auto Y_GRID_DIVS = 5;
  x_grid_step = niceStep(x_view_max - x_view_min, X_GRID_DIVS);
  x_grid_first = std::ceil(x_view_min / x_grid_step) * x_grid_step;
  y_grid_step = niceStep(view.vy_max - view.vy_min, Y_GRID_DIVS);
  y_grid_first = std::ceil(view.vy_min / y_grid_step) * y_grid_step;

  // Precompute screen-space transform so dataToScreenX/Y are multiply-adds in the hot loop.
  sx_scale = static_cast<float>(static_cast<double>(plot_area.w) / (x_view_max - x_view_min));
  sx_offset = plot_area.x - (static_cast<float>(x_view_min) * sx_scale);
  sy_scale = -static_cast<float>(static_cast<double>(plot_area.h) / (view.vy_max - view.vy_min));
  sy_offset = (plot_area.y + plot_area.h) - (static_cast<float>(view.vy_min) * sy_scale);
}

//-------------------------------------------------------------------------------------------------
float Window::Impl::dataToScreenX(double xd) const {
  return sx_offset + (static_cast<float>(xd) * sx_scale);
}

//-------------------------------------------------------------------------------------------------
float Window::Impl::dataToScreenY(double yd) const {
  return sy_offset + (static_cast<float>(yd) * sy_scale);
}

//-------------------------------------------------------------------------------------------------
double Window::Impl::screenToDataY(float sy) const {
  const double norm =
      1.0 - (static_cast<double>(sy - plot_area.y) / static_cast<double>(plot_area.h));
  return view.vy_min + (norm * (view.vy_max - view.vy_min));
}

//-------------------------------------------------------------------------------------------------
auto Window::Impl::isInsidePlot(float mx, float my) const -> bool {
  const auto pt = SDL_FPoint{ .x = mx, .y = my };
  return SDL_PointInRectFloat(&pt, &plot_area);
}

//-------------------------------------------------------------------------------------------------
auto Window::Impl::isLegendHit(float mx, float my) const -> bool {
  const auto pt = SDL_FPoint{ .x = mx, .y = my };
  return SDL_PointInRectFloat(&pt, &legend.rect);
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::rebuildTitleText(const std::string& text) {
  static constexpr auto COLOR = SDL_Color{ .r = 240, .g = 240, .b = 240, .a = SDL_ALPHA_OPAQUE };
  title_text.reset();
  if (text_engine != nullptr) {
    title_text.reset(
        SDL_CHECK(TTF_CreateText(text_engine.get(), title_font.get(), text.c_str(), 0)));
    if (title_text != nullptr) {
      SDL_CHECK(TTF_SetTextColor(title_text.get(), COLOR.r, COLOR.g, COLOR.b, COLOR.a));
    }
  }
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::rebuildLabelText(AxisId axis_id, const std::string& text) {
  static constexpr auto COLOR = SDL_Color{ .r = 160, .g = 160, .b = 255, .a = SDL_ALPHA_OPAQUE };

  const auto rebuild = [&](TTFTextPtr& target) {
    target.reset();
    if (text_engine != nullptr) {
      target.reset(
          SDL_CHECK(TTF_CreateText(text_engine.get(), default_font.get(), text.c_str(), 0)));
      if (target != nullptr) {
        SDL_CHECK(TTF_SetTextColor(target.get(), COLOR.r, COLOR.g, COLOR.b, COLOR.a));
      }
    }
  };

  switch (axis_id) {
    case AxisId::AxisX:
      rebuild(x_label_text);
      break;
    case AxisId::AxisY:
      rebuild(y_label_text);
      break;
  }
}

//-------------------------------------------------------------------------------------------------
auto Window::Impl::rebuildLegend() -> bool {
  legend.texts.clear();
  legend.name_maxlen = 0;
  legend.texture.reset();
  legend.colors.clear();

  if (text_engine == nullptr) {
    return false;
  }

  const auto num_traces = trace_views.size();
  if (num_traces == 0) {
    return true;
  }

  // Build TTF_Text objects and measure the widest label
  static constexpr auto TX_COLOR = SDL_Color{ .r = 230, .g = 230, .b = 230, .a = SDL_ALPHA_OPAQUE };
  legend.texts.reserve(num_traces);
  for (const auto& tv : trace_views) {
    TTFTextPtr txt{ SDL_CHECK(TTF_CreateText(text_engine.get(), default_font.get(), tv.name.data(),
                                             tv.name.size())),
                    TTF_DestroyText };
    if (txt == nullptr) {
      return false;
    }
    SDL_CHECK(TTF_SetTextColor(txt.get(), TX_COLOR.r, TX_COLOR.g, TX_COLOR.b, TX_COLOR.a));
    int tw = 0;
    SDL_CHECK(TTF_GetTextSize(txt.get(), &tw, nullptr));
    legend.name_maxlen = std::max(legend.name_maxlen, tw);
    legend.texts.push_back(std::move(txt));
  }

  // Build the legend texture
  static constexpr auto SWATCH_LEN = 16.0F;
  const auto font_ht = static_cast<float>(TTF_GetFontHeight(default_font.get()));
  const auto tex_w = static_cast<int>(MARGIN_SZ + SWATCH_LEN + MARGIN_SZ +
                                      static_cast<float>(legend.name_maxlen) + MARGIN_SZ);
  const auto tex_h =
      static_cast<int>(MARGIN_SZ + (font_ht * static_cast<float>(num_traces)) + MARGIN_SZ);
  legend.rect.w = static_cast<float>(tex_w);
  legend.rect.h = static_cast<float>(tex_h);

  legend.texture.reset(SDL_CHECK(SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA8888,
                                                   SDL_TEXTUREACCESS_TARGET, tex_w, tex_h)));
  if (legend.texture == nullptr) {
    return false;
  }
  SDL_CHECK(SDL_SetTextureBlendMode(legend.texture.get(), SDL_BLENDMODE_BLEND));
  SDL_CHECK(SDL_SetRenderTarget(renderer.get(), legend.texture.get()));

  SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 0);
  SDL_RenderClear(renderer.get());
  const SDL_FRect full{
    .x = 0.F, .y = 0.F, .w = static_cast<float>(tex_w), .h = static_cast<float>(tex_h)
  };
  static constexpr SDL_Color BGD_COLOR{ .r = 18, .g = 18, .b = 18, .a = 216 };
  SDL_SetRenderDrawColor(renderer.get(), BGD_COLOR.r, BGD_COLOR.g, BGD_COLOR.b, BGD_COLOR.a);
  SDL_RenderFillRect(renderer.get(), &full);
  static constexpr SDL_Color BDR_COLOR{ .r = 160, .g = 160, .b = 160, .a = SDL_ALPHA_OPAQUE };
  SDL_SetRenderDrawColor(renderer.get(), BDR_COLOR.r, BDR_COLOR.g, BDR_COLOR.b, BDR_COLOR.a);
  SDL_RenderRect(renderer.get(), &full);

  for (auto ii = 0UZ; ii < num_traces; ++ii) {
    const auto& tv = trace_views.at(ii);
    const auto ry = MARGIN_SZ + (static_cast<float>(ii) * font_ht);
    const float mid = ry + (font_ht / 2.0F);
    const auto color = toSDLColor(tv.color);
    SDL_SetRenderDrawColor(renderer.get(), color.r, color.g, color.b, color.a);
    SDL_RenderLine(renderer.get(), MARGIN_SZ, mid, MARGIN_SZ + SWATCH_LEN, mid);
    if (auto* txt = legend.texts.at(ii).get(); txt != nullptr) {
      TTF_DrawRendererText(txt, MARGIN_SZ + SWATCH_LEN + MARGIN_SZ, ry);
    }
  }

  SDL_CHECK(SDL_SetRenderTarget(renderer.get(), nullptr));

  legend.colors.reserve(num_traces);
  for (const auto& tv : trace_views) {
    legend.colors.push_back(tv.color);
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::renderTickText(const char* text, const SDL_FPoint& pos,
                                  const SDL_Color& col) const {
  if (text_engine == nullptr) {
    return;
  }
  // Reuse a single TTF_Text object across all tick labels to avoid per-tick alloc/free.
  if (tick_scratch == nullptr) {
    tick_scratch.reset(SDL_CHECK(TTF_CreateText(text_engine.get(), tick_font.get(), text, 0)));
    if (tick_scratch == nullptr) {
      return;
    }
  } else if (!SDL_CHECK(TTF_SetTextString(tick_scratch.get(), text, 0))) {
    return;
  }
  TTF_SetTextColor(tick_scratch.get(), col.r, col.g, col.b, col.a);
  TTF_DrawRendererText(tick_scratch.get(), pos.x, pos.y);
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::drawPointSymbol(PointStyle ps, const SDL_FPoint& pos,
                                   const SDL_Color& col) const {
  if (ps == PointStyle::None) {
    return;
  }
  auto* rdr = renderer.get();
  SDL_SetRenderDrawColor(rdr, col.r, col.g, col.b, col.a);
  static constexpr auto EXTENT = 4.F;

  const auto draw_cross = [&]() {
    SDL_RenderLine(rdr, pos.x - EXTENT, pos.y - EXTENT, pos.x + EXTENT, pos.y + EXTENT);
    SDL_RenderLine(rdr, pos.x + EXTENT, pos.y - EXTENT, pos.x - EXTENT, pos.y + EXTENT);
  };

  const auto draw_plus = [&]() {
    SDL_RenderLine(rdr, pos.x - EXTENT, pos.y, pos.x + EXTENT, pos.y);
    SDL_RenderLine(rdr, pos.x, pos.y - EXTENT, pos.x, pos.y + EXTENT);
  };

  const auto draw_square = [&]() {
    const SDL_FRect rect{
      .x = pos.x - EXTENT, .y = pos.y - EXTENT, .w = 2.F * EXTENT, .h = 2.F * EXTENT
    };
    SDL_RenderRect(rdr, &rect);
  };

  switch (ps) {
    case PointStyle::None:
      break;  // unreachable; guarded above

    case PointStyle::Dot: {
      // Actually a tiny filled square. hope nobody notices!
      static constexpr auto DOT_SIZE = 3.0F;
      static constexpr auto HALF_DOT_SIZE = 0.5F * DOT_SIZE;
      const auto dot = SDL_FRect{
        .x = pos.x - HALF_DOT_SIZE, .y = pos.y - HALF_DOT_SIZE, .w = DOT_SIZE, .h = DOT_SIZE
      };
      SDL_RenderFillRect(rdr, &dot);
      break;
    }

    case PointStyle::Cross:
      draw_cross();
      break;

    case PointStyle::Plus:
      draw_plus();
      break;

    case PointStyle::Square:
      draw_square();
      break;

    case PointStyle::Diamond:
      SDL_RenderLine(rdr, pos.x, pos.y - EXTENT, pos.x + EXTENT, pos.y);
      SDL_RenderLine(rdr, pos.x + EXTENT, pos.y, pos.x, pos.y + EXTENT);
      SDL_RenderLine(rdr, pos.x, pos.y + EXTENT, pos.x - EXTENT, pos.y);
      SDL_RenderLine(rdr, pos.x - EXTENT, pos.y, pos.x, pos.y - EXTENT);
      break;

    case PointStyle::Triangle:
      SDL_RenderLine(rdr, pos.x, pos.y - EXTENT, pos.x + EXTENT, pos.y + EXTENT);
      SDL_RenderLine(rdr, pos.x + EXTENT, pos.y + EXTENT, pos.x - EXTENT, pos.y + EXTENT);
      SDL_RenderLine(rdr, pos.x - EXTENT, pos.y + EXTENT, pos.x, pos.y - EXTENT);
      break;

    case PointStyle::TriangleInverted:
      SDL_RenderLine(rdr, pos.x, pos.y + EXTENT, pos.x + EXTENT, pos.y - EXTENT);
      SDL_RenderLine(rdr, pos.x + EXTENT, pos.y - EXTENT, pos.x - EXTENT, pos.y - EXTENT);
      SDL_RenderLine(rdr, pos.x - EXTENT, pos.y - EXTENT, pos.x, pos.y + EXTENT);
      break;

    case PointStyle::Star: {
      static constexpr auto TRI_H = EXTENT * 0.866F;
      static constexpr auto TRI_R2 = EXTENT * 0.5F;
      SDL_RenderLine(rdr, pos.x, pos.y - EXTENT, pos.x + TRI_H, pos.y + TRI_R2);
      SDL_RenderLine(rdr, pos.x + TRI_H, pos.y + TRI_R2, pos.x - TRI_H, pos.y + TRI_R2);
      SDL_RenderLine(rdr, pos.x - TRI_H, pos.y + TRI_R2, pos.x, pos.y - EXTENT);
      SDL_RenderLine(rdr, pos.x, pos.y + EXTENT, pos.x + TRI_H, pos.y - TRI_R2);
      SDL_RenderLine(rdr, pos.x + TRI_H, pos.y - TRI_R2, pos.x - TRI_H, pos.y - TRI_R2);
      SDL_RenderLine(rdr, pos.x - TRI_H, pos.y - TRI_R2, pos.x, pos.y + EXTENT);
      break;
    }

    case PointStyle::CrossSquare:
      draw_cross();
      draw_square();
      break;

    case PointStyle::PlusSquare:
      draw_plus();
      draw_square();
      break;
  }
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::drawGrid() const {
  static constexpr auto COLOR = SDL_Color{ .r = 48, .g = 48, .b = 48, .a = SDL_ALPHA_OPAQUE };
  auto* rdr = renderer.get();
  SDL_SetRenderDrawColor(rdr, COLOR.r, COLOR.g, COLOR.b, COLOR.a);
  for (int step = 0;; ++step) {
    const auto gx = x_grid_first + (static_cast<double>(step) * x_grid_step);
    if (gx > x_view_max + (x_grid_step * GRID_EPSILON)) {
      break;
    }
    SDL_RenderLine(rdr, dataToScreenX(gx), plot_area.y, dataToScreenX(gx),
                   plot_area.y + plot_area.h);
  }
  for (int step = 0;; ++step) {
    const auto gy = y_grid_first + (static_cast<double>(step) * y_grid_step);
    if (gy > view.vy_max + (y_grid_step * GRID_EPSILON)) {
      break;
    }
    SDL_RenderLine(rdr, plot_area.x, dataToScreenY(gy), plot_area.x + plot_area.w,
                   dataToScreenY(gy));
  }
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::drawAxes() const {
  static constexpr auto COLOR = SDL_Color{ .r = 200, .g = 200, .b = 200, .a = SDL_ALPHA_OPAQUE };
  auto* rdr = renderer.get();
  SDL_SetRenderDrawColor(rdr, COLOR.r, COLOR.g, COLOR.b, COLOR.a);
  const std::array<SDL_FPoint, 3> pts{
    SDL_FPoint{ .x = plot_area.x, .y = plot_area.y },
    SDL_FPoint{ .x = plot_area.x, .y = plot_area.y + plot_area.h },
    SDL_FPoint{ .x = plot_area.x + plot_area.w, .y = plot_area.y + plot_area.h }
  };
  SDL_RenderLines(rdr, pts.data(), static_cast<int>(pts.size()));
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::drawTraces() const {
  auto* rdr = renderer.get();
  auto& pts = pts_scratch;
  auto& step_pts = step_pts_scratch;
  for (const auto& tv : trace_views) {
    if (tv.samples.empty()) {
      continue;
    }

    const auto color = toSDLColor(tv.color);
    SDL_SetRenderDrawColor(rdr, color.r, color.g, color.b, color.a);

    pts.clear();
    pts.reserve(tv.samples.size());

    static constexpr auto CLIP_PAD = 0.005;
    const auto x_pad = (x_view_max - x_view_min) * CLIP_PAD;

    for (const auto& sample : tv.samples) {
      if (sample.x < x_view_min - x_pad || sample.x > x_view_max + x_pad) {
        continue;
      }
      pts.push_back({ .x = dataToScreenX(sample.x), .y = dataToScreenY(sample.y) });
    }
    if (pts.empty()) {
      continue;
    }

    switch (tv.line_style) {
      case LineStyle::Line:
        SDL_RenderLines(rdr, pts.data(), static_cast<int>(pts.size()));
        break;

      case LineStyle::Step: {
        step_pts.clear();
        step_pts.reserve((pts.size() * 2) - 1);
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        step_pts.push_back(pts[0]);
        for (std::size_t i = 1; i < pts.size(); ++i) {
          step_pts.push_back({ .x = pts[i].x, .y = pts[i - 1].y });
          step_pts.push_back(pts[i]);
        }
        // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        SDL_RenderLines(rdr, step_pts.data(), static_cast<int>(step_pts.size()));
        break;
      }

      case LineStyle::Lollipop: {
        const float base = std::clamp(dataToScreenY(0.0), plot_area.y, plot_area.y + plot_area.h);
        for (const auto& pp : pts) {
          SDL_RenderLine(rdr, pp.x, base, pp.x, pp.y);
        }
        break;
      }

      case LineStyle::None:
        break;
    }

    if (tv.point_style != PointStyle::None) {
      for (const auto& pp : pts) {
        drawPointSymbol(tv.point_style, pp, color);
      }
    }
  }
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::drawTicks() const {
  static constexpr SDL_Color TICK_COLOR{ .r = 180, .g = 180, .b = 180, .a = SDL_ALPHA_OPAQUE };
  static constexpr auto LBL_BUF_SZ = TICK_LABEL_LEN + 10U;
  static constexpr auto HALF = 0.5F;
  auto* rdr = renderer.get();
  auto* tfont = tick_font.get();

  // Y ticks
  SDL_SetRenderDrawColor(rdr, TICK_COLOR.r, TICK_COLOR.g, TICK_COLOR.b, TICK_COLOR.a);
  for (int step = 0;; ++step) {
    const auto gy = y_grid_first + (static_cast<double>(step) * y_grid_step);
    if (gy > view.vy_max + (y_grid_step * GRID_EPSILON)) {
      break;
    }
    const auto norm = static_cast<float>((gy - view.vy_min) / (view.vy_max - view.vy_min));
    if (norm < 0.0F || norm > 1.0F) {
      continue;
    }
    const auto sy = plot_area.y + plot_area.h - (norm * plot_area.h);
    SDL_RenderLine(rdr, plot_area.x - TICK_LEN, sy, plot_area.x, sy);
    auto lbl = std::array<char, LBL_BUF_SZ>{};
    std::vformat_to(lbl.begin(), "{:.{}g}", std::make_format_args(gy, TICK_LABEL_LEN));
    int tw = 0;
    int th = 0;
    TTF_GetStringSize(tfont, lbl.data(), 0, &tw, &th);
    const SDL_FPoint pos = { .x = plot_area.x - TICK_LEN - static_cast<float>(tw) - MARGIN_SZ,
                             .y = sy - (HALF * static_cast<float>(th)) };
    renderTickText(lbl.data(), pos, TICK_COLOR);
  }

  // X ticks
  const auto base_y = plot_area.y + plot_area.h;
  SDL_SetRenderDrawColor(rdr, TICK_COLOR.r, TICK_COLOR.g, TICK_COLOR.b, TICK_COLOR.a);
  for (int step = 0;; ++step) {
    const auto gx = x_grid_first + (static_cast<double>(step) * x_grid_step);
    if (gx > x_view_max + (x_grid_step * GRID_EPSILON)) {
      break;
    }
    const auto sx = dataToScreenX(gx);
    if (sx < plot_area.x || sx > plot_area.x + plot_area.w) {
      continue;
    }
    SDL_RenderLine(rdr, sx, base_y, sx, base_y + TICK_LEN);
    auto lbl = std::array<char, LBL_BUF_SZ>{};
    std::vformat_to(lbl.begin(), "{:.{}g}", std::make_format_args(gx, TICK_LABEL_LEN));
    int tw = 0;
    int th = 0;
    TTF_GetStringSize(tfont, lbl.data(), 0, &tw, &th);
    const SDL_FPoint pos = { .x = sx - (HALF * static_cast<float>(tw)),
                             .y = base_y + TICK_LEN + MARGIN_SZ };
    renderTickText(lbl.data(), pos, TICK_COLOR);
  }
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::drawAxisLabels() const {
  if (auto* txt = x_label_text.get(); txt != nullptr) {
    int tw = 0;
    int th = 0;
    TTF_GetTextSize(txt, &tw, &th);
    TTF_DrawRendererText(txt, plot_area.x + plot_area.w - static_cast<float>(tw),
                         static_cast<float>(render_h) - BORDER_SZ - static_cast<float>(th));
  }
  if (auto* txt = y_label_text.get(); txt != nullptr) {
    int tw = 0;
    int th = 0;
    TTF_GetTextSize(txt, &tw, &th);
    TTF_DrawRendererText(txt, plot_area.x - static_cast<float>(tw) - MARGIN_SZ,
                         plot_area.y - static_cast<float>(th));
  }
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::drawLegend() {
  if (not legend.enabled) {
    return;
  }

  const auto num_traces = trace_views.size();
  if (num_traces == 0) {
    return;
  }

  const auto color_eq = [](const Color& lhs, const Color& rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
  };
  const bool need_rebuild =
      (legend.texture == nullptr) || (legend.texts.size() != num_traces) ||
      (legend.colors.size() != num_traces) ||
      !std::equal(
          trace_views.begin(), trace_views.end(), legend.colors.begin(),
          [&](const Trace::View& tv, const Color& cached) { return color_eq(tv.color, cached); });

  if (need_rebuild && not rebuildLegend()) {
    return;
  }

  if (legend.texture == nullptr) {
    return;
  }

  const auto dst = SDL_FRect{
    .x = legend.rect.x,
    .y = legend.rect.y,
    .w = legend.rect.w,
    .h = legend.rect.h,
  };
  SDL_CHECK(SDL_RenderTexture(renderer.get(), legend.texture.get(), nullptr, &dst));
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::drawTitle() const {
  auto* txt = title_text.get();
  if (txt == nullptr) {
    return;
  }
  static constexpr auto HALF = 0.5F;
  int tw = 0;
  int th = 0;
  TTF_GetTextSize(txt, &tw, &th);
  TTF_DrawRendererText(txt, plot_area.x + (HALF * (plot_area.w - static_cast<float>(tw))),
                       BORDER_SZ);
}

//=================================================================================================
Window::Window(int width, int height, const std::string& title) : d_(std::make_unique<Impl>()) {
  SDL_CHECK(SDL_InitSubSystem(SDL_INIT_VIDEO));
  SDL_CHECK(TTF_Init());

  d_->window.reset(SDL_CHECK(SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_RESIZABLE)));

  const auto num_supported_renderers = SDL_GetNumRenderDrivers();
  auto supported_renderers = std::vector<std::string_view>{};
  for (int i = 0; i < num_supported_renderers; ++i) {
    supported_renderers.emplace_back(SDL_GetRenderDriver(i));
  }
  std::println(stderr, "Supported renderers: {}", supported_renderers);

  // Choose an available renderer in order of preferance, otherwise fallback to a default
  static constexpr auto PREFERRED_RENDERERS = { "gpu", "opengl", "opengles2" };
  for (const auto& renderer_name : PREFERRED_RENDERERS) {
    d_->renderer.reset(SDL_CHECK(SDL_CreateRenderer(d_->window.get(), renderer_name)));
    if (d_->renderer != nullptr) {
      break;
    }
  }
  if (d_->renderer == nullptr) {
    d_->renderer.reset(SDL_CHECK(SDL_CreateRenderer(d_->window.get(), nullptr)));
  }
  std::println(stderr, "Using renderer: {}", SDL_GetRendererName(d_->renderer.get()));

  d_->text_engine.reset(SDL_CHECK(TTF_CreateRendererTextEngine(d_->renderer.get())));
  if (!SDL_SetRenderVSync(d_->renderer.get(), SDL_RENDERER_VSYNC_ADAPTIVE)) {  // adaptive VSync
                                                                               // optional
    SDL_SetRenderVSync(d_->renderer.get(), 1);
  }
  d_->window_id = SDL_GetWindowID(d_->window.get());
  if (d_->window_id == 0) {
    std::println(stderr, "SDL_GetWindowID: {}", SDL_GetError());
  }

  // Load fonts
  const auto font_bytes = fontData();
  const auto open_font = [&](float pt) -> TTFFontPtr {
    auto* io = SDL_CHECK(SDL_IOFromConstMem(font_bytes.data(), font_bytes.size()));
    return { (io != nullptr) ? SDL_CHECK(TTF_OpenFontIO(io, /*closeio=*/true, pt)) : nullptr,
             TTF_CloseFont };
  };
  d_->default_font = open_font(Impl::DEFAULT_FONT_PT);
  d_->tick_font = open_font(Impl::TICK_FONT_PT);
  d_->title_font = open_font(Impl::TITLE_FONT_PT);
  if (d_->title_font != nullptr) {
    TTF_SetFontStyle(d_->title_font.get(), TTF_STYLE_BOLD);
  }
  d_->rebuildLabelText(AxisId::AxisX, "X");
  d_->rebuildLabelText(AxisId::AxisY, "Y");
  d_->rebuildTitleText(title);
  d_->recalcLayout();
  d_->zoom_history.reserve(Impl::MAX_ZOOM_HISTORY);
}

//-------------------------------------------------------------------------------------------------
Window::~Window() {
  d_.reset();
  TTF_Quit();
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

//-------------------------------------------------------------------------------------------------
void Window::setTitleText(const std::string& title) {
  d_->rebuildTitleText(title);
  SDL_CHECK(SDL_SetWindowTitle(d_->window.get(), title.c_str()));
}

//-------------------------------------------------------------------------------------------------
void Window::setAxisText(AxisId id, const std::string& label) {
  d_->rebuildLabelText(id, label);
}

//-------------------------------------------------------------------------------------------------
void Window::enableLegend(bool on) {
  d_->legend.enabled = on;
}

//-------------------------------------------------------------------------------------------------
auto Window::isLegendEnabled() const -> bool {
  return d_->legend.enabled;
}

//-------------------------------------------------------------------------------------------------
bool Window::isOpen() const {
  return d_->open;
}

//-------------------------------------------------------------------------------------------------
auto Window::trace(const std::string& name) -> Trace& {
  for (auto& tr : d_->traces) {
    if (tr->name() == name) {
      return *tr;
    }
  }

  auto trace = std::unique_ptr<Trace>(new Trace(name, Impl::MAX_POINTS_PER_TRACE));
  const auto color = generateRandomColor();
  trace->setColor(Color{ .r = color.r, .g = color.g, .b = color.b, .a = SDL_ALPHA_OPAQUE });
  return *(d_->traces.emplace_back(std::move(trace)));
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::onMouseButtonDown(const SDL_MouseButtonEvent& ev) {
  if (ev.button != SDL_BUTTON_LEFT) {
    return;
  }
  const float mx = ev.x;
  const float my = ev.y;
  if (legend.enabled && isLegendHit(mx, my)) {
    legend.dragging = true;
    legend.drag_delta = { .x = mx - legend.rect.x, .y = my - legend.rect.y };
  } else if (ev.clicks == 2 && isInsidePlot(mx, my)) {
    view.x_range_width = 0.0;
    view.y_manual = false;
    zoom_history.clear();
  } else if (isInsidePlot(mx, my)) {
    y_pan_dragging = true;
    y_last_pan = my;
  }
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::onMouseMotion(const SDL_MouseMotionEvent& ev) {
  if (legend.dragging) {
    legend.rect.x = ev.x - legend.drag_delta.x;
    legend.rect.y = ev.y - legend.drag_delta.y;
    int win_w = 0;
    int win_h = 0;
    SDL_GetWindowSize(window.get(), &win_w, &win_h);
    legend.norm.x = legend.rect.x / static_cast<float>(win_w);
    legend.norm.y = legend.rect.y / static_cast<float>(win_h);
    return;
  }
  if (y_pan_dragging) {
    const float dy = y_last_pan - ev.y;
    if (dy != 0.0F) {
      const double shift =
          -static_cast<double>(dy) * (view.vy_max - view.vy_min) / static_cast<double>(plot_area.h);
      view.vy_min += shift;
      view.vy_max += shift;
      view.y_manual = true;
    }
    y_last_pan = ev.y;
  }
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::onMouseWheel(const SDL_MouseWheelEvent& ev) {
  float mx = 0.F;
  float my = 0.F;
  SDL_GetMouseState(&mx, &my);
  if (!isInsidePlot(mx, my)) {
    return;
  }

  const bool ctrl = (SDL_GetModState() & SDL_KMOD_CTRL) != 0;
  static constexpr auto ZOOM_IN = 0.80;

  if (ctrl) {
    // Y zoom: scale range around the mouse pivot (works in both directions)
    const double pivot = screenToDataY(my);
    const double cur_rng = view.vy_max - view.vy_min;
    const double factor = (ev.y > 0) ? ZOOM_IN : (1.0 / ZOOM_IN);
    const double new_rng = cur_rng * factor;
    const double lo_frac = (pivot - view.vy_min) / cur_rng;
    view.vy_min = pivot - (lo_frac * new_rng);
    view.vy_max = view.vy_min + new_rng;
    view.y_manual = true;
  } else if (ev.y > 0) {
    // X zoom in — snapshot current state first, then apply (capped to avoid unbounded growth)
    if (zoom_history.size() < MAX_ZOOM_HISTORY) {
      zoom_history.push_back(view);
    }
    const double total_x = x_view_max - x_view_min;
    const double cur = (view.x_range_width > 0.0) ? view.x_range_width : total_x;
    static constexpr auto MIN_ZOOM_FRAC = 0.01;
    view.x_range_width = std::max(cur * ZOOM_IN, total_x * MIN_ZOOM_FRAC);
  } else if (ev.y < 0) {
    // X zoom out — restore the previous zoom-in step
    if (!zoom_history.empty()) {
      view = zoom_history.back();
      zoom_history.pop_back();
    }
  }
}

//-------------------------------------------------------------------------------------------------
auto Window::processEvents() -> bool {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_QUIT:
        d_->open = false;
        break;

      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        if (event.window.windowID == d_->window_id) {
          d_->open = false;
        }
        break;

      case SDL_EVENT_WINDOW_RESIZED:
        if (event.window.windowID == d_->window_id) {
          d_->recalcLayout();
        }
        break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        d_->onMouseButtonDown(event.button);
        break;

      case SDL_EVENT_MOUSE_MOTION:
        d_->onMouseMotion(event.motion);
        break;

      case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event.button.button == SDL_BUTTON_LEFT) {
          d_->legend.dragging = false;
          d_->y_pan_dragging = false;
        }
        break;

      case SDL_EVENT_MOUSE_WHEEL:
        d_->onMouseWheel(event.wheel);
        break;

      case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_F) {
          d_->fps.show = !d_->fps.show;
        }
        break;

      default:
        break;
    }
  }
  return d_->open;
}

//-------------------------------------------------------------------------------------------------
void Window::render() {
  static constexpr auto BG_COLOR = SDL_Color{ .r = 10, .g = 10, .b = 14, .a = SDL_ALPHA_OPAQUE };

  d_->trace_views.clear();
  for (auto& tr : d_->traces) {
    d_->trace_views.push_back(tr->snapshot());
  }

  d_->updateView();
  auto* rdr = d_->renderer.get();
  SDL_SetRenderDrawColor(rdr, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
  SDL_RenderClear(rdr);

  d_->drawTitle();
  d_->drawGrid();
  d_->drawAxes();
  d_->drawTraces();
  d_->drawTicks();
  d_->drawAxisLabels();
  d_->drawLegend();
  if (d_->fps.show) {
    d_->drawFps();
  }

  SDL_RenderPresent(rdr);
}

//-------------------------------------------------------------------------------------------------
void Window::Impl::drawFps() {
  const auto now = SDL_GetTicks();
  ++fps.frame_count;
  if (fps.last_ms == 0U) {
    fps.last_ms = now;
  }
  static constexpr auto UPDATE_INTERVAL_MS = Uint64{ 500 };
  static constexpr auto MILLI_PER_SEC = 1000.0F;
  const auto elapsed = now - fps.last_ms;
  if (elapsed >= UPDATE_INTERVAL_MS) {
    fps.frame_rate =
        static_cast<float>(fps.frame_count) * MILLI_PER_SEC / static_cast<float>(elapsed);
    fps.frame_count = 0;
    fps.last_ms = now;
  }

  static constexpr auto COLOR = SDL_Color{ .r = 0xFF, .g = 0xFF, .b = 0xFF, .a = SDL_ALPHA_OPAQUE };
  static constexpr auto TEXT_BUF_SIZE = 32;
  auto buf = std::array<char, TEXT_BUF_SIZE>{};
  std::format_to_n(buf.begin(), buf.size() - 1, "FPS: {:.1f}", static_cast<double>(fps.frame_rate));
  SDL_SetRenderDrawColor(renderer.get(), COLOR.r, COLOR.g, COLOR.b, COLOR.a);
  SDL_RenderDebugText(renderer.get(), MARGIN_SZ, MARGIN_SZ, buf.data());
}

}  // namespace grape::plot
