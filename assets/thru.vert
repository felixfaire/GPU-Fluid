#version 400


uniform mat4 ciModelViewProjection;

in vec4 ciPosition;
in vec2 ciTexCoord0;

out vec2 v_TexCoord0;

void main()
{
    // Simply pass the texture coordinate and the position.
    v_TexCoord0 = ciTexCoord0;
    gl_Position = ciModelViewProjection * ciPosition;
}
