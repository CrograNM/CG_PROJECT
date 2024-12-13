//링커-명령줄 : glew32.lib freeglut.lib
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gl/glew.h>			
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <random>
#include <vector>

#include <ctime>
#include <string>

time_t startTime;
time_t pauseTime;
time_t tempTime;
int elapsedSeconds = 0;
bool crushed = false;
bool invincible = false;

#define M_PI 3.14159265358979323846

// 클라이언트
#define clientWidth 900
#define clientHeight 600
GLfloat rColor = 1.0;
GLfloat gColor = 1.0;
GLfloat bColor = 1.0;

// 랜덤 실수값(min ~ max) 반환 함수
std::random_device rd;
std::mt19937 gen(rd()); // Mersenne Twister 엔진
float generateRandomFloat(float min, float max)
{
	std::uniform_real_distribution<float> dis(min, max);	// 인자로 범위 설정
	return dis(gen);
}

// 타이머 관련
#define TIMER_VELOCITY 16

// 투영, 은면
bool isProspect = true;		
bool isCull = false;

// 도형 관련
#define TRI_COUNT 12	//육면체의 삼각형 개수 : 12

// Car 초기화
void initCar();
#define CAR_SIZE 0.5f
GLfloat Block[4][TRI_COUNT * 3][3];
GLfloat	Block_Color[4][TRI_COUNT * 3][3];

void initObstacleCar();
GLfloat obstacle_car[TRI_COUNT * 3][3];
GLfloat obstacle_car_color[TRI_COUNT * 3][3];


// 핸들 초기화
#define HANDLE_SIZE 0.7f
#define HAND_RECT_SIZE HANDLE_SIZE / 4
GLfloat handle_rect[6][3] = {
	{-HAND_RECT_SIZE, 0, -HAND_RECT_SIZE}, {HAND_RECT_SIZE, 0, -HAND_RECT_SIZE}, {-HAND_RECT_SIZE, 0, HAND_RECT_SIZE},
	{-HAND_RECT_SIZE, 0, HAND_RECT_SIZE},  {HAND_RECT_SIZE, 0, -HAND_RECT_SIZE}, { HAND_RECT_SIZE, 0, HAND_RECT_SIZE}
};
GLfloat handle_rect_color[6][3] = {
	{0.5f, 0.5f, 0.5f},
	{0.5f, 0.5f, 0.5f},
	{0.5f, 0.5f, 0.5f},
	   		 	 
	{0.5f, 0.5f, 0.5f},
	{0.5f, 0.5f, 0.5f},
	{0.5f, 0.5f, 0.5f}
};

// 기어 초기화
GLfloat gear_rect[6][3] = {
	{-0.3f, 0, -1.0f}, {0.3f, 0, -1.0f}, {-0.3f, 0, 1.0f},
	{-0.3f, 0, 1.0f}, {0.3f, 0, -1.0f}, {0.3f, 0, 1.0f}
};
GLfloat gear_rect_color[6][3] = {
	{0.5f, 0.5f, 0.5f},
	{0.5f, 0.5f, 0.5f},
	{0.5f, 0.5f, 0.5f},
				 
	{0.5f, 0.5f, 0.5f},
	{0.5f, 0.5f, 0.5f},
	{0.5f, 0.5f, 0.5f}
};

// 기어 봉 초기화
GLfloat gear_stick_rect[6][3] = {
	{-0.1f, 0, -0.1f}, {0.1f, 0, -0.1f}, {-0.1f, 0, 0.1f},
	{-0.1f, 0, 0.1f}, {0.1f, 0, -0.1f}, {0.1f, 0, 0.1f}
};
GLfloat gear_stick_rect_color[6][3] = {
	{0.25f, 0.25f, 0.25f},
	{0.25f, 0.25f, 0.25f},
	{0.25f, 0.25f, 0.25f},
	   		  		 
	{0.25f, 0.25f, 0.25f},
	{0.25f, 0.25f, 0.25f},
	{0.25f, 0.25f, 0.25f}
};

// 텍스트 렌더링 함수
void RenderBitmapString(float x, float y, void* font, const char* string)
{
	glRasterPos2f(x, y);
	while (*string)
	{
		glutBitmapCharacter(font, *string);
		string++;
	}
}

// 바퀴 (육면체) 초기화
#define WHEEL_SIZE CAR_SIZE / 4
#define WHEEL_RECT_SIZE WHEEL_SIZE / 8
GLfloat wheel_rect[4][TRI_COUNT * 3][3];
GLfloat wheel_rect_color[4][TRI_COUNT * 3][3];

// 기어 상태를 나타내는 열거형
enum GearState {
	PARK,	    // P
	REVERSE,    // R
	NEUTRAL,	// N
	DRIVE		// D
};

// 현재 기어 상태를 저장하는 변수
GearState currentGear = DRIVE;

// 장애물
#define OBSTACLE_WIDTH CAR_SIZE * 0.7
#define OBSTACLE_HEIGHT CAR_SIZE * 1.1

// 도착지점
#define FINISH_SIZE 1.0f //(바깥쪽 사각형 크기)
#define FINISH_SIZE_2 0.9f
const float fheight = 0.75f;
const float fy = 0.0001f;
const float fy2 = 0.00015f;
// 도착 지점 위치 설정
float FINISH_OFFSET_X = 0.0f; // X축 오프셋
float FINISH_OFFSET_Z = 0.0f; // Z축 오프셋

GLfloat finish_rect[2][6][3] = {
	{	//바깥쪽 (z길이가 x길이의 두배로 설정)
		{-FINISH_SIZE/2, fy, -FINISH_SIZE * fheight},
		{ FINISH_SIZE/2, fy, -FINISH_SIZE * fheight},
		{-FINISH_SIZE/2, fy,  FINISH_SIZE * fheight},
		{-FINISH_SIZE/2, fy,  FINISH_SIZE * fheight}, 
		{ FINISH_SIZE/2, fy, -FINISH_SIZE * fheight},
		{ FINISH_SIZE/2, fy,  FINISH_SIZE * fheight}
	},
	{	//안쪽
		{-FINISH_SIZE_2 / 2, fy2, -FINISH_SIZE_2 * fheight},
		{ FINISH_SIZE_2 / 2, fy2, -FINISH_SIZE_2 * fheight},
		{-FINISH_SIZE_2 / 2, fy2,  FINISH_SIZE_2 * fheight},
		{-FINISH_SIZE_2 / 2, fy2,  FINISH_SIZE_2 * fheight},
		{ FINISH_SIZE_2 / 2, fy2, -FINISH_SIZE_2 * fheight},
		{ FINISH_SIZE_2 / 2, fy2,  FINISH_SIZE_2 * fheight}
	}
};
GLfloat finish_rect_color[2][6][3] = {
	{	//바깥쪽 (흰색)
		{1.0f, 1.0f, 1.0f},		{1.0f, 1.0f, 1.0f},		{1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f},		{1.0f, 1.0f, 1.0f},		{1.0f, 1.0f, 1.0f}
	},
	{	//안쪽 (회색)
		{0.8f, 0.8f, 0.8f},		{0.8f, 0.8f, 0.8f},		{0.8f, 0.8f, 0.8f},
		{0.8f, 0.8f, 0.8f},		{0.8f, 0.8f, 0.8f},		{0.8f, 0.8f, 0.8f}
	}
};

// 주차 상태를 나타내는 변수
bool isParked = false;
void UpdateParkingStatus(const std::vector<std::pair<float, float>>& carCorners);
// 장식용 주차공간 컬러 데이터
GLfloat not_finish_rect_color[2][6][3] = {
	{	//바깥쪽 (흰색)
		{1.0f, 1.0f, 1.0f},		{1.0f, 1.0f, 1.0f},		{1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f},		{1.0f, 1.0f, 1.0f},		{1.0f, 1.0f, 1.0f}
	},
	{	//안쪽 (회색)
		{0.8f, 0.8f, 0.8f},		{0.8f, 0.8f, 0.8f},		{0.8f, 0.8f, 0.8f},
		{0.8f, 0.8f, 0.8f},		{0.8f, 0.8f, 0.8f},		{0.8f, 0.8f, 0.8f}
	}
};

// 땅바닥 초기화
#define GROUND_SIZE 5.0f
GLfloat ground[6][3] = {
	{-GROUND_SIZE, 0.0f, -GROUND_SIZE}, {GROUND_SIZE, 0.0f, -GROUND_SIZE}, {-GROUND_SIZE, 0.0f, GROUND_SIZE},
	{-GROUND_SIZE, 0.0f, GROUND_SIZE}, {GROUND_SIZE, 0.0f, -GROUND_SIZE}, {GROUND_SIZE, 0.0f, GROUND_SIZE}
};
GLfloat ground_color[6][3] = {
	{0.8f, 0.8f, 0.8f},
	{0.8f, 0.8f, 0.8f},
	{0.8f, 0.8f, 0.8f},

	{0.8f, 0.8f, 0.8f},
	{0.8f, 0.8f, 0.8f},
	{0.8f, 0.8f, 0.8f},
};

