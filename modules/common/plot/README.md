# plot

2D time-series plotting

## Design considerations

- Allow a range of point and line styles
- Show/hide _bugs_ (visual markers)
- Show/hide legend, and its position
- Visually simple layout
  - Dark background
  - Monospace white text annotations and labels
  - Auto initialised visually pleasing trace colors 
- Functional but minimal interactive controls
  - Pointer left click + move up/down: pan vertically
  - Scroll wheel: zoom along X axis, with undo support
  - Ctrl + Scroll: zoom in and out Y axis, centered at the cursor
  - Double click: resets view
  - Draggable legend
  - 'f' key toggles FPS diagnostics

## TODO

- [ ] Add `Window::createTrace(name, capacity, color, line_style, point_style)`
- [ ] Refactor `Window::trace(name) -> std::optional<Trace&>`
- [ ] Highlight selected trace by reducing opacity of all others
- [ ] Set/hide visual bug

## Plot Window Layout

Constant          | Role
------------------|------
`BORDER_SZ`       | Outer padding on every edge 
`MARGIN_SZ`       | Gap between adjacent elements 
`TITLE_FONT_PT`   | Title text height 
`DEFAULT_FONT_PT` | Axis-label and legend text height |
`TICK_FONT_PT`    | Tick-label text height |
`TICK_LEN`        | Tick mark length (both axes) |
`TICK_LABEL_LEN`  | Max significant digits in a tick label |

![Layout](./docs/window_layout.drawio.svg)