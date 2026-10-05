# File formats

All multi-byte values are little-endian.

## `.splat`

From [antimatter15/splat](https://github.com/antimatter15/splat). No header; the file is an array
of 32 byte records, one per gaussian. The number of gaussians is `file size / 32`.

Offset | Type        | Field     | Notes
-------|-------------|-----------|------
0      | `float32[3]`| position  | x, y, z
12     | `float32[3]`| scale     | Standard deviation along each local axis (linear, not log)
24     | `uint8[4]`  | rgba      | r, g, b: colour in [0, 255]. a: peak opacity, `255·sigmoid(opacity)`
28     | `uint8[4]`  | rotation  | Quaternion w, x, y, z, each encoded as `clamp(q·128 + 128, 0, 255)`

Colour is the view-independent (DC) component: `255·(0.5 + C0·f_dc)`, where
`C0 = 0.28209479177387814` is the zeroth order spherical harmonic basis constant.

There is no up-axis information. Files converted from 3DGS training output are y-down (RDF).

## `.ply` (3D gaussian splatting)

Output of the [reference implementation](https://github.com/graphdeco-inria/gaussian-splatting).
A standard [PLY](https://paulbourke.net/dataformats/ply/) file with `format binary_little_endian 1.0`
and a `vertex` element with (at least) the following properties:

Property                     | Meaning
-----------------------------|--------
`x`, `y`, `z`                | Position
`f_dc_0`, `f_dc_1`, `f_dc_2` | DC spherical harmonic coefficients for r, g, b
`opacity`                    | Opacity logit. Peak opacity is `sigmoid(opacity)`
`scale_0`, `scale_1`, `scale_2` | Log standard deviation along each local axis
`rot_0` .. `rot_3`           | Quaternion w, x, y, z (not necessarily normalised)

Other properties (normals, `f_rest_*` higher order coefficients, etc) are ignored. Properties may
be of any PLY scalar type, in any order. ASCII and big-endian PLY, and list properties in or
before the `vertex` element, are not supported.

## `.spz` (conversion only)

Niantic's compressed format ([nianticlabs/spz](https://github.com/nianticlabs/spz)) is not read
by the viewer. Convert version 2 files with `scripts/spz_to_splat.py`, which also converts from
SPZ's y-up (RUB) to y-down (RDF) coordinates.
