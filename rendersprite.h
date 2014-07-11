/*	The MIT License (MIT)
*	
*	Copyright (c) 2014 Gerard Geer
*	
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files (the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions:
*	
*	The above copyright notice and this permission notice shall be included in
*	all copies or substantial portions of the Software.
*	
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*	THE SOFTWARE.
*/

#ifndef RENDERSPRITE_H
#define RENDERSPRITE_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

#include "lodepng.h"

// Alias a couple of OpenGL's format enumerations for "namespace" homogeneity.
#define RS_RGB GL_RGB
#define RS_RGBA GL_RGBA

// Just a few readability defines.
#define RS_NULL_BUFFER 0
#define RS_NULL_TEXTURE 0
#define RS_NULL_FBO 0
#define RS_NULL_SHADER 0
#define RS_NULL_PROGRAM 0
#define RS_NULL_FRAMEBUFFER 0


// Define a few values to make generating geometry sensible.
// How many data components are there for all the vertices.
#define RS_NUM_SQUARE_COMPONENTS 16
// How many indices are needed to draw the square.
#define RS_NUM_SQUARE_INDICES 4

// The maximum number of palette entries possible.
#define RS_MAX_PALETTE_ENTRIES 256

/*
	An RGBA color type that is used to simplify
	specifying color replacement and tinting.
	
	Members:
	r (GLfloat): The red term of the color.
	g (GLfloat): The green term of the color.
	b (GLfloat): The blue term of the color.
	a (GLfloat): The alpha term of the color.
*/
typedef struct
{
	GLfloat r, g, b, a;
} RS_Color;

/*
	An encapsulation of color replacement arrays
	in a way that frames them as a color palette,
	e.g., the Genesis VDP.
	
	Members:
	keys (RS_Color**): An array of RS_Color pointers
						that serves as the keys as to
						whether or not the current fragment
						needs to be replaced.
	entries (RS_Color**): An array of RS_Color pointers
						that serves as the substitutions
						for the fragments that need to be replaced.
	num (Unsigned Int): The number of entries--the length of
						either array.
*/
typedef struct 
{
	RS_Color ** keys;
	RS_Color ** entries;
	unsigned int num;
} RS_Palette;

/*
	Defines a RenderSprite sprite. Since a lot of these can ruin
	the functionality of the RenderSprite library, these fields are
	private.
	
	Members:
	width (GLuint)		The width of the Sprite.
	height (GLuint)		The height of the Sprite.
	tex (GLuint)		OpenGL's handle to the Sprite's
						texture object. This stores the
						sprite image.
	att (GLuint)		The texture that serves as the framebuffer's
						color attachment.
	fbo (GLuint)		OpenGL's handle to the Sprite's
						framebuffer object. 
	imageWidth(GLuint)	When a sprite is not animated, the width of
						the sprite and the image are the same. However,
						when multiple frames of animation are stored in
						the texture image, then the sprite is the width
						of a single frame of animation, and the image
						is the width of several of these frames. This
						stores the width of the entire sprite image in
						the case that it is animated.
	imageHeight(GLuint)	Stores the actual width of the sprite image.
	frameOffsetX(GLuint)	The offset from 0 the X texture coordinate is
							shifted to reach the current frame.
	frameOffsetY(GLuint)	The offset from 0 the Y texture coordinate is
							shifted to reach the current frame.
	format (RS_RGB(A))	The format of the image. Either RS_RGB 
						or RS_RGBA.
	rotation (GLfloat)	A transform variable that describes how far
						rotated around its center the sprite is,
						in radians.
	posX (GLint)		The X position of the sprite.
	posY (GLint)		The Y position of the sprite.
	scaleX (GLfloat)	A horizontal scale factor that used
						to modify the sprite.
	scaleY (GLfloat)	A likewise vertical scale factor.
	tint (RS_Color*)	A pointer to an RS_Color by which the
						color is multiplied in the name of tinting. 
	paletteA (RS_Palette*)	The first of two color replacement palettes.
	paletteB (RS_Palette*)	The second of two color replacement palettes.
	swapHeight (GLint)		The Y coordinate above which paletteA is used,
							and at or below paletteB is used.
							
	A few notes about how palettes work:
	
	As you see, there are two RS_Palettes stored, as well as a variable called
	swapHeight. These variables determine the color replacement behaviour
	encountered when rendering a sprite. 
	Palettes are used in the following way:
		For each fragment, if the fragment's color matches a key, it is
		re-colored with the color at the same index in the entries array.
	Now the fragment shader is passed both sets of keys and entries from
	each palette. If the Y-coordinate of the current fragment is above
	swapHeight, then the first palette is used; if it is at or below, then
	the second palette is considered.
	If either palette is NULL, then the differentiation does not take place
	and the remaining palette is used for the entire sprite.
	If both are NULL, then no color swapping occurs.	
*/

