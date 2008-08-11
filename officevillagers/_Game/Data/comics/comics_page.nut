dialogsThread <- false;
dialogsThreadinterval <- false;

function ComicsDialogsLoop()
{
	if(!dialogsThreadinterval)
	{
		return;
	}
	if(dialogsThread.getstatus()=="suspended"){
		dialogsThread.wakeup();
		return;
	}
	core_CancelInterval(dialogsThreadinterval);
	dialogsThreadinterval=false;
	dialogsThread=false;
}

function setComicsPageDialog(initialText)
{
	local regx = regexp(@"{dialogs:(.*)}");
	local isDialogsCaptcha = regx.capture(initialText);
	if(isDialogsCaptcha){
		local isDialogs=initialText.slice(isDialogsCaptcha[1].begin,isDialogsCaptcha[1].end);
		local isDialogsMethod = this[isDialogs];
		dialogsThread=::newthread(isDialogsMethod);
		dialogsThread.call();
		dialogsThreadinterval = core_SetInterval(200,"ComicsDialogsLoop()");
	}
}

g_DialogsTxt <- "";
function setComicsPageText(textNodeName,initialText)
{
	{// Переводной текст
		local regx = regexp(@"{translate:(.*)}");
		local transText = regx.capture(initialText);
		if(transText){
			local newtransText=initialText.slice(transText[1].begin,transText[1].end);
			initialText+="\n";
			initialText+=core_Translate(newtransText);
		}
	}
	{// Смена верхнего текста диалога...
		local regx = regexp(@"{setBgImage:(.*)}");
		local isBgImage = regx.capture(initialText);
		if(isBgImage){
			local newBgImage=initialText.slice(isBgImage[1].begin,isBgImage[1].end);
			core_SetBackgroundImage(newBgImage,0);
		}
		core_SetNodeText(textNodeName,initialText);
	}
	{// Выполняем показ диалогов...
		gui_ShowDialog(null,null);
		if(dialogsThreadinterval != false){
			core_CancelInterval(dialogsThreadinterval);
			dialogsThreadinterval = false;
			dialogsThread = false;
		}
		g_DialogsTxt = initialText;
		core_SetTimeout(1000,"setComicsPageDialog(g_DialogsTxt);");
	}
}

g_ComicsTxt <- "";
g_ComicsTxtNode <- "";
function getNextPageText(textNodeName, isSkip)
{
	local initialText=core_GetNodeText(textNodeName);
	local regx;
	if(isSkip){
		regx = regexp(@"{setSkipPage:(.*)}");
	}else{
		regx = regexp(@"{setNextPage:(.*)}");
	}
	local isNextPage = regx.capture(initialText);
	if(isNextPage){
		initialText=initialText.slice(isNextPage[1].begin,isNextPage[1].end);
		initialText=core_ReadTextFile(initialText);
		// Показываем переход с фейдом
		gui_FadeInX(1000);
		g_ComicsTxt=initialText;
		g_ComicsTxtNode=textNodeName;
		//setComicsPageText(textNodeName,initialText);
		core_SetTimeout(1000,"setComicsPageText(g_ComicsTxtNode,g_ComicsTxt);");
		core_SetTimeout(2000,"gui_FadeOutX(1000);");
		return true;
	}
	return false;
}

function showComics(pageFolder)
// Called from engine
{
	core_OpenDialog("comics_page");
	pageFolder=core_ReadTextFile(pageFolder);
	setComicsPageText("comics_text_body",pageFolder);
};

function actionCloseComics(dialogToClose)
{
	actionCloseComicsX(dialogToClose,"fade", false);
}

function actionCloseComicsBT(dialogToClose)
{
	if(core_GetNode("_fader")!=null){
		return;
	}
	actionCloseComicsX(dialogToClose,"fade", true);
}

function actionCloseComicsX(dialogToClose, mercType, isSkip)
{
	if(dialogsThreadinterval!=false)
	{
		core_CancelInterval(dialogsThreadinterval);
		dialogsThreadinterval=false;
		dialogsThread=false;
	}
	if(!getNextPageText("comics_text_body", isSkip)){
		gui_ShowDialog(null,null);
		local initialText=core_GetNodeText("comics_text_body");
		local regx = regexp(@"{setNextDialog:(.*)}");
		local isNextDialog = regx.capture(initialText);
		if(isNextDialog){
			local nextDialog=initialText.slice(isNextDialog[1].begin,isNextDialog[1].end);
			TransitionOpenDialog(nextDialog,mercType);
		}else{
			core_CloseDialog(dialogToClose);
		}
	}
}