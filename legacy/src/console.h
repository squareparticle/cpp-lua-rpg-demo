/*
Name: console.h
Created: May 5 2002

Description:
	A debugging console that will get displayed when the user has loaded the program in console mode
	and pressed the F12 key.  In this mode the user can type commands to set parameters in the game.

    Commands:
	display mapinfo
	display heroinfo

    set herox=(num x)
    set heroy=(num x)
    set weather=rain

    loadscript=scriptname.map


*/

void displayConsolePrompt();

void addMessagetoConsole(char * msg){
   scrollConsole();
   rg_ConsoleLine[ConsoleLine_MAX-1] = StringCopy(msg);
}

void displayConsolePrompt(){
   
   for(int i_ConsoleLine=0;i_ConsoleLine < ConsoleLine_MAX; i_ConsoleLine ++){
      sprintf(strMsg, "%s", rg_ConsoleLine[i_ConsoleLine]);
      AppOutputText(lpDDSBack, 4, (i_ConsoleLine * 15)+1, strMsg);
   }
   
   sprintf(strMsg, "%s", "Dragon Quest Console Ver 1.0.0");
   AppOutputText(lpDDSBack, 420, 1, strMsg);

   sprintf(strMsg, "> %s", msg_Console);
   AppOutputText(lpDDSBack, 4, (ConsoleLine_MAX * 15)+1, strMsg);
}

void scrollConsole(){
   for(int i_ConsoleLine=0;i_ConsoleLine < (ConsoleLine_MAX-1); i_ConsoleLine ++){
      rg_ConsoleLine[i_ConsoleLine] = StringCopy(rg_ConsoleLine[i_ConsoleLine+1]);
   }
   strcpy(rg_ConsoleLine[ConsoleLine_MAX-1],msg_Console);
}