typedef struct
{
	GLuint width, height;
	GLuint tex, att, fbo;
	GLuint format;
	
	GLuint imageWidth, imageHeight;
	GLuint frameOffsetX, frameOffsetY;
	
	GLfloat rotation;
	GLint posX, posY;
	GLfloat scaleX, scaleY;
	
	RS_Color * tint;
	
	RS_Palette * paletteA;
	RS_Palette * paletteB;
	GLint swapHeight;	
} RS_Sprite;

	
/*
	Initializes static variables in the RenderSprite
	library. This includes GPU buffers, rendering geometry,
	and shader compilation.
	
	NOTE: This must be called before any drawing can be done.
*/
void RS_init(void);

/*
	De-initializes the library, clearing all allocated data and
	GPU holdings.
*/
void RS_deInit(void);

/*
	Creates an empty RS_Sprite; that is, one without an image
	to begin with.
	
	Parameters:
		width (GLuint): The width of this new sprite.
		height (GLuint): The height of this new sprite.
		format (RS_RGB(A)): The image format of this texture,
							either RS_RGBA or RS_RGB.
							
	Returns:
		A reference to the sprite, which has been allocated
		on the stack.
*/
RS_Sprite * RS_mkEmptySprite(GLuint width, GLuint height, GLuint format);

/*
	Creates an RS_Sprite, initialized with the PNG loaded from
	the filename given.
	
	Parameters:
		filename (char*): The filename (and path).
	
	Returns:
		A reference to the new RS_Sprite.
*/
RS_Sprite * RS_mkSpriteFromPNG(char * filename);

/*
	Creates an RS_Sprite from a PNG containing multiple frames of animation.
	
	Parameters:
		filename (char*): The filename (and path).
		frameWidth (GLuint): The width of a single frame of animation.
		frameHeight (GLuint): The height of a single frame of animation.
		
	Returns:
		A reference to the new RS_Sprite.
*/
RS_Sprite * RS_mkAnimatedSpriteFromPNG(char * filename, GLuint frameWidth, GLuint frameHeight);

/*
	Deletes all of a given sprite's memory allocations
	and clears out its presence from the GPU.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to delete.
*/
void RS_deleteSprite(RS_Sprite * sprite);

/*
	THIS IS ONLY NECESSARY WHEN THE SPRITE WAS NOT CREATED WITH
	RS_mkAnimatedSpriteFromPNG.
	Readies an RS_Sprite to be animated. Note that this expects 
	frames to be of equal width, and positioned from left to 
	right in the PNG.
	
	Example.
	---------------------------------------------------------
	| Frame | Frame | Frame | Frame | Frame | Frame | Frame |
	| 1     | 2     | 3     | 4     | ...   | n-1   | n-2   | 
	|       |       |       |       |       |       |       | 
	---------------------------------------------------------
	
	Parameters:
		sprite (RS_Sprite*): The sprite to set up.
		frameWidth (GLuint): How wide, in texels each frame is.
							This should evenly divide the width
							of the image.
*/
void RS_initAnimation(RS_Sprite * sprite, GLuint frameWidth, GLuint frameHeight);

