function WaitTime(millisToWait)
{
	if(!("startUpTime" in this)){
		startUpTime <- core_GetTick();
	}
	while(core_GetTick()-startUpTime<millisToWait){
		::suspend();
	}
	startUpTime=core_GetTick();
};

function WaitTimeOrInput(millisToWait)
{
	if(!("startUpTime" in this)){
		startUpTime <- core_GetTick();
	}
	while(core_GetTick()-startUpTime<millisToWait && core_AnyInputOcured()==false){
		::suspend();
	}
	startUpTime=core_GetTick();
};

stepNum <- 0;
function animateLoadingProgress(preloadingPrc)
{
/*	if(preloadingPrc>=4)
	{
		if(stepNum<1)
		{
			stepNum=1;
			core_CreateSprite("gui\\preloader\\bg_step1.spr","_preloaderSpr");
		}
	}
	if(preloadingPrc>=30)
	{
		if(stepNum<2)
		{
			stepNum=2;
			core_CreateSprite("gui\\preloader\\bg_step2.spr","_preloaderSpr");
		}
	}
	if(preloadingPrc>=50)
	{
		if(stepNum<3)
		{
			stepNum=3;
			core_CreateSprite("gui\\preloader\\bg_step3.spr","_preloaderSpr");
		}
	}
	//core_Alert(preloadingPrc.tostring());
	if(preloadingPrc>=80)
	{
		if(stepNum<4)
		{
			stepNum=4;
			core_CreateSprite("gui\\preloader\\bg_step4.spr","_preloaderSpr");
		}
	}
*/
}

function startupSequence()
{
	//Сначала зачищаем экран
	core_SetBackgroundImage("gui\\preloader\\bg.jpg",0);
	core_SetBackgroundMusic("comics.ogg");
	::suspend();
	if(!core_IsDebug())
	{
		if(core_SetBackgroundImage("publisher_logo0.jpg",1)){
			WaitTimeOrInput(2000);
		}
		if(core_SetBackgroundImage("publisher_logo1.jpg",1)){
			WaitTimeOrInput(2000);
		}
		if(core_SetBackgroundImage("publisher_logo2.jpg",1)){
			WaitTimeOrInput(2000);
		}
	}
	core_SetBackgroundImage("gui\\preloader\\bg.jpg",2);
	local preloader=core_CreateSprite("gui\\preloader\\loading_bg.spr",{retSprite=1});
	local stateData={};
	while(core_GetGameState(stateData)==100){
		core_SetNodeAttribute(preloader,"preprc",stateData.preloadingProgress);
		animateLoadingProgress(stateData.preloadingProgress*100.0);
		::suspend();
	}
	animateLoadingProgress(100);
	WaitTime(500);
	core_DeleteSprite(preloader);
	// Открываем меню
	if(core_GetIniParameter("jumpcomics")=="yes")
	{
		local savedFile=core_GetIniParameter("savefile");
		local forceTutorial=true;
		if(savedFile.len()==0){
			savedFile="\\actors\\actions\\saved_state.txt";
			forceTutorial=false;
		}
		core_SetProfileOption("SaveFile",savedFile);
		startGameButton(forceTutorial);
		return;
	}
	core_OpenDialog("main_menu");
	return;
}

function startupLoop()
{
	// Возобновляем поток пока он не закончится
	if(startupSequenceFiber.getstatus()=="suspended"){
		startupSequenceFiber.wakeup();
		return;
	}
	//Все, инициализационный поток отработал. Закрываем скрипт вместе с вызвавшим его аниматором
	core_CancelInterval(startupLoopIOntervalID);
	delete startupLoopIOntervalID;
	delete startupSequenceFiber;
}

function startGame()
{

	// Метод который инициализирует предзагрузку и прочая
	startupSequenceFiber <- ::newthread(startupSequence);
	startupSequenceFiber.call();
	startupLoopIOntervalID <- core_SetInterval(100,"startupLoop();");
}
