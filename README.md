# RFont
Simple single header modular font rendering library\
The library is designed to be lightweight while also supporting the ablitiy to add your own rendering system.\

## Build statuses
![cplus workflow](https://github.com/ColleagueRiley/RFont/actions/workflows/linux.yml/badge.svg)
![cplus workflow windows](https://github.com/ColleagueRiley/RFont/actions/workflows/windows.yml/badge.svg)
![cplus workflow windows](https://github.com/ColleagueRiley/RFont/actions/workflows/macos.yml/badge.svg)

# Native supported rendering APIs

- OpenGL Legacy 
- Modern OpenGL (opengl 3.3 +)
- RLGL (OPENGL version abstraction layer)

# Documentation 
For documentation read the `RFont.h` file and check out the example in `./example`

# Credits

# stb_truetype
RFont uses [stb_truetype](https://github.com/nothings/stb) for working with fonts.

A modified version of stb_truetype is included in the RFont header.\
But you can use an external copy by adding `#define RFONT_EXTERNAL_STB` to your code

# Fontstash
[Fontstash](https://github.com/memononen/fontstash) is another font rendering library with simular goals.

Fontash was used as a reference for some of this code. 

However, this library is intended to have better perfromance and be more lightweight than fontstash.