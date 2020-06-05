#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <Windows.h>
#include <fstream>
using namespace std;

auto tp1 = chrono::system_clock::now(); // ��������� ����������
auto tp2 = chrono::system_clock::now(); // ����

class RayCasting
{
	int nScreenWidth = 240;			// �����
	int nScreenHeight = 80;

	const int nMapWidth = 16;	    // ���
	const int nMapHeight = 16;

	float fPlayerX = 14.7f;			// ��������� �������
	float fPlayerY = 5.09f;
	float fPlayerA = 0.0f;			// ���������� ���
	float fFOV = 3.14159f / 4.0f;	// ���� ����
	float fDepth = 16.0f;			// ����������� ������ ���������
	float fSpeed = 5.0f;			// �������� ����
	float fElapsedTime = 0.0f;      // ���������� ���

	// ����� ������
	wchar_t* screen;
	HANDLE hConsole;
	DWORD dwBytesWritten;

	// �����, . - ����� �������, # - ����
	wstring map = fillTheMap();
public:

	// ��������� ������ ������
	void createScreenBuffer()
	{
		screen = new wchar_t[nScreenWidth * nScreenHeight];
		hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleActiveScreenBuffer(hConsole);
		dwBytesWritten = 0;
	}

	// ���������� �����
	wstring fillTheMap()
	{
		wstring wMap, tempMap;
		wifstream f(L"map.txt");
		while (!f.eof())
		{
			f >> tempMap;
			wMap += tempMap;
		}
		f.close();
		return wMap;
	}

	// ��������� �����
	void startChoice()
	{
		string command = "";
		while (command != "start")
		{
			cin >> command;
			if (command == "map")
				editMapFile();
		}
	}

	// ³����� ���� � ������
	void editMapFile()
	{
		ShellExecute(NULL, L"open", L"map.txt", NULL, NULL, SW_SHOWNORMAL);
	}

	// �������� ������
	void turningAround()
	{
		// ������� �� ������������
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;

		// ������� ����� �����������
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (fSpeed * 0.75f) * fElapsedTime;
	}

	// ��� ������
	void movingForwardAndBackward()
	{
		// ��� ������
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;;
			fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;;
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#') // ������� ������� � ������
			{
				fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;;
				fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;;
			}
		}

		// ��� �����
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;;
			fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;;
			if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#') // ������� ������� � ������
			{
				fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;;
				fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;;
			}
		}
	}

	// ̳�������
	void showMiniMap()
	{
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';
	}

	// ���� �����, �� �������� �� �������
	short drawWalls(float fDistanceToWall, bool bBoundary)
	{
		short nShade = ' ';
		if (fDistanceToWall <= fDepth / 4.0f)			nShade = 0x2588;	// ���� �������	
		else if (fDistanceToWall < fDepth / 3.0f)		nShade = 0x2593;
		else if (fDistanceToWall < fDepth / 2.0f)		nShade = 0x2592;
		else if (fDistanceToWall < fDepth)				nShade = 0x2591;
		else											nShade = ' ';		// ���� ������

		if (bBoundary)nShade = ' '; // ����������
		return nShade;
	}

	// ���� ������
	short drawFloor(float b, short nShade)
	{
		if (b < 0.25)	    return '#';
		else if (b < 0.5)	return 'x';
		else if (b < 0.75)	return '.';
		else if (b < 0.9)	return '-';
		else				return ' ';
	}

	// ������� ����
	void gameProcess()
	{
		startChoice();
		fillTheMap();
		createScreenBuffer();
		while (1) // ������� ����
		{
			// ������������� ���� ���� ��� ��������� �����������
			// ���������� ����, ��� ������������ ������� ��������
			tp2 = chrono::system_clock::now();
			chrono::duration<float> elapsedTime = tp2 - tp1;
			tp1 = tp2;
			fElapsedTime = elapsedTime.count();

			turningAround();

			movingForwardAndBackward();

			for (int x = 0; x < nScreenWidth; x++)
			{
				float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV; // ������ �������
				// ³������ �� �����


				float fStepSize = 0.1f;		  // ��������� ��� ��������, ��������� =  ��������� ���									
				float fDistanceToWall = 0.0f; // ³������ �� ��������� �� ������� fRayAngle

				bool bHitWall = false;		// ������ ����� �����
				bool bBoundary = false;		// ������ ����� ���� �� �������

				float fEyeX = sinf(fRayAngle); // ���������� fRayAngle
				float fEyeY = cosf(fRayAngle);

				// ���� �� ��������� � ����� ��� �� ������ �� ����� ��������
				while (!bHitWall && fDistanceToWall < fDepth)
				{
					fDistanceToWall += fStepSize;
					int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall); // ʳ���� ����������
					int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall); // �������

					// ���� ������ �� ����
					if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
					{
						bHitWall = true;
						fDistanceToWall = fDepth;
					}
					else
					{
						// ������ �������, ����������, �� ������ ����� �� ����
						if (map.c_str()[nTestX * nMapWidth + nTestY] == '#')
						{
							// ������ �������� � ����
							bHitWall = true;

							vector<pair<float, float>> p;   // ������ �����

							// ��������� �� ��� 4 ������
							for (int tx = 0; tx < 2; tx++)
								for (int ty = 0; ty < 2; ty++)
								{
									float vy = (float)nTestY + ty - fPlayerY; // ���������� �������, ��
									float vx = (float)nTestX + tx - fPlayerX; // ��� �� ����������� �� �����
									float d = sqrt(vx * vx + vy * vy); // ������ ����� �������
									float dot = (fEyeX * vx / d) + (fEyeY * vy / d); // ��������� �������
									p.push_back(make_pair(d, dot)); // �������� ��������� � �����
								}

							// ������� ����� �� �� ������� �������
							sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

							// ����� 2/3 ��������� (4 �������� ���������)
							float fBound = 0.01; // ���, ��� ����� �������� ������ �����
							if (acos(p.at(0).second) < fBound) bBoundary = true;
							if (acos(p.at(1).second) < fBound) bBoundary = true;
							if (acos(p.at(2).second) < fBound) bBoundary = true;
						}
					}
				}

				// ������ ������� �� ���� � ������
				int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
				int nFloor = nScreenHeight - nCeiling;

				// ����������� �����, � ��������� �� ��������
				short nShade = drawWalls(fDistanceToWall, bBoundary);

				for (int y = 0; y < nScreenHeight; y++)
				{
					// ����� ���
					if (y <= nCeiling)
						screen[y * nScreenWidth + x] = ' ';
					else if (y > nCeiling && y <= nFloor)
						screen[y * nScreenWidth + x] = nShade;
					else // ϳ�����
					{
						// ����������� ������, � ��������� �� �������
						float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
						nShade = drawFloor(b, nShade);
						screen[y * nScreenWidth + x] = nShade;
					}
				}
			}

			// ����������
			swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

			// ³���������� ��������
			showMiniMap();

			// ³���������� �����
			screen[nScreenWidth * nScreenHeight - 1] = '\0';
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		}
	}
};