# SDF Rendering Test

This is a test I wrote to experiment with signed distance field rendering. I made SDF functions for a circle and for a square and wrote a shader to color it in different ranges. I noticed that when allowing the distance function to overflow, it made interesting patterns, so I added animation to the radius parameter.

![screenshot1](screenshot1.jpg)

## Building

GLFW3 and AntTweakBar are required. Binaries for macOS are included in the repo.

The CMake was only tested macOS. The libraries may need to be modified on other platforms.

To build on macOS:
```
mkdir build
cd build
cmake ..
cmake --build . --target install
```

![screenshot2](screenshot2.jpg)

## Controls

Drag with mouse to rotate view. Scroll to zoom in/out.

- *Draw Circle* - toggle between circle/square SDF
- *Bilinear Filter* - toggle bilinear/nearest sampling
- *SDF Shader* - toggle SDF/grayscale shader
- *Tex Pow* - resolution (2^pow x 2^pow) of the texture
- *Radius* - the computed radius for the shape function

## Authors

Trevor Smith - [LinkedIn](https://linkedin.com/in/trevorsm/)
