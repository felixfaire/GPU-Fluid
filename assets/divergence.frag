#version 400

in vec2	v_TexCoord0;

uniform sampler2D u_Vel;
uniform vec2 u_CellSize;
uniform vec2 u_HalfDivCellSize;

out vec4 Out;

void main()
{
    vec2 p = v_TexCoord0;
    
    float diffX = texture(u_Vel, p + vec2(1.0, 0.0) * u_CellSize).x
                - texture(u_Vel, p - vec2(1.0, 0.0) * u_CellSize).x;
    float diffY = texture(u_Vel, p + vec2(0.0, 1.0) * u_CellSize).y
                - texture(u_Vel, p - vec2(0.0, 1.0) * u_CellSize).y;
    float div = u_HalfDivCellSize.x * diffX + u_HalfDivCellSize.y * diffY;
    
    Out = vec4(div, 0.0, 0.0, 0.0);
}
