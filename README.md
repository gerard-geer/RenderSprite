RenderSprite
============
RenderSprite is an OpenGL-wrapping sprite renderer.

Each sprite is essentially a textured quad, drawn at a specified pixel coordinate.
They contain state that defines placement, tinting, scaling and rotation, among others.

Features
--------
* Pixel-perfect rendering
* Per-pixel color swapping / palette rendering
* Y-coordinate-centric palette swapping
* Rendering to sprites
* Simple Animation
* Extremely low geometry usage

Dependencies
------------
* OpenGL 2.1
* OpenGL Extension Wrangler (GLEW) Support is needed for OpenGL 2.1, so any version of GLEW above 1.3.5 "should" work. Tested working with version 1.10.0. http://glew.sourceforge.net/index.html
* LodePNG: This simple, strangely named library is used for loading PNGs. http://lodev.org/lodepng/

Usage
=====
This library simply wraps some geometry and the drawing of it. That means it's up to the user to create an OpenGL rendering context. 

RenderSprite Structures
-----------------------
The library defines three types:
* RS_Color: A simple RGBA color. Has members r, g, b, and a.
* RS_Palette: Wraps two arrays of pointers to RS_Colors; the first a list of "key" colors to look out for, and the second a list of "palette entry" colors. When the fragment shader detects an instance of a key color, it is replaced by the palette entry at the same index. This type also stores the number of pointers in each list.
* RS_Sprite: This type is the basis of the library.

Initialization
--------------
Before using any feature of RenderSprite one must initialize the shader program and geometry buffers by calling RS_init().

Creating a sprite
-----------------
An RS_Sprite can be created either with no image (Potentially for strictly rendering-surface usage) or from either a 24 or 32 bit .PNG image.
Calling RS_mkEmptySprite(), RS_mkSpriteFromPNG() or RS_mkAnimatedSpriteFromPNG() will return a reference to a freshly constructed RS_Sprite. Animation will be discussed later.

Basic sprite usage
------------------
Sprites are state-objects, meaning that various properties must be set before drawing, and such properties are persistent.
The RS_set*() functions allow manipulation of a sprite.

Drawing a sprite can be done with the RS_renderSpriteToSprite() and RS_renderSpriteToScreen() functions.

Rendering a sprite to the screen
--------------------------------
This requires the creation and maintainance of an OpenGL rendering surface. Sprites are drawn directly with glDrawElements() into the current framebuffer.

Rendering a sprite to another sprite
------------------------------------
sprites are drawn at full scale to each other. That means drawing a larger sprite to a smaller one will result in the larger sprite exceeding the boundaries of the smaller. The ratio of the images of the sprite being rendered and the sprite being rendered to is user specified. Animation state is honored for each.

Palette usage
-------------
RenderSprite palettes are more flexible than experience with classic video hardware might lead one to believe. An RS_Palette is simply a list of key colors, a list of palette entry colors, and a variable to store the length of each list. These are used in the fragment shader; whenever a texel of a sprite being rendered matches a key color, it is immediately replaced by the palette entry at the same index. It is important to note that this occurs before tinting is applied.

Each sprite stores references to TWO RS_Palettes: paletteA and paletteB. If both palettes are occupied, then the value of a third value, swapHeight, is used to define a Y-coordinate above which paletteA is used, and at and below paletteB is used.

If either are NULL, then the remaining one is used for the entire sprite. If both are NULL, then palettes are not used.

Rendering to a sprite directly
------------------------------
Each sprite's framebuffer can be rendered to directly using RS_beginRenderToSprite(). Note, though, that they have no depth buffer.
Also, don't forget to call RS_endRenderToSprite() when finished.

Animation
---------
Animation works by stretching the texture coordinates of a sprite to center on only a portion--a frame--of the sprite image, then moving these coordinates around to highlight other frames.
Animation requires that the image you've selected for a sprite is a story-board-like frame image, where each frame of animation occupies the same amount of pixels within the image. The frames must also be ordered from left to right, then from top to bottom.
Second, the Sprite must be created. It is recommended to use RS_mkAnimatedSpriteFromPNG(). However, a sprite can be created with RS_mkSpriteFromPNG(), but RS_initAnimation() must be called with the sprite in order to set up frame dimensions for correct drawing.

Stepping through the frames of animation is as simple as a call to RS_iterFrame(). This function wraps around to the first frame when the last is reached.


