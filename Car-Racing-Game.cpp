#include <iostream>
#include <string>
using namespace std;

#include "olcConsoleGameEngine.h"
class OneLoneCoder_FormulaOLC : public olcConsoleGameEngine
{
public:
	OneLoneCoder_FormulaOLC()
	{
		m_sAppName = L"Formula OLC";
	}

private:
	float fCarPos = 0.0f;
	float fDistance = 0.0f;
	float fSpeed = 0.0f;

	// Track for the car racing with curvature and distance
	vector<pair<float, float>> vecTrack;

	// Curvature at any given point in time
	float fCurvature = 0.0f;
	// Curvature of the single particle moving along the track
	float fTrackCurvature = 0.0f;
	// Player curvature
	float fPlayerCurvature = 0.0f;
	// Track Distance
	float fTrackDistance = 0.0f;

	// Reccord lap time
	float fCurrentLapTime = 0.0f;
	list<float> listLapTimes; 

	
protected:

	virtual bool OnUserCreate()
	{
		vecTrack.push_back(make_pair(0.0f, 10.0f));
		vecTrack.push_back(make_pair(0.0f, 200.0f));
		vecTrack.push_back(make_pair(1.0f, 200.0f));
		vecTrack.push_back(make_pair(0.0f, 400.0f));
		vecTrack.push_back(make_pair(-1.0f, 100.0f));
		vecTrack.push_back(make_pair(0.0f, 200.0f));
		vecTrack.push_back(make_pair(-1.0f, 200.0f));
		vecTrack.push_back(make_pair(1.0f, 200.0f));

		for (auto itr : vecTrack)
			fTrackDistance += itr.second;

		listLapTimes = { 0, 0, 0, 0, 0 };

		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime)
	{
		if (m_keys[VK_UP].bHeld)
			fSpeed += 2.0f * fElapsedTime;
		else
			fSpeed -= 1.0f * fElapsedTime;

		int nCarDirection = 0;

		if (m_keys[VK_LEFT].bHeld) {
			fPlayerCurvature -= 0.7f * fElapsedTime;
			nCarDirection = -1;
		}

		if (m_keys[VK_RIGHT].bHeld) {
			fPlayerCurvature += 0.7f * fElapsedTime;
			nCarDirection = +1;
		}

		// Decrease car speed if on grass
		if (fabs(fPlayerCurvature - fTrackCurvature) >= 0.8f)
			fSpeed -= 5.0f * fElapsedTime;

		// Clamp speed
		if (fSpeed < 0.0f)
			fSpeed = 0.0f;
		if (fSpeed > 1.0f)
			fSpeed = 1.0f;

		// Distance based on car speed
		fDistance += (70.0f * fSpeed) * fElapsedTime;

		// Position on Track
		float fOffset = 0;
		int nTrackSection = 0;

		fCurrentLapTime += fElapsedTime;

		if (fDistance >= fTrackDistance) {
			fDistance -= fTrackDistance;
			
			listLapTimes.push_front(fCurrentLapTime);
			listLapTimes.pop_back();

			fCurrentLapTime = 0.0f;
		}


		while (nTrackSection < vecTrack.size() && fOffset <= fDistance) {
			fOffset += vecTrack[nTrackSection].second;
			nTrackSection++;
		}

		float fTargetCurvature = vecTrack[nTrackSection - 1].first;
		float fTrackCurveDiff = (fTargetCurvature - fCurvature) * fElapsedTime * fSpeed;
		fCurvature += fTrackCurveDiff; 

		fTrackCurvature += (fCurvature)*fElapsedTime * fSpeed;

		// Sky in the upper half
		for (int y = 0; y < ScreenHeight(); y++) {
			for (int x = 0; x < ScreenWidth(); x++) {
				Draw(x, y, y < ScreenHeight() / 4 ? PIXEL_HALF : PIXEL_SOLID, FG_DARK_BLUE);
			}
		}

		// Mountains in the background
		for (int x = 0; x < ScreenWidth(); x++) {
			int nHillHeight = (int)(fabs(sinf(x * 0.01f + fTrackCurvature) * 16.0f));

			for (int y = (ScreenHeight() / 2) - nHillHeight; y < ScreenHeight() / 2; y++) {
				Draw(x, y, PIXEL_SOLID, FG_DARK_YELLOW);
			}
		}
		
		// Dividing screen into half (vertically) for road and grass (each horizontally)
		for (int y = 0; y < ScreenHeight() / 2; y++) {
			for (int x = 0; x < ScreenWidth(); x++) {

				float fPerspective = (float)y / (ScreenHeight() / 2.0f);

				float fMiddlePoint = 0.5f + fCurvature * powf((1 - fPerspective), 3);
				float fRoadWidth= 0.1f + fPerspective * 0.8f;
				float fClipWidth = fRoadWidth * 0.15f;

				fRoadWidth *= 0.5f;

				int nLeftGrass = ( fMiddlePoint - fRoadWidth - fClipWidth )  * ScreenWidth();
				int nRightGrass = ( fMiddlePoint + fRoadWidth + fClipWidth ) * ScreenWidth();

				int nLeftClip = (fMiddlePoint - fRoadWidth) * ScreenWidth();
				int nRightClip = (fMiddlePoint + fRoadWidth) * ScreenWidth();

				int nRow = ScreenHeight() / 2 + y;

				int nGrassColour = sinf(20.0f * powf(1.0f - fPerspective, 3) + fDistance * 0.1f) > 0.0f ? FG_DARK_GREEN : FG_GREEN;
				int nClipColour = sinf(80.0f * powf(1.0f - fPerspective, 3) + fDistance * 0.1f) > 0.0f ? FG_BLACK: FG_WHITE;

				int nRoadColour = (nTrackSection - 1) == 0 ? FG_WHITE : FG_DARK_GREY;

				if (x >= 0 && x < nLeftGrass) {
					Draw(x, nRow, PIXEL_SOLID, nGrassColour);
				}
				if ( x >= nLeftGrass && x < nLeftClip ) {
					Draw(x, nRow, PIXEL_SOLID, nClipColour);
				}
				if (x >= nLeftClip && x < nRightClip) {
					Draw(x, nRow, PIXEL_SOLID, nRoadColour);
				}
				if (x >= nRightClip && x < nRightGrass) {
					Draw(x, nRow, PIXEL_SOLID, nClipColour);
				} 
				if (x >= nRightGrass && x < ScreenWidth()) {
					Draw(x, nRow, PIXEL_SOLID, nGrassColour);
				}
			}
		}

		// Car Drawing

		// Drift car left or right based on the curvature
		fCarPos = fPlayerCurvature - fTrackCurvature;

		int nCarPos = ScreenWidth() / 2 + ((int)(ScreenWidth() * fCarPos) / 2.0f) - 7;
		switch (nCarDirection)
		{
		case 0:
			DrawStringAlpha(nCarPos, 80, L"   ||####||   ");
			DrawStringAlpha(nCarPos, 81, L"      ##      ");
			DrawStringAlpha(nCarPos, 82, L"     ####     ");
			DrawStringAlpha(nCarPos, 83, L"     ####     ");
			DrawStringAlpha(nCarPos, 84, L"|||  ####  |||");
			DrawStringAlpha(nCarPos, 85, L"|||########|||");
			DrawStringAlpha(nCarPos, 86, L"|||  ####  |||");
			break;

		case +1:
			DrawStringAlpha(nCarPos, 80, L"      //####//");
			DrawStringAlpha(nCarPos, 81, L"         ##   ");
			DrawStringAlpha(nCarPos, 82, L"       ####   ");
			DrawStringAlpha(nCarPos, 83, L"      ####    ");
			DrawStringAlpha(nCarPos, 84, L"///  ####//// ");
			DrawStringAlpha(nCarPos, 85, L"//#######///O ");
			DrawStringAlpha(nCarPos, 86, L"/// #### //// ");
			break;

		case -1:
			DrawStringAlpha(nCarPos, 80, L"\\\\####\\\\      ");
			DrawStringAlpha(nCarPos, 81, L"   ##         ");
			DrawStringAlpha(nCarPos, 82, L"   ####       ");
			DrawStringAlpha(nCarPos, 83, L"    ####      ");
			DrawStringAlpha(nCarPos, 84, L" \\\\\\\\####  \\\\\\");
			DrawStringAlpha(nCarPos, 85, L" O\\\\\\#######\\\\");
			DrawStringAlpha(nCarPos, 86, L" \\\\\\\\ #### \\\\\\");
			break;
		}

		// Draw Stats
		DrawString(0, 0, L"Distance: " + to_wstring(fDistance));
		DrawString(0, 1, L"Target Curvature: " + to_wstring(fCurvature));
		DrawString(0, 2, L"Player Curvature: " + to_wstring(fPlayerCurvature));
		DrawString(0, 3, L"Player Speed    : " + to_wstring(fSpeed));
		DrawString(0, 4, L"Track Curvature : " + to_wstring(fTrackCurvature));

		auto disp_time = [](float t) {
			int nMinutes = t / 60.0f;
			int nSeconds = t - (nMinutes * 60.0f);
			int nMilliSeconds = (t - (float)nSeconds) * 1000.0f;

			return to_wstring(nMinutes) + L":" + to_wstring(nSeconds) + L":" + to_wstring(nMilliSeconds);
		};

		//Display current lap time
		DrawString(10, 8, disp_time(fCurrentLapTime));

		// Display last 5 lap times
		int drawOnNextLine = 10;
		for (auto l : listLapTimes)
		{
			DrawString(10, drawOnNextLine, disp_time(l));
			drawOnNextLine++;
		}

		return true;
	}
};


int main()
{
	OneLoneCoder_FormulaOLC game;
	game.ConstructConsole(160, 100, 8, 8);
	game.Start();
	 
	return 0;
}