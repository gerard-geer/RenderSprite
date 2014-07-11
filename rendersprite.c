#include "rendersprite.h"

// Our handle to the shader object on the GPU.
static GLuint shader;
// Source code for the shader program.
static char * vertSource = "shaders/framebuffer.vert";
static char * fragSource = "shaders/framebuffer.frag";

// Position of the vertex attributes in the shader.
static GLuint posAttrib;
static GLuint uvAttrib;

// Position of the uniform variables in the shader.
static GLuint canvasFrameOffsetUniform; // 2D vector
static GLuint mediumFrameOffsetUniform; // 2D vector
static GLuint canvasFrameSizeUniform; // 2D vector
static GLuint mediumFrameSizeUniform; // 2D vector
static GLuint canvasImageSizeUniform; // 2D vector
static GLuint mediumImageSizeUniform; // 2D vector
static GLuint rotationUniform; 	// Float
static GLuint scaleUniform; 	// 2D vector
static GLuint positionUniform;	// 2D vector
static GLuint tintUniform;		// 4D vector
static GLuint mixUniform;		// Float
static GLuint paletteAKeysUniform;		// 4D vector array
static GLuint paletteAEntriesUniform;	// 4D vector array
static GLuint numPaletteAUniform;		// Unsigned integer
static GLuint paletteBKeysUniform;		// 4D vector array
static GLuint paletteBEntriesUniform;	// 4D vector array
static GLuint numPaletteBUniform;	// Unsigned integer
static GLuint swapHeightUniform;	// Float.
static GLuint canvasTextureUniform; // Integer, referring to a texture object.
static GLuint mediumTextureUniform;	// Integer, referring to a texture object.

// The handle to the GPU-side data buffer storing
// all the vertex data.
static GLuint vertexBuffer;

// The handle to the indexing buffer used to order
// the drawing of vertex data in the vertex buffer.
static GLuint indexBuffer;

/*
	Generates vertex, color, UV, and normal information for a square,
	inserting it homologated into the given vertex data array. It
	then records the proper vertex access order into the given index array.
	
	Doing things this way leaves us with the smallest footprint on the
	GPU. Four vertices of four 16 bit floats each in vertex data, and
	four unsigned bytes to access them. Note that position isn't even
	stored in 3D.
	
	However, allocating the arrays and transferring the data to
	the GPU is left up to the caller.
	
	Also, this index arrangement assumes that the geometry will be drawn
	with GL_TRIANGLE_STRIP.
*/
static void generateSquare(GLfloat * vertexData, GLubyte * indexData)
{	
	// This isn't pretty.
	// Top left vertex.
	vertexData[0] = 0.0;	// X
	vertexData[1] = 0.0;	// Y
	vertexData[2] = 0.0;	// U
	vertexData[3] = 0.0;	// V
	// Top right vertex.	
	vertexData[4] = 1.0;	// X
	vertexData[5] = 0.0;	// Y
	vertexData[6] = 0.0;	// U
	vertexData[7] = 0.0;	// V
	// Bottom left vertex.
	vertexData[8] = 0.0;	// X
	vertexData[9] = 1.0;	// Y
	vertexData[10] = 0.0;	// U
	vertexData[11] = 0.0;	// V
	// Bottom right vertex.	
	vertexData[12] = 0.0;	// X
	vertexData[13] = 1.0;	// Y
	vertexData[14] = 0.0;	// U
	vertexData[15] = 0.0;	// V
	
	// This isn't pretty either.
	indexData[0] = 2;	// Bottom left
	indexData[1] = 0;	// Top left
	indexData[2] = 3;	// Bottom Right
	indexData[3] = 1;	// Top Right
}

/*
	Initializes all of a basic square's gpu and cpu variables, so that it
	can be drawn easily, with your choice of shader.
*/
static void initSquare(void)
{
	// Generate GPU buffers.
	glGenBuffers(1, &vertexBuffer);
	glGenBuffers(1, &indexBuffer);
	
	// Again, allocate space for generating geometry data.
	GLubyte * indexData = malloc(sizeof(GLfloat)*RS_NUM_SQUARE_INDICES);
	GLfloat * vertexData = malloc(sizeof(GLfloat)*RS_NUM_SQUARE_COMPONENTS);
	
	// Use one of our geometry helper functions to populate our
	// buffers with data for a square.
	generateSquare(vertexData, indexData);
	
	// Now that the data is generated and stored, we can copy
	// that data over to the GPU.
	// First the index data.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLfloat)*RS_NUM_SQUARE_INDICES, indexData, GL_STATIC_DRAW);
	// Then vertex data.
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*RS_NUM_SQUARE_COMPONENTS, vertexData, GL_STATIC_DRAW);
	
	// Unbind for safety!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RS_NULL_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, RS_NULL_BUFFER);
	
	// Free the CPU side data.
	free(vertexData);
	free(indexData);
}

