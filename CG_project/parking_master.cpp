﻿//링커-명령줄 : glew32.lib freeglut.lib
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

// 도형 변수 및 함수
void initFigure();
void initRobot();
bool isProspect = true;		//투영
bool isCull = false;			//은면
bool isWire = false;		//솔리드와이어
#define MAX_FIGURE 6	// 육면체 = 삼각형 12개, 육면체 '6개'로 크레인 구현					
#define TRI_COUNT 12


#define ROBOT_SIZE 0.02f

//Box 그리기
void initBox();
#define BOX_SIZE 0.5f
GLfloat Box[14 * 3][3];
GLfloat	Box_Color[14 * 3][3];

//Block 그리기 - 랜덤 위치에 3종류
void initBlock();
#define BLOCK_SIZE 0.5f
#define WHEEL_SIZE BLOCK_SIZE / 2
GLfloat Block[3][12 * 3][3];
GLfloat	Block_Color[3][12 * 3][3];

// 크레인 그리기 - vao 2
GLfloat	figure[MAX_FIGURE][TRI_COUNT * 3][3];	//0: 몸통, 1: 중간몸통, 2,3: 위팔, 4,5: 포신
GLfloat	figure_Color[MAX_FIGURE][TRI_COUNT * 3][3];

// 땅바닥 그리기 - vao 1
GLfloat ground[6][3] = {
	{-1, 0, -1}, {1, 0, -1}, {-1, 0, 1},
	{-1, 0, 1}, {1, 0, -1}, {1, 0, 1}
};
GLfloat ground_color[6][3] = {
	{0.8f, 0.8f, 0.8f},
	{0.8f, 0.8f, 0.8f},
	{0.8f, 0.8f, 0.8f},

	{0.8f, 0.8f, 0.8f},
	{0.8f, 0.8f, 0.8f},
	{0.8f, 0.8f, 0.8f},
};

// 3차원 도형 모델 생성 (구, 원뿔)
GLUquadricObj* qobj;

// 필요 변수 선언
GLint width, height;
GLchar* vertexSource, * fragmentSource;			//--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader;			//--- 세이더 객체
GLuint shaderProgramID;							//--- 셰이더 프로그램
GLuint vao[5], vbo[6];							//--- VAO, VBO

// 사용자 정의 함수
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);

char* filetobuf(const char* file);
void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
GLvoid InitBuffer();

// 타이머 관련
#define TIMER_VELOCITY 16
void TimerFunction_angleY(int value);
int	timer_angleY = false;
bool isPlus = true;

// 변환
glm::mat4 SRT_MATRIX()
{
	//glm::mat4 T = glm::mat4(1.0f);			//--- 이동 행렬 선언
	glm::mat4 Rx = glm::mat4(1.0f);			//--- 회전 행렬 선언
	glm::mat4 Ry1 = glm::mat4(1.0f);		//--- 회전 행렬 선언
	//glm::mat4 Ry2 = glm::mat4(1.0f);
	glm::mat4 S = glm::mat4(1.0f);

	S = glm::scale(S, glm::vec3(1.0, 1.0, 1.0));								//--- 스케일 변환 행렬
	//Rx = glm::rotate(Rx, glm::radians(20.0f), glm::vec3(1.0, 0.0, 0.0));		//--- x축 회전 행렬 (고정)
	//T = glm::translate(T, glm::vec3(dx, dy, dz));								//--- 이동 행렬

	if (true)
	{
		Rx = glm::rotate(Rx, glm::radians(20.0f), glm::vec3(1.0, 0.0, 0.0));		//--- x축 회전 행렬 (고정)
		Ry1 = glm::rotate(Ry1, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));		//--- y축 회전 행렬
	}

	return Ry1;
}
// 바퀴를 원점으로 이동시키는 함수 + 기본적으로 90도 회전시킴
glm::mat4 Wheel_on_000(int type)
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Ry = glm::mat4(1.0f);
	//glm::mat4 S = glm::mat4(1.0f);

	Ry = glm::rotate(Ry, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
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

	return Ry * T;
}

