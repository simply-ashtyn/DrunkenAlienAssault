// handles everything
#include "MainMenu.h"
// program entry point
int newLevel = 1;
bool mainMenuCheck;
bool gameExit = false;

int main()
{
	Application DrunkenAllienAssault;
	DrunkenAllienAssault.Init();
	while (!gameExit)
	{
		if (!mainMenuCheck)
		{
			MainMenu(&DrunkenAllienAssault, &newLevel);
		}
		mainMenuCheck = true;
		if (DrunkenAllienAssault.Run(newLevel))
		{
			//CreateScore(&DrunkenAllienAssault); 
			mainMenuCheck = false;
			continue;
		}
		gameExit = true;
	}
	DrunkenAllienAssault.Shutdown();

	return 1;
}