# Splat

Interactive viewer for 3D gaussian splat scenes.

![Demo](./docs/demo.png)

- [Primer on gaussian splatting](./docs/gaussian_splatting.md)
- [Viewer design](./docs/design.md)
- [Supported file formats](./docs/file_formats.md)
- [Sample scenes](./models/README.md)

## Usage

```bash
grape_splat_view --file=modules/experimental/splat/models/hornedlizard.splat
```

Option | Default | Description
-------|---------|------------
`file` | -       | Scene file (`.splat` or `.ply`)
`up`   | `-y`    | World axis shown as screen-up (`x`, `-x`, `y`, `-y`, `z`, `-z`). Most captured scenes use y-down (RDF) coordinates
`fov`  | `50`    | Initial vertical field of view (degrees, 1 to 150). View all places the camera to fit the subject, so its size on screen is the same at any FOV; only the perspective changes. Use `Ctrl` + scroll to zoom

## Controls

Navigation follows the [OpenInventor](https://github.com/coin3d/coin) examiner viewer. The camera
orbits a focal point at the centre of the view.

Input                                                           | Action
----------------------------------------------------------------|--------
Left drag                                                       | Rotate about the focal point (virtual trackball)
Release left button while moving                                | Keep spinning. Click to stop.
Middle drag, or Ctrl/Shift + left drag                          | Pan
Right drag, left + middle drag, Ctrl + middle drag, or Ctrl + Shift + left drag | Dolly (drag up to move closer)
Scroll wheel                                                    | Dolly
`Ctrl` + scroll wheel                                           | Zoom (change field of view without moving the camera)
`S`, then left click on the scene                               | Seek: re-centre on the clicked point, and move half way towards it
`Esc`                                                           | Cancel seek
`V`                                                             | View all
`H`                                                             | Go to home view
`Shift` + `H`                                                   | Set home view to the current view