float c_dx = 0.0f;
float c_dy = 1.0f;
float c_dz = 3.0f;
float c_angleY = 0.0f;
float c_rotateY = 0.0f;
bool timer_rotateCam = false;
bool timer_rotateCam_isPlus = false;
void TimerFunction_RotateCamera(int value)
{
	if (timer_rotateCam == true)
	{
		if (timer_rotateCam_isPlus)
		{
			c_rotateY += 1.0f;
		}
		else
		{
			c_rotateY -= 1.0f;
		}

		glutPostRedisplay(); // 화면 다시 그리기
		glutTimerFunc(TIMER_VELOCITY, TimerFunction_RotateCamera, 1);
	}
	else
	{
		std::cout << "stop a\n";
	}
}

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	srand(time(0));
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

	glEnable(GL_DEPTH_TEST);
	//setFigures(); // init
	//initFigure();
	//initBox();
	initBlock();

	//--- 세이더 읽어와서 세이더 프로그램 만들기
	make_shaderProgram();
	InitBuffer();

	glutDisplayFunc(drawScene);					//--- 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);					//--- 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard);					// 키보드 입력
	//glutSpecialFunc(SpecialKeyboard);			// 키보드 입력(방향키 등 스페셜)
	//glutMouseFunc(Mouse);						// 마우스 입력
	//glutMotionFunc(Motion);					// 마우스 움직임
	glutMainLoop();								//--- 이벤트 처리 시작
	gluDeleteQuadric(qobj);
}

//그리기 함수
void drawObjects(int modelLoc, int mod);
GLvoid drawScene()
{
	glViewport(0, 0, clientWidth, clientHeight);

	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	int modelLoc = glGetUniformLocation(shaderProgramID, "model");
	int viewLoc = glGetUniformLocation(shaderProgramID, "view");
	int projLoc = glGetUniformLocation(shaderProgramID, "projection");

	if (isCull)
	{
		glDisable(GL_DEPTH_TEST);
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
	}
	//--- 모델링변환, 뷰잉변환, 투영변환 행렬을 설정한 후, 버텍스 세이더에 저장한다.
	if (true)
	{
		//뷰잉변환
		// glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, cameraUp));
		glm::vec3 orbitCenter = glm::vec3(0.0f, 0.0f, 0.0f); // 공전 중심점
		glm::vec3 cameraPos = glm::vec3(c_dx, c_dy, c_dz);
		glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		// 화면 중심의 Y축 기준으로 공전 적용
		glm::mat4 cameraRotateMat = glm::rotate(glm::mat4(1.0f), glm::radians(c_rotateY), glm::vec3(0.0, 1.0, 0.0));
		cameraPos = glm::vec3(cameraRotateMat * glm::vec4(cameraPos - orbitCenter, 1.0f)) + orbitCenter;
		// 공전 이후 카메라 방향 업데이트
		cameraDirection = glm::normalize(orbitCenter - cameraPos);
		// 카메라의 Y축 기준으로 회전 적용
		cameraDirection = glm::rotate(glm::mat4(1.0f), glm::radians(c_angleY), cameraUp) * glm::vec4(cameraDirection, 0.0f);

		glm::mat4 vTransform = glm::mat4(1.0f);
		vTransform = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

		//투영변환
		glm::mat4 pTransform = glm::mat4(1.0f);
		if (isProspect == false)
		{	// 직각투영
			pTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 100.0f);
		}
		else
		{	// 원근투영
			pTransform = glm::perspective(glm::radians(45.0f), (float)clientWidth / (float)clientHeight, 0.1f, 50.0f);
		}
		//pTransform = glm::translate(pTransform, glm::vec3(0.0, 0.0, 0.0));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);

		//모델 그리기
		drawObjects(modelLoc, 0);
	}
	glutSwapBuffers();  // 화면에 출력하기
}
void draw_wheels(int modelLoc)
{
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_on_000(0)));
	GLUquadricObj* qobj1;
	qobj1 = gluNewQuadric();						// 객체 생성하기
	gluCylinder(qobj1, WHEEL_SIZE, WHEEL_SIZE, WHEEL_SIZE / 2, 20, 8);
	gluDeleteQuadric(qobj1);
	// 바퀴 뚜껑1
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_on_000(1)));
	GLUquadricObj* qobj2;
	qobj2 = gluNewQuadric();
	gluDisk(qobj2, 0.0f, WHEEL_SIZE, 20, 8);
	gluDeleteQuadric(qobj2);
	// 바퀴 뚜껑2
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Wheel_on_000(2)));
	GLUquadricObj* qobj3;
	qobj3 = gluNewQuadric();
	gluDisk(qobj3, 0.0f, WHEEL_SIZE, 20, 8);
	gluDeleteQuadric(qobj3);
}
void drawObjects(int modelLoc, int mod)
{
	// 바닥
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(SRT_MATRIX()));
	glBindVertexArray(vao[0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//// 차체
	//glBindVertexArray(vao[1]);
	//glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
	//glDrawArrays(GL_TRIANGLES, 36, 6 * 6);

	// 바퀴 Wheel_on_000
	draw_wheels(modelLoc);	//1
	draw_wheels(modelLoc);	//2
	draw_wheels(modelLoc);	//3
	draw_wheels(modelLoc);	//4
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}
GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':		// 프로그램 종료
	{
		std::cout << "--Quit--\n";
		glutLeaveMainLoop(); // OpenGL 메인 루프 종료
		break;
	}
	
	//은면제거
	case 'h':
	{
		if (isCull)
		{
			isCull = false;
		}
		else
		{
			isCull = true;
		}
		break;
	}
	//직각/원근 투영
	case 'p':
	{
		if (isProspect)
		{
			isProspect = false;
		}
		else
		{
			isProspect = true;
		}
		break;
	}

	//카메라 이동
	case 'z': case 'Z':
	{
		if (key == 'z')
		{
			c_dz += 0.1f;
		}
		else if (key == 'Z')
		{
			c_dz -= 0.1f;
		}
		break;
	}
	case 'x': case 'X':
	{
		if (key == 'x')
		{
			c_dx += 0.1f;
		}
		else if (key == 'X')
		{
			c_dx -= 0.1f;
		}
		break;
	}
	
	//카메라 공전
	case 'y': case 'Y':
	{
		if (key == 'y')
		{
			c_rotateY += 5.0f;
		}
		else if (key == 'Y')
		{
			c_rotateY -= 5.0f;
		}
		break;
	}
	}
	glutPostRedisplay(); //--- refresh
}

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
	//glGenVertexArrays(1, &vao);		//--- VAO 를 지정하고 할당하기
	//glBindVertexArray(vao);			//--- VAO를 바인드하기

	glGenVertexArrays(3, vao);
	glGenBuffers(6, vbo);

	// 좌표축 VAO, VBO 초기화

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

	// 도형 및 색상 데이터용 VAO, VBO 초기화
	glBindVertexArray(vao[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(figure), figure, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(figure_Color), figure_Color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);

}

