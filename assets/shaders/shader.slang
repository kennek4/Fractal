struct VertexInput {
    float2 inPos;
    float3 inColor;
};

struct VertexOutput {
    float4 position : SV_Position;
    float3 color;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input) {
    VertexOutput output;
    output.position = float4(input.inPos, 0.0, 1.0);
    output.color    = input.inColor;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
    float3 color = inVert.color;
    return float4(color, 1.0);
}

