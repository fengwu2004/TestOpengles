#version 300 es
layout(location = 0)in vec4    aPosition;
layout(location = 1)in vec4    aColor;
uniform mat4                   uProjMat;
uniform mat4                   uMapMat;
uniform mat4                   uObjMat;
out vec4                       vColor;
void main() {
  gl_Position = uProjMat*uMapMat*uObjMat*aPosition;
  vColor = aColor;                                 
}