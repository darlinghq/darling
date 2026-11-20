// Copyright © 2022 Apple Inc. All rights reserved.

#ifndef SWIFTUI_METAL_H
#define SWIFTUI_METAL_H

#import <TargetConditionals.h>

#if !TARGET_OS_WATCH && __METAL_VERSION__

#include <metal_stdlib>

namespace SwiftUI {

/// A region of a rasterized layer.
struct Layer {
  metal::texture2d<half> tex;
  float2 info[5];

  /// Samples the layer at `p`, in user-space coordinates,
  /// interpolating linearly between pixel values. Returns an RGBA
  /// pixel value, with color components premultipled by alpha (i.e.
  /// [R*A, G*A, B*A, A]), in the layer's working color space.
  half4 sample(float2 p) const {
    p = metal::fma(p.x, info[0], metal::fma(p.y, info[1], info[2]));
    p = metal::clamp(p, info[3], info[4]);
    return tex.sample(metal::sampler(metal::filter::linear), p);
  }
};

} // namespace SwiftUI

#endif // !TARGET_OS_WATCH && __METAL_VERSION__
#endif // SWIFTUI_METAL_H
