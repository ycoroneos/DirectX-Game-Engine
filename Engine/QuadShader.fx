Texture2D ColorTexture : register( t1 );

SamplerState SampleType : register( s0 );

//////////////
// TYPEDEFS //
//////////////

struct PSInQuad
{
	float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;	
};

struct DummyInput
{
};

///////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
DummyInput QuadVertexShader(float4 input:POSITION) : SV_POSITION
{
    DummyInput output;
	return output;
}

////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////

[maxvertexcount(4)]
void GSMainQuad( point DummyInput inputPoint[1], inout TriangleStream<PSInQuad> outputQuad, uint primitive : SV_PrimitiveID )
{
    PSInQuad output;
    
    output.position.z = 0.5;
    output.position.w = 1.0;
    
    output.position.x = -1.0;
    output.position.y = 1.0;
    output.texCoord.xy = float2( 0.0, 0.0 );
    outputQuad.Append( output );
    
    output.position.x = 1.0;
    output.position.y = 1.0;
    output.texCoord.xy = float2( 1.0, 0.0 );
    outputQuad.Append( output );
    
    output.position.x = -1.0;
    output.position.y = -1.0;
    output.texCoord.xy = float2( 0.0, 1.0 );
    outputQuad.Append( output );
        
    output.position.x = 1.0;
    output.position.y = -1.0;
    output.texCoord.xy = float2( 1.0, 1.0 );
    outputQuad.Append( output );
    
    outputQuad.RestartStrip();
}
/////////////
// GLOBALS //
/////////////

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 QuadPixelShader(PSInQuad input ) : SV_TARGET
{   
    return ColorTexture.Sample( SampleType, input.texCoord );
}