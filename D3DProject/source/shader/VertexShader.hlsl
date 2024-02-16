struct vertexIn
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct vertexOut
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

vertexOut main(vertexIn input)
{
    vertexOut output;
    
    output.pos = float4(input.pos, 1.0f);
    output.color = input.color;
    
    return output;
}
