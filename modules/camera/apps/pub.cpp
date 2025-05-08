//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

// capture frame -> ring_buffer -> lz4 compression -> pub
//   - main thread captures and pushes frame into ring buffer
//   - pub thread pops frame, compresses it and publishes it
// sub -> decompress frame -> ring_buffer -> view (fps controlled)
//   - sub thread receives frame and pushes it into ring buffer
//   - main thread pops frame, decompresses it and displays it with the right delay (SDL operations
//     should be on main thread)

// TODO
// - command line interface to start and stop capture
// - configuration file to set
//   - camera device
//   - select camera spec (width, height, format, fps)
//   - compression parameters
//   - publisher parameters (topic name, etc)
// - publish per session metadata
//   - camera name
//   - camera spec
//   - compression parameters
// - Include per-frame information
//   - absolute capture timestamp
//   - frame number
// - code organisation
//   - capture thread (main) captures and pushes frames to ring buffer
//   - publish thread pops frame from ring buffer, compresses it, publishes it