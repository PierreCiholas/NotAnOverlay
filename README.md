# NotAnOverlay

WARNING: This is only an experiment and not a ready-off-the-shelf solution.

This program uses GDI functions (including GetDC, CreateCompatibleDC, CreateCompatibleBitmap, BitBlt, StretchBlt) to create a copy of an area of the screen as a bitmap in memory, that it then sets as background image of a window.
With this window having as background the copy of another area of the screen, any overlay can be applied on it (using DirectX, OpenGL, GDI or other).
This eliminate the detection vectors of having a window with the properties WS_EX_TOPMOST, WS_EX_TRANSPARENT, and WS_EX_LAYERED, having also a client area size of precisely the size of the game client area located at the same coordinates.

GDI being not the best performence-wise, I doubted to have something running smoothly, but this works with a framerate of 60 FPS, so this is pretty good.
