#version 100

precision highp float;
uniform sampler2D uTexture0;
uniform sampler2D uTexture1;
uniform vec3 uDiffuseDirection;

varying mat3 vTBN;
varying vec2 vTextureCoords;

void main()
{
    vec3 normal = vTBN[2];
    vec3 rgb_norm = texture2D(uTexture1, vTextureCoords).rgb;
    rgb_norm = normalize(vTBN * (rgb_norm * 2.0 - 1.0));

    mediump float coeff = 1.0 - max(dot(rgb_norm, uDiffuseDirection), 0.0) * 1.0;
    vec4 tex = texture2D(uTexture0, vTextureCoords);

    //Not necessary anymore - ALPHA_TO_COVERAGE rules
    //if (tex.a < 0.1) discard;
    gl_FragColor = vec4(vec3(tex) * coeff, tex.a);
}   