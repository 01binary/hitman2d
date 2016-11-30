/*------------------------------------------------------------------*\
|
| blur.fx
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D blur effect
| Created: 12/09/2010
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Static Constants
\*----------------------------------------------------------*/

// Tap locations for unit disc (blur algorithm), values taken from NVIDIA paper
// http://developer.download.nvidia.com/whitepapers/2008/PCSS_Integration.pdf

static float2 POISSON_DISC[16] = {
	float2( -0.94201624, -0.39906216 ),
	float2( 0.94558609, -0.76890725 ),
	float2( -0.094184101, -0.92938870 ),
	float2( 0.34495938, 0.29387760 ),
	float2( -0.91588581, 0.45771432 ),
	float2( -0.81544232, -0.87912464 ),
	float2( -0.38277543, 0.27676845 ),
	float2( 0.97484398, 0.75648379 ),
	float2( 0.44323325, -0.97511554 ),
	float2( 0.53742981, -0.47373420 ),
	float2( -0.26496911, -0.41893023 ),
	float2( 0.79197514, 0.19090188 ),
	float2( -0.24188840, 0.99706507 ),
	float2( -0.81409955, 0.91437590 ),
	float2( 0.19984126, 0.78641367 ),
	float2( 0.14383161, -0.14100790 ) };

/*----------------------------------------------------------*\
| External Constants
\*----------------------------------------------------------*/

extern float4x4 wvp: WORLDVIEWPROJ;
extern texture base: BASETEXTURE;
extern float2 texelsize;
extern float factor;

/*----------------------------------------------------------*\
| Samplers
\*----------------------------------------------------------*/

sampler2D samp = sampler_state
{
   Texture = base;
   ADDRESSU = CLAMP;
   ADDRESSV = CLAMP;
   MINFILTER = LINEAR;
   MAGFILTER = LINEAR;
};

/*----------------------------------------------------------*\
| Structures
\*----------------------------------------------------------*/

// Blend Texture Technique

struct VS_INPUT_BLUR
{
   float2 position: POSITION0;
   float2 texcoord: TEXCOORD0;
};

struct VS_OUTPUT_BLUR
{
   float4 position: POSITION0;
   float2 texcoord: TEXCOORD0;
};

struct PS_INPUT_BLUR
{
   float2 texcoord : TEXCOORD0;
};

/*----------------------------------------------------------*\
| Blurmix Shader
\*----------------------------------------------------------*/

// Vertex Shader

VS_OUTPUT_BLUR vs_blur(VS_INPUT_BLUR input)
{
	VS_OUTPUT_BLUR output;

	output.position = mul(float4(input.position.x,
		input.position.y, 0, 1), wvp);

	output.texcoord = input.texcoord;

	return output;
}

// Pixel Shader

float4 ps_blur(PS_INPUT_BLUR input) : COLOR0
{
	// Take a sample at the disc’s center

	float4 sampleAccum = tex2D( samp, input.texcoord );

	// Take samples in poisson disc

	for ( int nTapIndex = 0; nTapIndex < 16; nTapIndex++ )
	{
		// Compute new texture coord inside disc

		float2 tapCoord = input.texcoord + texelsize * POISSON_DISC[nTapIndex] * factor;

		// Accumulate samples

		sampleAccum += tex2D( samp, tapCoord );
	}	

	return sampleAccum / 17.0;
}

/*----------------------------------------------------------*\
| Techniques
\*----------------------------------------------------------*/

technique blur
{
	pass p0
	{
		VertexShader = compile vs_2_0 vs_blur();
		PixelShader = compile ps_2_0 ps_blur();
	}
}