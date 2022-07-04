#include "interface.h"
#include "global.h"
#include "terrain.h"
#include "background.h"
#include "filehandler.h"


Interface::Interface() :
	command
	{
		"Eagle, status?", // random questions 0 - 4
		"This is command, status?",
		"How's it going up there?",
		"Eagle, sitrep.",
		"Status report, Eagle.",
	},
	crew
	{
		"This is Eagle, all good.", // responses 0-3
		"We're doing good\nout here!",
		"We could use some beer,\notherwise good!",
		"Command, we're just fine.",
		"Damn, nice view\nout here!", // random 4-6
		"I think I can\nsee my house from here...",
		"Earth is beautiful...",
		"Carefully...", // low alt 7-9
		"Slowly...",
		"Steady...",
		"Capt. James: ", // names 10-12
		"John: ",
		"Fred: ",
		"\nWe're low on fuel!" // fuel
	}
{
}

void Interface::Draw(olc::PixelGameEngine* pge, Player* player, FileHandler* fileHandler, float fElapsedTime)
{
	static float textTime = 0.0f;

	/////////////////////////////////////////////////////////////
	//                       Main UI                           //
	/////////////////////////////////////////////////////////////
	if (textTime < 3.0f)
	{
		pge->DrawStringDecal({ int(screenWidth * 0.27f), int(screenHeight * 0.25f) }, "Land on the highlighted segments!");
		textTime += fElapsedTime;
	}

	// Distinguish between ESC-pause and landed-pause
	if (paused && (int)player->altitude)
	{
		pge->DrawStringDecal({ int(screenWidth * 0.33f), int(screenHeight * 0.7f) }, "         PAUSED\n\nPress SPACE to continue!");
		pge->DrawStringDecal({ int(screenWidth * 0.05f), int(screenHeight * 0.1f) }, "Velocity", olc::DARK_GREY);
		pge->DrawStringDecal({ int(screenWidth * 0.85f), int(screenHeight * 0.1f) }, "Fuel\nLandings", olc::DARK_GREY);
		pge->DrawStringDecal({ int(screenWidth * 0.45f), int(screenHeight * 0.1f) }, "Score", olc::DARK_GREY);
		pge->DrawStringDecal({ int(screenWidth * 0.55f), int(screenHeight * 0.4f), }, "Altitude", olc::DARK_GREY);
	}

	std::vector<std::pair<olc::vf2d, std::string>> ui = {
		{ {screenWidth * 0.05f, screenHeight * 0.05f}, "H.V. " + std::to_string(player->normalizedHorizontalVelocity) + "m/s" },
		{ {screenWidth * 0.05f, screenHeight * 0.07f}, "V.V. " + std::to_string(player->normalizedVerticalVelocity) + "m/s" },
		{ {screenWidth * 0.85f, screenHeight * 0.05f}, "F " + std::to_string((int)player->fuel) + "kg"},
		{ {screenWidth * 0.85f, screenHeight * 0.07f}, "L " + std::to_string(player->landings) },
		{ {screenWidth * 0.47f, screenHeight * 0.05f}, std::to_string((int)player->score) },
		{ player->position * scale + player->adjustedPosition, std::to_string((int)player->altitude) + "m" } };

	for (auto& line : ui)
	{
		if (line != ui.back())
			pge->DrawStringDecal(line.first, line.second, olc::GREY);
		else
			if (!player->dead)
				pge->DrawRotatedStringDecal(
					line.first,
					line.second,
					player->angle,
					{ -10.0f, 10.0f },
					olc::GREY,
					olc::vf2d(1.0f, 1.0f) * scale);
	}

	if (paused && !player->dead && player->altitude < 0.7f)
		LandingMessages(pge, player->normalizedHorizontalVelocity + player->normalizedVerticalVelocity, player->gainedScore);

	if (paused && player->dead)
		DeathMessages(pge, fileHandler, player->normalizedHorizontalVelocity + player->normalizedVerticalVelocity, player->score);
	/////////////////////////////////////////////////////////////
	//                        Comms                            //
	/////////////////////////////////////////////////////////////
	static float   _time               = 0.0f;
	static float   randomTime         = RandFloat(50.0f, 70.0f);
	static bool    randomVariablesSet = false;
	static bool    isMessageOnScreen  = false;
	static uint8_t randomControlQuestion;
	static uint8_t randomCrewReponse;
	static uint8_t randomCrewChatter;
	static uint8_t randomLowAltWarning;
	static uint8_t randomName;

	if (!randomVariablesSet || player->dead)
	{
		randomControlQuestion = rand() % 5;
		randomCrewReponse = rand() % 4;
		randomCrewChatter = rand() % 3 + 4;
		randomLowAltWarning = rand() % 3 + 7;
		randomName = rand() % 3 + 10;
		randomVariablesSet = true;
	}

	if (!paused)
		_time += fElapsedTime;

	// Allow messsages in ESC-pause, but not landed-pause
	if (player->altitude > 0.7f)
	{
		if (_time > randomTime)
		{
			// Command message
			if (_time < randomTime + 8.0f)
				pge->DrawStringDecal(
					{ screenWidth * 0.05f, screenHeight * 0.25f },
					"Command:\n" + command[randomControlQuestion],
					olc::GREY);

			// Crew response
			if (_time > randomTime + 3.0f && _time < randomTime + 8.0f)
				pge->DrawStringDecal(
					{ screenWidth * 0.05f, screenHeight * 0.3f },
					"Eagle:\n" + crew[randomCrewReponse],
					olc::GREY);

			// Reset timers when done
			if (_time > randomTime + 8.0f)
			{
				randomVariablesSet = false;
				randomTime = RandFloat(50.0f, 70.0f);
				_time = 0.0f;
			}
		}

		// Crew chatter
		if (_time > randomTime / 2 && _time < randomTime / 2 + 7.0f && player->altitude > 4.0f)
			pge->DrawStringDecal(
				{ screenWidth * 0.05f, screenHeight * 0.35f },
				crew[randomName] + crew[randomCrewChatter],
				olc::GREY);
		
		// Low fuel
		if (player->fuel < 500)
		{
			pge->DrawStringDecal(
				{ screenWidth * 0.05f, screenHeight * 0.45f },
				crew[10] + crew[13],
				olc::GREY);
		}

		// Low alt
		if (player->altitude < 4.0f && abs(player->currentSegmentAngle) <= 0.349f)
			pge->DrawStringDecal(
				{ screenWidth * 0.05f, screenHeight * 0.4f },
				crew[randomName] + crew[randomLowAltWarning],
				olc::GREY);
		else
			randomLowAltWarning = rand() % 3 + 7;
	}
}