// 벽 초기화
#define WALL_HEIGHT 0.5f
#define WALL_THICKNESS 0.1f
GLfloat walls[24][3] = {
	// Front Wall
	{-GROUND_SIZE, 0.0f, -GROUND_SIZE}, {GROUND_SIZE, 0.0f, -GROUND_SIZE}, {-GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE},
	{-GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE}, {GROUND_SIZE, 0.0f, -GROUND_SIZE}, {GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE},
	// Back Wall
	{-GROUND_SIZE, 0.0f, GROUND_SIZE}, {GROUND_SIZE, 0.0f, GROUND_SIZE}, {-GROUND_SIZE, WALL_HEIGHT, GROUND_SIZE},
	{-GROUND_SIZE, WALL_HEIGHT, GROUND_SIZE}, {GROUND_SIZE, 0.0f, GROUND_SIZE}, {GROUND_SIZE, WALL_HEIGHT, GROUND_SIZE},
	// Left Wall
	{-GROUND_SIZE, 0.0f, -GROUND_SIZE}, {-GROUND_SIZE, 0.0f, GROUND_SIZE}, {-GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE},
	{-GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE}, {-GROUND_SIZE, 0.0f, GROUND_SIZE}, {-GROUND_SIZE, WALL_HEIGHT, GROUND_SIZE},
	// Right Wall
	{GROUND_SIZE, 0.0f, -GROUND_SIZE}, {GROUND_SIZE, 0.0f, GROUND_SIZE}, {GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE},
	{GROUND_SIZE, WALL_HEIGHT, -GROUND_SIZE}, {GROUND_SIZE, 0.0f, GROUND_SIZE}, {GROUND_SIZE, WALL_HEIGHT, GROUND_SIZE}
};
GLfloat wall_colors[24][3] = {
	{0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f},
	{0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f},
	{0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f},
	{0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f},
	{0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f},
	{0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f},
	{0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f},
	{0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f}, {0.3f, 0.3f, 0.3f}
};

// 필요 변수 선언
GLint width, height;
GLchar* vertexSource, * fragmentSource;		
GLuint vertexShader, fragmentShader;		
GLuint shaderProgramID;						
GLuint vao[20], vbo[40];						

// 필수 함수 정의
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid KeyboardUp(unsigned char key, int x, int y);
GLvoid SpecialKeyboard(int key, int x, int y);

// 마우스 관련
int lastMouseX = -1, lastMouseY = -1;
bool is_mouse_on_camera = false;
bool is_mouse_on_handle = false;
void MouseMotion(int x, int y);
void MouseButton(int button, int state, int x, int y);

// 세이더 정의
char* filetobuf(const char* file);
void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
GLvoid InitBuffer();

// 기본 적용 변환 - 테스트용
glm::mat4 SRT_MATRIX()
{
	glm::mat4 S = glm::mat4(1.0f);
	//glm::mat4 T = glm::mat4(1.0f);			//--- 이동 행렬 선언
	//glm::mat4 Rx = glm::mat4(1.0f);			//--- 회전 행렬 선언

	S = glm::scale(S, glm::vec3(1.0, 1.0, 1.0));								//--- 스케일 변환 행렬
	//Rx = glm::rotate(Rx, glm::radians(20.0f), glm::vec3(1.0, 0.0, 0.0));		//--- x축 회전 행렬 (고정)
	//T = glm::translate(T, glm::vec3(dx, dy, dz));								//--- 이동 행렬

	return S;
}

