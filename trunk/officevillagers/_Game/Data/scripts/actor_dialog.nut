::dialogSpr <- false;
function show_DialogPage(text, icon, dialogData)
{
	if(dialogSpr == false){
		//_pageText
		local _x = 0;
		local _y = -11.5;
		if("_x" in dialogData){
			_x+=dialogData._x;
		}
		if("_y" in dialogData){
			_y=-9+dialogData._y;
		}
		local lDialogBg="\\gui\\empty.png";
		if("_dialogBg" in dialogData){
			lDialogBg=dialogData._dialogBg;
		}
		local lDialogSprite = format("gui\\cutscene_dialog.spr?bgimage=%s",lDialogBg);
		dialogSpr = core_CreateSprite(lDialogSprite,"level_interface_static",{x=_x, y=_y, z=-0.1, w=1, h=1, retSprite=1});
	}
	//Left
	local sFormat="<dx=-13.7><dy=1.2><left>";
	// Иконка "Далее"
	local hideMoreIcon=("_noMoreIcon" in dialogData && dialogData._noMoreIcon==true)?true:false;
	local btnClick2Next=core_GetNode("_click2Next");
	btnClick2Next._alpha=1;
	if(hideMoreIcon){
		btnClick2Next._alpha=0;
	}
	core_SetNode("_click2Next",btnClick2Next);
	// Эффект печатания
	if(dialogData._close==true){
		sFormat+="<font-effect:typer>";
		// Из-за того что диалоги удаляюбтся с задержкой (фейд), тайпер с маркером часто показывает некст баттон у СЛЕДУЮЩЕГО диалоге, где он может быть запрещен!
		/*
		if(hideMoreIcon){
			sFormat+="<font-effect:typer>";
		}else{
			sFormat+="<font-effect:typer[marker:_click2Next]>";
		}
		*/
	}
	// реплейсим в text заморочки на значения
	if("_vars" in dialogData){
		local i;
		for(i=0;i<dialogData._vars.len();i++)
		{
			text=core_StrReplace(text,"@"+dialogData._vars[i].name,dialogData._vars[i].value);
		}
	}
	core_SetNodeText(dialogSpr._name,sFormat+"\n"+text);
	core_DeleteSprite("cutscene_dialog_ico");
	core_CreateSprite(format("gui\\cutscene_dialog_ico.spr?iconame=%s",icon),dialogSpr._name, {x=-16.8, y=-0.27, z=-0.1, w=1, h=1} );
}

function isBeginPageLine(line)
{
	return core_GetStrSlice(line,"#PAGEBEGIN:(",")");
}

function isAutolistPageLine(line)
{
	return core_GetStrSlice(line,"#PAGESETAUTOLIST:(",")");
}

function isEndPageLine(line)
{
	/*return regexp("#PAGEEND").capture(line);*/
	local endMarkPos=line.find("#PAGEEND");
	if(endMarkPos!=null){
		//core_Alert(format("!!! %s - %i",line,endMarkPos));
		return true;
	}
	//core_Alert(format("!!! %s - null",line));
	return false;
}

function gui_ShowDialog(dialogFile,dialogData)
{
	if(!dialogFile || dialogFile==""){
		if(dialogSpr != false){
			core_DeleteSprite(dialogSpr._name);
		}
		dialogSpr = false;
		return;
	}
	local waitType=0;
	local dialogs=core_ReadTextFile(dialogFile);
	local reString=format("\\[%s\\]\\n(.*)\\[",dialogData._section);
	local dialogRE=regexp(reString);
	local dialogBE=dialogRE.capture(dialogs);
	if(dialogBE && dialogBE.len()>0){
		// Получаем строки диалога
		local dialogText=dialogs.slice(dialogBE[1].begin,dialogBE[1].end); 
		local dialogStringsBefore=split(dialogText,"\n");
		local dialogStringsAfter=[];
		local i = 0;
		for(i=0;i<dialogStringsBefore.len();i++){
			local line=dialogStringsBefore[i];
			if(line.len()<2 || strstr(line,"//")){
				continue;
			}
			if(strstr(line,"#")==false){
				line=core_JustifyText(line,3.8);
			}
			local subLines=split(line,"\n");
			dialogStringsAfter.extend(subLines);
		}
		local iPageStringsMax=3;
		local iCurrentLineCounter=0;
		local sPageIcon = "";
		// Бежим по всем строкам
		while(iCurrentLineCounter<dialogStringsAfter.len()){
			local iPageStrings=0;
			local sPage="";
			local lMorePages=true;
			while(true){
				local line=dialogStringsAfter[iCurrentLineCounter];
				iCurrentLineCounter++;
				local lText=true;
				local sIco=isBeginPageLine(line);
				if(sIco){
					sPageIcon = sIco;
					sPage="";
					iPageStrings=0;
					lText=false;
				}
				if(isEndPageLine(line)){
					iPageStrings=iPageStringsMax;
					lMorePages=false;
					lText=false;
				}
				local newWaitType=isAutolistPageLine(line);
				if(newWaitType){
					waitType=newWaitType.tointeger();
					iPageStrings=0;
					lMorePages=false;
					lText=false;
				}
				if(lText){
					sPage+=line;
					sPage+="\n";
					if(iCurrentLineCounter<dialogStringsAfter.len() && isEndPageLine(dialogStringsAfter[iCurrentLineCounter])==false){// Если следующая строка - не служебная
						iPageStrings++;
					}
				}
				if(iPageStrings>=iPageStringsMax){
					break;
				}
			}
			//core_Alert(sPage);
			show_DialogPage(sPage,sPageIcon,dialogData);
			local bWaitAnyKey = (waitType==0 || lMorePages)?true:false;
			if(!lMorePages && dialogData._close==false){
				bWaitAnyKey = false;
				//core_Warning("Press any key skipped #"+dialogData._section);
			}
			if(bWaitAnyKey){
				gui_WaitAnyKey();
			}
			if(waitType>0){
				local lTimeCheck=core_GetTick()+waitType;
				while(lTimeCheck>core_GetTick()){
					::suspend();
				}
			}
		}
		if(dialogData._close){
			gui_ShowDialog(null,null);
		}
	}else{core_Alert("Failed to find dialog section "+dialogData._section+" in "+dialogFile);};
}

function gui_WaitAnyKey()
{
	core_ClearAnyInputOcured();
	while(core_AnyInputOcured()==false){
		::suspend();
	}
}

function gui_IsDialogOpen()
{
	if(dialogSpr == false)
	{
		return false;
	}
	return true;
}