/*
	Creates an RS_Color.
	
	Parameters:
		r (GLfloat): The red term of the color.
		g (GLfloat): The green term of the color.
		b (GLfloat): The blue term of the color.
		a (GLfloat): The alpha term of the color.
		
	Returns:
		A reference to the freshly made RS_Color.
*/
RS_Color * RS_mkColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

/*
	Destroys an RS_Color instance, freeing all of its
	members. Exists mainly for consistency with RS_destroySprite().
	
	Parameters:
		color (RS_Color*): The RS_Color to delete.
*/
void RS_deleteColor(RS_Color * color);

/*
	Creates an RS_Palette.
	
	Parameters:
		keys (RS_Color**): An array of RS_Color pointers used
						in determining whether a fragment needs
						to be replaced.
		entries(RS_Color**): An array of RS_Color pointers used
						to replace occurrences of the colors
						specified in the "keys" array.
		numPairs (unsigned integer): The number of 1-to-1 
									key-entry pairs.
*/
RS_Palette * RS_mkPalette(RS_Color ** keys, RS_Color ** entries, unsigned int numPairs);

/*
	Deletes all references of RS_Color held in the
	given RS_Palette. Note that all references to
	these RS_Colors will become null.
	
	Parameters:
		palette (RS_Palette*): The RS_Palette to operate on.
*/
void RS_scrubPalette(RS_Palette * palette);

/*
	Destroys an RS_Palette instance, freeing all of its
	members. Exists mainly for consistency with RS_destroySprite().
	Does not delete or free any constituent RS_Colors. 
	
	Parameters:
		palette (RS_Palette*): The RS_Palette to delete.
*/
void RS_deletePalette(RS_Palette * palette);

/*
	Sets the rotation transform on the given
	RS_Sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to operate on.
		rads (GLfloat): The amount to rotate the sprite when
						drawing, in radians.
*/
void RS_setRotation(RS_Sprite * sprite, GLfloat rads);

/*
	Sets the scale transform of the given RS_Sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to operate on.
		x (GLfloat): The new horizontal scale factor.
		y (GLfloat): The new vertical scale factor.
*/
void RS_setScale(RS_Sprite * sprite, GLfloat x, GLfloat y);

/*
	Sets the position at which to render the top left corner
	of the sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to operate on.
		x (GLfloat): The X position at which to render
					the sprite.
		y (GLfloat): The Y position at which to render
					the sprite.
*/
void RS_setPosition(RS_Sprite * sprite, GLint x, GLint y);

/*
	Sets the tint color of the sprite. Works like so:
	s.r*t.r s.g*t.g s.b*t.b s.a*t.a
	Where s is the color of the sprite and t is the color
	of the tint.
	Note that in cases where the tint reference is NULL,
	1, 1, 1, 1 is passed as the color reference.
	Therefore, to remove tinting, one only needs to
	provide a NULL pointer for tint.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to operate on.
		tint (RS_Color*): The color to tint the sprite with.
*/
void RS_setTint(RS_Sprite * sprite, RS_Color * tint);

/*
	Sets the height along the canvas that palette A will
	stop being used and palette B begins usage.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to operate on.
		height (GLint): The new height.
*/
void RS_setSwapHeight(RS_Sprite * sprite, GLint height);

/*
	Assigns an RS_Palette to the first palette slot on
	the given RS_Sprite. NULL can be passed as the palette
	to prevent the use of the sprite's paletteA, as well
	as prevent height-dependent palette choice.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to operate on.
		palette (RS_Palette*): The palette to give to the sprite
								as its palette A.

*/
void RS_setPaletteA(RS_Sprite * sprite, RS_Palette * palette);

/*
	Assigns an RS_Palette to the second palette slot on
	the given RS_SPrite. Again, NULL can be passed to prevent
	the use of this palette slot and height-dependent palette
	usage.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to operate on.
		palette (RS_Palette*): The palette to give to the sprite
								as its second palette.
*/
void RS_setPaletteB(RS_Sprite * sprite, RS_Palette * palette);

