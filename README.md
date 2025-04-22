# SphericImageProjector

This is just to project my astrophotography photos onto a sphere and look around

## Download and Build

```sh
git clone https://github.com/Mossford/SphericImageProjector --depth=1 --recurse-submodules
cd SphericImageProjector
# for glsl to spirv, glslang needs external tools
cd glslang
./update_glslang_sources.py
# might have to make executable
./build.sh
```
