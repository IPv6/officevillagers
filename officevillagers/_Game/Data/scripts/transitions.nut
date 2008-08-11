// Эти методы обеспечивают различные виды художественного открытия диалога (левела) // - перетекание через затемнение экрана
transitionSequenceThread <- 0;
transitionInterval <- 0;
flag_TransitionState <- 0;
fader_Speed <- 0.02;
_stopFader <- false;
function runTransitionInThread_Fade(dialogToOpen)
{
	local fader;
	flag_TransitionState = 1;
	core_CreateSprite("gui\\fade.spr","pole");
	// Фейдим вручную
	flag_TransitionState = 2;
	_stopFader = false;
	fader=core_GetNode("_fader");
	fader._fade=0.0;
	core_SetNode("_fader",fader);
	while(fader._fade<0.99 && _stopFader==false){
		fader._fade+=fader_Speed;
		core_SetNode("_fader",fader);
		wait(1);
	}
	fader._fade=1;
	core_SetNode("_fader",fader);
	flag_TransitionState = 3;
	core_OpenDialog(dialogToOpen);
	if(dialogToOpen=="main_office_level"){
		// Ждем окончания загрузки уровня
		level_WaitForLoad();
	}
	// АнФейдим вручную
	flag_TransitionState = 4;
	_stopFader = false;
	fader=core_GetNode("_fader");
	fader._fade=1.0;
	core_SetNode("_fader",fader);
	while(fader._fade>0.01 && _stopFader==false){
		fader._fade=max(0,fader._fade-fader_Speed);
		core_SetNode("_fader",fader);
		wait(1);
	}
	core_DeleteNode("_fader");
	flag_TransitionState = 0;
	return;
}

function onFadeClick(param)
{
	_stopFader = true;
}

function gui_FadeIn()
{
	gui_FadeInX(2000);
}

function gui_FadeInX(time)
{
	if(core_GetNode("_fader")!=null){
		return;
	}
	core_CreateSprite("gui\\fade.spr","pole");
	local fader=core_GetNode("_fader");
	fader._fade=0.0;
	core_SetNode("_fader",fader);
	core_FadeSprite("_fader",0.0,1.0,time,0);
}

function gui_FadeOut()
{
	gui_FadeOutX(2000);
}

function gui_FadeOutX(time)
{
	if(core_GetNode("_fader")==null){
		// Еще не
		return;
	}
	core_FadeSprite("_fader",-1.0,0.0,time,1);
}

function gui_IsFade()
{
	if(core_GetNode("_fader")==null){
		// Еще не
		return false;
	}
	return true;
}

function TransitionLoop()
{
	if(transitionSequenceThread.getstatus()=="suspended"){
		transitionSequenceThread.wakeup();
		return;
	}
	core_CancelInterval(transitionInterval);
	core_DeleteNode("_fader");// Повторно
	transitionSequenceThread=0;
	transitionInterval=0;
}

function TransitionOpenDialog(dialogToOpen,transitionName)
{
	//core_Alert(dialogToOpen+" "+transitionName);
	if(transitionName.len()==0)
	{
		core_OpenDialog(dialogToOpen);
		return;
	}
	if(transitionInterval != 0)
	{
		//core_Alert("!!! Error !!!");
		return;//Больше одного нельзя!
	}
	flag_TransitionState = 0;
	// Создаем отдельный поток на аниматоре, который запустит переход, дождется транзишна и закончит переход
	if(transitionName=="fade"){
		transitionSequenceThread <- ::newthread(runTransitionInThread_Fade);
	}
	transitionSequenceThread.call(dialogToOpen);
	transitionInterval <- core_SetInterval(10,"TransitionLoop();");
}

function enableSprite(dprNmae,newAlpha)
{
	local nd=core_GetNode(dprNmae);
	nd._alpha=newAlpha;
	core_SetNode(dprNmae,nd);
}

