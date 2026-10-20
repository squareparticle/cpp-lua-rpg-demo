void CenterDoorOpen();
void CenterDoorClose();

void transitionController(){
	switch (TRANSITION_STATE){
	   case 0:
	  	  CenterDoorOpen();
	   break;
	   case 1:
	  	  CenterDoorClose();
	   break;
	}
}

void CenterDoorOpen(){
  RECT clipper;
  clipper.top = 0;
  clipper.bottom = SCREEN_HEIGHT;
  clipper.left = 0;
  clipper.right = SCREEN_WIDTH;

  lpDDSBack->BltFast(0,0,lpDDSScreenShot,&clipper,DDBLTFAST_WAIT);
}
void CenterDoorClose(){
}
