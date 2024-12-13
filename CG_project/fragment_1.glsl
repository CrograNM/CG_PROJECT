#version 330 core
in vec3 FragPos;      // 버텍스 셰이더에서 받아온 위치 값
in vec3 Normal;       // 버텍스 셰이더에서 받아온 법선 벡터
in vec3 VertexColor;  // 버텍스 셰이더에서 받아온 색상 (사용 안 할 시 삭제 가능)

out vec4 FragColor;

uniform vec3 lightPos;      // 기존 조명(점광원) 위치
uniform vec3 lightColor;    // 기존 조명 색상

uniform vec3 viewPos;       // 관찰자(카메라) 위치
uniform vec3 objectColor;
uniform vec3 ambientLight;

// 헤드라이트(두 번째 광원) 관련 uniform
uniform vec3 headLightPos;       // 헤드라이트 위치
uniform vec3 headLightColor;     // 헤드라이트 색상
uniform vec3 headLightDir;       // 헤드라이트 방향 (자동차 전방)
uniform float headLightCutOff = cos(radians(15.0));   // 내부 컷오프(라디안)
uniform float headLightOuterCutOff = cos(radians(20.0)); // 외부 컷오프(라디안), 외곽 부드럽게

void main()
{
    // -------------------------
    // 기존 광원 계산
    // Ambient Lighting (주변광)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    vec3 ambient = ambientLight * lightColor;

    // Diffuse Lighting (난반사)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular Lighting (거울반사)
    float shininess = 128.0;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;


    // -------------------------
    // 헤드라이트(스포트라이트) 계산
    // 스포트라이트는 headLightPos에서 FragPos로 향하는 벡터와 headLightDir 사이의 각도를 비교
    vec3 spotLightDir = normalize(headLightPos - FragPos);
    float theta = dot(spotLightDir, normalize(-headLightDir));

    // 컷오프 각도에 따라 빛이 닿는지 판단
    // headLightCutOff: 스포트라이트 중심 각도
    // headLightOuterCutOff: 그 외곽의 부드러운 경계
    float epsilon = headLightCutOff - headLightOuterCutOff;
    float intensity = clamp((theta - headLightOuterCutOff) / epsilon, 0.0, 1.0);

    // Diffuse for headLight
    float diffHL = max(dot(norm, spotLightDir), 0.0);
    vec3 diffuseHL = diffHL * headLightColor;

    // Specular for headLight
    vec3 reflectDirHL = reflect(-spotLightDir, norm);
    float specHL = pow(max(dot(viewDir, reflectDirHL), 0.0), shininess);
    vec3 specularHL = specHL * headLightColor;

    // ambient는 굳이 헤드라이트에 추가하지 않아도 됨(스포트라이트는 주로 특정 방향성 빛)
    // 필요하면 ambientHL = ambientLight * headLightColor; 추가 가능
    vec3 headLightResult = (diffuseHL + specularHL) * intensity * objectColor;

    // -------------------------
    // 두 광원 합산
    vec3 finalColor = result + headLightResult;

    FragColor = vec4(finalColor, 1.0);
}
