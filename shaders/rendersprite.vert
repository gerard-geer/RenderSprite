#version 120
// The coordinates of the incoming vertex.
attribute vec2 vertPosition;
// The texture coordinates of the incoming vertex.
attribute vec2 vertUV;

// The amount to rotate incoming vertices by.
uniform float rotation;
// The amount by which to scale the sprite.
uniform vec2 scale;
// The position of the sprite within the current
// rendering context.
uniform vec2 position;
// The dimensions of a single frame of animation for
// both surfaces.
uniform vec2 canvasFrameSize;
uniform vec2 mediumFrameSize;
// The integer texture coordinate offset to reach the current
// frame of animation for both surfaces.
uniform vec2 canvasFrameOffset;
uniform vec2 mediumFrameOffset;
// The dimensions of each texture image.
uniform vec2 canvasImageSize;
uniform vec2 mediumImageSize;
// The 2D vector texture coordinates we pass through the rasterizer
// and interpolator to the fragment shader.
varying vec2 canvasUV, mediumUV; 

/* 
	Rotates a coordinate around a center point
	by the amount of radians specified.
*/
void rotate(inout vec2 subject, in vec2 center, in float amount)
{
	subject -= center;
	subject.x *= cos(amount);
	subject.y *= sin(amount);
	subject += center;
}

/*
	The main function of this vertex shader.
*/
void main(void)
{
	// Send over the current vertex' UV coordinate.
	canvasUV = vertUV;
	mediumUV = vertUV;
	// But not before we transform it to rest at the
	// proper frame in a multi frame texture image.
	canvasUV = (canvasFrameSize/canvasImageSize) + (canvasFrameOffset/canvasImageSize);
	mediumUV = (mediumFrameSize/mediumImageSize) + (mediumFrameOffset/mediumImageSize);
	
	// Create a local copy of the read-only
	// vertex position attribute.
	vec2 vert = vec2(vertPosition);
	// Rotate that position.
	rotate(vert, vec2(.5), rotation);
	// Scale the vertex to be the original
	// size of the sprite.
	vert *= mediumFrameSize/canvasFrameSize;
	// Apply secondary scaling.
	vert *= scale;
	// Move the vertex to the sprite's
	// intended position.
	vert += position/canvasFrameSize;
	// Give the finished product over to the
	// rest of the pipeline.
	gl_Position = vec4(vert, 0.0, 1.0);
}
