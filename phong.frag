#version 120
varying vec3 normal;
varying vec3 position;

uniform vec3 lightPosition;
uniform vec3 lightColor;

uniform float shininess;

uniform sampler2D textureSampler;

void main()
{
    vec3 N = normalize(normal);
    vec3 L = normalize(lightPosition - position);
    vec3 V = normalize(-position);
    vec3 R = reflect(-L, N);

    vec3 ambientColor = vec3(0.2, 0.2, 0.2);
    vec3 ambient = ambientColor * lightColor;

    vec3 diffuseColor = vec3(gl_Color.rgb);
    float diffIntensity = max(dot(N, L), 0.0);
    vec3 diffuse = diffIntensity * diffuseColor * lightColor;
    vec3 specularColor = vec3(1.0, 1.0, 1.0);
    float specIntensity = pow(max(dot(V, R), 0.0), shininess);
    vec3 specular = specularColor * specIntensity * lightColor;

    vec4 texColor = texture2D(textureSampler, gl_TexCoord[0].st);
    
    if (texColor.a == 0.0 || textureSampler == 0) {
        texColor = gl_Color;
    }
    
    vec3 finalLighting = ambient + diffuse + specular;
    gl_FragColor = vec4(finalLighting * texColor.rgb, texColor.a);
}
