# DXFImportPlugin

A Qt-based plugin for importing DXF (AutoCAD Drawing Exchange Format) files into [SIM-VICUS](https://sim-vicus.de), a building and district energy simulation tool.

## Build Status

[![Ubuntu 24.04](https://github.com/VICUS-Software/DXFImportPlugin/actions/workflows/DXFImportPlugin-Ubuntu2404.yml/badge.svg)](https://github.com/VICUS-Software/DXFImportPlugin/actions/workflows/DXFImportPlugin-Ubuntu2404.yml)
[![Windows VC2022](https://github.com/VICUS-Software/DXFImportPlugin/actions/workflows/DXFImportPlugin-WindowsVC2022.yml/badge.svg)](https://github.com/VICUS-Software/DXFImportPlugin/actions/workflows/DXFImportPlugin-WindowsVC2022.yml)

## Building

Requires Qt6 (Widgets, Xml, Svg, PrintSupport, Network, Concurrent) and CMake 3.16+.

### Linux

```bash
cd build/cmake
./build.sh release
```

### Windows (Visual Studio 2022)

```bat
cd build\cmake
build.bat
```

The output shared library (`libDXFImportPlugin.so` / `DXFImportPlugin.dll`) is copied to `bin/release/`.

## License

GPL 3.0 — see [LICENSE](LICENSE) for details.