/*
	Clears positioning, scaling, rotation, and tint
	transformations on the given sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to operate on.
*/
void RS_clearTransforms(RS_Sprite * sprite);

/*
	Adds a color replacement pair to a given palette.
	
	Parameters:
		palette (RS_Palette*): The palette to operate on.
		oldColor (RS_Color*): A color to be replaced.
		oldColor (RS_Color*): A color to replace with.
*/
void RS_pushColorReplacement(RS_Palette* palette, RS_Color * oldColor, RS_Color * newColor);

/*
	Removes the most recent color replacement pair
	given to an RS_Palette.
	
	Parameters:
		palette (RS_Palette*): The palette to operate on.
*/
void RS_popColorReplacement(RS_Palette* palette);

/*
	Provides alternate functionality to 
	RS_popColorReplacement(), allowing for the clearing of
	all color replacement pairs for this palette.
	
	Parameters:
		palette (RS_Palette*): The sprite to operate on.
*/
void RS_clearColorReplacements(RS_Palette * palette);

/*
	Increments frames of animation for the given sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite whose animation frame
							to increment.
*/
void RS_iterFrame(RS_Sprite * sprite);

/*
	Renders one sprite to another.
	Specifics:
	The "medium" sprite's texture (which is also that
	sprite's framebuffer's color attachment) is used
	to texture a quad rendered into the "canvas" sprite's
	framebuffer.
	This quad's dimensions are that	of the "medium"
	sprite, transformed by all the transformations
	performed on that sprite.
	
	Parameters:
		canvas (RS_Sprite*): The sprite to draw onto.
		medium (RS_Sprite*): The sprite to draw onto the other.
		mix (GLfloat): How much of the medium sprite image to use
						at the expense of the canvas sprite image.
						Clamped to the range of [0.0 ... 1.0].
*/
void RS_renderSpriteToSprite(RS_Sprite * canvas, RS_Sprite * medium, GLfloat blend);

/*
	Renders the given sprite to the window, or
	the current framebuffer being used.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to draw to the screen.
*/
void RS_renderSpriteToScreen(RS_Sprite * sprite);

/*
	Binds OpenGL's current framebuffer to that of the sprite,
	forcing all subsequent drawing calls to be done into
	that framebuffer; that is, until this action is reversed.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to render to.
*/
void RS_beginRenderToSprite(RS_Sprite * sprite);

/*
	Binds OpenGL's current framebuffer to NULL, leaving it
	to render to the main application window.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to draw to the screen.
*/
void RS_endRenderToSprite(RS_Sprite * sprite);

/*
	Returns the sprite's OpenGL texture object handle.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
		
	Returns:
		The sprite's OpenGl texture handle.
*/
GLuint RS_getTexture(RS_Sprite * sprite);

/*
	Returns the sprite's OpenGl framebuffer object handle.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
	
	Returns:
		The sprite's OpenGL framebuffer handle.
*/
GLuint RS_getFBO(RS_Sprite * sprite);

/*
	Returns the width of the sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
	
	Returns:
		The width of the sprite.
*/
GLuint RS_getWidth(RS_Sprite * sprite);

/*
	Returns the height of the sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
	
	Returns:
		The height of the sprite.
*/
GLuint RS_getHeight(RS_Sprite * sprite);

/*
	Returns the current rotation transformation of
	the sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
	
	Returns:
		The current rotation of the sprite.
*/
GLfloat RS_getRot(RS_Sprite * sprite);

/*
	Returns the X position of the sprite.
	
	Parameters:
		sprite(RS_Sprite*): The sprite to access.
	
	Returns:
		The X position of the sprite.
*/
GLint RS_getXPos(RS_Sprite * sprite);

/*
	Returns the Y position of the sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
		
	Returns:
		The Y position of the sprite.
*/
GLint RS_getYPos(RS_Sprite * sprite);

/*
	Returns the current horizontal scale factor of 
	the sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
		
	Returns:
		The horizontal scale factor of the sprite.
*/
GLfloat RS_getXScale(RS_Sprite * sprite);