void Interface::TitleScreen(olc::PixelGameEngine* pge, Background* background, Player* player, FileHandler* fileHandler, Audio* audio)
{
	int highScore = fileHandler->ReadOrCreateFile();

	if (highScore != -1)
		pge->DrawString({ 30, 70 }, "High score: " + std::to_string(highScore));

	//TODO: CONVERT TO VECTOR OF PAIRS LIKE MAIN UI
	pge->DrawSprite({ 430, 30 }, background->sprEarth.get(), 3U);

	pge->DrawRotatedStringDecal({ screenWidth * 0.05f, screenHeight * 0.05f }, "   CONTROLS", 0.0f, { 0.0f, 0.0f }, olc::GREY);

	pge->DrawRotatedStringDecal(
		{ screenWidth * 0.05f, screenHeight * 0.07f },
		"W,A,D - Thrusters",
		0.0f,
		{ 0.0f, 0.0f },
		olc::GREY);

	pge->DrawRotatedStringDecal(
		{ screenWidth * 0.05f, screenHeight * 0.09f },
		"SHIFT - Zoom",
		0.0f,
		{ 0.0f, 0.0f },
		olc::GREY);

	pge->DrawRotatedStringDecal(
		{ screenWidth * 0.05f, screenHeight * 0.11f },
		"ESC   - Pause",
		0.0f,
		{ 0.0f, 0.0f },
		olc::GREY);

	pge->DrawString({ int(screenWidth * 0.25f), int(screenHeight * 0.3f) },
		"LUNAR LANDER",
		olc::WHITE,
		3);

	pge->DrawString(
		{ int(screenWidth * 0.35f), int(screenHeight * 0.8f) },
		"Press SPACE to start!",
		olc::GREY);

	pge->DrawRotatedDecal(
		{ screenWidth * 0.52f, screenHeight * 0.55f },
		player->decPlayer.get(),
		0.4f,
		{ 8.0f, 8.0f },
		{ 7.0f, 7.0f });

	if (pge->GetKey(olc::SPACE).bPressed)
	{
		titleScreen = false;
		audio->soundPlayed = false;
		audio->PlaySoundSample(pge, 1, 11);
		audio->soundPlayed = false;
	}
}