/*
	Draws the square that was set up, assuming the existence of a shader,
	in use when this function is called, with 2D position, 4D color, and 
	2D UV attributes available.
*/
static void drawSquare(void)
{
	// Bind to the vertex buffer objects so that they will
	// be used in place of an explicitly sourced array of data
	// in glVertexAttribPointer().
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	// Enable all the vertex attributes so that they will
	// be usable in the vertex shader.
	glEnableVertexAttribArray(posAttrib);
	glEnableVertexAttribArray(uvAttrib);
	
	// Set up data feeding to those vertex attributes.
	// Remember how we homologated both UV and position
	// data into the same buffer? Here's where we tell
	// OpenGL how to dig through it.
	glVertexAttribPointer(posAttrib, // State which attribute this buffer will be fed to.
						2, 	// Specify the number of components per vertex for this attribute.
						GL_FLOAT, 	// Specify the type of these components.
						GL_FALSE, 	// Should these be normalized? Nope.
						RS_NUM_SQUARE_COMPONENTS*sizeof(GLfloat),	// Byte offset between
													// consecutive occurrences of this attribute.
						0);	// "If a non-zero named buffer object is bound to the GL_ARRAY_BUFFER 
							// target (see glBindBuffer) while a generic vertex attribute array 
							// is specified, pointer is treated as a byte offset into the buffer
							// object's data store." (From the OpenGL man pages.)
	GLint offset = sizeof(GLfloat)*2;
	glVertexAttribPointer(uvAttrib,
						2,	// This directly how the incoming vectors are set up for the vertex shader.
						GL_FLOAT,
						GL_FALSE,	// Normalization is best when dealing with integer data values.
									// All values are divided by the largest.
						RS_NUM_SQUARE_COMPONENTS*sizeof(GLfloat), 
						&offset);	// The number of bytes to traverse to get to the first
											// occurrence of this attribute in the buffer. Since
											// we have to over come two floats to pass the first
											// position...
		
	// Now that buffer feeding is set up, we can tell OpenGL draw the 
	// geometry. Hopefully the desired shader is being used and all 
	// desired uniforms are set up by this point.
	glDrawElements(GL_TRIANGLES, RS_NUM_SQUARE_INDICES, GL_UNSIGNED_BYTE, 0);

	// Now that we're all done with this draw call, we should disable
	// these attributes to prevent GL state discontinuity.
	glDisableVertexAttribArray(posAttrib);
	glDisableVertexAttribArray(uvAttrib);
	
	// Now remember children, unbind so you
	// don't accidentally do something, and
	// you leave everything how you left it.
	// you make no persistent changes to state
	// when you call this function.
	glBindBuffer(GL_ARRAY_BUFFER, RS_NULL_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RS_NULL_BUFFER);	
}

/*
	Loads shader source code from the given filename.
*/
static char * loadShaderSource(char * filename)
{
	// Create a character to store the next element in the
	// file.
	char next;
	// A counter of how many elements we've loaded.
	// We start at one for safe keeping of the null
	// terminator.
	int chars = 1;
	// Create a character array to store the input data.
	char * elements = (char *) malloc(sizeof(char));
	// Open the file.
	FILE* f = fopen(filename, "r");
	// Get the first character in the file stream.
	next = fgetc(f);
	// While that next character is not the end of
	// file marker, we...
	while( next != EOF )
	{
		// store the character,
		elements[chars] = next;
		// and gently expand our storage array.
		// If this realloc ever fails, things will
		// crash pretty fast.
		elements = realloc(elements, ++chars*sizeof(char));
		// get the new next character.
		next = fgetc(f);
	}
	return elements;
}

/*
	Compiles shader and returns a reference to the compiled shader object.
*/
static GLint compileShader(const GLchar * source, GLenum type)
{
	// Make sure the user passes a valid shader type.
	if(type != GL_VERTEX_SHADER || type != GL_FRAGMENT_SHADER)
		return RS_NULL_SHADER;
		
	// Create the shader on the GPU.
	GLuint shader = glCreateShader(type);
	// Pass in the shader source.
	GLint length = strlen(source);
	glShaderSource(shader, 1, &source, &length);
	// Compile that shader on the GPU.
	glCompileShader(shader);
	
	// Create an integer to store the compilation status of the shader.
	GLint status;
	// Get that status.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	// Return the shader handle only if it compiled.
	return status == GL_FALSE ? RS_NULL_SHADER : shader;
}

/*
	Links a vertex and fragment shader into a single shader program, and
	returns a reference to it.
*/
static GLint linkShaderProgram(GLint vert, GLint frag)
{
	// Create a shader program on the GPU.
	GLint program = glCreateProgram();
	// Attach the shader objects to our new program.
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	// Link the program right on up.
	glLinkProgram(program);
	
	// Create an integer to store the status of the link operation.
	GLint linked;
	// Get that status.
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	
	// Now that we've tried to link the program, the
	// shader objects themselves are no longer needed.
	glDeleteShader(vert);
	glDetachShader(program, vert);
	glDeleteShader(frag);
	glDetachShader(program, frag);
	
	// Finally we return the value.
	return linked == GL_FALSE ? RS_NULL_PROGRAM : program;
}

