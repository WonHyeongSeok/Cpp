#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <thread>
#include <string>
#include <vector>

using namespace std;

enum Keycode
{
	Right = 0,
	Left = 1,
	Down = 2,
	Z = 3
};
int fieldWidth = 12;  //내가 콘솔창 안에서 테트리스 게임할 넓이
int fieldHeight = 18; //내가 콘솔창 안에서 테트리스 게임할 높이
unsigned char *pField = nullptr; //필드창을 가르킬포인터
wchar_t *screen = nullptr; //콘솔 스크린을 가르킬 포인터
wstring tetromino[7];
int screenWidth = 80; //Console창 전체의 스크린 넓이 사이즈
int screenHeight = 30; //Console창 전체의  스크린 높이 사이즈
bool getKey[4];
int currentPieceNum = 0;
int currentRotNum = 0;
int currentX = fieldWidth / 2;
int currentY = 0;
int speed = 20;
int speedCnt = 0;
bool isDown = false;
bool isHold = true;
int nPieceCount = 0;
int totalScore = 0;
int score = 25;
vector<int> vLines;
bool isGameOver = false;

//Render
HANDLE hConsole;
DWORD dwBytesWritten = 0;

int Rotate(int px, int py, int rotNum);
bool DoesPieceFit(int tetromino, int rotNum, int posX, int posY);
void Init();
void CreateTetromino();
void Rendering();
void Update();
void KeyEvent();
void GameEnd();
int main()
{
	Init();

	Update();

	GameEnd();

	return 0;
}

/*게임 로직 무한반복*/
void Update()
{
	while (!isGameOver)
	{
		// Timing
		this_thread::sleep_for(50ms); // Small Step = 1 Game Tick
		speedCnt++;
		isDown = (speedCnt == speed);
		KeyEvent();
		//시간 지남에 따라 테트리스 조각이 강제로 밑으로 내려감
		if (isDown)
		{
			// Update difficulty every 50 pieces
			speedCnt = 0;
			nPieceCount++;
			if (nPieceCount % 50 == 0)
				if (speed >= 10) speed--;

			// 지금 테트리스 조각이 밑으로 내려갈 수 있는지 체크
			if (DoesPieceFit(currentPieceNum, currentRotNum, currentX, currentY + 1))
				currentY++; // 내려감
			else
			{
				//내려가지 못할 때 체크
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[currentPieceNum][Rotate(px, py, currentRotNum)] != L'.')
							pField[(currentY + py) * fieldWidth + (currentX + px)] = currentPieceNum + 1;

				// 라인 체크
				for (int py = 0; py < 4; py++)
					if (currentY + py < fieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < fieldWidth - 1; px++)
							bLine &= (pField[(currentY + py) * fieldWidth + px]) != 0;

						if (bLine)
						{
							//라인이 꽉 찾을데 제거함
							for (int px = 1; px < fieldWidth - 1; px++)
								pField[(currentY + py) * fieldWidth + px] = 8;
							vLines.push_back(currentY + py);
						}
					}

				//테트리스 조각을 꽉 채웠을 때 점수 계산
				if (!vLines.empty())
					totalScore += score;

				// 새로운 조각 0~6번 과 위치 다시 생성
				currentX = fieldWidth / 2;
				currentY = 0;
				currentRotNum = 0;
				currentPieceNum = rand() % 7;

				isGameOver = !DoesPieceFit(currentPieceNum, currentRotNum, currentX, currentY);
			}
		}
		Rendering();
	}
}

