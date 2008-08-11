locFilePaths <- [];
locFilePaths.append("\\actors\\office\\locations.txt");
locFilePaths.append("\\actors\\office\\map\\navmap.txt");
locFileIndex <- 0;
function chooseNLGroupToEdit()
{
	local fnames=[],i=1;
	/*// Список имен из профиля...
	while(1)
	{
		local str=core_GetProfileOption(format("_editor.NamlocPrefix%i",i));
		if(str!=false){
			fnames.append(str);
			i++
			continue;
		}
		break;
	}*/
	// Список имен из имеющихся локаций
	local loadedDots=nloc_Load(locFileIndex,locFilePaths[locFileIndex]);
	if(loadedDots.len()>0){
		local i=0,j=0;
		for(i=0;i<loadedDots.len();i++)
		{
			local name=loadedDots[i]._name;
			if(strstr(name,"::")!=false){
				local prefix=name.slice(0,name.find("::"));
				local bNew=true;
				for(j=0;j<fnames.len();j++)
				{
					if(fnames[j]==prefix){
						bNew=false;
						break;
					}
				}
				if(bNew){
					fnames.append(prefix);
				}
			}
		}
	}
	fnames.append("- CUSTOM -");
	fnames.sort();
	local choice=core_AskChoice(fnames,"Выберите группу");
	if(choice==-1)
	{
		return;
	}
	local newPref="";
	if(fnames[choice]=="- CUSTOM -")
	{
		newPref=core_AskString("Введите префикс, пусто - все!","???");
		choice=i;
		if(newPref.len()==0){
			newPref="";
		}else{
			fnames.resize(choice+1);
			fnames[choice]=newPref;
			//core_SetProfileOption(format("_editor.NamlocPrefix%i",choice),newPref);
			core_SaveProfile();
		}
	}else{
		newPref=fnames[choice];
	}
	core_SetProfileOption("_editor.NamlocFile",newPref);
	core_OpenDialog("editor_namedloc");
}

function chooseLocGroupToEdit()
{
	locFileIndex <- 0;
	chooseNLGroupToEdit();
}

function chooseWalkGroupToEdit()
{
	locFileIndex <- 1;
	chooseNLGroupToEdit();
}

function AskProf()
{
	// Список профессий
	local fnames=[],i,choice;
	for(i=0;i<100;i++){
		local profStat=actor_GetProfessionStats(i);
		if(!profStat){
			break;
		}
		fnames.append({Name=format("%s: %i",profStat.Name,profStat.ActorsCount), val=profStat.Name});
	}
	fnames.sort(compareAskLists);
	choice=core_AskChoice(fnames,"Выберите",5);
	if(choice!=-1)
	{
		return fnames[choice].val;
	}
	return "";
}

function AskAction()
{
	local fnames=[],i,choice;
	for(i=0;i<100;i++){
		local profStat=event_GetEvent(i);
		if(!profStat){
			break;
		}
		fnames.append({Name=format("%s: %i",profStat.Name,profStat.ActionUsageCount), val=profStat.Name});
	}
	fnames.sort(compareAskLists);
	choice=core_AskChoice(fnames,"Выберите",4);
	if(choice!=-1)
	{
		return fnames[choice].val;
	}
	return "";
}

g_LastEGA_Actor <- null;
g_LastEGA_Mask <- "*";
g_LastEGA_Skip <- 0;
g_EGA_Slots <- 0;
g_LastMode <- "ATTRIBUES";
g_LastItemsArray <- [];
function SetModeAttrEditDialog(newMode)
{
	g_LastMode <- newMode;
	fillAttrEditDialog("*", g_LastEGA_Skip);
}