/*
	Loads a vertex and fragment shader from source, compiles, and links
	them, returning a pointer to the final shader program.
*/
static GLint createShaderProgram(char * vertFile, char * fragFile)
{
	// Load shader source code.
	char * vertSource = loadShaderSource(vertFile);
	char * fragSource = loadShaderSource(fragFile);
	
	// Compile shaders.
	GLint vert = compileShader(vertSource, GL_VERTEX_SHADER);
	GLint frag = compileShader(vertSource, GL_FRAGMENT_SHADER);
	
	// Link together the shader program and return it.
	return linkShaderProgram(vert, frag);
}

/*
	Generates a texture object for the given instance
	of RS_Sprite.
*/
static void generateTexture(GLuint * textureHandle, GLuint width, GLuint height, GLuint format, unsigned char * data)
{
	// Initialize that texture that will be used
	// as the color buffer of the new framebuffer.
	glGenTextures(1, textureHandle);
	
	// Now we've got to bind that texture to the current
	// GL_TEXTURE_2D so we can do dirty stuff to it.
	glBindTexture(GL_TEXTURE_2D, *textureHandle);

	// Format the texture image itself.
	glTexImage2D(GL_TEXTURE_2D, // Which texture buffer to use.
				0, 				// L.O.D.
				format, 		// Internal pixel format the texture shall use.
				width, 			// The width of the texture.
				height, 		// The height of the texture.
				0, 				// Border width. Always 0.
				format, 		// The pixel format of the incoming data.
				GL_FLOAT,		// The type of each color term being received.
				data);			// The image data itself.

	// AH YEAH OOH AHH YOU TAKE THOSE 
	// NO-MIPMAP TEXTURE PARAMETERS.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	
	// OH YEAH YOU LIKE THAT NEAREST
	// NEIGHBOR FILTERING.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// >Clamps down onto image-space
	// >in the heat of the moment.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// Pull out.
	glBindTexture(GL_TEXTURE_2D, RS_NULL_TEXTURE);
}

/*
	Resizes a texture object.
*/
static void resizeTexture(GLuint * textureHandle, GLuint width, GLuint height, GLuint format)
{
	glBindTexture(GL_TEXTURE_2D, *textureHandle);
	
	// Resize the texture.
	glTexImage2D(GL_TEXTURE_2D, // Which texture buffer to use.
				0, 				// L.O.D.
				format, 		// Internal pixel format the texture shall use.
				width, 			// The width of the texture.
				height, 		// The height of the texture.
				0, 				// Border width. Always 0.
				format, 		// The pixel format of the incoming data.
				GL_FLOAT,		// The type of each color term being received.
				NULL);			// The image data itself.
}

/*
	Generates a framebuffer object for a given instance
	of RS_Sprite.
*/
static void generateFramebuffer(GLuint * fboHandle, GLuint * textureHandle)
{
	// Set the mood.
	glGenFramebuffers(1, fboHandle);
	
	// Make sure the texture object knows that it's the only one.
	glBindTexture(GL_TEXTURE_2D, *textureHandle);

	// >Looks like someone is having some bonding time.
	glBindFramebuffer(GL_FRAMEBUFFER, *fboHandle);
	
	// >gives her the color attachment.
	glFramebufferTexture2D(GL_FRAMEBUFFER, 
							GL_COLOR_ATTACHMENT0,
							GL_TEXTURE_2D, 
							*textureHandle, 
							0);

	// Unbind with a deep sense of affection and longing.
	glBindFramebuffer(GL_FRAMEBUFFER, RS_NULL_FRAMEBUFFER);
}

