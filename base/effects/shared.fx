/*------------------------------------------------------------------*\
|
| shared.fx
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D shared effect
| Created: 12/22/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Extern Constants
\*----------------------------------------------------------*/

extern float4x4 wvp: WORLDVIEWPROJ;
extern texture base: BASETEXTURE;

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

struct VS_INPUT_BLENDTEXTURE
{
   float2 position: POSITION0;
   float4 blend: COLOR0;
   float2 texcoord: TEXCOORD0;
};

struct VS_OUTPUT_BLENDTEXTURE
{
   float4 position: POSITION0;
   float4 blend: COLOR0;
   float2 texcoord: TEXCOORD0;
};

struct PS_INPUT_BLENDTEXTURE
{
   float4 blend: COLOR0;
   float2 texcoord : TEXCOORD0;
};

// Point Texture Technique

struct VS_INPUT_POINTTEXTURE
{
   float3 position: POSITION0;
   float4 blend: COLOR0;
   float size: PSIZE;
};

struct VS_OUTPUT_POINTTEXTURE
{
   float4 position: POSITION0;
   float4 blend: COLOR0;
   float2 texcoord: TEXCOORD0;
   float size: PSIZE;   
};

struct PS_INPUT_POINTTEXTURE
{
   float4 blend: COLOR0;
   float2 texcoord: TEXCOORD0;
};

// Blend Technique

struct VS_INPUT_BLEND
{
   float2 position: POSITION0;
   float4 blend: COLOR0;
};

struct VS_OUTPUT_BLEND
{
   float4 position: POSITION0;
   float4 blend: COLOR0;
};

struct PS_INPUT_BLEND
{
   float4 blend: COLOR0;
};

/*----------------------------------------------------------*\
| Blend/Texture Shader
\*----------------------------------------------------------*/

// Vertex Shader

VS_OUTPUT_BLENDTEXTURE vs_blendtexture(VS_INPUT_BLENDTEXTURE input)
{
	VS_OUTPUT_BLENDTEXTURE output;

	output.position = mul(float4(input.position.x,
		input.position.y, 0, 1), wvp);

	output.blend = input.blend;

	output.texcoord = input.texcoord;

	return output;
}

// Pixel Shader

float4 ps_blendtexture(PS_INPUT_BLENDTEXTURE input) : COLOR0
{
	return input.blend * tex2D(samp, input.texcoord);
}

/*----------------------------------------------------------*\
| Blend Shader
\*----------------------------------------------------------*/

// Vertex Shader

VS_OUTPUT_BLEND vs_blend(VS_INPUT_BLEND input)
{
	VS_OUTPUT_BLEND output;

	output.position = mul(float4(input.position.x,
		input.position.y, 0, 1), wvp);

	output.blend = input.blend;

	return output;
}

// Pixel Shader

float4 ps_blend(PS_INPUT_BLEND input) : COLOR0
{
	return input.blend;
}

/*----------------------------------------------------------*\
| PointTexture Shader
\*----------------------------------------------------------*/

// Vertex Shader

VS_OUTPUT_POINTTEXTURE vs_pointtexture(VS_INPUT_POINTTEXTURE input)
{
	VS_OUTPUT_POINTTEXTURE output;

	output.position = mul(float4(input.position.x,
		input.position.y, input.position.z, 1), wvp);

	output.blend = input.blend;

	output.texcoord = float2(0, 0);

	output.size = input.size;

	return output;
}

// Pixel Shader

float4 ps_pointtexture(PS_INPUT_POINTTEXTURE input) : COLOR0
{
	return input.blend * tex2D(samp, input.texcoord);
}

/*----------------------------------------------------------*\
| Techniques
\*----------------------------------------------------------*/

technique blendtexture
{
	pass p0
	{
		VertexShader = compile vs_2_0 vs_blendtexture();
		PixelShader = compile ps_2_0 ps_blendtexture();
	}
}

technique blend
{
	pass p0
	{
		VertexShader = compile vs_2_0 vs_blend();
		PixelShader = compile ps_2_0 ps_blend();
	}
}

technique wireframe
{
	pass p0
	{
		FillMode = WIREFRAME;

		VertexShader = compile vs_2_0 vs_blend();
		PixelShader = compile ps_2_0 ps_blend();
	}
}

technique pointtexture
{
	pass p0
	{
		SrcBlend = One;
		DestBlend = One;

		PointSpriteEnable = true;
		PointScaleEnable = false;

		VertexShader = compile vs_2_0 vs_pointtexture();
		PixelShader = compile ps_2_0 ps_pointtexture();
	}
}