function edit_Getitems_Params()
{
	local thisActor=actor_GetActorPos(g_LastEGA_Actor);
	local res=[];
	local item,i;
	res.append({_isNull=false, name="!Name/Rus", value=format("%s/%s",thisActor.Name,thisActor.NameLocalized)});
	res.append({_isNull=false, name="Prof", value=thisActor.Profession});
	res.append({_isNull=false, name="Action", value=thisActor.Action});
	res.append({_isNull=false, name="Enabled", value=thisActor.Enabled});
	
	// Неизменяемое
	res.append({_isNull=false, name="!Prof_N/ValidProf", value=format("%s/'%s'",thisActor.ProfessionN,thisActor.ValidProfessions)});
	res.append({_isNull=false, name="!Location/LocationEntrance", value=format("%s/'%s'",thisActor.Location,thisActor.LocationEntrance)});
	res.append({_isNull=false, name="!X:Y/ActorType/BodyLock", value=format("%.02f:%.02f/%i/%i",thisActor._x,thisActor._y, thisActor.ActorType,thisActor.BodyLock)});
	local itemslist="";
	i=0;
	while(true){
		local actorItem=actor_GetItem(i);
		if(!actorItem){
			break;
		}
		itemslist+=" ";
		itemslist+=actorItem._name;
		i++;
	}
	if(i>0){
		res.append({_isNull=false, name="!Items", value=itemslist});
	}
	return res;
}

function edit_Getitems_Attributes()
{
	local res=[],i=0;
	local absItemOrder=0,attrByNum;
	for(i=0; i<g_EGA_Slots; i++)
	{
		absItemOrder=g_LastEGA_Skip+i;
		local realNum=0,skipNum=0;
		while(skipNum<999){
			attrByNum=actor_GetAttribute(actor_GetActor(g_LastEGA_Actor),skipNum);
			if(!attrByNum){
				break;
			}
			if(core_StrMask(attrByNum.name,g_LastEGA_Mask)){
				if(realNum==absItemOrder){
					break;
				}
				realNum++;
			}
			skipNum++;
		}
		attrByNum=actor_GetAttribute(actor_GetActor(g_LastEGA_Actor),skipNum);
		if(!attrByNum){
			attrByNum={_isNull=true};
			res.append(attrByNum);
		}else{
			attrByNum._isNull<-false;
			res.append(attrByNum);
		}
	}
	return res;
}

function edit_Getitems_Quests()
{
	local res=[],i=0,queste;
	local fileListing=core_GetFileListing(core_GetDataPath("\\actors\\quests\\*.txt"));
	for(i=0;i<fileListing.len();i++){
		if(i<g_LastEGA_Skip || i>g_LastEGA_Skip+g_EGA_Slots){
			continue;
		}
		local qfile=core_GetPathPart(fileListing[i],false,false,true,false);
		local qqsta=gui_QuestCheck(qfile);
		local qqa=quest_GetQuest(qfile);
		local state=qqsta.tointeger();
		if(qqsta==3){
			state="Закрыт";
		}
		if(qqsta==2){
			state="Открыт";
		}
		if(qqsta==1){
			state="Пока не открыт";
		}
		queste={_isNull=false, name=qqa.ID, value=format("%s [%s]",state,qqa.Name.slice(0,15))};
		res.append(queste);
		//fnames.append(core_GetPathPart(fileListing[i],false,false,true,false));
	}
	return res;
}