// 핸들 변환 - 마우스에 따라 회전 적용
float handle_rotateZ = 0.0f;
float lastAngle = 0.0f;						// 이전 프레임의 각도
float cumulativeAngle = 0.0f;				// 누적된 핸들 회전 각도
glm::mat4 Handle()
{
	glm::mat4 T = glm::mat4(1.0f);			//--- 이동 행렬 선언
	glm::mat4 Rx = glm::mat4(1.0f);			//--- 회전 행렬 선언
	glm::mat4 Rz = glm::mat4(1.0f);			//--- 회전 행렬 선언

	if (true)
	{
		T = glm::translate(T, glm::vec3(0.0, HANDLE_SIZE - HANDLE_SIZE/4, 0.1));
		Rx = glm::rotate(Rx, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
		Rz = glm::rotate(Rz, glm::radians(handle_rotateZ), glm::vec3(0.0, 0.0, 1.0));
	}

	return Rz * T * Rx;
}
glm::mat4 Gear()
{
	glm::mat4 T = glm::mat4(1.0f);			//--- 이동 행렬 선언
	glm::mat4 Rx = glm::mat4(1.0f);			//--- 회전 행렬 선언

	if (true)
	{
		T = glm::translate(T, glm::vec3(0.5, -0.2, 0.1));
		Rx = glm::rotate(Rx, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
	}

	return T * Rx;
}
glm::mat4 PointMode()
{
	glm::mat4 T = glm::mat4(1.0f);			//--- 이동 행렬 선언
	glm::mat4 Rx = glm::mat4(1.0f);			//--- 회전 행렬 선언
	glm::mat4 S = glm::mat4(1.0f);

	if (true)
	{
		S = glm::scale(S, glm::vec3(5.0, 5.0, 5.0));
		Rx = glm::rotate(Rx, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
	}

	return T * Rx * S;
}
glm::mat4 Gear_Stick()
{
	glm::mat4 T = glm::mat4(1.0f);			//--- 이동 행렬 선언
	glm::mat4 Rx = glm::mat4(1.0f);			//--- 회전 행렬 선언

	// 기어 상태에 따라 y좌표 변경
	float gearYOffset = 0.0f;
	switch (currentGear) {
	case PARK:
		gearYOffset = 0.55f;
		break;
	case REVERSE:
		gearYOffset = 0.1f;
		break;
	case NEUTRAL:
		gearYOffset = -0.35f;
		break;
	case DRIVE:
		gearYOffset = -0.75f;
		break;
	}

	if (true)
	{
		T = glm::translate(T, glm::vec3(0.6, gearYOffset, 0.1));
		Rx = glm::rotate(Rx, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
	}

	return T * Rx;
}

// 차체의 변환 - 이를 기준으로 헤드라이트, 바퀴 등의 위치가 정해진다.
float car_dx = 0.0f, car_dy = WHEEL_SIZE, car_dz = -3.0f;
float car_rotateY = 0.0f;
glm::mat4 Car_Body()
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Ry = glm::mat4(1.0f);
	//glm::mat4 S = glm::mat4(1.0f);

	Ry = glm::rotate(Ry, glm::radians(car_rotateY), glm::vec3(0.0, 1.0, 0.0));
	T = glm::translate(T, glm::vec3(car_dx, car_dy, car_dz));

	return SRT_MATRIX() * T * Ry;
}

// 헤드라이트 변환 - 차량의 앞으로 고정
glm::mat4 Headlights(int left_right)
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Ry = glm::mat4(1.0f);
	//glm::mat4 S = glm::mat4(1.0f);

	if (left_right == 0)
	{
		T = glm::translate(T, glm::vec3(-CAR_SIZE / 3, CAR_SIZE / 8, CAR_SIZE));
	}
	else if (left_right == 1)
	{
		T = glm::translate(T, glm::vec3(CAR_SIZE / 3, CAR_SIZE / 8, CAR_SIZE));
	}

	return Car_Body() * T;
}

// 바퀴 변환 - 앞바퀴 회전
float front_wheels_rotateY = 0.0f;
float wheel_rect_rotateX = 0.0f;
glm::mat4 Wheels(int num)
{
	glm::mat4 T2 = glm::mat4(1.0f);
	if (num == 1) //앞 기준 왼쪽 앞
	{
		T2 = glm::translate(T2, glm::vec3(-(CAR_SIZE / 2 + WHEEL_SIZE / 4), 0.0f, CAR_SIZE * 0.5f));
	}
	if (num == 2) //오른쪽 앞
	{
		T2 = glm::translate(T2, glm::vec3(CAR_SIZE / 2 + WHEEL_SIZE / 4, 0.0f, CAR_SIZE * 0.5f));
	}
	if (num == 3) //왼쪽 뒤
	{
		T2 = glm::translate(T2, glm::vec3(-(CAR_SIZE / 2 + WHEEL_SIZE / 4), 0.0f, -CAR_SIZE * 0.5f));
	}
	if (num == 4)  //오른쪽 뒤
	{
		T2 = glm::translate(T2, glm::vec3(CAR_SIZE / 2 + WHEEL_SIZE / 4, 0.0f, -CAR_SIZE * 0.5f));
	}
	return Car_Body() * T2;
}
glm::mat4 Wheel_rects(int num)
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 T2 = glm::mat4(1.0f);
	glm::mat4 Ry = glm::mat4(1.0f);

	if (num == 1 || num == 2)
	{
		//앞바퀴들에게 회전 변환 추가 적용
		Ry = glm::rotate(Ry, glm::radians(front_wheels_rotateY), glm::vec3(0.0, 1.0, 0.0));
	}
	if (num == 1) //앞 기준 왼쪽 앞
	{
		T = glm::translate(T, glm::vec3(-(0.001f), 0.0f, 0.0f));
		T2 = glm::translate(T2, glm::vec3(-(CAR_SIZE / 2 + WHEEL_SIZE / 4), 0.0f, CAR_SIZE * 0.5f));
	}
	if (num == 2) //오른쪽 앞
	{
		T = glm::translate(T, glm::vec3((0.001f), 0.0f, 0.0f));
		T2 = glm::translate(T2, glm::vec3(CAR_SIZE / 2 + WHEEL_SIZE / 4, 0.0f, CAR_SIZE * 0.5f));
	}
	if (num == 3) //왼쪽 뒤
	{
		T = glm::translate(T, glm::vec3(-(0.001f), 0.0f, 0.0f));
		T2 = glm::translate(T2, glm::vec3(-(CAR_SIZE / 2 + WHEEL_SIZE / 4), 0.0f, -CAR_SIZE * 0.5f));
	}
	if (num == 4)  //오른쪽 뒤
	{
		T = glm::translate(T, glm::vec3((0.001f), 0.0f, 0.0f));
		T2 = glm::translate(T2, glm::vec3(CAR_SIZE / 2 + WHEEL_SIZE / 4, 0.0f, -CAR_SIZE * 0.5f));
	}

	glm::mat4 Rx = glm::mat4(1.0f);
	Rx = glm::rotate(Rx, glm::radians(wheel_rect_rotateX), glm::vec3(1.0, 0.0, 0.0));

	return Car_Body() * T2 * Ry * Rx * T;
}
glm::mat4 Wheel_on_000(int num, int type) //num은 4개 바퀴의 번호, type은 실린더, 뚜껑 객체 종류
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Ry = glm::mat4(1.0f);
	glm::mat4 Ry2 = glm::mat4(1.0f);
	//glm::mat4 S = glm::mat4(1.0f);

	Ry = glm::rotate(Ry, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
	if (num == 1 || num == 2)
	{
		//앞바퀴들에게 회전 변환 추가 적용
		Ry2 = glm::rotate(Ry2, glm::radians(front_wheels_rotateY), glm::vec3(0.0, 1.0, 0.0));
	}
	if (type == 0)		//바퀴
	{
		T = glm::translate(T, glm::vec3(0.0f, 0.0f, -WHEEL_SIZE / 4));
	}
	else if (type == 1) //바퀴 옆면1
	{
		T = glm::translate(T, glm::vec3(0.0f, 0.0f, -WHEEL_SIZE / 4));
	}
	else if (type == 2) //바퀴 옆면2
	{
		T = glm::translate(T, glm::vec3(0.0f, 0.0f, WHEEL_SIZE / 4));
	}
	//Car_Body() 이미 적용됨
	return Wheels(num) * Ry2 * Ry * T;
}

// 장애물 차 변환
float obstacle_xz[5][2] = {
	{FINISH_OFFSET_X - 1.05f, FINISH_OFFSET_Z},
	{FINISH_OFFSET_X + 1.05f, FINISH_OFFSET_Z},
	{FINISH_OFFSET_X - 1.05f * 2, FINISH_OFFSET_Z},
	{FINISH_OFFSET_X + 1.05f * 2, FINISH_OFFSET_Z},
	{100.0f, 100.0f}
};
glm::mat4 ObstacleCar(int index)
{

	glm::mat4 T = glm::mat4(1.0f);
	glm::vec3 positions[] = {
		glm::vec3(obstacle_xz[0][0], fy, obstacle_xz[0][1]),
		glm::vec3(obstacle_xz[1][0], fy, obstacle_xz[1][1]),
		glm::vec3(obstacle_xz[2][0], fy, obstacle_xz[2][1]),
		glm::vec3(obstacle_xz[3][0], fy, obstacle_xz[3][1]),
		glm::vec3(obstacle_xz[4][0], fy, obstacle_xz[4][1])
	};

	T = glm::translate(T, positions[index]);
	return T;
}

// 도착지점 변환
glm::mat4 FinishRect()
{

	glm::mat4 T = glm::mat4(1.0f);

	T = glm::translate(T, glm::vec3(FINISH_OFFSET_X, fy, FINISH_OFFSET_Z));
	return T;
}

// 후방 카메라 뷰
glm::mat4 RearCameraView() {
	// 자동차의 위치와 방향을 기준으로 후방 카메라 뷰 설정
	glm::vec3 carPosition(car_dx, car_dy, car_dz); // 자동차 위치
	float radians = glm::radians(car_rotateY);     // 자동차 회전 각도
	glm::vec3 carDirection(-sin(radians), 0.0f, -cos(radians)); // 자동차 뒤쪽 방향

	// 카메라를 자동차 바로 뒤쪽에 배치
	glm::vec3 cameraPosition = carPosition + carDirection * 0.45f + glm::vec3(0.0f, 0.15f, 0.0f);


	glm::vec3 lookAtTarget = carPosition + carDirection * 1.0f;
	glm::vec3 upVector(0.0f, 1.0f, 0.0f); // 월드 업 벡터

	return glm::lookAt(cameraPosition, lookAtTarget, upVector);
}

// 점이 다각형 내부에 있는지 검사 (반직선 교차법)
bool isPointInsidePolygon(const std::vector<std::pair<float, float>>& polygon, float x, float z)
{
	int intersections = 0;
	int n = polygon.size();
	for (int i = 0; i < n; ++i)
	{
		auto p1 = polygon[i];
		auto p2 = polygon[(i + 1) % n];

		// 두 점이 z축 방향에서 교차하는지 확인
		if ((p1.second > z) != (p2.second > z))
		{
			float intersectionX = p1.first + (z - p1.second) * (p2.first - p1.first) / (p2.second - p1.second);
			if (intersectionX > x)
			{
				intersections++;
			}
		}
	}
	return intersections % 2 == 1; // 홀수 교차이면 내부
}

// 선분 교차 검사
bool doLinesIntersect(float x1, float z1, float x2, float z2, float x3, float z3, float x4, float z4)
{
	auto cross = [](float ax, float ay, float bx, float by)
		{
			return ax * by - ay * bx;
		};

	float d1 = cross(x3 - x1, z3 - z1, x4 - x1, z4 - z1);
	float d2 = cross(x3 - x2, z3 - z2, x4 - x2, z4 - z2);
	float d3 = cross(x1 - x3, z1 - z3, x2 - x3, z2 - z3);
	float d4 = cross(x1 - x4, z1 - z4, x2 - x4, z2 - z4);

	return (d1 * d2 < 0 && d3 * d4 < 0); // 교차 조건
}

// 자동차 이동-회전 애니메이션 관련
float car_speed = 0.0f;						// 현재 자동차 속도
bool isAcceleratingForward = false;
bool isAcceleratingBackward = false;
const float MAX_SPEED = 0.01f;				// 최대 속도
const float acceleration = 0.001f;			// 가속도
const float deceleration = 0.002f;			// 감속도 (브레이크)
const float friction = 0.001f;				// 마찰력 (자연 감속)
bool isBraking = false;						// 브레이크 상태
const float speed = 0.05f;
const float HANDLE_RETURN_SPEED = 3.0f;		// 복원 속도
const float CAR_SPEED = 0.05f;				// 자동차 이동 속도

// 자동차 꼭짓점 추출 함수
std::vector<std::pair<float, float>> getRotatedCarCorners(float carX, float carZ, float carSize, float carRotateY)
{
	float halfWidth = CAR_SIZE / 2;
	float halfHeight = CAR_SIZE;

	// 꼭짓점의 상대 좌표
	std::vector<std::pair<float, float>> corners = {
		{-halfWidth, -halfHeight}, // 좌하단
		{halfWidth, -halfHeight},  // 우하단
		{halfWidth, halfHeight},   // 우상단
		{-halfWidth, halfHeight}   // 좌상단
	};

	// 회전 각도(라디안)
	float radians = glm::radians(-carRotateY);

	// 회전된 꼭짓점 좌표
	std::vector<std::pair<float, float>> rotatedCorners;
	for (const auto& corner : corners)
	{
		float rotatedX = corner.first * cos(radians) - corner.second * sin(radians);
		float rotatedZ = corner.first * sin(radians) + corner.second * cos(radians);
		rotatedCorners.emplace_back(carX + rotatedX, carZ + rotatedZ);
	}
	return rotatedCorners;
}

// 벽과 충돌하는 경우
bool checkCollisionWalls(const std::vector<std::pair<float, float>>& carCorners, float wallX, float wallZ, float wallWidth, float wallHeight)
{
	// 벽의 AABB
	float wallMinX = wallX - wallWidth / 2;
	float wallMaxX = wallX + wallWidth / 2;
	float wallMinZ = wallZ - wallHeight / 2;
	float wallMaxZ = wallZ + wallHeight / 2;

	// 차량 꼭짓점 중 하나라도 충돌하면 true
	for (const auto& corner : carCorners)
	{
		float cornerX = corner.first;
		float cornerZ = corner.second;

		if (cornerX > wallMinX && cornerX < wallMaxX &&
			cornerZ > wallMinZ && cornerZ < wallMaxZ)
		{
			return true;
		}
	}

	// 충돌 없음
	return false;
}

// 장애물과 충돌하는 경우
bool checkCollisionObstacle(const std::vector<std::pair<float, float>>& carCorners)
{
	// 각 장애물에 대해 충돌 여부를 확인
	for (const auto& obstacle : obstacle_xz)
	{
		float obstacleMinX = obstacle[0] - OBSTACLE_WIDTH;
		float obstacleMaxX = obstacle[0] + OBSTACLE_WIDTH;
		float obstacleMinZ = obstacle[1] - OBSTACLE_HEIGHT;
		float obstacleMaxZ = obstacle[1] + OBSTACLE_HEIGHT;

		// 차량의 꼭짓점이 장애물 영역에 있는지 확인
		for (const auto& corner : carCorners)
		{
			if (obstacleMinX <= corner.first && corner.first <= obstacleMaxX &&
				obstacleMinZ <= corner.second && corner.second <= obstacleMaxZ)
			{
				return true; // 차량 꼭짓점이 장애물 내부에 있음
			}
		}

		// 장애물의 사각형 꼭짓점을 계산
		std::vector<std::pair<float, float>> obstacleCorners = {
			{obstacleMinX, obstacleMinZ},
			{obstacleMaxX, obstacleMinZ},
			{obstacleMaxX, obstacleMaxZ},
			{obstacleMinX, obstacleMaxZ}
		};

		// 장애물의 꼭짓점이 차량 내부에 있는지 확인
		for (const auto& corner : obstacleCorners)
		{
			if (isPointInsidePolygon(carCorners, corner.first, corner.second))
			{
				return true; // 장애물 꼭짓점이 차량 내부에 있음
			}
		}

		// 차량의 모서리와 장애물의 모서리가 교차하는지 확인
		int carSize = carCorners.size();
		int obstacleSize = obstacleCorners.size();
		for (int i = 0; i < carSize; ++i)
		{
			for (int j = 0; j < obstacleSize; ++j)
			{
				if (doLinesIntersect(
					carCorners[i].first, carCorners[i].second,
					carCorners[(i + 1) % carSize].first, carCorners[(i + 1) % carSize].second,
					obstacleCorners[j].first, obstacleCorners[j].second,
					obstacleCorners[(j + 1) % obstacleSize].first, obstacleCorners[(j + 1) % obstacleSize].second))
				{
					return true; // 차량 모서리와 장애물 모서리가 교차
				}
			}
		}
	}

	return false; // 충돌 없음
}

// 주차 상태를 업데이트하는 함수
float PARKING_X_MIN = -FINISH_SIZE / 2 + FINISH_OFFSET_X;
float PARKING_X_MAX = FINISH_SIZE / 2 + FINISH_OFFSET_X;
float PARKING_Z_MIN = -FINISH_SIZE * fheight + FINISH_OFFSET_Z;
float PARKING_Z_MAX = FINISH_SIZE * fheight + FINISH_OFFSET_Z;
void UpdateParkingStatus(const std::vector<std::pair<float, float>>& carCorners) {
	bool newIsParked = false;
	int checkCount = 0;
	//std::cout << "==============================\n";
	for (const auto& corner : carCorners)
	{
		float cornerX = corner.first;
		float cornerZ = corner.second;
		//std::cout << "x:" << cornerX << "\n";
		//std::cout << "z:" << cornerZ << "\n";
		if (PARKING_X_MIN <= cornerX && cornerX <= PARKING_X_MAX &&
			PARKING_Z_MIN <= cornerZ && cornerZ <= PARKING_Z_MAX)
		{
			checkCount++;
		}
	}
	if (checkCount >= 4)
	{
		newIsParked = true;
	}

	if (newIsParked != isParked) {
		isParked = newIsParked;
		if (isParked) {
			// 주차 공간 색상을 연두색으로 변경
			for (int i = 0; i < 6; ++i) {
				finish_rect_color[0][i][0] = 0.5f; // R
				finish_rect_color[0][i][1] = 1.0f; // G
				finish_rect_color[0][i][2] = 0.5f; // B
			}
		}
		else {
			// 주차 공간 색상을 원래대로 복원
			// 바깥쪽 (흰색)
			for (int i = 0; i < 6; ++i) {
				finish_rect_color[0][i][0] = 1.0f;
				finish_rect_color[0][i][1] = 1.0f;
				finish_rect_color[0][i][2] = 1.0f;
			}
		}

		// VBO 업데이트
		glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(finish_rect_color), finish_rect_color);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

// 자동차 이동 및 회전 애니메이션
// 다음 스테이지 (수치 변경)
int current_stage = 1;
bool pause_mode = false;
bool isClear = false;
void nextStage()
{
	// 각도 초기화
	car_rotateY = 0.0f;
	front_wheels_rotateY = 0.0f;
	wheel_rect_rotateX = 0.0f;

	cumulativeAngle = 0.0f;
	handle_rotateZ = 0.0f;
	lastAngle = 0.0f;

	// 기어 초기화
	currentGear = DRIVE;

	// 시간 초기화
	startTime = time(nullptr);
	pauseTime = startTime - time(nullptr);

	// 충돌여부 초기화
	crushed = false;

	if (current_stage == 1)
	{
		//next stage
		current_stage++;

		// 도착지점 위치 변경
		FINISH_OFFSET_X = 3.0f;
		FINISH_OFFSET_Z = 0.0f;

		PARKING_X_MIN = -FINISH_SIZE / 2 + FINISH_OFFSET_X;
		PARKING_X_MAX = FINISH_SIZE / 2 + FINISH_OFFSET_X;
		PARKING_Z_MIN = -FINISH_SIZE * fheight + FINISH_OFFSET_Z;
		PARKING_Z_MAX = FINISH_SIZE * fheight + FINISH_OFFSET_Z;

		// 장애물 위치 변경
		obstacle_xz[0][0] = FINISH_OFFSET_X;
		obstacle_xz[0][1] = FINISH_OFFSET_Z + 1.55f;

		obstacle_xz[1][0] = FINISH_OFFSET_X;
		obstacle_xz[1][1] = FINISH_OFFSET_Z - 1.55f;

		obstacle_xz[2][0] = FINISH_OFFSET_X - 1.05;
		obstacle_xz[2][1] = FINISH_OFFSET_Z - 1.55f;

		obstacle_xz[3][0] = FINISH_OFFSET_X - 1.05 * 2;
		obstacle_xz[3][1] = FINISH_OFFSET_Z - 1.55f;

		// 차 위치 변경
		car_dx = 2.0f;
		car_dz = -4.0f;
	}
	else if (current_stage == 2)
	{
		//next stage
		current_stage++;

		// 도착지점 위치 변경
		FINISH_OFFSET_X = -2.0f;
		FINISH_OFFSET_Z = -4.0f;

		PARKING_X_MIN = -FINISH_SIZE / 2 + FINISH_OFFSET_X;
		PARKING_X_MAX = FINISH_SIZE / 2 + FINISH_OFFSET_X;
		PARKING_Z_MIN = -FINISH_SIZE * fheight + FINISH_OFFSET_Z;
		PARKING_Z_MAX = FINISH_SIZE * fheight + FINISH_OFFSET_Z;

		// 장애물 위치 변경
		obstacle_xz[0][0] = FINISH_OFFSET_X - 1.05;
		obstacle_xz[0][1] = FINISH_OFFSET_Z;

		obstacle_xz[1][0] = FINISH_OFFSET_X - 1.05;
		obstacle_xz[1][1] = FINISH_OFFSET_Z + 1.55f;

		obstacle_xz[2][0] = FINISH_OFFSET_X ;
		obstacle_xz[2][1] = FINISH_OFFSET_Z + 1.55f;

		obstacle_xz[3][0] = FINISH_OFFSET_X + 1.05;
		obstacle_xz[3][1] = FINISH_OFFSET_Z + 1.55f;

		obstacle_xz[4][0] = FINISH_OFFSET_X + 1.05;
		obstacle_xz[4][1] = FINISH_OFFSET_Z + 1.55f*2;

		// 차 위치 변경
		car_dx = -4.0f;
		car_dz = -4.0f;
	}
	else if (current_stage == 3)
	{
		//finish
		std::cout << "--Quit--\n";
		glutLeaveMainLoop(); // OpenGL 메인 루프 종료
	}
}

time_t currentTime;
void TimerFunction_UpdateMove(int value)
{
	front_wheels_rotateY = (handle_rotateZ / 900.0f) * 30.0f;

	currentTime = time(nullptr);
	if (!pause_mode)
	{
		elapsedSeconds = static_cast<int>(currentTime - pauseTime - startTime);
	}

	// 속도 계산
	if (currentGear == PARK || currentGear == NEUTRAL)
	{
		car_speed = 0.0f; // 정지
	}

	// 후진 처리
	if (currentGear == REVERSE && isAcceleratingBackward)
	{
		car_speed -= acceleration;
		if (car_speed < -MAX_SPEED)
			car_speed = -MAX_SPEED;
	}

	// 전진 처리
	if (currentGear == DRIVE && isAcceleratingForward)
	{
		car_speed += acceleration;
		if (car_speed > MAX_SPEED)
			car_speed = MAX_SPEED;
	}
	if (isBraking)
	{
		if (car_speed > 0.0f)
		{
			car_speed -= deceleration;
			if (car_speed < 0.0f)
				car_speed = 0.0f;
		}
		else if (car_speed < 0.0f)
		{
			car_speed += deceleration;
			if (car_speed > 0.0f)
				car_speed = 0.0f;
		}
	}
	if (!isAcceleratingForward && !isAcceleratingBackward && !isBraking)
	{
		// 자연 감속
		if (car_speed > 0.0f)
		{
			car_speed -= friction;
			if (car_speed < 0.0f)
				car_speed = 0.0f;
		}
		else if (car_speed < 0.0f)
		{
			car_speed += friction;
			if (car_speed > 0.0f)
				car_speed = 0.0f;
		}
	}

	// 차량의 꼭짓점 계산
	auto carCorners = getRotatedCarCorners(car_dx, car_dz, CAR_SIZE, car_rotateY);
	// 주차 상태 업데이트
	UpdateParkingStatus(carCorners);

	if (car_speed != 0.0f)
	{
		// 이동 후의 새로운 위치 계산
		float radians = glm::radians(car_rotateY);
		float new_dx = car_dx + car_speed * sin(radians);
		float new_dz = car_dz + car_speed * cos(radians);

		// 차량의 꼭짓점 계산
		bool isColliding = false;
		auto carCorners = getRotatedCarCorners(new_dx, new_dz, CAR_SIZE, car_rotateY);

		if(!invincible)
		{
			// 벽과의 충돌 여부 확인
			for (int i = 0; i < 4; ++i)
			{
				float wallX = (i % 2 == 0) ? 0.0f : (i == 1 ? GROUND_SIZE : -GROUND_SIZE);
				float wallZ = (i % 2 == 1) ? 0.0f : (i == 2 ? GROUND_SIZE : -GROUND_SIZE);
				float wallWidth = (i % 2 == 0) ? GROUND_SIZE * 2 : WALL_THICKNESS;
				float wallHeight = (i % 2 == 1) ? GROUND_SIZE * 2 : WALL_THICKNESS;

				if (checkCollisionWalls(carCorners, wallX, wallZ, wallWidth, wallHeight))
				{
					isColliding = true;
					break;
				}
			}

			// 장애물과 충돌 확인
			if (checkCollisionObstacle(carCorners))
			{
				isColliding = true;
			}
		}

		// 충돌이 없을 때만 이동 업데이트
		if (!isColliding)
		{

			// 회전 업데이트 (바퀴 회전에 따라 차량 회전)
			float n = (MAX_SPEED * 2) / MAX_SPEED;
			car_rotateY += front_wheels_rotateY * n * car_speed; // n * MAX_SPEED = 0.02 가 적당
			car_dx = new_dx;
			car_dz = new_dz;
			wheel_rect_rotateX += car_speed * 200.0f;
		}
		else
		{
			crushed = true;
		}

		// 핸들과 바퀴 복원 로직
		if (!is_mouse_on_handle)
		{
			// 핸들 복원
			if (handle_rotateZ > 0.0f)
			{
				handle_rotateZ = std::max(0.0f, handle_rotateZ - HANDLE_RETURN_SPEED); //HANDLE_RETURN_SPEED -> (MAX_SPEED = 0.01)일때 3.0가 적당
			}
			else if (handle_rotateZ < 0.0f)
			{
				handle_rotateZ = std::min(0.0f, handle_rotateZ + HANDLE_RETURN_SPEED);
			}
			cumulativeAngle = handle_rotateZ;

			// 복원된 핸들 값에 따라 바퀴 회전량 동기화
			front_wheels_rotateY = (handle_rotateZ / 900.0f) * 30.0f;
		}
	}

	// 화면 갱신 요청 및 타이머 재설정
	glutPostRedisplay();
	glutTimerFunc(TIMER_VELOCITY, TimerFunction_UpdateMove, 1);
}

float c_dx = 0.0f;
float c_dy = 1.0f;
float c_dz = -3.0f;
float c_angleY = 0.0f;
float c_rotateY = 0.0f;
int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//srand(time(0));
	width = clientWidth;
	height = clientHeight;

	//--- 윈도우 생성하기
	glutInit(&argc, argv);												//--- glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);			//--- 디스플레이 모드 설정 (+ 깊이 검사 추가)
	glutInitWindowPosition(100, 100);									//--- 윈도우의 위치 지정
	glutInitWindowSize(width, height);									//--- 윈도우의 크기 지정
	glutCreateWindow("parking_master");									//--- 윈도우 생성(윈도우 이름)

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) //--- glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	// 시간 초기화
	startTime = time(nullptr);
	pauseTime = startTime - time(nullptr);

	glEnable(GL_DEPTH_TEST);
	initCar();
	initObstacleCar();
	// 자동체 액셀 브레이크 감지 - 이동 애니메이션
	glutTimerFunc(TIMER_VELOCITY, TimerFunction_UpdateMove, 1);

	//--- 세이더 읽어와서 세이더 프로그램 만들기
	make_shaderProgram();
	InitBuffer();
	

	glutDisplayFunc(drawScene);					//--- 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);					//--- 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard);					// 키보드 입력
	glutKeyboardUpFunc(KeyboardUp);					// 키보드 입력
	//glutSpecialFunc(SpecialKeyboard);			// 키보드 입력(방향키 등 스페셜)
	glutMouseFunc(MouseButton);					// 마우스 버튼 콜백 등록
	glutMotionFunc(MouseMotion);				// 마우스 드래그 콜백 등록
	glutMainLoop();								//--- 이벤트 처리 시작
}

