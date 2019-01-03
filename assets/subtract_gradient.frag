#version 400

in vec2	v_TexCoord0;

uniform sampler2D u_Vel;
uniform sampler2D u_Pressure;

uniform vec2 u_CellSize;
uniform vec2 u_HalfDivCellSize;

out vec4 Out;

void main()
{
    vec2 p = v_TexCoord0;
    
    float diffX = texture(u_Pressure, p + vec2(1.0, 0.0) * u_CellSize).x
                - texture(u_Pressure, p - vec2(1.0, 0.0) * u_CellSize).x;
    float diffY = texture(u_Pressure, p + vec2(0.0, 1.0) * u_CellSize).x
                - texture(u_Pressure, p - vec2(0.0, 1.0) * u_CellSize).x;
    
    Out = texture(u_Vel, p);
    Out.xy -= u_HalfDivCellSize * vec2(diffX, diffY) * 4.0;
}
