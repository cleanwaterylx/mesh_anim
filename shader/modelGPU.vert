#version 330 core
 
#define Max_Bones 58
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexcoord;
layout (location = 2) in vec4 boneWeights;
layout (location = 3) in vec4 boneIndexs;
 
out vec2 sTexcoord;
 
uniform mat4[Max_Bones] boneMatrixs;
 
uniform mat4 model;
uniform mat4 view;
uniform mat4 project;
 
vec4 ComputeVertexByBoneMatrix();
 
void main()
{
    sTexcoord = aTexcoord;
    vec4 pos = ComputeVertexByBoneMatrix();
    gl_Position = project * view * model * pos;
}
 
vec4 ComputeVertexByBoneMatrix()
{
  mat4 matTrans =  boneMatrixs[int(boneIndexs.x)] * boneWeights.x;
  matTrans += boneMatrixs[int(boneIndexs.y)] * boneWeights.y;
  matTrans += boneMatrixs[int(boneIndexs.z)] * boneWeights.z;
  float finalWeight = 1.0f - boneWeights.x - boneWeights.y - boneWeights.z;
  matTrans += boneMatrixs[int(boneIndexs.w)] * finalWeight;
  vec4 pos = matTrans * vec4(aPos,1.0);
  return pos;
}