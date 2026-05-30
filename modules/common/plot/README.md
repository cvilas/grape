# Plot

2D time-series plotting

## Design considerations

- Support a range of point and line styles
- Efficient in memory, CPU and GPU usage
- Able to update at display frame rates
- Show/hide legend
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

![Demo](./docs/demo.png)

## Implementation

The renderer follows these rules to be CPU-, GPU- and memory-efficient:

- Minimises draw calls per frame. (One large draw is cheaper than many small ones)
- Keeps data buffers contiguous and pre-allocated for cache coherence
- Uses lock-free, wait-free queues to marshall data from frontend to rendering backend
- Decimates visual data to screen resolution; i.e., does't submit more vertices than there are 
  pixels. Uses min-max instead of stride to preserve peaks.
- Caches static text as textures for reuse

### Plot Window Layout

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

