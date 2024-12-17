
#version 330 core
layout (location = 0) in vec3 vPos;      // 버텍스 위치
layout (location = 1) in vec3 vNormal;   // 버텍스 법선
layout (location = 2) in vec3 vColor;    // 버텍스 색상

out vec3 FragPos;        // 프래그먼트 셰이더로 전달될 위치 값
out vec3 Normal;         // 프래그먼트 셰이더로 전달될 법선 벡터
out vec3 VertexColor;    // 프래그먼트 셰이더로 전달될 색상

uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;


void main()
{
    gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPos, 1.0);
    FragPos = vec3(modelTransform * vec4(vPos, 1.0));
    
    // 법선 변환 (법선 행렬 사용)
    mat3 normalMatrix = transpose(inverse(mat3(modelTransform)));
    Normal = normalize(normalMatrix * vNormal);

    // 색상 전달
    VertexColor = vColor;
}
