# Sample models

Freely licensed gaussian splat scenes and point clouds for use with `grape_splat_view`.

File                 | Gaussians / points | Size  | Recommended `--up` | Source | Licence
---------------------|-----------|-------|--------------------|--------|--------
`hornedlizard.splat` | 786,233   | 25 MB | `-y` (default)     | [nianticlabs/spz](https://github.com/nianticlabs/spz) `samples/hornedlizard.spz` | MIT ([LICENSE_NIANTIC_SPZ](./LICENSE_NIANTIC_SPZ))
`racoonfamily.splat` | 932,560   | 30 MB | `-y` (default)     | [nianticlabs/spz](https://github.com/nianticlabs/spz) `samples/racoonfamily.spz` | MIT ([LICENSE_NIANTIC_SPZ](./LICENSE_NIANTIC_SPZ))
`f3d_small.splat`    | 52,293    | 1.6 MB| `-y` (default)     | [f3d-app/f3d](https://github.com/f3d-app/f3d) `testing/data/small.splat` | BSD-3-Clause ([LICENSE_F3D.md](./LICENSE_F3D.md))
`sample-point-cloud-100k-binary.ply` | 100,000 | 2.7 MB | `y` | [FreeToolsForWeb](https://freetoolsforweb.com/sample-files/3d/ply/2-7-mb) | [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/)

Example:

```bash
grape_splat_view --file=modules/splat/models/racoonfamily.splat
grape_splat_view --file=modules/splat/models/sample-point-cloud-100k-binary.ply --up=y
```

## Provenance

- `f3d_small.splat`: Unmodified copy.
- `sample-point-cloud-100k-binary.ply`: Unmodified generated torus-knot point cloud; the file
  header identifies it as CC0.
- `hornedlizard.splat`, `racoonfamily.splat`: Converted from SPZ v2 with
  `../scripts/spz_to_splat.py <name>.spz <name>.splat`. The conversion drops higher order
  spherical harmonics (view dependent colour) and rotates the scene from SPZ's y-up (RUB) to
  y-down (RDF) coordinates to match the `.splat` convention.

## Finding more scenes

Scenes in `.splat` or binary little-endian `.ply` format can be viewed directly. Check the licence
before adding any to this folder; many published captures are for non-commercial use only.