/*
	Initializes the RenderSprite shader program, and retrieves
	all attribute and uniform locations from it.
*/
static void initShaders(void)
{
	// Create the shader program.
	shader = createShaderProgram(vertSource, fragSource);
	
	// OH DEAR LAWDY THESE POSITION QUERIES.
	posAttrib = glGetAttribLocation(shader, "vertPosition");
	uvAttrib = glGetAttribLocation(shader, "vertUV");
	canvasFrameOffsetUniform = glGetUniformLocation(shader, "canvasFrameOffset");
	canvasFrameSizeUniform = glGetUniformLocation(shader, "canvasFrameSize");
	canvasImageSizeUniform = glGetUniformLocation(shader, "canvasImageSize");
	mediumFrameOffsetUniform = glGetUniformLocation(shader, "mediumFrameOffset");
	mediumFrameSizeUniform = glGetUniformLocation(shader, "mediumFrameSize");
	mediumImageSizeUniform = glGetUniformLocation(shader, "mediumImageSize");
	rotationUniform = glGetUniformLocation(shader, "rotation"); 	
	scaleUniform = glGetUniformLocation(shader, "scale"); 	
	positionUniform = glGetUniformLocation(shader, "position");	
	tintUniform = glGetUniformLocation(shader, "tint");		
	mixUniform = glGetUniformLocation(shader, "canvasMediumMix");
	paletteAKeysUniform = glGetUniformLocation(shader, "paletteAKeys");	
	paletteAEntriesUniform = glGetUniformLocation(shader, "paletteAEntries");	
	numPaletteAUniform = glGetUniformLocation(shader, "numPaletteA");	
	paletteBKeysUniform = glGetUniformLocation(shader, "paletteBKeys");	
	paletteBEntriesUniform = glGetUniformLocation(shader, "paletteBEntries");	
	numPaletteBUniform = glGetUniformLocation(shader, "numPaletteB");	
	swapHeightUniform = glGetUniformLocation(shader, "swapHeight");
	canvasTextureUniform = glGetUniformLocation(shader, "canvas");
	mediumTextureUniform = glGetUniformLocation(shader, "medium");
}

void RS_init(void)
{
	// Initialize GLEW and test it.
	glewInit();
	// Initialize the rendering square.
	initSquare();
	// Initialize the shader program and its position constants.
	initShaders();
}

void RS_deInit(void)
{
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteProgram(shader);
}

static RS_Sprite * generateRawSprite(void)
{	
	// Allocate an RS_Sprite-sized hunk of memory.
	RS_Sprite * sprite = malloc(sizeof(RS_Sprite));
	// Set up the transformation and animation variables.
	sprite->frameOffsetX = 0;
	sprite->frameOffsetY = 0;
	sprite->rotation = 0.0;
	sprite->posX = 0;
	sprite->posY = 0;
	sprite->scaleX = 1.0;
	sprite->scaleY = 1.0;
	sprite->tint = NULL;
	sprite->paletteA = NULL;
	sprite->paletteB = NULL;
	sprite->swapHeight = 0;
	// Return the sprite.
	return sprite;
}

RS_Sprite * RS_mkEmptySprite(GLuint width, GLuint height, GLuint format)
{
	// Create a raw, nubile RS_Sprite for prep.
	RS_Sprite * sprite = generateRawSprite();
	
	// Store the width and height and format of the sprite.
	sprite->width = width;
	sprite->imageWidth = width;
	sprite->imageHeight = height;
	sprite->height = height;
	
	// Generate the texture object for this sprite.
	generateTexture(&sprite->tex, 2, 2, format, NULL);
	generateTexture(&sprite->att, sprite->width, sprite->height, format, NULL);
	
	// Generate the framebuffer object for this sprite,
	// using the sprite's texture as the FBO's color
	// attachment.
	generateFramebuffer(&sprite->fbo, &sprite->att);
	
	return sprite;
}

RS_Sprite * RS_mkSpriteFromPNG(char * filename)
{
	// Create a pointer to reference data loaded by LoadPNG.
	unsigned char * imageData;
	
	// Create an instance of RS_Sprite.
	RS_Sprite * sprite = generateRawSprite();
	
	// Load the image, storing any potential error.
	unsigned lodePngError = lodepng_decode32_file(&imageData,
												&sprite->imageWidth,
												&sprite->imageHeight,
												filename);
	// Since we proportedly just loaded a 32-bit RGBA image,
	// we set the format of the sprite to RGBA.
	sprite->format = RS_RGBA;
	
	// If something happens we just assume for the first try
	// that it was an error with the bit depth.
	if(lodePngError)
	{
		lodePngError = lodepng_decode24_file(&imageData,
											&sprite->imageWidth,
											&sprite->imageHeight,
											filename);
		sprite->format = RS_RGB;
	}
	
	// If there's still something wrong, well poop.
	if(lodePngError)
	{
		fprintf(stderr, 
				"Error loading PNG %d: %s", 
				lodePngError, 
				lodepng_error_text(lodePngError));
	}
	
	sprite->width = sprite->imageWidth;
	sprite->height = sprite->imageHeight;
	
	// Generate the sprite's texture object and store the
	// loaded image date in it.
	generateTexture(&sprite->tex, sprite->width, sprite->height, sprite->format, imageData);
	generateTexture(&sprite->att, sprite->width, sprite->height, sprite->format, NULL);
	
	// Get rid of our copy of the image, since we don't need
	// it any more.
	free(imageData);
	
	// Create the sprite's framebuffer.
	generateFramebuffer(&sprite->fbo, &sprite->att);
	
	// At long last!
	return sprite;
}

