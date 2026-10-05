#!/usr/bin/env -S uv run --script
#
# Copyright (C) 2026 GRAPE Contributors
#
# /// script
# requires-python = ">=3.10"
# dependencies = ["numpy"]
# ///

"""
Convert a Niantic SPZ (version 2) gaussian splat file to the antimatter15 .splat format read by
grape_splat_view.

SPZ stores scenes in RUB (x right, y up, z back) coordinates. By default, output is converted to
RDF (x right, y down, z forward), the convention of 3DGS training output and most .splat files.
View the result with `grape_splat_view --up=-y` (the default).

Only zeroth order (view independent) colour is converted. Higher order spherical harmonics are
dropped.

Refs:
- SPZ format: https://github.com/nianticlabs/spz (src/cc/load-spz.cc)
- .splat format: https://github.com/antimatter15/splat
- See also ../docs/file_formats.md

Usage: spz_to_splat.py input.spz output.splat [--keep-rub]
"""

import argparse
import gzip
import struct
import sys

import numpy as np

SPZ_MAGIC = 0x5053474E  # 'NGSP'
SPZ_HEADER = struct.Struct("<IIIBBBB")
SH_C0 = 0.28209479177387814
SPZ_COLOR_SCALE = 0.15


def load_spz(path):
    """
    Decode an SPZ v2 file
    :param path: Path to .spz file
    :return: Dictionary of numpy arrays: position (N,3), scale (N,3), rotation (N,4; w,x,y,z),
             rgba (N,4; uint8)
    """
    raw = gzip.open(path).read()
    magic, version, count, _sh_degree, fractional_bits, _flags, _reserved = (
        SPZ_HEADER.unpack_from(raw)
    )
    if magic != SPZ_MAGIC:
        raise ValueError(f"{path}: Not an SPZ file")
    if version != 2:
        raise ValueError(f"{path}: SPZ version {version} is not supported (only version 2)")

    offset = SPZ_HEADER.size

    def take(nbytes_per_point, cols):
        nonlocal offset
        size = count * nbytes_per_point
        if offset + size > len(raw):
            raise ValueError(f"{path}: File is truncated")
        data = np.frombuffer(raw, np.uint8, size, offset).reshape(count, cols)
        offset += size
        return data

    # Positions: 24-bit signed fixed point per component
    pos = take(9, 9).reshape(count, 3, 3).astype(np.int32)
    fixed = pos[:, :, 0] | (pos[:, :, 1] << 8) | (pos[:, :, 2] << 16)
    fixed = np.where(fixed & 0x800000, fixed - (1 << 24), fixed)
    position = fixed.astype(np.float32) / float(1 << fractional_bits)

    # Opacity: sigmoid(opacity) quantised to 8 bits. Identical to .splat alpha.
    alpha = take(1, 1)[:, 0]

    # Colour: DC spherical harmonic coefficient, scaled and offset
    dc = (take(3, 3).astype(np.float32) / 255.0 - 0.5) / SPZ_COLOR_SCALE
    rgb = np.clip(np.round((0.5 + SH_C0 * dc) * 255.0), 0, 255).astype(np.uint8)

    # Scale: log(scale) quantised with 4 fractional bits, offset by -10
    scale = np.exp(take(3, 3).astype(np.float32) / 16.0 - 10.0)

    # Rotation: quaternion x, y, z quantised to 8 bits. w >= 0 is implied.
    xyz = take(3, 3).astype(np.float32) / 127.5 - 1.0
    w = np.sqrt(np.maximum(0.0, 1.0 - np.sum(xyz * xyz, axis=1)))
    rotation = np.column_stack([w, xyz])

    return {
        "position": position,
        "scale": scale,
        "rotation": rotation,
        "rgba": np.column_stack([rgb, alpha]),
    }


def rub_to_rdf(scene):
    """
    Rotate scene by 180 degrees about x: (x, y, z) -> (x, -y, -z)
    """
    scene["position"] = scene["position"] * np.array([1.0, -1.0, -1.0], dtype=np.float32)
    # q' = r * q, where r = (0, 1, 0, 0) is the 180 degree rotation about x. Sign is irrelevant.
    w, x, y, z = scene["rotation"].T
    scene["rotation"] = np.column_stack([-x, w, -z, y])
    return scene


def save_splat(path, scene):
    """
    Write scene as antimatter15 .splat: 32 byte records of
    float32 position[3], float32 scale[3], uint8 rgba[4], uint8 rotation[4] (w, x, y, z)
    """
    count = scene["position"].shape[0]
    record = np.dtype(
        [("position", "<f4", 3), ("scale", "<f4", 3), ("rgba", "u1", 4), ("rotation", "u1", 4)]
    )
    rot = scene["rotation"]
    rot = rot / np.maximum(np.linalg.norm(rot, axis=1, keepdims=True), 1e-12)
    out = np.empty(count, dtype=record)
    out["position"] = scene["position"]
    out["scale"] = scene["scale"]
    out["rgba"] = scene["rgba"]
    out["rotation"] = np.clip(np.round(rot * 128.0 + 128.0), 0, 255).astype(np.uint8)
    out.tofile(path)


def main():
    parser = argparse.ArgumentParser(description="Convert SPZ v2 gaussian splats to .splat")
    parser.add_argument("input", help="Input .spz file")
    parser.add_argument("output", help="Output .splat file")
    parser.add_argument(
        "--keep-rub",
        action="store_true",
        help="Keep SPZ RUB coordinates (view with --up=y) instead of converting to RDF",
    )
    args = parser.parse_args()

    scene = load_spz(args.input)
    if not args.keep_rub:
        scene = rub_to_rdf(scene)
    save_splat(args.output, scene)
    print(f"Wrote {scene['position'].shape[0]} splats to {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
