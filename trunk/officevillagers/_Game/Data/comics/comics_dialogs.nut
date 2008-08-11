function comics_dialog0()
{
	//local dialogFile = core_GetDataPath("\\text\\comics_dialogs.lng");
	//gui_ShowDialog(dialogFile, { _section="DIALOG0", _close=true, _y=0});
	gui_WaitAnyKey();
	actionCloseComics("comics_page");
}

function comics_dialog1()
{
	local dialogFile = core_GetDataPath("\\text\\comics_dialogs.lng");
	gui_ShowDialog(dialogFile, { _section="DIALOG1-1", _close=false, _noMoreIcon=true, _dialogBg="\\gui\\comics_interface_dialog.png", _dialogBg="\\gui\\comics_interface_dialog.png"});
	gui_WaitAnyKey();
	gui_ShowDialog(null,null);
	actionCloseComics("comics_page");
}

function comics_dialog2()
{
	local dialogFile = core_GetDataPath("\\text\\comics_dialogs.lng");
	wait(1000);
	gui_ShowDialog(dialogFile, { _section="DIALOG1-2", _close=true, _noMoreIcon=true, _dialogBg="\\gui\\comics_interface_dialog.png"});
	wait(2000);
	gui_ShowDialog(dialogFile, { _section="DIALOG1", _close=true, _noMoreIcon=true, _dialogBg="\\gui\\comics_interface_dialog.png"});
	actionCloseComics("comics_page");
}

function comics_dialog3()
{
	local dialogFile = core_GetDataPath("\\text\\comics_dialogs.lng");
	gui_ShowDialog(dialogFile, { _section="DIALOG2-1", _close=false, _noMoreIcon=true, _dialogBg="\\gui\\comics_interface_dialog.png"});
	gui_WaitAnyKey();
	gui_ShowDialog(null,null);
	actionCloseComics("comics_page");
}

function comics_dialog4()
{
	local dialogFile = core_GetDataPath("\\text\\comics_dialogs.lng");
	gui_ShowDialog(dialogFile, { _section="DIALOG2", _close=true, _noMoreIcon=true, _dialogBg="\\gui\\comics_interface_dialog.png"});
	core_CreateSprite("gui\\fadefast.spr","pole");
	wait(150);
	core_FadeSprite("_fader",1.0,0.0,50,0);
	wait(150);
	core_FadeSprite("_fader",0.0,1.0,50,0);
	wait(80);
	core_FadeSprite("_fader",1.0,0.0,50,0);
	wait(110);
	core_FadeSprite("_fader",0.0,1.0,50,0);
	wait(70);
	core_FadeSprite("_fader",1.0,0.0,50,0);
	wait(220);
	core_FadeSprite("_fader",0.0,1.0,50,0);
	wait(300);
	core_SetBackgroundImage(core_GetDataPath("comics\\comicspage_blackout.jpg"),0);
	core_FadeSprite("_fader",1.0,0.0,400,1);
	wait(1000);
	gui_ShowDialog(dialogFile, { _section="DIALOG3a", _close=true, _noMoreIcon=true, _dialogBg="\\gui\\comics_interface_dialog.png"});
	actionCloseComics("comics_page");
}

function comics_dialog5()
{
	local dialogFile = core_GetDataPath("\\text\\comics_dialogs.lng");
	gui_ShowDialog(dialogFile, { _section="DIALOG4", _close=true, _noMoreIcon=true, _dialogBg="\\gui\\comics_interface_dialog.png"});
	actionCloseComics("comics_page");
}

function comics_dialog6()
{
	local dialogFile = core_GetDataPath("\\text\\comics_dialogs.lng");
	gui_ShowDialog(dialogFile, { _section="DIALOG5", _close=true, _noMoreIcon=true, _dialogBg="\\gui\\comics_interface_dialog.png"});
	actionCloseComics("comics_page");
}