//그리기 함수
void draw_handle(int modelLoc, int num)
{
	// 그리기
	int useUniformColorLoc = glGetUniformLocation(shaderProgramID, "useUniformColor");
	int u_colorLoc = glGetUniformLocation(shaderProgramID, "u_color");

	glUniform1i(useUniformColorLoc, 1);           // uniform color 사용하도록 설정
	glUniform3f(u_colorLoc, 0.25f, 0.25f, 0.25f);    // u_color를 회색으로 설정

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	GLUquadricObj* qobj1;
	qobj1 = gluNewQuadric();
	gluDisk(qobj1, HANDLE_SIZE - HANDLE_SIZE / 2, HANDLE_SIZE, 20, 8);
	gluDeleteQuadric(qobj1);

	glUniform1i(useUniformColorLoc, 0); // 다시 passColor 사용

	glBindVertexArray(vao[2]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Handle()));
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
void draw_gear(int modelLoc, int num)
{
	glBindVertexArray(vao[6]); // 기어용 VAO 사용
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Gear()));
	glDrawArrays(GL_TRIANGLES, 0, 6); // 사각형 그리기
}
void draw_gear_stick(int modelLoc, int num)
{
	glBindVertexArray(vao[7]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Gear_Stick()));
	glDrawArrays(GL_TRIANGLES, 0, 6); // 사각형 그리기
}
void draw_pointMode(int modelLoc, int num)
{
	glBindVertexArray(vao[6]); // 기어용 VAO 사용
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(PointMode()));
	glDrawArrays(GL_TRIANGLES, 0, 6); // 사각형 그리기
}

