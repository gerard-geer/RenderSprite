#version 120
#define SWAP_SENSITIVITY .0001
#define MAX_PALETTE_ENTRIES 256

uniform vec4 tint;
uniform float canvasMediumMix;
uniform sampler2D canvas;
uniform sampler2D medium;

uniform vec4[MAX_PALETTE_ENTRIES] paletteAKeys;
uniform vec4[MAX_PALETTE_ENTRIES] paletteAEntries;
uniform int numPaletteA;

uniform vec4[MAX_PALETTE_ENTRIES] paletteBKeys;
uniform vec4[MAX_PALETTE_ENTRIES] paletteBEntries;
uniform int numPaletteB;

uniform float swapHeight;

varying vec2 canvasUV;
varying vec2 mediumUV;

bool compare(vec4 a, vec4 b, float variance)
{
	for(int i = 0; i < 4; i++)
	{
		if( abs(a[i]-b[i]) > variance )
			return false;
	}
	return true;
}

void attemptSwap(inout vec4 subject, 
				in vec4 keys[MAX_PALETTE_ENTRIES], 
				in vec4 entries[MAX_PALETTE_ENTRIES], 
				in int numEntries)
{
	for(int i = 0; i < numEntries; i ++)
	{
		if(compare(subject, keys[i], SWAP_SENSITIVITY))
		{
			subject = entries[i];
			return;
		}
	}
}

void main(void)
{
	vec4 canvasTexel = texture2D(canvas, canvasUV);
	vec4 mediumTexel = texture2D(medium, mediumUV);
	
	if(numPaletteA > 0 && numPaletteB > 0)
	{
		if(gl_FragCoord.y < swapHeight)
			attemptSwap(mediumTexel, paletteAKeys, paletteAEntries, numPaletteA);
		else
			attemptSwap(mediumTexel, paletteBKeys, paletteBEntries, numPaletteB);
	}
	else if(numPaletteA > 0)
		attemptSwap(mediumTexel, paletteAKeys, paletteAEntries, numPaletteA);
	else if(numPaletteB > 0)
		attemptSwap(mediumTexel, paletteBKeys, paletteBEntries, numPaletteB);
		
	gl_FragColor = mix(canvasTexel, mediumTexel, canvasMediumMix);
	gl_FragColor *= tint;
}