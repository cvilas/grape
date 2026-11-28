# Splat

Interactive viewer for 3D gaussian splat scenes and binary PLY point clouds.

![Demo](./docs/demo.png)

- [Primer on gaussian splatting](./docs/gaussian_splatting.md)
- [Viewer design](./docs/design.md)
- [Supported file formats](./docs/file_formats.md)
- [Sample scenes and point clouds](./models/README.md)

## Usage

```bash
grape_splat_view --file=modules/splat/models/hornedlizard.splat
grape_splat_view --file=modules/splat/models/sample-point-cloud-100k-binary.ply --up=y
```

Point clouds use the same soft, depth-sorted renderer as gaussian splats. Points are shown as
small, fixed-screen-size markers; see [file formats](./docs/file_formats.md) for supported PLY
properties and limitations.

Option | Default | Description
-------|---------|------------
`file` | -       | Scene file (`.splat`, Gaussian PLY, or point-cloud PLY)
`up`   | `-y`    | World axis shown as screen-up (`x`, `-x`, `y`, `-y`, `z`, `-z`). Captured gaussian scenes often use y-down (RDF); point clouds may use a different convention
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

View all frames the median distance from the median position, not the entire cloud; use dolly to
see more of a widely distributed scene.