RS_Sprite * RS_mkAnimatedSpriteFromPNG(char * filename, GLuint frameWidth, GLuint frameHeight)
{
	unsigned char * imageData;
	RS_Sprite * sprite = generateRawSprite();
	unsigned lodePngError = lodepng_decode32_file(&imageData,
												&sprite->imageWidth,
												&sprite->imageHeight,
												filename);
	sprite->format = RS_RGBA;
	if(lodePngError)
	{
		lodePngError = lodepng_decode24_file(&imageData,
											&sprite->imageWidth,
											&sprite->imageHeight,
											filename);
		sprite->format = RS_RGB;
	}
	if(lodePngError)
	{
		fprintf(stderr, 
				"Error loading PNG %d: %s", 
				lodePngError, 
				lodepng_error_text(lodePngError));
	}
	
	sprite->width = frameWidth;
	sprite->height = frameHeight;
	
	// Store the image in the sprite's texture, 
	// but keep the framebuffer the size of a single frame.
	generateTexture(&sprite->tex, sprite->imageWidth, sprite->imageHeight, sprite->format, imageData);
	generateTexture(&sprite->att, sprite->width, sprite->height, sprite->format, NULL);
	free(imageData);
	generateFramebuffer(&sprite->fbo, &sprite->att);
	return sprite;
}

void RS_deleteSprite(RS_Sprite * sprite)
{
	// Delete FBO.
	glDeleteFramebuffers(1, &sprite->fbo);
	// Delete texture objects.
	glDeleteTextures(1, &sprite->tex);
	glDeleteTextures(1, &sprite->att);
	// Free the structure. Bye bye!
	free(sprite);
}

RS_Color * RS_mkColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	RS_Color *color = malloc(sizeof(RS_Color));
	
	color->r = r;
	color->g = g;
	color->b = b;
	color->a = a;
}

void RS_deleteColor(RS_Color * color)
{
	free(color);
}

RS_Palette * RS_mkPalette(RS_Color ** keys, RS_Color ** entries, unsigned int numPairs)
{
	RS_Palette *p = malloc(sizeof(RS_Palette));
	p->keys = keys;
	p->entries = entries;
	p->num = numPairs;
	return p;
}

void RS_scrubPalette(RS_Palette * palette)
{
	unsigned int i;
	for(i = 0; i < palette->num; i++)
	{
		RS_deleteColor(palette->keys[i]);
		RS_deleteColor(palette->entries[i]);
	}
	palette->num = 0;
	palette->keys = realloc(palette->keys, sizeof(RS_Color*));
	palette->entries = realloc(palette->entries, sizeof(RS_Color*));
}

void RS_deletePalette(RS_Palette * palette)
{
	free(palette);
}

void RS_setRotation(RS_Sprite * sprite, GLfloat rads)
{
	sprite->rotation = rads;
}

void RS_setScale(RS_Sprite * sprite, GLfloat x, GLfloat y)
{
	sprite->scaleX = x;
	sprite->scaleY = y;
}

void RS_setPosition(RS_Sprite * sprite, GLint x, GLint y)
{
	sprite->posX = x;
	sprite->posY = y;
}

void RS_setTint(RS_Sprite * sprite, RS_Color * tint)
{
	sprite->tint = tint;
}

void RS_setSwapHeight(RS_Sprite * sprite, GLint height)
{
	sprite->swapHeight = height;
}

void RS_setPaletteA(RS_Sprite * sprite, RS_Palette * palette)
{
	sprite->paletteA = palette;
}

void RS_setPaletteB(RS_Sprite * sprite, RS_Palette * palette)
{
	sprite->paletteB = palette;
}

void RS_clearTransforms(RS_Sprite * sprite)
{
	// zero out all the things.
	sprite->rotation = 0.0;
	sprite->posX = 0;
	sprite->posY = 0;
	sprite->scaleX = 1.0;
	sprite->scaleY = 1.0;
	sprite->tint = NULL;
}
	