void draw_wheels(int modelLoc, int num)
{
	// shaderProgramID에 바인딩된 쉐이더 프로그램 사용 중이라고 가정
	int useUniformColorLoc = glGetUniformLocation(shaderProgramID, "useUniformColor");
	int u_colorLoc = glGetUniformLocation(shaderProgramID, "u_color");

	glUniform1i(useUniformColorLoc, 1);           // uniform color 사용하도록 설정
	glUniform3f(u_colorLoc, 0.25f, 0.25f, 0.25f);    // u_color를 회색으로 설정

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_on_000(num, 0)));
	GLUquadricObj* qobj1;
	qobj1 = gluNewQuadric();						// 객체 생성하기
	gluCylinder(qobj1, WHEEL_SIZE, WHEEL_SIZE, WHEEL_SIZE / 2, 20, 8);
	gluDeleteQuadric(qobj1);
	// 바퀴 뚜껑1
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_on_000(num, 1)));
	GLUquadricObj* qobj2;
	qobj2 = gluNewQuadric();
	gluDisk(qobj2, 0.0f, WHEEL_SIZE, 20, 8);
	gluDeleteQuadric(qobj2);
	// 바퀴 뚜껑2
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_on_000(num, 2)));
	GLUquadricObj* qobj3;
	qobj3 = gluNewQuadric();
	gluDisk(qobj3, 0.0f, WHEEL_SIZE, 20, 8);
	gluDeleteQuadric(qobj3);

	glUniform1i(useUniformColorLoc, 0); // 다시 passColor 사용
}
void drawCar(int modelLoc, int mod)
{
	// 차체
	glBindVertexArray(vao[1]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Car_Body()));
	glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
	glDrawArrays(GL_TRIANGLES, 36, 6 * 6);
	// 헤드라이트
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Headlights(0)));
	glDrawArrays(GL_TRIANGLES, 72, 6 * 6);	//왼쪽
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Headlights(1)));
	glDrawArrays(GL_TRIANGLES, 108, 6 * 6); //오른쪽

	// 바퀴 Wheel_on_000
	draw_wheels(modelLoc, 1);	//1
	draw_wheels(modelLoc, 2);	//2
	draw_wheels(modelLoc, 3);	//3
	draw_wheels(modelLoc, 4);	//4

	// 바퀴 사각형

	int useUniformColorLoc = glGetUniformLocation(shaderProgramID, "useUniformColor");
	int u_colorLoc = glGetUniformLocation(shaderProgramID, "u_color");

	glUniform1i(useUniformColorLoc, 1);           // uniform color 사용하도록 설정
	glUniform3f(u_colorLoc, 0.5f, 0.5f, 0.5f);    // u_color를 회색으로 설정

	glBindVertexArray(vao[4]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_rects(1)));
	glDrawArrays(GL_TRIANGLES, 0, 6 * 6);	//앞바퀴 1
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_rects(2)));
	glDrawArrays(GL_TRIANGLES, 36, 6 * 6);	//앞바퀴 2
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_rects(3)));
	glDrawArrays(GL_TRIANGLES, 72, 6 * 6);	//뒷바퀴 1
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_rects(4)));
	glDrawArrays(GL_TRIANGLES, 108, 6 * 6);	//뒷바퀴 2

	glUniform1i(useUniformColorLoc, 0); // 다시 passColor 사용
}
void drawWalls(int modelLoc)
{
	glBindVertexArray(vao[3]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(SRT_MATRIX()));
	glDrawArrays(GL_TRIANGLES, 0, 24);
}
void drawGround(int modelLoc)
{
	// 바닥
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(SRT_MATRIX()));
	glBindVertexArray(vao[0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
void drawFinishRect(int modelLoc)
{
	// 바닥
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(FinishRect()));
	glBindVertexArray(vao[5]);
	glDrawArrays(GL_TRIANGLES, 0, 12);
}
void drawObstacleCars(int modelLoc)
{
	// 장식용 주차공간
	glBindVertexArray(vao[8]); 
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ObstacleCar(0)));
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ObstacleCar(1)));
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ObstacleCar(2)));
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ObstacleCar(3)));
	glDrawArrays(GL_TRIANGLES, 0, 12);
	if (current_stage == 3)
	{
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ObstacleCar(4)));
		glDrawArrays(GL_TRIANGLES, 0, 12);
	}

	// 장애물
	glBindVertexArray(vao[9]);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ObstacleCar(0)));
	glDrawArrays(GL_TRIANGLES, 0, TRI_COUNT * 3);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ObstacleCar(1)));
	glDrawArrays(GL_TRIANGLES, 0, TRI_COUNT * 3);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ObstacleCar(2)));
	glDrawArrays(GL_TRIANGLES, 0, TRI_COUNT * 3);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ObstacleCar(3)));
	glDrawArrays(GL_TRIANGLES, 0, TRI_COUNT * 3);
	if(current_stage == 3)
	{
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ObstacleCar(4)));
		glDrawArrays(GL_TRIANGLES, 0, TRI_COUNT * 3);
	}
}
void drawCarCorners(int modelLoc)
{
	// 충돌체크용 차 꼭짓점 그리기
	auto carCorners = getRotatedCarCorners(car_dx, car_dz, CAR_SIZE, car_rotateY);
	for (const auto& corner : carCorners)
	{
		float cornerX = corner.first;
		float cornerZ = corner.second;
	
		glm::mat4 T = glm::mat4(1.0f);
		T = glm::translate(T, glm::vec3(cornerX, CAR_SIZE*0.75, cornerZ));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(T));
		GLUquadricObj* qobj1;
		qobj1 = gluNewQuadric();						// 객체 생성하기
		gluSphere(qobj1, WHEEL_SIZE/3, 10, 10);
		gluDeleteQuadric(qobj1);
	}
}