function fillAttrEditDialog(mask, skipTo)
{
	local thisActor=actor_GetActor(g_LastEGA_Actor);
	if(skipTo<0){
		skipTo=0;
	}
	if(thisActor.Name=="Office"){
		core_EnableNode("editAttrOffgl",true);
	}else{
		core_EnableNode("editAttrOffgl",false);
	}
	g_LastEGA_Skip=skipTo;
	g_LastEGA_Mask=mask;
	core_SetNodeText("editor_global_attr_status",format("<font-size:-1>\nActor '%s', mode='%s', Mask='%s', Listing from %i",thisActor.Name, g_LastMode, g_LastEGA_Mask, g_LastEGA_Skip));
	local egaSlot0=core_GetNode("egaslot0");
	local i=0;
	if(!egaSlot0){
		g_EGA_Slots = 0;
		for(i=0;i<14;i++){
			core_CreateSprite("gui\\editor\\editor_egaslot.spr","editor_global_attr",{x=0, y=7.5-1.4*i, z=-0.0002, w=20, h=1.4, name=format("egaslot%i",g_EGA_Slots++) });
		}
	}
	g_LastItemsArray = [];
	if(g_LastMode == "ATTRIBUES"){
		g_LastItemsArray=edit_Getitems_Attributes();
	}
	if(g_LastMode == "PARAMS"){
		// Имя, кооррдинаты и прочая
		g_LastItemsArray=edit_Getitems_Params();
	}
	if(g_LastMode == "QUESTS"){
		g_LastItemsArray=edit_Getitems_Quests();
	}
	local attrByNum;
	for(i=0; i<g_EGA_Slots; i++)
	{
		local absItemOrder=g_LastEGA_Skip+i;
		if(i>=g_LastItemsArray.len()){
			attrByNum={_isNull=true};
		}else{
			attrByNum=g_LastItemsArray[i];
		}
		if(attrByNum._isNull){
			core_EnableNode(format("egaslot%i",i),false);
		}else{
			local slotNode=core_GetNode(format("egaslot%i",i));
			core_EnableNode(slotNode,true);
			core_SetNodeText(slotNode,format("<font-size:-1><left><dx=-9>\n%i. %s: \"%s\"",absItemOrder,attrByNum.name,attrByNum.value.tostring()));
			core_SetNodeAction(slotNode,format("editAttrEditDialog(\"%s\",\"%s\");",thisActor.Name,attrByNum.name));
		}
	}
}

function editAttrEditDialog(thisActor,attrName)
{
	if(attrName.len() && attrName[0]=='!'){
		core_Alert("Этот параметр нельзя отредактировать");
		return;
	}
	if(g_LastMode == "ATTRIBUES"){
		editAttrEditDialog_Attr(thisActor,attrName);
	}
	if(g_LastMode == "PARAMS"){
		editAttrEditDialog_Param(thisActor,attrName);
		return;
	}
	if(g_LastMode == "QUESTS"){
		//editAttrEditDialog_Param(thisActor,attrName);
		local fnames=[];
		fnames.append({Name="Открыть", val=2});
		fnames.append({Name="Закрыть", val=3});
		fnames.append({Name="Еще не открыт", val=1});
		local choice=core_AskChoice(fnames,"Выберите");
		if(choice!=-1)
		{
			quest_set(attrName,fnames[choice].val);
			fillAttrEditDialog(g_LastEGA_Mask,g_LastEGA_Skip);
		}
		return;
	}
}

function editAttrEditDialog_Param(thisActor,attrName)
{
	thisActor=actor_GetActor(g_LastEGA_Actor);
	local value="???", i=0;
	if(attrName=="Prof"){
		value=AskProf();
	}else
	if(attrName=="Action"){
		value=AskAction();
	}else
	{
		for(i=0;i<g_LastItemsArray.len();i++){
			if(g_LastItemsArray[i]._isNull==false && g_LastItemsArray[i].name==attrName){
				value=g_LastItemsArray[i].value;
			}
		}
		 value=core_AskString("Новое значение",value.tostring());
	}
	if(value.len()==0){
		return;
	}
	
	if(attrName=="Prof"){
		actor_SetProfession_i(thisActor,value);
	}else if(attrName=="Action"){
		actor_SwitchToAction(thisActor,value,true);
	}else if(attrName=="Enabled"){
		actor_SetActorEnabled(thisActor,(value=="0" || (value.len()>1 && value.len()!=4))?false:true);
	}else{
		core_Alert("Unknown param "+value);
	}
	fillAttrEditDialog(g_LastEGA_Mask,g_LastEGA_Skip);
}

function editAttrEditDialog_Attr(thisActor,attrName)
{
	thisActor=actor_GetActor(g_LastEGA_Actor);
	local value=actor_GetAttribute(thisActor,attrName);
	if(!value){
		value=0;
	}
	local newVal=core_AskString("Новое значение",value.tostring());
	if(newVal.len()==0){
		return;
	}
	if(newVal==" "){
		newVal="";
	}
	actor_DelAttribute(thisActor,attrName);
	if(newVal.len()>0 && newVal.slice(0,1)>="0" && newVal.slice(0,1)<="9"){
		actor_SetAttribute(thisActor,attrName,newVal.tointeger());
		//core_Alert(format("%s = %s [i]",attrName,newVal));
	}else{
		actor_SetAttribute(thisActor,attrName,newVal);
		//core_Alert(format("%s = '%s' [s]",attrName,newVal));
	}
	fillAttrEditDialog(g_LastEGA_Mask,g_LastEGA_Skip);
}