/*
	Updates the color replacement uniforms.
*/
static void updateColorSwapUniforms(RS_Sprite * sprite)
{
	// Since we can't feed the GPU raw RS_Colors, we have to unpack the
	// color pairs.
	GLfloat * paletteAKeyTerms = malloc(sizeof(GLfloat)*sprite->paletteA->num*4);
	GLfloat * paletteAEntryTerms = malloc(sizeof(GLfloat)*sprite->paletteA->num*4);
	GLfloat * paletteBKeyTerms = malloc(sizeof(GLfloat)*sprite->paletteB->num*4);
	GLfloat * paletteBEntryTerms = malloc(sizeof(GLfloat)*sprite->paletteB->num*4);
	
	// Unpacking RS_Colors is thirsty work. Time for some lemonade.
	// Also, standard C for loops are a bit cumbersome.
	unsigned int i = 0;
	for(i = 0; i < sprite->paletteA->num; i++)
	{
		paletteAKeyTerms[(i*4)+0] = sprite->paletteA->keys[i]->r;
		paletteAKeyTerms[(i*4)+3] = sprite->paletteA->keys[i]->a;
		paletteAKeyTerms[(i*4)+1] = sprite->paletteA->keys[i]->g;
		paletteAKeyTerms[(i*4)+2] = sprite->paletteA->keys[i]->b;
		
		paletteAEntryTerms[(i*4)+0] = sprite->paletteA->entries[i]->r;
		paletteAEntryTerms[(i*4)+1] = sprite->paletteA->entries[i]->g;
		paletteAEntryTerms[(i*4)+2] = sprite->paletteA->entries[i]->b;
		paletteAEntryTerms[(i*4)+3] = sprite->paletteA->entries[i]->a;
	}
	
	// Store the unpacked values on the GPU.
	glUniform4fv(paletteAKeysUniform, sprite->paletteA->num*4, paletteAKeyTerms);
	glUniform4fv(paletteAEntriesUniform, sprite->paletteA->num*4, paletteAEntryTerms);
	glUniform1i(numPaletteAUniform, sprite->paletteA->num);
	free(paletteAKeyTerms);
	free(paletteAEntryTerms);

	for(i = 0; i < sprite->paletteB->num; i++)
	{
		paletteBKeyTerms[(i*4)+0] = sprite->paletteB->keys[i]->r;
		paletteBKeyTerms[(i*4)+1] = sprite->paletteB->keys[i]->g;
		paletteBKeyTerms[(i*4)+2] = sprite->paletteB->keys[i]->b;
		paletteBKeyTerms[(i*4)+3] = sprite->paletteB->keys[i]->a;
		
		paletteBEntryTerms[(i*4)+0] = sprite->paletteB->entries[i]->r;
		paletteBEntryTerms[(i*4)+3] = sprite->paletteB->entries[i]->a;
		paletteBEntryTerms[(i*4)+1] = sprite->paletteB->entries[i]->g;
		paletteBEntryTerms[(i*4)+2] = sprite->paletteB->entries[i]->b;
	}
	
	// Store the unpacked values on the GPU.
	glUniform4fv(paletteBKeysUniform, sprite->paletteB->num*4, paletteBKeyTerms);
	glUniform4fv(paletteBEntriesUniform, sprite->paletteB->num*4, paletteBEntryTerms);
	glUniform1i(numPaletteBUniform, sprite->paletteB->num);
	free(paletteBKeyTerms);
	free(paletteBEntryTerms);
}

void RS_addColorReplacement(RS_Palette * palette, RS_Color * oldColor, RS_Color * newColor)
{
	if(palette->num == RS_MAX_PALETTE_ENTRIES) return;
	palette->keys[palette->num] = oldColor;
	palette->entries[palette->num] = newColor;

	palette->keys = realloc(palette->keys, sizeof(RS_Color *)*++palette->num);
	palette->entries = realloc(palette->entries, sizeof(RS_Color *)*palette->num);
}

void RS_popColorReplacement(RS_Palette * palette)
{
	if(palette->num == 0) return;
	
	palette->keys = realloc(palette->keys, sizeof(RS_Color *)*palette->num);
	palette->entries = realloc(palette->entries, sizeof(RS_Color *)*palette->num--);
}
	
void RS_clearColorReplacements(RS_Palette * palette)
{
	// Clear out the color pairs.
	palette->keys = realloc(palette->keys, sizeof(RS_Color *));
	palette->entries = realloc(palette->entries, sizeof(RS_Color *));
	palette->num = 0;
}

/*
	Updates the uniforms of the shader to the values
	stored in the given RS_Sprite instance.
*/
static void updateSpriteUniformState(RS_Sprite * sprite)
{
	// Do as told; update all the uniform variables in the shader
	// to reflect the state of the given sprite.
	glUniform1f(rotationUniform, sprite->rotation);
	glUniform2f(scaleUniform, sprite->scaleX, sprite->scaleY);
	glUniform2f(positionUniform, (GLfloat)sprite->posX, (GLfloat)sprite->posY);
	glUniform1f(swapHeightUniform, (GLfloat)sprite->swapHeight);
	updateColorSwapUniforms(sprite);	// Populate the color swap uniforms.
	// The tint is tricky, since having no tint leaves us
	// with a null reference.
	if(sprite->tint)
		glUniform4f(tintUniform, 
					sprite->tint->r,
					sprite->tint->g,
					sprite->tint->b,
					sprite->tint->a);
	else
		glUniform4f(tintUniform, 1.0, 1.0, 1.0, 1.0);
}

void RS_iterFrame(RS_Sprite * sprite)
{
	sprite->frameOffsetX += sprite->width;
	if(sprite->frameOffsetX >= sprite->imageWidth)
	{
		sprite->frameOffsetX = 0;
		sprite->frameOffsetY += sprite->height;
	}
	if(sprite->frameOffsetY >= sprite->imageHeight)
	{
		sprite->frameOffsetX = 0;
		sprite->frameOffsetY = 0;
	}
}

