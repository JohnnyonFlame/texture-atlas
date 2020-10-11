#version 100

uniform mat4 uMVPMat;      //Vanilla model view projection matrix
uniform mat4 uMVMat_trinv; //Transposed inverse of the model view projection matrix

attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec3 aTangent;
attribute vec2 aTextureCoords;

varying mat3 vTBN;
varying vec2 vTextureCoords;

void main()
{
    vTextureCoords = aTextureCoords;
    vec3 vNormal = vec3(uMVMat_trinv * vec4(aNormal, 0.0));
    vec3 vTangent = vec3(uMVMat_trinv * vec4(aTangent, 0.0));
    vec3 vBitangent = cross(vNormal, vTangent);
    vTBN = mat3(vTangent, vBitangent, vNormal);
    gl_Position = uMVPMat * vec4(aPosition, 1.0);
}