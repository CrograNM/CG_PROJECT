#version 330 core
//--- in_Position: attribute index 0 
//--- in_Color: attribute index 1

layout (location = 0) in vec3 vPos;	//--- 위치 변수: attribute position 0
layout (location = 1) in vec3 vColor;		//--- 컬러 변수: attribute position 1

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 passColor;		//--- 프래그먼트 세이더에게 전달

void main(void) 
{
	gl_Position = projection * view * model * vec4(vPos, 1.0);
	//vec4 (vPos.x, vPos.y, vPos.z, 1.0);
	passColor = vColor;
}