void RS_renderSpriteToSprite(RS_Sprite * canvas, RS_Sprite * medium, GLfloat mix)
{
	// First off let's normalize blend.
	if(mix > 1.0) mix = 1.0;
	if(mix < 0.0) mix = 0.0;
	
	// Bind to the framebuffer of the canvas RS_Sprite, so the
	// rendering pipeline outputs into its texture.
	glBindFramebuffer(GL_FRAMEBUFFER, canvas->fbo);
	// We don't have a depth texture or renderbuffer.
	glDisable(GL_DEPTH_TEST);
	// Begin use of the RenderSprite shader.
	glUseProgram(shader);
	
	// Swap over to the first texture slot so we can
	// populate it with the canvas texture.
	glActiveTexture(GL_TEXTURE0+0);
	glBindTexture(GL_TEXTURE_2D, canvas->tex);
	glUniform1i(canvasTextureUniform, 0);
	
	// We also need to supply the medium texture.
	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, medium->tex);
	glUniform1i(mediumTextureUniform, 1);

	// Set the blending uniform.
	glUniform1f(mixUniform, mix);
	
	// Since we are rendering to the canvas sprite's texture,
	// the essential size of the screen is the width and the
	// height of that very texture.
	
	// Supply info about frame sizes so we don't draw all frames
	// of animation at once.
	glUniform2f(canvasFrameSizeUniform, (GLfloat)canvas->width, (GLfloat)canvas->height);
	glUniform2f(canvasFrameOffsetUniform, (GLfloat)canvas->frameOffsetX, (GLfloat)canvas->frameOffsetY);
	glUniform2f(canvasImageSizeUniform, canvas->imageWidth, canvas->imageHeight);
	glUniform2f(mediumFrameSizeUniform, (GLfloat)medium->width, (GLfloat)medium->height);
	glUniform2f(mediumFrameOffsetUniform, (GLfloat)medium->frameOffsetX, (GLfloat)medium->frameOffsetY);
	glUniform2f(mediumImageSizeUniform, medium->imageWidth, medium->imageHeight);
	// Set the transform uniform variables to the medium sprite.
	updateSpriteUniformState(medium);
	
	// Now that all the uniforms are set up, we can call
	// our drawing function.
	drawSquare();
	
	// State-persistence time!
	glActiveTexture(GL_TEXTURE0+0);
	glBindTexture(GL_TEXTURE_2D, RS_NULL_TEXTURE);
	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, RS_NULL_TEXTURE);
	glUseProgram(RS_NULL_PROGRAM);
	glBindFramebuffer(GL_FRAMEBUFFER, RS_NULL_FRAMEBUFFER);
	
}

void RS_renderSpriteToScreen(RS_Sprite * sprite)
{
	// Make sure that we are using the main framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, RS_NULL_FRAMEBUFFER);
	
	// Set up the first texture object slot so we
	// can use the sprite's texture.
	glActiveTexture(GL_TEXTURE0+0);
	glBindTexture(GL_TEXTURE_2D, sprite->tex);
	glUniform1i(canvasTextureUniform, 0);
	
	// As a compatibility thing we also send in
	// the sprite's texture to the other texture
	// unit. This way the shader has nothing to
	// worry about.
	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, sprite->tex);
	glUniform1i(mediumTextureUniform, 1);
	
	// Get the width and height of the window.
	GLuint data[4];	// Window X, Y, width and height.
	glGetIntegerv(GL_VIEWPORT, data);
	glUniform2f(canvasFrameSizeUniform, (GLfloat)data[2], (GLfloat)data[3]);
	glUniform2f(canvasFrameOffsetUniform, 0.0, 0.0);
	glUniform2f(mediumFrameSizeUniform, (GLfloat)sprite->width, (GLfloat)sprite->height);
	glUniform2f(mediumFrameOffsetUniform, (GLfloat)sprite->frameOffsetX, (GLfloat)sprite->frameOffsetY);
	
	// Still have to set the mix uniform, but since
	// the canvas and medium textures are the same,
	// the value doesn't really matter.
	glUniform1f(mixUniform, .5);
	
	// Don't forget to tell the shader all about how
	// to manipulate the sprite.
	updateSpriteUniformState(sprite);
	
	// Now that all the uniforms are set up, we can call
	// our drawing function.
	drawSquare();
	
	// State-persistence time!
	glActiveTexture(GL_TEXTURE0+0);
	glBindTexture(GL_TEXTURE_2D, RS_NULL_TEXTURE);
	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, RS_NULL_TEXTURE);
	glUseProgram(RS_NULL_PROGRAM);
	glBindFramebuffer(GL_FRAMEBUFFER, RS_NULL_FRAMEBUFFER);
}

void RS_beginRenderToSprite(RS_Sprite * sprite)
{
	// Bind to the framebuffer of the canvas RS_Sprite, so the
	// rendering pipeline outputs into its texture.
	glBindFramebuffer(GL_FRAMEBUFFER, sprite->fbo);
	// We don't have a depth texture or renderbuffer.
	glDisable(GL_DEPTH_TEST);
}

