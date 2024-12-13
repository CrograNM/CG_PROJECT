
#version 330 core
in vec3 FragPos;      // 버텍스 셰이더에서 받아온 위치 값
in vec3 Normal;       // 버텍스 셰이더에서 받아온 법선 벡터
in vec3 VertexColor;  // 버텍스 셰이더에서 받아온 색상

out vec4 FragColor;

uniform vec3 lightPos;      // 조명의 위치
uniform vec3 lightColor;    // 조명의 색상
uniform vec3 viewPos;       // 관찰자의 위치
uniform vec3 objectColor;
uniform vec3 ambientLight;

void main()
{
    // Ambient Lighting (주변광)
    vec3 ambient = ambientLight * lightColor;

    // Diffuse Lighting (난반사광)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular Lighting (거울반사광)
    float shininess = 128.0;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * lightColor;

    // 최종 색상 계산
    //vec3 result = (ambient + diffuse + specular) * VertexColor;
    vec3 result = (ambient + diffuse+specular) * objectColor;
    FragColor = vec4(result, 1.0);
}

