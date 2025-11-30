#version 120
varying vec3 normal;
varying vec3 position;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    normal = normalize(gl_NormalMatrix * gl_Normal);
    position = vec3(gl_ModelViewMatrix * gl_Vertex);
    gl_TexCoord[0] = gl_MultiTexCoord0;
}