void initBox()
{
	GLfloat rgb[8][3] = {
			{1.0f, 0.0f, 0.0f},		
			{0.0f, 1.0f, 0.0f},		
			{0.0f, 0.0f, 1.0f},		
			{1.0f, 1.0f, 0.0f},		
			{0.0f, 1.0f, 1.0f},		
			{0.0f, 1.0f, 1.0f},		
			{0.0f, 1.0f, 1.0f},		
			{0.0f, 1.0f, 1.0f},		
	};
	GLfloat BoxColorData[14 * 3][3] = {
		// Front face - 2tri
		{rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[0][0], rgb[0][1], rgb[0][2]},
		{rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[0][0], rgb[0][1], rgb[0][2]},

		// Front face - 2tri
		{rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[0][0], rgb[0][1], rgb[0][2]},
		{rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[0][0], rgb[0][1], rgb[0][2]},

		// Back face - 2tri
		{rgb[4][0], rgb[4][1], rgb[4][2]}, {rgb[5][0], rgb[5][1], rgb[5][2]}, {rgb[6][0], rgb[6][1], rgb[6][2]},
		{rgb[4][0], rgb[4][1], rgb[4][2]}, {rgb[6][0], rgb[6][1], rgb[6][2]}, {rgb[7][0], rgb[7][1], rgb[7][2]},

		// Left face - 2tri
		{rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[1][0], rgb[1][1], rgb[1][2]},
		{rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[1][0], rgb[1][1], rgb[1][2]},

		// Right face - 2tri
		{rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[1][0], rgb[1][1], rgb[1][2]},
		{rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[1][0], rgb[1][1], rgb[1][2]},

		// Top face - 2tri
		{rgb[2][0], rgb[2][1], rgb[2][2]}, {rgb[2][0], rgb[2][1], rgb[2][2]}, {rgb[2][0], rgb[2][1], rgb[2][2]},
		{rgb[2][0], rgb[2][1], rgb[2][2]}, {rgb[2][0], rgb[2][1], rgb[2][2]}, {rgb[2][0], rgb[2][1], rgb[2][2]},

		// Bottom face - 2tri
		{rgb[3][0], rgb[3][1], rgb[3][2]}, {rgb[3][0], rgb[3][1], rgb[3][2]}, {rgb[3][0], rgb[3][1], rgb[3][2]},
		{rgb[3][0], rgb[3][1], rgb[3][2]}, {rgb[3][0], rgb[3][1], rgb[3][2]}, {rgb[3][0], rgb[3][1], rgb[3][2]}
	};
	GLfloat vertices[10][3] = {
			{-BOX_SIZE, BOX_SIZE, BOX_SIZE},			// Vertex 0 앞, 왼쪽 위
			{-BOX_SIZE, -BOX_SIZE, BOX_SIZE},			// Vertex 1 앞, 왼쪽 아래
			{BOX_SIZE, -BOX_SIZE, BOX_SIZE},			// Vertex 2 앞, 오른쪽 아래
			{BOX_SIZE, BOX_SIZE, BOX_SIZE},			// Vertex 3 앞, 오른쪽 위
			{-BOX_SIZE, BOX_SIZE, -BOX_SIZE},			// Vertex 4 뒤, 왼쪽 위
			{-BOX_SIZE, -BOX_SIZE, -BOX_SIZE},			// Vertex 5 뒤, 왼쪽 아래
			{BOX_SIZE, -BOX_SIZE, -BOX_SIZE},			// Vertex 6 뒤, 오른쪽 아래
			{BOX_SIZE, BOX_SIZE, -BOX_SIZE},			// Vertex 7 뒤, 오른쪽 위

			{ 0.0f, BOX_SIZE, BOX_SIZE },		// Vertex 8 앞, 가운데 위
			{ 0.0f, -BOX_SIZE, BOX_SIZE}			// Vertex 9 앞, 가운데 아래
	};
	GLfloat BoxFigure[14 * 3][3] = {
		// 앞면(왼쪽) //019 098 892 823
		{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[9][0], vertices[9][1], vertices[9][2]},  // 삼각형 1
		{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[9][0], vertices[9][1], vertices[9][2]}, {vertices[8][0], vertices[8][1], vertices[8][2]},  // 삼각형 2

		// 앞면(오른쪽)
		{vertices[8][0], vertices[8][1], vertices[8][2]}, {vertices[9][0], vertices[9][1], vertices[9][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]},  // 삼각형 3
		{vertices[8][0], vertices[8][1], vertices[8][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]},  // 삼각형 4

		// 뒷면
		{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},  // 삼각형 5
		{vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},  // 삼각형 6

		// 왼쪽면
		{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[1][0], vertices[1][1], vertices[1][2]},  // 삼각형 7
		{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]},  // 삼각형 8

		// 오른쪽면
		{vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},  // 삼각형 9
		{vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},  // 삼각형 10

		// 윗면
		{vertices[0][0], vertices[0][1], vertices[0][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[4][0], vertices[4][1], vertices[4][2]},  // 삼각형 11
		{vertices[4][0], vertices[4][1], vertices[4][2]}, {vertices[3][0], vertices[3][1], vertices[3][2]}, {vertices[7][0], vertices[7][1], vertices[7][2]},  // 삼각형 12

		// 바닥면
		{vertices[1][0], vertices[1][1], vertices[1][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[5][0], vertices[5][1], vertices[5][2]},  // 삼각형 13
		{vertices[5][0], vertices[5][1], vertices[5][2]}, {vertices[2][0], vertices[2][1], vertices[2][2]}, {vertices[6][0], vertices[6][1], vertices[6][2]},  // 삼각형 14
	};
	//아래 몸통 
	for (int j = 0; j < 14 * 3; j++)
	{
		for (int k = 0; k < 3; k++)
		{
			Box[j][k] = BoxFigure[j][k];
			Box_Color[j][k] = BoxColorData[j][k];
		}
	}
}
void initBlock()
{
	GLfloat rgb[8][3] = {
		{1.0f, 1.0f, 1.0f},		// Vertex 0 white
		{1.0f, 1.0f, 0.0f},		// Vertex 1 110
		{1.0f, 0.0f, 0.0f},		// Vertex 2 red
		{1.0f, 0.0f, 1.0f},		// Vertex 3 101
		{0.0f, 1.0f, 1.0f},		// Vertex 4 011
		{0.0f, 1.0f, 0.0f},		// Vertex 5 green
		{0.0f, 0.0f, 0.0f},		// Vertex 6 black
		{0.0f, 0.0f, 1.0f},		// Vertex 7 blue
	};
	GLfloat CubeColorData[1][TRI_COUNT * 3][3] = {
		{
			// Front face - 2tri
			{rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[2][0], rgb[2][1], rgb[2][2]},
			{rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[2][0], rgb[2][1], rgb[2][2]}, {rgb[3][0], rgb[3][1], rgb[3][2]},

			// Back face - 2tri
			{rgb[4][0], rgb[4][1], rgb[4][2]}, {rgb[5][0], rgb[5][1], rgb[5][2]}, {rgb[6][0], rgb[6][1], rgb[6][2]},
			{rgb[4][0], rgb[4][1], rgb[4][2]}, {rgb[6][0], rgb[6][1], rgb[6][2]}, {rgb[7][0], rgb[7][1], rgb[7][2]},

			// Left face - 2tri
			{rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[4][0], rgb[4][1], rgb[4][2]}, {rgb[7][0], rgb[7][1], rgb[7][2]},
			{rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[7][0], rgb[7][1], rgb[7][2]}, {rgb[3][0], rgb[3][1], rgb[3][2]},

			// Right face - 2tri
			{rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[5][0], rgb[5][1], rgb[5][2]}, {rgb[6][0], rgb[6][1], rgb[6][2]},
			{rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[6][0], rgb[6][1], rgb[6][2]}, {rgb[2][0], rgb[2][1], rgb[2][2]},

			// Top face - 2tri
			{rgb[3][0], rgb[3][1], rgb[3][2]}, {rgb[2][0], rgb[2][1], rgb[2][2]}, {rgb[6][0], rgb[6][1], rgb[6][2]},
			{rgb[3][0], rgb[3][1], rgb[3][2]}, {rgb[6][0], rgb[6][1], rgb[6][2]}, {rgb[7][0], rgb[7][1], rgb[7][2]},

			// Bottom face - 2tri
			{rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[1][0], rgb[1][1], rgb[1][2]}, {rgb[5][0], rgb[5][1], rgb[5][2]},
			{rgb[0][0], rgb[0][1], rgb[0][2]}, {rgb[5][0], rgb[5][1], rgb[5][2]}, {rgb[4][0], rgb[4][1], rgb[4][2]}
		}
	};

	//아래 몸체
	if (true)
	{
		GLfloat vertices[8][3] = {
			{-BLOCK_SIZE / 2,	0.0f,			-BLOCK_SIZE},	// Vertex 0
			{ BLOCK_SIZE / 2,	0.0f,			-BLOCK_SIZE},	// Vertex 1
			{ BLOCK_SIZE / 2,	BLOCK_SIZE / 2,	-BLOCK_SIZE},   // Vertex 2
			{-BLOCK_SIZE / 2,	BLOCK_SIZE / 2, -BLOCK_SIZE },  // Vertex 3
			{-BLOCK_SIZE / 2,	0.0f,			 BLOCK_SIZE },  // Vertex 4
			{ BLOCK_SIZE / 2,	0.0f,			 BLOCK_SIZE },  // Vertex 5
			{ BLOCK_SIZE / 2,	BLOCK_SIZE / 2,  BLOCK_SIZE },  // Vertex 6
			{-BLOCK_SIZE / 2,	BLOCK_SIZE / 2,  BLOCK_SIZE }   // Vertex 7
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
			{-BLOCK_SIZE / 3,	BLOCK_SIZE / 2,						-BLOCK_SIZE/3*2},	// Vertex 0
			{ BLOCK_SIZE / 3,	BLOCK_SIZE / 2,						-BLOCK_SIZE / 3 * 2},	// Vertex 1
			{ BLOCK_SIZE / 3,	BLOCK_SIZE / 2 + BLOCK_SIZE / 3,	-BLOCK_SIZE / 3 * 2},   // Vertex 2
			{-BLOCK_SIZE / 3,	BLOCK_SIZE / 2 + BLOCK_SIZE / 3,	-BLOCK_SIZE / 3 * 2 },  // Vertex 3
			{-BLOCK_SIZE / 3,	BLOCK_SIZE / 2,						BLOCK_SIZE / 3 * 2 },  // Vertex 4
			{ BLOCK_SIZE / 3,	BLOCK_SIZE / 2,						BLOCK_SIZE / 3 * 2 },  // Vertex 5
			{ BLOCK_SIZE / 3,	BLOCK_SIZE / 2 + BLOCK_SIZE / 3,	BLOCK_SIZE / 3 * 2 },  // Vertex 6
			{-BLOCK_SIZE / 3,	BLOCK_SIZE / 2 + BLOCK_SIZE / 3,	BLOCK_SIZE / 3 * 2 }   // Vertex 7
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
}