void drawScene()
{
	glViewport(0, 0, clientWidth, clientHeight);

	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	int modelLoc = glGetUniformLocation(shaderProgramID, "model");
	int viewLoc = glGetUniformLocation(shaderProgramID, "view");
	int projLoc = glGetUniformLocation(shaderProgramID, "projection");

	// 쿼터뷰
	if (currentGear != REVERSE)
	{
		if (true)
		{
			if (isCull)
			{
				glDisable(GL_DEPTH_TEST);
			}
			else
			{
				glEnable(GL_DEPTH_TEST);
			}

			// 차체 중심을 공전 중심으로 설정
			glm::vec3 orbitCenter = glm::vec3(car_dx, car_dy, car_dz);

			// 카메라 위치 계산
			float cameraDistance = c_dz; // `c_dz`를 카메라 거리로 사용
			glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
			glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

			glm::mat4 cameraRotateMat = glm::rotate(glm::mat4(1.0f), glm::radians(c_rotateY), glm::vec3(0.0, 1.0, 0.0));
			glm::vec3 cameraOffset = glm::vec3(cameraRotateMat * glm::vec4(0.0f, 1.9f, cameraDistance, 1.0f)); // Y축으로 살짝 올림
			glm::vec3 cameraPos = orbitCenter + cameraOffset;

			// 카메라 방향 업데이트 (살짝 아래로 보기)
			glm::vec3 lookTarget = orbitCenter + glm::vec3(0.0f, -0.2f, 0.0f); // 아래로 약간 이동
			cameraDirection = glm::normalize(lookTarget - cameraPos);

			// 뷰 행렬 설정
			glm::mat4 vTransform = glm::lookAt(cameraPos, lookTarget, cameraUp);
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

			// 투영변환
			glm::mat4 pTransform = glm::mat4(1.0f);
			if (!isProspect)
			{
				pTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 100.0f);
			}
			else
			{
				pTransform = glm::perspective(glm::radians(45.0f), (float)clientWidth / (float)clientHeight, 0.1f, 50.0f);
			}
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);
		}

		// 바닥 그리기
		drawGround(modelLoc);

		// 모델 그리기
		drawCar(modelLoc, 0);

		// 장애물 차 그리기
		drawObstacleCars(modelLoc);

		// 벽 그리기
		drawWalls(modelLoc);

		// 도착지점 그리기
		drawFinishRect(modelLoc);

		// 차 꼭지점 (좌표에 따라) 그리기 (디버깅, 무적)
		if(invincible)
			drawCarCorners(modelLoc);
	}
	// 후방 카메라 뷰
	if (currentGear == REVERSE)
	{
		// 후방 카메라 뷰포트 설정
		//int rearViewWidth = clientWidth / 3;
		//int rearViewHeight = clientHeight / 3;
		//int rearViewX = (clientWidth - rearViewWidth) / 2; // 화면 중앙 상단
		//int rearViewY = clientHeight - rearViewHeight;

		int rearViewWidth = clientWidth;
		int rearViewHeight = clientHeight;
		int rearViewX = 0;// 화면 중앙 상단
		int rearViewY = 0;

		glViewport(rearViewX, rearViewY, rearViewWidth, rearViewHeight);

		// 후방 카메라 뷰 행렬 설정
		glm::mat4 rearViewTransform = RearCameraView();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(rearViewTransform));

		// 동일한 투영 행렬 사용
		glm::mat4 rearProjTransform = glm::perspective(glm::radians(45.0f), (float)rearViewWidth / (float)rearViewHeight, 0.1f, 50.0f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(rearProjTransform));

		// 자동차, 바닥, 벽 등 모든 객체를 다시 그리기
		drawGround(modelLoc);
		drawCar(modelLoc, 0);
		drawObstacleCars(modelLoc);
		drawWalls(modelLoc);
		drawFinishRect(modelLoc);
	}
	glDisable(GL_DEPTH_TEST);
	// 핸들 - 뷰포트 설정으로 그리기
	if (true)
	{
		int miniMapWidth = 900 / 3;
		int miniMapHeight = 900 / 3;
		int miniMapX = 900 - miniMapWidth;
		int miniMapY = 900 - miniMapHeight;
		glViewport(miniMapX, 0, miniMapWidth, miniMapHeight);

		// 정면 뷰용 카메라 설정
		glm::mat4 topViewTransform = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 1.0f),	// 카메라 위치
			glm::vec3(0.0f, 0.0f, 0.0f),	// 어디를 바라볼 것인가
			glm::vec3(0.0f, 1.0f, 0.0f)		// 업 벡터
		);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &topViewTransform[0][0]);

		// 투영 변환 (직교 투영)
		glm::mat4 orthoTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 1.5f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, &orthoTransform[0][0]);

		// 핸들 그리기
		draw_handle(modelLoc, 0);
	}
	// 기어 그리기
	if (true)
	{
		int miniMapWidth = clientWidth / 3;
		int miniMapHeight = clientHeight / 3;
		int miniMapX = clientWidth - miniMapWidth;
		int miniMapY = clientHeight - miniMapHeight;
		glViewport(miniMapX, miniMapY, miniMapWidth, miniMapHeight);

		glm::mat4 topViewTransform = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 1.0f), // 카메라 위치
			glm::vec3(0.0f, 0.0f, 0.0f), // 바라보는 위치
			glm::vec3(0.0f, 1.0f, 0.0f)  // 업 벡터
		);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(topViewTransform));

		glm::mat4 orthoTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 1.5f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(orthoTransform));

		draw_gear(modelLoc, 0);

		// 텍스트 그리기
		// 텍스트 렌더링을 위해 쉐이더 프로그램 비활성화
		glUseProgram(0);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, miniMapWidth, 0, miniMapHeight);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// 텍스트 위치 계산 (픽셀 단위)
		float textScale = 1.0f; // 텍스트 크기 조절
		float p_x = miniMapWidth * 0.65f;
		float r_x = miniMapWidth * 0.65f;
		float n_x = miniMapWidth * 0.65f;
		float d_x = miniMapWidth * 0.65f;
		float y = miniMapHeight * 0.5f;

		int star_count = 1;
		if (elapsedSeconds < 30)
			glColor3f(1.0f, 1.0f, 1.0f); // 흰색
		else if (elapsedSeconds < 60)
			glColor3f(1.0f, 1.0f, 0.0f); // 노란색
		else if (elapsedSeconds >= 60)
			glColor3f(1.0f, 0.0f, 0.0f); // 빨간색
		std::string timeString = std::to_string(elapsedSeconds) + "s";

		glPushMatrix();
		glTranslatef(p_x + 25, y + 84, 0.0f);
		glScalef(textScale, textScale, textScale);
		RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, timeString.c_str());
		glPopMatrix();

		// OpenGL의 기본 행렬을 사용하여 텍스트를 그립니다.
		// 글씨 하나씩 위치를 조절하며 그립니다.
		glColor3f(1.0f, 1.0f, 1.0f); // 흰색
		glPushMatrix();
		glTranslatef(p_x, y + 50, 0.0f);
		glScalef(textScale, textScale, textScale);
		RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, "P");
		glPopMatrix();

		glPushMatrix();
		glTranslatef(r_x, y + 5, 0.0f);
		glScalef(textScale, textScale, textScale);
		RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, "R");
		glPopMatrix();

		glPushMatrix();
		glTranslatef(n_x, y - 40, 0.0f);
		glScalef(textScale, textScale, textScale);
		RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, "N");
		glPopMatrix();

		glPushMatrix();
		glTranslatef(d_x, y - 80, 0.0f);
		glScalef(textScale, textScale, textScale);
		RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, "D");
		glPopMatrix();

		glPopMatrix(); // 모델뷰
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glUseProgram(shaderProgramID); // 쉐이더 프로그램 재활성화
	}
	// 기어 스틱 그리기
	if (true)
	{
		int miniMapWidth = clientWidth / 3;
		int miniMapHeight = clientHeight / 3;
		int miniMapX = clientWidth - miniMapWidth;
		int miniMapY = clientHeight - miniMapHeight;
		glViewport(miniMapX, miniMapY, miniMapWidth, miniMapHeight);

		glm::mat4 topViewTransform = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 1.0f), // 카메라 위치
			glm::vec3(0.0f, 0.0f, 0.0f), // 바라보는 위치
			glm::vec3(0.0f, 1.0f, 0.0f)  // 업 벡터
		);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(topViewTransform));

		glm::mat4 orthoTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 1.5f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(orthoTransform));

		draw_gear_stick(modelLoc, 0);
	}
	glEnable(GL_DEPTH_TEST);

	if (pause_mode)
	{
		int miniMapWidth = clientWidth / 2;
		int miniMapHeight = clientHeight / 2;
		int miniMapX = clientWidth - miniMapWidth * 1.5;
		int miniMapY = clientHeight - miniMapHeight * 1.5;
		glViewport(miniMapX, miniMapY, miniMapWidth, miniMapHeight);

		glm::mat4 topViewTransform = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 1.0f), // 카메라 위치
			glm::vec3(0.0f, 0.0f, 0.0f), // 바라보는 위치
			glm::vec3(0.0f, 1.0f, 0.0f)  // 업 벡터
		);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(topViewTransform));

		glm::mat4 orthoTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 1.5f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(orthoTransform));

		draw_pointMode(modelLoc, 0);

		// 텍스트 그리기
		// 텍스트 렌더링을 위해 쉐이더 프로그램 비활성화
		glUseProgram(0);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, miniMapWidth, 0, miniMapHeight);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// 텍스트 위치 계산 (픽셀 단위)
		float textScale = 1.0f; // 텍스트 크기 조절
		float mx = miniMapWidth * 0.5;
		float my = miniMapHeight * 0.5;
	
		if (!isClear) // 정지 모드
		{
			glColor3f(1.0f, 1.0f, 1.0f); // 흰색
			std::string String = "PAUSE";
			glPushMatrix();
			glTranslatef(mx - 20, my + 50, 0.0f);
			glScalef(textScale, textScale, textScale);
			RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
			glPopMatrix();

			String = "Press 'ESC' to resume";
			glPushMatrix();
			glTranslatef(mx - 80, my - 50, 0.0f);
			glScalef(textScale, textScale, textScale);
			RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
			glPopMatrix();
		}
		else // 클리어 표시
		{
			glColor3f(1.0f, 1.0f, 1.0f); // 흰색
			std::string String = "stage " + std::to_string(current_stage) + " clear!!";

			glPushMatrix();
			glTranslatef(mx - 50, my + 50, 0.0f);
			glScalef(textScale, textScale, textScale);
			RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
			glPopMatrix();

			glColor3f(1.0f, 1.0f, 0.0f); // 노란색
			int star_count = 1;
			if (elapsedSeconds <= 60)
			{
				star_count++;
			}
			if (!crushed)
			{
				star_count++;
			}

			String = "your star count : " + std::to_string(star_count);
			glPushMatrix();
			glTranslatef(mx - 65, my, 0.0f);
			glScalef(textScale, textScale, textScale);
			RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
			glPopMatrix();

			glColor3f(1.0f, 1.0f, 1.0f); // 흰색
			if (current_stage <= 2)
			{
				String = "Press 'n' to next stage";
				glPushMatrix();
				glTranslatef(mx - 80, my - 50, 0.0f);
				glScalef(textScale, textScale, textScale);
				RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
				glPopMatrix();
			}
			else
			{
				String = "Press 'n' to quit game";
				glPushMatrix();
				glTranslatef(mx - 80, my - 50, 0.0f);
				glScalef(textScale, textScale, textScale);
				RenderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_18, String.c_str());
				glPopMatrix();
			}
		}

		glPopMatrix(); // 모델뷰
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glUseProgram(shaderProgramID); // 쉐이더 프로그램 재활성화
	}

	glutSwapBuffers();
}

