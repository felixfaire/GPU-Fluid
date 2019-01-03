#version 400

in vec2	v_TexCoord0;

uniform sampler2D u_Vel;

uniform vec2  u_CellSize;

out vec4 Out;

float curl2D(vec2 p)
{
    float dudy = texture(u_Vel, p + vec2(0.0, u_CellSize.y)).x - texture(u_Vel, p - vec2(0.0, u_CellSize.y)).x;
    float dvdx = texture(u_Vel, p + vec2(u_CellSize.x, 0.0)).y - texture(u_Vel, p - vec2(u_CellSize.x, 0.0)).y;

    return (dudy - dvdx) * 0.5;
}

void main()
{
    vec2 p = v_TexCoord0;
    float edge = (1.0 - step(1.0 - u_CellSize.x * 2.0, abs(p.x - 0.5) * 2.0))
               * (1.0 - step(1.0 - u_CellSize.y * 2.0, abs(p.y - 0.5) * 2.0));
    
    float curl = curl2D(v_TexCoord0);
    float curlLength = abs(curl);
    

    Out = vec4(curl * edge, curlLength * edge, 0.0, 0.0);
}
