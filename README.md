# fun

Simple C++ debug overlay module for an FPS-style engine.

## Files
- `include/debug_overlay.hpp` – engine-facing API and data structures.
- `src/debug_overlay.cpp` – implementation of entity filtering, visibility, target selection, and rendering.

## Build check
```bash
g++ -std=c++17 -Iinclude -c src/debug_overlay.cpp -o /tmp/debug_overlay.o
```
