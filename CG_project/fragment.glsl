﻿#version 330 core
//--- out_Color: 버텍스 세이더에서 입력받는 색상 값
//--- FragColor: 출력할 색상의 값으로 프레임 버퍼로 전달 됨. 

in vec3 passColor;		//--- 버텍스 세이더에게서 전달 받음
out vec4 FragColor;		//--- 색상 출력

void main(void) 
{
	FragColor = vec4 (passColor, 1.0);
}