function chooseActorsEditDialog(nameMask,profMask,actionMask)
{
	local objectsArray=actor_GetActors( {_byName=nameMask, _byProfession=profMask, _byAction=actionMask,_includeDisabled=true});
	local fnames=[],i=0;
	for(i=0;i<objectsArray.len();i++){
		fnames.append(objectsArray[i].Name);
	}
	fnames.sort();
	fnames.append("ByProf...");
	fnames.append("ByAction...");
	local choice=core_AskChoice(fnames,"Выберите");
	if(choice!=-1)
	{
		if(fnames[choice]=="ByProf..."){
			local prof=AskProf();
			if(prof!="")
			{
				chooseActorsEditDialog(nameMask,prof,null);
			}
			return;
		}
		if(fnames[choice]=="ByAction..."){
			local act=AskAction();
			if(act!="")
			{
				chooseActorsEditDialog(nameMask,null,act);
			}
			return;
		}
		g_LastEGA_Actor=fnames[choice];
		g_LastMode="ATTRIBUES";
		fillAttrEditDialog("*",0);
	}
}

function saveAttrEditDialog()
{
	local thisActor=actor_GetActor(g_LastEGA_Actor);
	if(thisActor.Name=="Office"){
		local str=actor_PackAttributes(null,"GLOBAL::*");
		core_SaveFile(core_AddDataPath("\\actors\\actions\\office_globals.txt"),str);
		core_Alert("Saved GLOBAL* to 'actors\\actions\\office_globals.txt'");
	}else{
		local str=actor_PackAttributes(thisActor);
		core_Alert(format("Actor %s attribs:\n%s",thisActor.Name,str));
	}
}

function SaveGameAttrEditDialog()
{
	local newVal=core_AskString("Имя сейва","");
	if(!game_SaveGame(newVal)){
		core_Alert("Не могу сейчас (катсцена?)");
	}
	core_Warning("Game Saved to "+core_GetPathPart(core_GetProfileOption("SaveFile"),false,false,true,false)+"!");
}

function LoadGameAttrEditDialog()
{
	local fileListing=core_GetFileListing(core_GetActiveProfileFolder()+"\\*.sav");
	local fnames=[],i=0;
	for(i=0;i<fileListing.len();i++){
		fnames.append(core_GetPathPart(fileListing[i],false,false,true,false));
	}
	if(fnames.len()==0){
		core_Alert("No saves found");
		return;
	}
	local choice=core_AskChoice(fnames,"Выберите",0);
	if(choice!=-1)
	{
		//game_LoadGame(fnames[choice]);
		core_SetProfileOption("SaveFile",core_GetActiveProfileFolder()+fnames[choice]+".sav");
		startGameButton(false);
		core_Warning("Game Loaded from "+core_GetPathPart(core_GetProfileOption("SaveFile"),false,false,true,false)+"!");
	}
}

function CloneAttrEditDialog()
{
	local thisActor=actor_GetActor(g_LastEGA_Actor);
	if(thisActor.Name=="Office"){
		core_Alert("Офис не клонировать!");
		return;
	}
	local newActor=actor_CloneActor(thisActor);
	g_LastEGA_Actor=newActor.Name;
	fillAttrEditDialog("*",0);
}

function DeleteAttrEditDialog()
{
	local thisActor=actor_GetActor(g_LastEGA_Actor);
	if(thisActor.Name=="Office"){
		core_Alert("Офис не удалять!");
		return;
	}
	if(core_AskYesNo("Удаляю "+thisActor.Name+"!")){
		actor_DeleteActor(thisActor);
		core_Warning("Удалили "+thisActor.Name+"!");
		g_LastEGA_Actor="Office";
		fillAttrEditDialog("*",0);
	}
}