void RS_endRenderToSprite(RS_Sprite * sprite)
{
	// Bind back to the normal framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, RS_NULL_FRAMEBUFFER);
}

GLuint RS_getTexture(RS_Sprite * sprite)
{
	return sprite->tex;
}

GLuint RS_getFBO(RS_Sprite * sprite)
{
	return sprite->fbo;
}

GLuint RS_getWidth(RS_Sprite * sprite)
{
	return sprite->width;
}

GLuint RS_getHeight(RS_Sprite * sprite)
{
	return sprite->height;
}

GLfloat RS_getRot(RS_Sprite * sprite)
{
	return sprite->rotation;
}

GLint RS_getXPos(RS_Sprite * sprite)
{
	return sprite->posX;
}

GLint RS_getYPos(RS_Sprite * sprite)

{
	return sprite->posY;
}

GLfloat RS_getXScale(RS_Sprite * sprite)
{
	return sprite->scaleX;
}

GLfloat RS_getYScale(RS_Sprite * sprite)
{
	return sprite->scaleY;
}

RS_Color * RS_getTint(RS_Sprite * sprite)
{
	return sprite->tint;
}

GLfloat * RS_getTexelData(RS_Sprite * sprite)
{
	// Allocate data to store what the GPU gives us.
	GLfloat * data;
	if(sprite->format == RS_RGB)
		data = malloc(sizeof(GLfloat)*sprite->width*sprite->height*3);
	else
		data = malloc(sizeof(GLfloat)*sprite->width*sprite->height*4);
		
	// Make sure that we are reading from the correct framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, sprite->fbo);
	// It's kind of like palm reading, but with VRAM.
	glReadPixels(0,	// Top left of rectangle to read out. (x)
				0,	// Top left of rectangle to read out. (y)
				sprite->width,	// The width of that rectangle.
				sprite->height,	// The height of that rectangle.
				sprite->format,	// The format of that data that we're expecting.
				GL_FLOAT,	// The type of that data.
				data);	// A container for this frame data.
	return data;
}

GLfloat * RS_getTexelGroup(RS_Sprite * sprite, GLuint x, GLuint y, GLuint width, GLuint height)
{
	GLfloat * data;
	if(sprite->format == RS_RGB)
		data = malloc(sizeof(GLfloat)*(width-x)*(height-y)*3);
	else
		data = malloc(sizeof(GLfloat)*(width-x)*(height-y)*4);

	glBindFramebuffer(GL_FRAMEBUFFER, sprite->fbo);
	// Oh look now you get to specify the parameters to glReadPixels().
	glReadPixels(x,
				y,
				width,
				height,
				sprite->format,
				GL_FLOAT,
				data);
	glBindFramebuffer(GL_FRAMEBUFFER, RS_NULL_FRAMEBUFFER);
	return data;
}

void RS_getColorAt(RS_Color * container, RS_Sprite * sprite, GLuint x, GLuint y)
{
	// Get a single pixel of color data.
	GLfloat * data = RS_getTexelGroup(sprite, x, y, 1, 1);
	// Stuff it into the container RS_Color.
	container->r = data[0];
	container->g = data[1];
	container->b = data[2];
	// If the sprite is merely RGB, then we wouldn't have
	// gotten a fourth term from RS_getPixelData().
	if(sprite->format == RS_RGBA)
		container->a = data[3];
	else
		container->a = 1.0;
		
	// Free that pixel from the wrath of definition.
	free(data);
}

GLfloat RS_getRedAt(RS_Sprite * sprite, GLuint x, GLuint y)
{
	// Get that same single pixel.
	GLfloat * data = RS_getTexelGroup(sprite, x, y, 1, 1);
	// Store its red value.
	GLfloat red = data[0];
	// Rid ourselves of this filth.
	free(data);
	// Give back to the community.
	return red;
}

GLfloat RS_getGreenAt(RS_Sprite * sprite, GLuint x, GLuint y)
{
	// Hmm.
	GLfloat * data = RS_getTexelGroup(sprite, x, y, 1, 1);
	GLfloat green = data[1];
	free(data);
	return green;
}
	
GLfloat RS_getBlueAt(RS_Sprite * sprite, GLuint x, GLuint y)
{
	// This leaves a familiar taste in my mouth.
	GLfloat * data = RS_getTexelGroup(sprite, x, y, 1, 1);
	GLfloat blue = data[2];
	free(data);
	return blue;
}

GLfloat RS_getAlphaAt(RS_Sprite * sprite, GLuint x, GLuint y)
{
	// So if the sprite has a strictly RGB format, the fourth
	// term will be either null or the next red, depending on
	// how glReadPixels() is implemented. So therefore we
	// have to check to make sure that 
	// Things aren't quite so opaque.
	GLfloat * data = RS_getTexelGroup(sprite, x, y, 1, 1);
	GLfloat alpha = data[3];
	free(data);
	return alpha;
}