int Rotate(int px, int py, int rotNum)
{
	int pieceIndex = 0;
	switch (rotNum % 4)
	{												// 0 도 정위치
	case 0:										// 0  1  2  3
		pieceIndex = py * 4 + px;			// 4  5  6  7
		break;										// 8  9 10 11
													//12 13 14 15

													// 90도 오른쪽 돌릴 때
	case 1: 										//12  8  4  0
		pieceIndex = 12 + py - (px * 4);	//13  9  5  1
		break;										//14 10  6  2
													//15 11  7  3

													// 180도 오른쪽 돌릴 때
	case 2:										//15 14 13 12
		pieceIndex = 15 - (py * 4) - px;	//11 10  9  8
		break;										// 7  6  5  4
													// 3  2  1  0
													// 270 도 오른쪽 돌릴 때
	case 3: 										// 3  7 11 15
		pieceIndex = 3 - py + (px * 4);		// 2  6 10 14
		break;										// 1  5  9 13
	}												// 0  4  8 12

	return pieceIndex;
}
bool DoesPieceFit(int pieceNum, int rotNum, int posX, int posY)
{
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			// 4*4인 테트리스 조각의 인덱스
			int pieceIndex = Rotate(px, py, rotNum);

			// 테트리스 게임 필드 인덱스
			int fieldIndex = (posY + py) * fieldWidth + (posX + px);

			// tetromino는 4*4로 이루어진 테트리스 조각 X는 우리가 보여지는 조각이고 그 조각이 충돌 되었는지 안되었는지 체크 할것
			if (posX + px >= 0 && posX + px < fieldWidth)
			{
				if (posY + py >= 0 && posY + py < fieldHeight)
				{
					if (tetromino[pieceNum][pieceIndex] == L'X' && pField[fieldIndex] != 0)// 0 = ' '을 제외하면 충돌이라고 판정
						return false; // fail on first hit
				}
			}
		}

	return true;
}
void CreateTetromino()
{
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"..X.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".X..");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"..X.");
	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"....");
	tetromino[5].append(L".XX.");
	tetromino[5].append(L"..X.");
	tetromino[5].append(L"..X.");

	tetromino[6].append(L"....");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L".X..");
	tetromino[6].append(L".X..");
}

/* 게임에 필요한 화면, 테트리스 필드, 테트리스 조각 초기화 및 생성.*/
void Init()
{
	//콘솔크기 창 할당
	screen = new wchar_t[screenWidth*screenHeight];
	for (int i = 0; i < screenWidth*screenHeight; i++)
		screen[i] = L' ';

	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);

	//테트리스 필드 할당
	pField = new unsigned char[fieldWidth*fieldHeight]; // Create play field buffer
	for (int x = 0; x < fieldWidth; x++) // Board Boundary
		for (int y = 0; y < fieldHeight; y++)
			pField[y*fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 9 : 0;

	CreateTetromino();
}
void Rendering()
{
	//콘솔창 렌더링
	for (int x = 0; x < fieldWidth; x++)
		for (int y = 0; y < fieldHeight; y++)
			screen[(y + 2)*screenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y*fieldWidth + x]];

	// 현재 조각 렌더링
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
			if (tetromino[currentPieceNum][Rotate(px, py, currentRotNum)] != L'.')
				screen[(currentY + py + 2)*screenWidth + (currentX + px + 2)] = currentPieceNum + 65;

	// 점수 렌더링
	swprintf_s(&screen[2 * screenWidth + fieldWidth + 6], 16, L"SCORE: %8d", totalScore);

	if (!vLines.empty())
	{
		//콘솔에서 렌더링 할 때 필요한 기능
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
		this_thread::sleep_for(400ms);

		for (auto &v : vLines)
			for (int px = 1; px < fieldWidth - 1; px++) // 0 과 마지막은 #으로 채워져 있으니 그것을 제외하고
			{
				for (int py = v; py > 0; py--)
					pField[py * fieldWidth + px] = pField[(py - 1) * fieldWidth + px];
				pField[px] = 0; // 0 = ' '; 없애버림
			}

		vLines.clear();
	}
	//콘솔에서 렌더링 할 때 필요한 기능
	WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
}

/*사용자의 Input을 받는 곳*/
void KeyEvent()
{
	for (int i = 0; i < 4; i++)								// Reft   Light   Down Z(rot)
		getKey[i] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[i]))) != 0;

	// 키를 눌렀을 때 테트리스의 이동
	currentX += (getKey[Right] && DoesPieceFit(currentPieceNum, currentRotNum, currentX + 1, currentY)) ? 1 : 0;
	currentX -= (getKey[Left] && DoesPieceFit(currentPieceNum, currentRotNum, currentX - 1, currentY)) ? 1 : 0;
	currentY += (getKey[Down] && DoesPieceFit(currentPieceNum, currentRotNum, currentX, currentY + 1)) ? 1 : 0;

	// 회전키 누르기
	if (getKey[Z])
	{
		currentRotNum += (isHold && DoesPieceFit(currentPieceNum, currentRotNum + 1, currentX, currentY)) ? 1 : 0;
		isHold = false;
	}
	else
		isHold = true;
}
void GameEnd()
{
	CloseHandle(hConsole);
	cout << "Game Over!! Score:" << totalScore << endl;
	system("pause");
}