/*
	Returns the current vertical scale factor of
	the sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
	
	Returns:
		The vertical scale factor of the sprite.
*/
GLfloat RS_getYScale(RS_Sprite * sprite);

/*
	Returns the current tint being performed on
	the sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
		
	Returns:
		The "tint" RS_Color reference of the
		given sprite.
*/
RS_Color * RS_getTint(RS_Sprite * sprite);

/*
	Queries the color attachment of the given sprite's
	framebuffer, and returns a 1-dimensional array containing
	all the pixel data of the sprite, in the format of the 
	sprite. If the format of the sprite is RS_RGBA, then
	the data returned will be formatted as:
		[r, g, b, a, r, g, b, a, ..., r, g, b, a]
	Where each repetition is another pixel, from left to
	right, from top to bottom.
	
	NOTE: This data is allocated with malloc(), so free it
	when done.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
		
	Returns:
		The requested texel data, formatted as a 1-D array.
*/
GLfloat * RS_getTexelData(RS_Sprite * sprite);

/*
	Operates the same as getTexelData(RS_Sprite*), but allows
	the querying of a specific rectangle of the sprite image.
	Format rules are the same.
	
	NOTE: Like RS_getPixeldata(RS_Sprite*), this data is malloc'd,
	don't be afraid to free it.
	
	Parameters;
		sprite (RS_Sprite*): The sprite to access.
		x (GLuint): The top left corner of the rectangle of interest.
		y (GLuint): The top left corner of the rectangle of interest.
		width (GLuint): The width of the rectangle of interest.
		height (GLuint): The height of the rectangle of interest.
	
	Returns:
		The requested texel data, formatted as a 1-D array.
*/
GLfloat * RS_getTexelGroup(RS_Sprite * sprite, GLuint x, GLuint y, GLuint width, GLuint height);

/*
	Populates the given container RS_Color reference with the
	color from the given sprite at the given location.
	
	Parameters:
		container (RS_Color*): An instance of RS_Color to store
								the queried texel color.
		sprite (RS_Sprite*): The sprite to access.
		x (GLuint): The x location of the texel to query.
		y (GLuint): The y location of the texel to query.
		
	Returns:
		The texel data of the requested region, formatted as
		a 1-D array.
	
*/
void RS_getColorAt(RS_Color * container, RS_Sprite * sprite, GLuint x, GLuint y);

/*
	Returns the red term of the texel at (x,y) in the 
	given sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
		x (GLuint): The x location of the texel to query.
		y (GLuint): The y location of the texel to query.
	
	Returns:
		The red term of the texel at (x, y) in the
		given sprite.
*/
GLfloat RS_getRedAt(RS_Sprite * sprite, GLuint x, GLuint y);

/*
	Returns the green term of the texel at (x,y) in the 
	given sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
		x (GLuint): The x location of the texel to query.
		y (GLuint): The y location of the texel to query.
	
	Returns:
		The green term of the texel at (x, y) in the
		given sprite.
*/
GLfloat RS_getGreenAt(RS_Sprite * sprite, GLuint x, GLuint y);

/*
	Returns the blue term of the texel at (x,y) in the 
	given sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
		x (GLuint): The x location of the texel to query.
		y (GLuint): The y location of the texel to query.
	
	Returns:
		The blue term of the texel at (x, y) in the
		given sprite.
*/
GLfloat RS_getBlueAt(RS_Sprite * sprite, GLuint x, GLuint y);

/*
	Returns the opacity term of the texel at (x,y) in the 
	given sprite.
	
	Parameters:
		sprite (RS_Sprite*): The sprite to access.
		x (GLuint): The x location of the texel to query.
		y (GLuint): The y location of the texel to query.
	
	Returns:
		The alpha term of the texel at (x, y) in the
		given sprite.
*/
GLfloat RS_getAlphaAt(RS_Sprite * sprite, GLuint x, GLuint y);

#endif