#version 330 core
in vec3 FragPos;      // ���ؽ� ���̴����� �޾ƿ� ��ġ ��
in vec3 Normal;       // ���ؽ� ���̴����� �޾ƿ� ���� ����
in vec3 VertexColor;  // ���ؽ� ���̴����� �޾ƿ� ���� (��� �� �� �� ���� ����)

out vec4 FragColor;

uniform vec3 lightPos;      // ���� ����(������) ��ġ
uniform vec3 lightColor;    // ���� ���� ����

uniform vec3 viewPos;       // ������(ī�޶�) ��ġ
uniform vec3 objectColor;
uniform vec3 ambientLight;

// ������Ʈ(�� ��° ����) ���� uniform
uniform vec3 headLightPos;       // ������Ʈ ��ġ
uniform vec3 headLightColor;     // ������Ʈ ����
uniform vec3 headLightDir;       // ������Ʈ ���� (�ڵ��� ����)
uniform float headLightCutOff = cos(radians(15.0));   // ���� �ƿ���(����)
uniform float headLightOuterCutOff = cos(radians(20.0)); // �ܺ� �ƿ���(����), �ܰ� �ε巴��

void main()
{
    // -------------------------
    // ���� ���� ���
    // Ambient Lighting (�ֺ���)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    vec3 ambient = ambientLight * lightColor;

    // Diffuse Lighting (���ݻ�)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular Lighting (�ſ�ݻ�)
    float shininess = 128.0;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;


    // -------------------------
    // ������Ʈ(����Ʈ����Ʈ) ���
    vec3 spotLightDir = normalize(headLightPos - FragPos);
    float theta = dot(spotLightDir, normalize(-headLightDir));

    // �ƿ��� ������ ���� ���� ����� �Ǵ�
    float epsilon = headLightCutOff - headLightOuterCutOff;
    float intensity = clamp((theta - headLightOuterCutOff) / epsilon, 0.0, 1.0);

    float diffHL = max(dot(norm, spotLightDir), 0.0);
    vec3 diffuseHL = diffHL * headLightColor;

    vec3 reflectDirHL = reflect(-spotLightDir, norm);
    float specHL = pow(max(dot(viewDir, reflectDirHL), 0.0), shininess);
    vec3 specularHL = specHL * headLightColor;

    // ambient�� ���� ������Ʈ�� �߰����� �ʾƵ� ��(����Ʈ����Ʈ�� �ַ� Ư�� ���⼺ ��)
    vec3 headLightResult = (diffuseHL + specularHL) * intensity * objectColor;
    // -------------------------

    // �� ���� �ջ�
    vec3 finalColor = result + headLightResult;

    FragColor = vec4(finalColor, 1.0);
}
