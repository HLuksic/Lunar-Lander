﻿//#define DEBUG

#include <iostream>
#include <random>
#include <ctime>
#include <deque>
#include <algorithm>
#include "olcPixelGameEngine.h"

#include "background.h"
#include "interface.h"
#include "player.h"
#include "terrain.h"
#include "global.h"

// TODO: 
// FOREGROUND,
// DEBRIS PARTICLES, 
// SOUNDS,
// HIGH SCORE, 
// UI TEXT FIXES,
// DRAW EARTH ON TITLE SCREEN

class lunarLander : public olc::PixelGameEngine
{
public:
	lunarLander()
	{
		sAppName = "Lunar lander";
	}

private:
	Player* player;
	Terrain* terrain;
	Interface* userInterface;
	Background* background;

public:
	bool OnUserCreate() override
	{
		srand((unsigned int)time(0));
		player			= new Player;
		terrain			= new Terrain;
		userInterface	= new Interface;
		background		= new Background;
		paused			= false;
		titleScreen		= true;
		scale			= 0.5f;

		return true;
	}

	#ifdef DEBUG
	void Debug()
	{
		// Highlight leftmost and rightmost segments
		if (!deqSegments.empty())
		{
			DrawLine(
				deqSegments.front().leftNode * scale + adjustedPos, 
				deqSegments.front().rightNode * scale + adjustedPos, 
				olc::GREEN
			);
			DrawCircle(
				deqSegments.front().rightNode * scale + adjustedPos, 
				5, 
				olc::GREEN
			);
			DrawLine(
				deqSegments.back().leftNode * scale + adjustedPos, 
				deqSegments.back().rightNode * scale + adjustedPos, 
				olc::RED
			);
			DrawCircle(
				deqSegments.back().leftNode * scale + adjustedPos, 
				5, 
				olc::RED
			);
		}
		// Highlight segment & exact point under player
		for (auto& segment : deqSegments)
		{
			if (segment.leftNode.x < playerPos.x && segment.rightNode.x > playerPos.x)
			{
				DrawCircle(segment.leftNode * scale + adjustedPos, 5, olc::YELLOW);
				DrawCircle(segment.rightNode * scale + adjustedPos, 5, olc::YELLOW);
		
				float diffy = segment.rightNode.y - segment.leftNode.y;
				float diffx = segment.rightNode.x - segment.leftNode.x;
				float mult = (playerPos.x - segment.leftNode.x) / diffx;
				olc::vf2d newpoint = { playerPos.x, segment.leftNode.y + diffy * mult };
				DrawCircle(newpoint * scale + adjustedPos, 5, olc::YELLOW);
			}
		}
		// Player collision circle
		DrawCircle(playerPos * scale + adjustedPos, int(7 * scale), olc::RED);
		// Deque sizes
		DrawString(50, 50, std::to_string(deqSegments.size()));
		DrawString(60, 60, std::to_string(deqBgSegments.size()));
	}
	#endif

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);

		if (GetKey(olc::ESCAPE).bPressed)
			paused = true;

		if (GetKey(olc::SPACE).bPressed)
			paused = false;

		if (titleScreen)
			userInterface->TitleScreen(this, player);
		else
		{
			background->Draw(this, player);
			terrain->Spawn(player);
			terrain->Collision(this, player, background, userInterface);
			terrain->Draw(this, player);
			player->Physics(this, terrain, fElapsedTime);
			player->Draw(this, fElapsedTime);
			userInterface->Draw(this, player, fElapsedTime);
			
			#ifdef DEBUG
			Debug();
			#endif
		}
		return true;
	}
};

int main() 
{
	lunarLander game;
	
	if (game.Construct(550, 390, 2, 2/*300, 240, 3, 3*/))
		game.Start();

	return 0;
}