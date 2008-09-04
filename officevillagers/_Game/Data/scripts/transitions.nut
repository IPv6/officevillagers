// Эти методы обеспечивают различные виды художественного открытия диалога (левела) // - перетекание через затемнение экрана
::transitionSequenceThread <- 0;
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
	if(dialogToOpen=="main_office_level"){
		// Ждем окончания загрузки уровня
		level_StartForLoad();
	}
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

function TransitionLoop()
{
	if(::transitionSequenceThread.getstatus()=="suspended"){
		::transitionSequenceThread.wakeup();
		return;
	}
	core_CancelInterval(transitionInterval);
	core_DeleteNode("_fader");// Повторно
	::transitionSequenceThread=0;
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
		$ifdebug core_Alert("!!! Error !!! double transition");
		return;//Больше одного нельзя!
	}
	flag_TransitionState = 0;
	// Создаем отдельный поток на аниматоре, который запустит переход, дождется транзишна и закончит переход
	if(transitionName=="fade"){
		::transitionSequenceThread = ::newthread(runTransitionInThread_Fade);
	}
	::transitionSequenceThread.call(dialogToOpen);
	transitionInterval <- core_SetInterval(10,"TransitionLoop();");
}

function enableSprite(dprNmae,newAlpha)
{
	local nd=core_GetNode(dprNmae);
	nd._alpha=newAlpha;
	core_SetNode(dprNmae,nd);
}

/////////////////////////////////////////////////////
// Скриптовый фейд для катсцен и прочая
/////////////////////////////////////////////////////
::runFadenThread_state <- 0;
::runFadenThread_stopFader <- false;
::runFadenThread_transitionInterval <- 0;
function runFadenThread()
{
	local fader;
	runFadenThread_state = 1;
	core_CreateSprite("gui\\fade.spr","pole");
	// Фейдим вручную
	runFadenThread_state = 2;
	runFadenThread_stopFader = false;
	fader=core_GetNode("_fader");
	fader._fade=0.0;
	if(core_GetNode("puzzle_main")){// Из паззла нужно поднять выше
		fader._z-=5.0;
	}
	core_SetNode("_fader",fader);
	while(fader._fade<0.99 && runFadenThread_stopFader==false){
		fader._fade+=fader_Speed;
		core_SetNode("_fader",fader);
		wait(1);
	}
	fader._fade=1;
	core_SetNode("_fader",fader);
	runFadenThread_state = 3;
	// Ждем анфейда
	local tm=core_GetTick();
	while(core_GetTick()-tm<5000 && runFadenThread_stopFader==false){
		wait(1);
	}
	// АнФейдим вручную
	runFadenThread_state = 4;
	runFadenThread_stopFader = false;
	fader=core_GetNode("_fader");
	fader._fade=1.0;
	core_SetNode("_fader",fader);
	while(fader._fade>0.01 && runFadenThread_stopFader==false){
		fader._fade=max(0,fader._fade-fader_Speed);
		core_SetNode("_fader",fader);
		wait(1);
	}
	core_DeleteNode("_fader");
	runFadenThread_state = 0;
	return;
}

function FadeLoop()
{
    try{
     if("runFadenThread_tseqThread" in this && (::runFadenThread_tseqThread).getstatus()=="suspended"){
          (::runFadenThread_tseqThread).wakeup();
          return;
      }
    }catch(e){};
     core_CancelInterval(runFadenThread_transitionInterval);
     core_DeleteNode("_fader");
     runFadenThread_state=0;
     runFadenThread_stopFader = false;
     ::runFadenThread_tseqThread=0;
     runFadenThread_transitionInterval=0;
}



function runFadenThread2()
{

}

::runFadenThread_tseqThread <- 0;//core_GetTick();
function gui_FadeInX(time)
{
	gui_StopSoftActorWatch(); 
	if(runFadenThread_state!=0){
		$ifdebug core_Warning("gui_FadeInX - fail!!! st="+runFadenThread_state.tostring()+" gt3="+runFadenThread_tseqThread.tostring());
		return;
	}
	$ifdebug core_Warning("gui_FadeInX - begin gt="+runFadenThread_tseqThread.tostring());
	::runFadenThread_tseqThread = ::newthread(runFadenThread);
	::runFadenThread_tseqThread.call();
	runFadenThread_transitionInterval <- core_SetInterval(10,"FadeLoop();");
}

function gui_FadeIn()
{
	gui_FadeInX(2000);
}

function gui_FadeOut()
{
	gui_FadeOutX(2000);
}

function gui_FadeOutX(time)
{
	$ifdebug core_Warning("gui_FadeOutX - begin");
	runFadenThread_stopFader <- true;
}

function gui_IsFade()
{
	if(core_GetNode("_fader")==null){
		// Еще не
		return false;
	}
	return true;
}


