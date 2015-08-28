
cbuffer ConstantBuffer : register(b0)
{
  float4 g_Offset;
};

struct VS_IN 
{
  float4 Pos  : POSITION;
  float4 Color: COLOR0;
};

struct VS_OUT
{
  float4 Pos   : SV_POSITION;
  float4 Color : COLOR0;
};

VS_OUT VS_MAIN( VS_IN vs_in)
{
  VS_OUT vs_out;
  vs_in.Pos.w = 1.0f;
  vs_out.Pos = vs_in.Pos + g_Offset;

  vs_out.Color = vs_in.Color;

  return vs_out;
}