// GLSL
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	//if (key == 'b')
	//{
	//	//디버그
	//	nextStage();
	//}
	if (key == 'n')
	{
		if (isClear)
		{
			nextStage();
			pause_mode = false;
			isClear = false;
		}
	}
	if (!isClear)
	{
		if (key == 27) // ESC 키
		{
			if (pause_mode) //해제
			{
				//startTime = time(nullptr) - (pauseTime - startTime); // 정지 시간만큼 보정
				pauseTime += time(nullptr) - tempTime;
				pause_mode = false;
			}
			else //정지
			{
				tempTime = currentTime;
				pause_mode = true;
			}
		}
	}
	if (!pause_mode)
	{
		switch (key)
		{
		case 'i':
		{
			if (invincible)
				invincible = false;
			else
				invincible = true;
			break;
		}
		case 'q': // 이전 기어
		{
			if (currentGear > PARK)
				currentGear = static_cast<GearState>(currentGear - 1);

			if (currentGear == PARK)
			{
				if (isParked)
				{
					pause_mode = true;
					isClear = true;
				}
			}
			break;
		}
		case 'e': // 다음 기어
		{
			if (currentGear < DRIVE)
				currentGear = static_cast<GearState>(currentGear + 1);
			break;
		}
		case 'w': // 액셀
		{
			if (currentGear == DRIVE)
			{
				isAcceleratingForward = true; // 전진
			}
			else if (currentGear == REVERSE)
			{
				isAcceleratingBackward = true; // 후진
			}
			break;
		}
		case ' ': // 브레이크
		{
			// isAcceleratingForward = false;
			// isAcceleratingBackward = true;
			isBraking = true;
			break;
		}
		}
	}
	glutPostRedisplay(); //--- refresh
}
GLvoid KeyboardUp(unsigned char key, int x, int y) {
	switch (key)
	{
	case 'w': // 액셀 해제
		isAcceleratingForward = false;
		isAcceleratingBackward = false;
		break;
	case ' ': // 액셀 해제
		isBraking = false;
		break;
	}
	glutPostRedisplay(); //--- refresh
}

void MouseButton(int button, int state, int x, int y)
{
	if (!pause_mode)
	{
		if (button == GLUT_LEFT_BUTTON)
		{ // 좌클릭
			if (state == GLUT_DOWN)
			{
				if (x > 600 && y > 300) //750, 450이 핸들의 중심좌표
				{
					lastAngle = 0.0f;
					is_mouse_on_handle = true;
					//std::cout << "x :" << x << "  y :" << y << std::endl;
				}
				else
				{
					is_mouse_on_camera = true; // 마우스 눌림 상태
					lastMouseX = x;           // 초기 위치 저장
				}
			}
			else if (state == GLUT_UP)
			{
				if (is_mouse_on_handle)
				{
					lastAngle = 0.0f;
					is_mouse_on_handle = false;
				}
				if (is_mouse_on_camera)
				{
					is_mouse_on_camera = false; // 마우스 떼기 상태
					lastMouseX = -1;           // 초기화
				}
			}
		}
		else if (button == 3)
		{ // 휠 위로 스크롤
			c_dz -= 0.1f;       // 카메라를 앞으로 이동
			glutPostRedisplay(); // 화면 갱신 요청
		}
		else if (button == 4)
		{ // 휠 아래로 스크롤
			c_dz += 0.1f;       // 카메라를 뒤로 이동
			glutPostRedisplay(); // 화면 갱신 요청
		}
	}
}
void MouseMotion(int x, int y)
{
	if (is_mouse_on_handle)
	{
		// 기준점과 현재 마우스 위치의 상대 위치 계산
		int dx = x - 750;
		int dy = y - 450;

		// 기준 각도를 y축 음의 방향으로 설정
		float currentAngle = -atan2(dx, -dy) * (180.0f / M_PI);

		// 각도 차이 계산 (누적 회전을 위해)
		float deltaAngle;
		deltaAngle = currentAngle - lastAngle;

		// 경계 처리 (-180 ~ 180 사이의 점프 방지)
		if (deltaAngle > 180.0f)
			deltaAngle -= 360.0f;
		else if (deltaAngle < -180.0f)
			deltaAngle += 360.0f;

		// 회전 누적
		if ( -900.0f <= handle_rotateZ && handle_rotateZ <= 900.0f)
		{
			cumulativeAngle += deltaAngle;
			if (cumulativeAngle > 900.0f)
				cumulativeAngle = 900.0f;
			else if (cumulativeAngle < -900.0f)
				cumulativeAngle = -900.0f;

			// handle_rotateZ 업데이트 (누적 각도)
			handle_rotateZ = cumulativeAngle;
		}

		// 현재 각도를 저장 (다음 프레임 비교를 위해)
		lastAngle = currentAngle;
	}
	if (is_mouse_on_camera)
	{
		if (lastMouseX == -1)
		{
			// 초기 마우스 위치 저장
			lastMouseX = x;
			return;
		}
		// 마우스 이동 차이 계산
		int dx = x - lastMouseX;

		// Y축 회전 갱신 (좌우 이동)
		c_rotateY += dx * 0.1f;

		// 갱신된 위치를 저장
		lastMouseX = x;
	}
	// 화면 갱신 요청
	glutPostRedisplay();
}

// 세이더
char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb");			// Open file for reading 
	if (!fptr)							// Return NULL on failure 
		return NULL;
	fseek(fptr, 0, SEEK_END);			// Seek to the end of the file 
	length = ftell(fptr);				// Find out how many bytes into the file we are 
	buf = (char*)malloc(length + 1);	// Allocate a buffer for the entire length of the file and a null terminator 
	fseek(fptr, 0, SEEK_SET);			// Go back to the beginning of the file 
	fread(buf, length, 1, fptr);		// Read the contents of the file in to the buffer 
	fclose(fptr);						// Close the file 
	buf[length] = 0;					// Null terminator 
	return buf;							// Return the buffer 
}
void make_vertexShaders()
{
	vertexSource = filetobuf("vertex.glsl");

	//--- 버텍스 세이더 객체 만들기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);

	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexShader);

	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}
void make_fragmentShaders()
{
	fragmentSource = filetobuf("fragment.glsl");

	//--- 프래그먼트 세이더 객체 만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);

	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentShader);

	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}