void Interface::LandingMessages(olc::PixelGameEngine* pge, int totalVelocity, int gainedScore)
{
	switch (totalVelocity)
	{
	case 0:
		pge->DrawStringDecal({ int(screenWidth * 0.39f), int(screenHeight * 0.25f) }, "PERFECT LANDING!");
		break;
	case 1:
		pge->DrawStringDecal({ int(screenWidth * 0.4f), int(screenHeight * 0.25f) }, "Great landing!");
		break;
	case 2:
		pge->DrawStringDecal({ int(screenWidth * 0.4f), int(screenHeight * 0.25f) }, "Good landing!");
		break;
	case 3:
		pge->DrawStringDecal({ int(screenWidth * 0.41f), int(screenHeight * 0.25f) }, "You made it!");
		break;
	case 4:
		pge->DrawStringDecal({ int(screenWidth * 0.39f), int(screenHeight * 0.25f) }, "Bit shaky there!");
		break;
	case 5:
		pge->DrawStringDecal({ int(screenWidth * 0.39f), int(screenHeight * 0.25f) }, "Almost lost it!");
		break;
	default:
		break;
	}

	pge->DrawStringDecal(
		{ int(screenWidth * 0.47f), int(screenHeight * 0.2f) }, "+" + std::to_string(gainedScore));

	pge->DrawStringDecal(
		{ int(screenWidth * 0.435f), int(screenHeight * 0.3f) },
		"+500 fuel",
		olc::DARK_GREY);

	pge->DrawStringDecal(
		{ screenWidth * 0.33f, screenHeight * 0.8f },
		"Press SPACE to continue!");
}

void Interface::DeathMessages(olc::PixelGameEngine* pge, FileHandler* fileHandler, int totalVelocity, int currentScore)
{
	int highScore = fileHandler->ReadOrCreateFile();

	if (highScore == -1 || highScore < currentScore)
		fileHandler->OverwriteScore(currentScore);

	if (highScore == currentScore)
		pge->DrawStringDecal({ int(screenWidth * 0.4f), int(screenHeight * 0.3f) }, "NEW HIGH SCORE!");

	if (totalVelocity < 7)
		pge->DrawStringDecal({ int(screenWidth * 0.31f), int(screenHeight * 0.25f) }, "You broke the landing gear!");
	else if (totalVelocity < 12)
		pge->DrawStringDecal({ int(screenWidth * 0.4f), int(screenHeight * 0.25f) }, "You crashed it!");
	else if (totalVelocity < 20)
		pge->DrawStringDecal({ int(screenWidth * 0.4f), int(screenHeight * 0.25f) }, "You wrecked it!");
	else
		pge->DrawStringDecal({ int(screenWidth * 0.37f), int(screenHeight * 0.25f) }, "YOU BLEW A CRATER!");

	pge->DrawStringDecal({ screenWidth * 0.33f, screenHeight * 0.8f }, "Press SPACE to restart!");
}