void make_shaderProgram()
{
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기

	//-- shader Program
	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);

	//--- 세이더 삭제하기
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//--- Shader Program 사용하기
	glUseProgram(shaderProgramID);
}
void InitBuffer()
{

	glGenVertexArrays(10, vao);
	glGenBuffers(20, vbo);

	// 땅
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ground), ground, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ground_color), ground_color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

	// 블록(차체)
	glBindVertexArray(vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Block), Block, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Block_Color), Block_Color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

	// 핸들
	glBindVertexArray(vao[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(handle_rect), handle_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(handle_rect_color), handle_rect_color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

	// 벽
	glBindVertexArray(vao[3]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(walls), walls, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall_colors), wall_colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

	// 바퀴
	glBindVertexArray(vao[4]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wheel_rect), wheel_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wheel_rect_color), wheel_rect_color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

	// 도착지점 (주차 공간)
	glBindVertexArray(vao[5]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[10]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(finish_rect), finish_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(finish_rect_color), finish_rect_color, GL_DYNAMIC_DRAW); // 변경: GL_DYNAMIC_DRAW
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

	// 기어 데이터 초기화
	glBindVertexArray(vao[6]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[12]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gear_rect), gear_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[13]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gear_rect_color), gear_rect_color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

	// 기어봉 데이터 초기화
	glBindVertexArray(vao[7]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[14]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gear_stick_rect), gear_stick_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[15]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gear_stick_rect_color), gear_stick_rect_color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

	//장식용 주차공간
	glBindVertexArray(vao[8]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[16]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(finish_rect), finish_rect, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[17]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(not_finish_rect_color), not_finish_rect_color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

	// 장애물 차
	glBindVertexArray(vao[9]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[18]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(obstacle_car), obstacle_car, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[19]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(obstacle_car_color), obstacle_car_color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

}

// 자동차 초기화
void initCar()
{
	//아래 몸체
	if (true)
	{
		GLfloat vertices[8][3] = {
			{-CAR_SIZE / 2,	0.0f,			-CAR_SIZE},	// Vertex 0
			{ CAR_SIZE / 2,	0.0f,			-CAR_SIZE},	// Vertex 1
			{ CAR_SIZE / 2,	CAR_SIZE / 2,	-CAR_SIZE},   // Vertex 2
			{-CAR_SIZE / 2,	CAR_SIZE / 2, -CAR_SIZE },  // Vertex 3
			{-CAR_SIZE / 2,	0.0f,			 CAR_SIZE },  // Vertex 4
			{ CAR_SIZE / 2,	0.0f,			 CAR_SIZE },  // Vertex 5
			{ CAR_SIZE / 2,	CAR_SIZE / 2,  CAR_SIZE },  // Vertex 6
			{-CAR_SIZE / 2,	CAR_SIZE / 2,  CAR_SIZE }   // Vertex 7
		};
		//큐브 데이터 초기화
		GLfloat CubeFigure[1][TRI_COUNT * 3][3] = {
			{
				// Front face - 2tri	012 023 -02
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]},

				// Back face - 2tri		456 467 -46
				{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},

				// Left face - 2tri		047 073 -07
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]},

				// Right face - 2tri	156 162 -16
				{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]},

				// Top face - 2tri		326 367 -36
				{vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},

				// Bottom face - 2tri	015 054 -05
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}
			}
		};

		for (int j = 0; j < TRI_COUNT * 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				Block[0][j][k] = CubeFigure[0][j][k];
			}
			Block_Color[0][j][0] = 0.0f;
			Block_Color[0][j][1] = 0.0f;
			Block_Color[0][j][2] = 1.0f;
		}
	}

	//윗 몸체(창문)
	if (true)
	{
		GLfloat vertices[8][3] = {
			{-CAR_SIZE / 3,	CAR_SIZE / 2,						-CAR_SIZE / 3 * 2},	// Vertex 0
			{ CAR_SIZE / 3,	CAR_SIZE / 2,						-CAR_SIZE / 3 * 2},	// Vertex 1
			{ CAR_SIZE / 3,	CAR_SIZE / 2 + CAR_SIZE / 3,	-CAR_SIZE / 3 * 2},   // Vertex 2
			{-CAR_SIZE / 3,	CAR_SIZE / 2 + CAR_SIZE / 3,	-CAR_SIZE / 3 * 2 },  // Vertex 3
			{-CAR_SIZE / 3,	CAR_SIZE / 2,						CAR_SIZE / 3 * 2 },  // Vertex 4
			{ CAR_SIZE / 3,	CAR_SIZE / 2,						CAR_SIZE / 3 * 2 },  // Vertex 5
			{ CAR_SIZE / 3,	CAR_SIZE / 2 + CAR_SIZE / 3,	CAR_SIZE / 3 * 2 },  // Vertex 6
			{-CAR_SIZE / 3,	CAR_SIZE / 2 + CAR_SIZE / 3,	CAR_SIZE / 3 * 2 }   // Vertex 7
		};
		//큐브 데이터 초기화
		GLfloat CubeFigure[1][TRI_COUNT * 3][3] = {
			{
				// Front face - 2tri	012 023 -02
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]},

				// Back face - 2tri		456 467 -46
				{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},

				// Left face - 2tri		047 073 -07
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]},

				// Right face - 2tri	156 162 -16
				{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]},

				// Top face - 2tri		326 367 -36
				{vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},

				// Bottom face - 2tri	015 054 -05
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}
			}
		};

		for (int j = 0; j < TRI_COUNT * 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				Block[1][j][k] = CubeFigure[0][j][k];
			}
			Block_Color[1][j][0] = 0.0f;
			Block_Color[1][j][1] = 1.0f;
			Block_Color[1][j][2] = 1.0f;
		}
	}

	//헤드라이트 1, 2 (정육면체)
	if (true)
	{
		GLfloat vertices[8][3] = {
			{-CAR_SIZE / 8,	-CAR_SIZE / 8, -CAR_SIZE / 8},	// Vertex 0
			{ CAR_SIZE / 8,	-CAR_SIZE / 8, -CAR_SIZE / 8},	// Vertex 1
			{ CAR_SIZE / 8,	CAR_SIZE / 8, -CAR_SIZE / 8},  // Vertex 2
			{-CAR_SIZE / 8,	CAR_SIZE / 8, -CAR_SIZE / 8},  // Vertex 3
			{-CAR_SIZE / 8,	-CAR_SIZE / 8, CAR_SIZE / 8},	// Vertex 4
			{ CAR_SIZE / 8,	-CAR_SIZE / 8, CAR_SIZE / 8},	// Vertex 5
			{ CAR_SIZE / 8,	CAR_SIZE / 8, CAR_SIZE / 8},	// Vertex 6
			{-CAR_SIZE / 8,	CAR_SIZE / 8, CAR_SIZE / 8}	// Vertex 7
		};
		//큐브 데이터 초기화
		GLfloat CubeFigure[1][TRI_COUNT * 3][3] = {
			{
				// Front face - 2tri	012 023 -02
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]},

				// Back face - 2tri		456 467 -46
				{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},

				// Left face - 2tri		047 073 -07
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]},

				// Right face - 2tri	156 162 -16
				{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]},

				// Top face - 2tri		326 367 -36
				{vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},

				// Bottom face - 2tri	015 054 -05
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}
			}
		};

		for (int j = 0; j < TRI_COUNT * 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				Block[2][j][k] = CubeFigure[0][j][k];

				Block[3][j][k] = CubeFigure[0][j][k];
			}
			Block_Color[2][j][0] = 1.0f;
			Block_Color[2][j][1] = 1.0f;
			Block_Color[2][j][2] = 0.0f;

			Block_Color[3][j][0] = 1.0f;
			Block_Color[3][j][1] = 1.0f;
			Block_Color[3][j][2] = 0.0f;
		}
	}

	//바퀴 사각형
	if(true)
	{
		GLfloat vertices[8][3] = {
				{-WHEEL_SIZE/4,	-WHEEL_SIZE/8,  -WHEEL_SIZE },		// Vertex 0
				{ WHEEL_SIZE/4,	-WHEEL_SIZE/8,  -WHEEL_SIZE },		// Vertex 1
				{ WHEEL_SIZE/4,	 WHEEL_SIZE/8, 	-WHEEL_SIZE },		// Vertex 2
				{-WHEEL_SIZE/4,	 WHEEL_SIZE/8, 	-WHEEL_SIZE },		// Vertex 3
				{-WHEEL_SIZE/4,	-WHEEL_SIZE/8,   WHEEL_SIZE },	// Vertex 4
				{ WHEEL_SIZE/4,	-WHEEL_SIZE/8,   WHEEL_SIZE },	// Vertex 5
				{ WHEEL_SIZE/4,	 WHEEL_SIZE/8, 	 WHEEL_SIZE },		// Vertex 6
				{-WHEEL_SIZE/4,	 WHEEL_SIZE/8, 	 WHEEL_SIZE }		// Vertex 7
		};
		//큐브 데이터 초기화
		GLfloat CubeFigure[1][TRI_COUNT * 3][3] = {
			{
				// Front face - 2tri	012 023 -02
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]},

				// Back face - 2tri		456 467 -46
				{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},

				// Left face - 2tri		047 073 -07
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]},

				// Right face - 2tri	156 162 -16
				{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]},

				// Top face - 2tri		326 367 -36
				{vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},

				// Bottom face - 2tri	015 054 -05
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}
			}
		};

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < TRI_COUNT * 3; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					wheel_rect[i][j][k] = CubeFigure[0][j][k];
				}
				wheel_rect_color[i][j][0] = 0.0f;
				wheel_rect_color[i][j][1] = 0.0f;
				wheel_rect_color[i][j][2] = 1.0f;
			}
		}
	}
}
// 장애물 초기화
void initObstacleCar() {
	if (true)
	{
		GLfloat vertices[8][3] = {
			{-OBSTACLE_WIDTH,	0.0f,			-OBSTACLE_HEIGHT },	// Vertex 0
			{ OBSTACLE_WIDTH,	0.0f,			-OBSTACLE_HEIGHT },	// Vertex 1
			{ OBSTACLE_WIDTH,	CAR_SIZE,		-OBSTACLE_HEIGHT },   // Vertex 2
			{-OBSTACLE_WIDTH,	CAR_SIZE,		-OBSTACLE_HEIGHT },  // Vertex 3
			{-OBSTACLE_WIDTH,	0.0f,			 OBSTACLE_HEIGHT },  // Vertex 4
			{ OBSTACLE_WIDTH,	0.0f,			 OBSTACLE_HEIGHT },  // Vertex 5
			{ OBSTACLE_WIDTH,	CAR_SIZE,		 OBSTACLE_HEIGHT },  // Vertex 6
			{-OBSTACLE_WIDTH,	CAR_SIZE,		 OBSTACLE_HEIGHT }   // Vertex 7
		};
		//큐브 데이터 초기화
		GLfloat CubeFigure[1][TRI_COUNT * 3][3] = {
			{
				// Front face - 2tri	012 023 -02
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]},

				// Back face - 2tri		456 467 -46
				{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},

				// Left face - 2tri		047 073 -07
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]},

				// Right face - 2tri	156 162 -16
				{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]},

				// Top face - 2tri		326 367 -36
				{vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},
				{vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},

				// Bottom face - 2tri	015 054 -05
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]},
				{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}
			}
		};

		for (int j = 0; j < TRI_COUNT * 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				obstacle_car[j][k] = CubeFigure[0][j][k];
			}
			obstacle_car_color[j][0] = 0.3f;
			obstacle_car_color[j][1] = 0.3f;
			obstacle_car_color[j][2] = 0.3f;
		}
	}
}
