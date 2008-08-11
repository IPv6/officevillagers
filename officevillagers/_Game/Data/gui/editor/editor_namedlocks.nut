lastUsedNLId <- 0;
connectorLastId <- 0;
connectorLastIdShift <- 0;
useConnectors <- false;
antiChange <- 1;
allUsedNL <- [];
allUsedCn <- [];
allUsedNLLoaded <- [];
currentPrefix <- "";
lastW <- 1.0;
lastH <- 1.0;

function addNamedLoc(doti,name)
{
	local sprInf = core_CreateSprite("gui\\editor\\editor_nl_dor.spr","mainFloor",{x=doti._x, y=doti._y, z=-0.0001, w=doti._w, h=doti._h, retSprite=1});
	lastUsedNLId = sprInf._id;
	allUsedNL.resize(allUsedNL.len()+1);
	allUsedNL[allUsedNL.len()-1] = {};
	allUsedNL[allUsedNL.len()-1]._id <- sprInf._id;
	allUsedNL[allUsedNL.len()-1]._name <- name;
	core_SetNodeText(sprInf,"");
}

function addConnectLoc(name1,name2)
{
	if(!useConnectors){return;};
	local id1=getLocIDByName(name1);
	local id2=getLocIDByName(name2);
	if(id1==0 || id2==0)
	{
		return;//Бага, нету такого
	}
	core_CreateSprite(format("raw:\\-Texture:\\gui\\editor\\connector.png\n-Animation.ConnectSprites:f%i t%i\n",id1,id2),"mainFloor");
	allUsedCn.resize(allUsedCn.len()+1);
	allUsedCn[allUsedCn.len()-1] = {};
	allUsedCn[allUsedCn.len()-1]._from <- name1;
	allUsedCn[allUsedCn.len()-1]._to <- name2;
}

function initNamedLocDialog()
{
	core_SetKeyHandlerMode(2);
	allUsedNL <- [];
	allUsedCn <- [];
	lastUsedNLId <- 0;
	connectorLastId <- 0;
	connectorLastIdShift <- 0;
	allUsedNLLoaded <- [];
	antiChange <- 0;
	currentPrefix=core_GetProfileOption("_editor.NamlocFile");
	local lineCurrentPrefix=core_JustifyText(currentPrefix+"::*",1.3);
	core_SetNodeText("curPrefixName","<font-size:-1><dy=1.8>\n"+lineCurrentPrefix);
	local loadedDots=nloc_Load(locFileIndex,locFilePaths[locFileIndex]);
	if(loadedDots.len()>0){
		local i=0;
		local floorPos = core_GetNode("mainFloor");
		for(i=0;i<loadedDots.len();i++)
		{
			local dot=loadedDots[i];
			if(currentPrefix.len()>0 && dot._name.find(currentPrefix+"::")==null){
				//core_Warning(format("skipping %s %s",dot._name,currentPrefix));
				continue;
			}
			if(dot._isdot)
			{
				allUsedNLLoaded.append(dot._name);
				addNamedLoc({
					_x = dot._x*floorPos._w/2,
					_y = dot._y*floorPos._h/2,
					_w = dot._w*floorPos._w/2,
					_h = dot._h*floorPos._h/2
				},dot._name);
			}else{
				addConnectLoc(dot._from,dot._to);
			}
		}
	}
	//core_Warning(format("Loaded %i locations with prefix '%s'!",allUsedNL.len(),currentPrefix));
}

lFloorChanged <- {ch=0, _x=0, _y=0, _z=0, _clPosX=0, _clPosY=0, _trackFloor=false};
function onkeyNamedLocDialog(keyInfo)
{
	//if(keyInfo._key<250){core_Warning(keyInfo._key.tostring());}
	{
		lFloorChanged.ch=0;
		if(keyInfo._control){
			if(keyInfo._key==83){//S
				namlocExport();
			}
		}
		/*if(keyInfo._key==1 && !keyInfo){
			core_Warning("click!");
			local curCurPos = core_GetCurrentCursorPosition("mainFloor",true);
			lFloorChanged._clPosX = curCurPos._x;
			lFloorChanged._clPosY = curCurPos._y;
		}*/
		if(keyInfo._key==252 && nlDotDragInterval==0){
			if(core_GetIsLeftButtonDown()){
				if(lFloorChanged._trackFloor==false){
					lFloorChanged._trackFloor=true;
					local curCurPos = core_GetCurrentCursorPosition("mainFloor",true);
					lFloorChanged._clPosX = curCurPos._x;
					lFloorChanged._clPosY = curCurPos._y;
				}
				local curCurPos = core_GetCurrentCursorPosition("mainFloor");
				lFloorChanged._x <- (-lFloorChanged._clPosX+curCurPos._x);
				lFloorChanged._y <- (-lFloorChanged._clPosY+curCurPos._y);
				lFloorChanged._z <- 0;
				lFloorChanged.ch=-2;
			}else{
				lFloorChanged._trackFloor=false;
			}
		}
		if(keyInfo._key==253 || keyInfo._key==254){//MWheel down/up
			local curCurPos = core_GetCurrentCursorPosition("mainFloorX",true);
			lFloorChanged._z <- (keyInfo._key==254?1:-1);
			if(lFloorChanged._z<0){
				lFloorChanged._x <- curCurPos._x*0.05*lFloorChanged._z;
				lFloorChanged._y <- curCurPos._y*0.05*lFloorChanged._z;
			}else{
				//core_Warning("!!!");
				local fllor=core_GetNode("mainFloor");
				lFloorChanged._x <- (-fllor._x*0.1);
				lFloorChanged._y <- (-fllor._y*0.1);
			}
			//lFloorChanged.ch=-2;
			lFloorChanged.ch++;
		}
		if(keyInfo._key==96){//Num0
			lFloorChanged._z <- 10;
			lFloorChanged._x <- 0;
			lFloorChanged._y <- 0;
			lFloorChanged.ch=-1;
		}
		if(lFloorChanged.ch!=0){
			local fllor=core_GetNode("mainFloor");
			if(lFloorChanged.ch==-2){
				fllor._x=lFloorChanged._x;
				fllor._y=lFloorChanged._y;
				fllor._z+=lFloorChanged._z;
			}else if(lFloorChanged.ch==-1){
				fllor._x=lFloorChanged._x;
				fllor._y=lFloorChanged._y;
				fllor._z=lFloorChanged._z;
			}else{
				fllor._x+=lFloorChanged._x;
				fllor._y+=lFloorChanged._y;
				fllor._z+=lFloorChanged._z;
			}
			core_SetNode("mainFloor",fllor);
		}
	}
	
	//65 68
	if(!keyInfo._control){
		local fSizeShiftx=0;
		local fSizeShifty=0;
		if(keyInfo._key==65)
		{
			fSizeShiftx = 0.2;
		}
		if(keyInfo._key==68)
		{
			fSizeShiftx = -0.2;
		}

		if(keyInfo._key==87)
		{
			fSizeShifty = 0.2;
		}
		if(keyInfo._key==83)
		{
			fSizeShifty = -0.2;
		}
		if(fSizeShiftx!=0 || fSizeShifty!=0)
		{
			if(lastUsedNLId == 0)
			{
				core_Alert("Сначала создайте или передвиньте локацию!");
				return;
			}
			local spritePos = core_GetNode(lastUsedNLId);
			if(spritePos)
			{
				spritePos._w += fSizeShiftx;
				spritePos._h += fSizeShifty;
				if(spritePos._w<0.1){
					spritePos._w=0.1;
				}
				if(spritePos._h<0.1){
					spritePos._h=0.1;
				}
				lastW = spritePos._w;
				lastH = spritePos._h;
				core_SetNode(lastUsedNLId,spritePos);
				enableSprite("nlocSaveBt",1);
			}else{
				lastUsedNLId = 0;
			}
		}
	}
	{
		local xShift=0.0;
		local yShift=0.0;
		if(keyInfo._key==38){//Up
			yShift+=0.2;
		}
		if(keyInfo._key==40){//Down
			yShift-=0.2;
		}
		if(keyInfo._key==37){//Left
			xShift-=0.2;
		}
		if(keyInfo._key==39){//Right
			xShift+=0.2;
		}
		if(xShift!=0 || yShift!=0)
		{
			if(lastUsedNLId == 0)
			{
				core_Alert("Сначала создайте или передвиньте локацию!");
				return;
			}
			local spritePos = core_GetNode(lastUsedNLId);
			if(spritePos)
			{
				if(keyInfo._shift){
					xShift=xShift*0.1;
					yShift=yShift*0.1;
				}
				spritePos._x += xShift;
				spritePos._y += yShift;
				core_SetNode(lastUsedNLId,spritePos);
				enableSprite("nlocSaveBt",1);
			}else{
				lastUsedNLId = 0;
			}
		}
	}
}

function showNlNames()
{
	local i=0;
	for(i=0;i<allUsedNL.len();i++){
		local sprInf=core_GetNode(allUsedNL[i]._id);
		if(core_GetNodeText(sprInf).len()==0){
			core_SetNodeText(sprInf,"<font-size:-2>\n"+allUsedNL[i]._name);
		}else{
			core_SetNodeText(sprInf,"");
		}
	}
	core_SetNodeText("curDotName","");
}

function nlRClick(clickedNode)
{
	local fnames=[];
	lastUsedNLId = clickedNode._id;
	local i=0;
	local lName;
	for(i=0;i<allUsedNL.len();i++)
	{
		if(allUsedNL[i]._id == lastUsedNLId){
			lName=allUsedNL[i]._name;
			fnames.append(allUsedNL[i]._name+" -> clipboard");
			break;
		}
	}
	//fnames.append("CREATE");
	fnames.append("DELETE");
	fnames.append("RENAME");
	local choice=core_AskChoice(fnames,"Action?");
	if(choice<0){
		return;
	}
	if(fnames[choice]=="DELETE"){
		nlDotDelete(clickedNode);
	}else if(fnames[choice]=="RENAME"){
		nlRenameLocation(clickedNode);
	}else if(fnames[choice]=="CREATE"){
		mainFloorNLClickRaw();
	}else{
		core_SetClipboard(lName);
	}
}

function nlDotDelete(clickedNode)
{
	local i=0;

	// Сначала смотрим соединения...
	local clickedNodeName=getLocNameByID(clickedNode._id);
	for(i=0;i<allUsedCn.len();i++)
	{
		if(allUsedCn[i]._from == clickedNodeName || allUsedCn[i]._to == clickedNodeName)
		{
			allUsedCn.remove(i);
			break;
		}
	}

	// Теперь саму точку...
	for(i=0;i<allUsedNL.len();i++)
	{
		if(allUsedNL[i]._id == clickedNode._id)
		{
			allUsedNL.remove(i);
			break;
		}
	}
	lastUsedNLId = 0;
	connectorLastId = 0;
	connectorLastIdShift = 0;
	core_DeleteSprite(clickedNode._id);
	enableSprite("nlocSaveBt",1);
}

function nlOnOver(clickNode)
{
	local i=0;
	for(i=0;i<allUsedNL.len();i++)
	{
		if(allUsedNL[i]._id == clickNode._id)
		{
			core_SetNodeText("curDotName",allUsedNL[i]._name);
			/*local ps=core_GetNode("curDotName");
			local nd=core_GetNode(clickNode._id);
			ps._x=nd._x-5;
			ps._y=nd._y+2;
			core_SetNode("curDotName",ps);*/
			break;
		}
	}
}

function nlRenameLocation(clickNode)
{
	if(lastUsedNLId == 0)
	{
		core_Alert("Сначала создайте или передвиньте локацию!");
		return;
	}
	local i=0;
	for(i=0;i<allUsedNL.len();i++)
	{
		if(allUsedNL[i]._id == lastUsedNLId)
		{
			local curName = allUsedNL[i]._name;
			local newNmae=core_AskString("Rename location...",curName);
			if(newNmae && newNmae.len()>0){
				allUsedNL[i]._name = newNmae;
				for(i=0;i<allUsedCn.len();i++)
				{
					if(allUsedCn[i]._from == curName)
					{
						allUsedCn[i]._from = newNmae;
					}
					if(allUsedCn[i]._to == curName)
					{
						allUsedCn[i]._to = newNmae;
					}
				}
				enableSprite("nlocSaveBt",1);
			}
			break;
		}
	}
}

lastCurNlPos <- {};
nlDotDragInterval <- 0;
function getNextFreeName(nameCnt)
{
	local i=0;
	local dotName;
	local bOk=false;
	while(!bOk)
	{
		bOk=true;
		dotName=format("%s::DOT%i",currentPrefix,nameCnt);
		for(i=0;i<allUsedNL.len();i++)
		{
			if(allUsedNL[i]._name==dotName){
				nameCnt++;
				bOk=false;
				break;
			}
		}
	}
	return dotName;
}

function getLocNameByID(locId)
{
	local i=0;
	for(i=0;i<allUsedNL.len();i++)
	{
		if(allUsedNL[i]._id == locId)
		{
			return allUsedNL[i]._name;
			break;
		}
	}
	core_Alert(format("cant find LOCATION for id %i !!!",locId));
	return "";
}

function getLocIDByName(locName)
{
	local i=0;
	for(i=0;i<allUsedNL.len();i++)
	{
		if(allUsedNL[i]._name == locName)
		{
			return allUsedNL[i]._id;
			break;
		}
	}
	core_Alert(format("cant find id for LOCATION %s !!!",locName));
	return 0;
}

function mainFloorNLClickRaw()
{
	local dotName = getNextFreeName(1);
	if(core_KeyPressed(20)){//CapsLock
		//core_Alert("!");
		local i=0;
		for(i=0;i<allUsedNL.len();i++)
		{
			if(allUsedNL[i]._id == lastUsedNLId)
			{
				dotName = allUsedNL[i]._name;
				break;
			}
		}
		dotName += "!";
	}
	local curCurPos = core_GetCurrentCursorPosition("mainFloor");
	local spritePos = core_GetNode("mainFloor");
	addNamedLoc({_x=curCurPos._x-spritePos._x, _y=curCurPos._y-spritePos._y, _w=lastW, _h=lastH},dotName);
	/*if(!core_KeyPressed(0x11) && connectorLastId!=0)//VK_CONTROL
	{
		addConnectLoc(getLocNameByID(connectorLastId),getLocNameByID(lastUsedNLId));
	}*/
	enableSprite("nlocSaveBt",1);
	connectorLastId = lastUsedNLId;
	if(core_KeyPressed(16)){//Shift
		nlRenameLocation(null);
	}
}

function mainFloorNLClick()
{
	if(!core_KeyPressed(17)){//Control
		return;
	}
	mainFloorNLClickRaw();
}

function nlDotDrag(clickedNode)
{
	local curCurPos = core_GetCurrentCursorPosition("mainFloor");
	if(curCurPos._x != lastCurNlPos._x || curCurPos._y != lastCurNlPos._y)
	{
		local spritePos = core_GetNode(clickedNode._id);
		spritePos._x += (curCurPos._x - lastCurNlPos._x);
		spritePos._y += (curCurPos._y - lastCurNlPos._y);
		core_SetNode(clickedNode._id,spritePos);
	}
	lastCurNlPos = curCurPos;
	if(!core_GetIsLeftButtonDown())
	{
		core_CancelInterval(nlDotDragInterval);
		nlDotDragInterval = 0;
	}	
}

function nlDotDragBegin(clickedNode)
{
	if(core_KeyPressed(16))//VK_shift
	{
		/*if(connectorLastIdShift==0)
		{
			connectorLastIdShift = clickedNode._id;
		}else{
			//core_CreateSprite(format("raw:\\-Texture:\\gui\\editor\\connector.png\n-Animation.ConnectSprites:f%i t%i\n",connectorLastId,clickedNode._id),"mainFloor");
			addConnectLoc(getLocNameByID(connectorLastIdShift),getLocNameByID(clickedNode._id));
			connectorLastIdShift = 0;
		}*/
		// Выбор из нескольких?
		local fnames=[];
		local fnamesIds=[];
		local curCurPos = core_GetCurrentCursorPosition("mainFloor");
		local floorPos = core_GetNode("mainFloor");
		curCurPos._x-=floorPos._x;
		curCurPos._y-=floorPos._y;
		local i=0;
		for(i=0;i<allUsedNL.len();i++){
			local sprInf=core_GetNode(allUsedNL[i]._id);
			//core_Warning(format("with %f-%f",sprInf._x,sprInf._y));
			if(curCurPos._x>sprInf._x-sprInf._w*0.5 && curCurPos._x<sprInf._x+sprInf._w*0.5){
				if(curCurPos._y>sprInf._y-sprInf._h*0.5 && curCurPos._y<sprInf._y+sprInf._h*0.5)
				{
					fnames.append(allUsedNL[i]._name);
					fnamesIds.append(i);
				}
			}
		}
		//core_Warning(format("%f-%f",curCurPos._x,curCurPos._y));
		if(fnames.len()>0){
			local choice=core_AskChoice(fnames,"Location?");
			if(choice!=-1){
				//core_Alert(choice.tostring());
				lastUsedNLId = allUsedNL[fnamesIds[choice]]._id;
			}
		}
		return;
	}
	if(nlDotDragInterval==0)
	{
		enableSprite("nlocSaveBt",1);
		lastCurNlPos = core_GetLastClickPosition("mainFloor");
		nlDotDragInterval = core_SetIntervalLocal("main_office_level",20,"nlDotDrag",clickedNode);
	}
	lastUsedNLId = clickedNode._id;
}
// ===============================================================
function namlocExport()
{
	if(antiChange!=0){
		core_Alert("Вы обновляли скрипты, надо перевойти!");
		return;
	}
	local i=0;
	for(i=0;i<allUsedNLLoaded.len();i++){
		nloc_Remove(allUsedNLLoaded[i]);
	}
	local floor=core_GetNode("mainFloor");
	for(i=0;i<allUsedNL.len();i++){
		nloc_Remove(allUsedNL[i]._name);
		local dot=core_GetNode(allUsedNL[i]._id);
		nloc_Add(allUsedNL[i]._name,2*dot._x/floor._w,2*dot._y/floor._h,2*dot._w/floor._w,2*dot._h/floor._h);
	}
	nloc_Save(locFilePaths[locFileIndex]);
	enableSprite("nlocSaveBt",0.1);
	core_Alert(format("Saved %i locations with prefix '%s'!",allUsedNL.len(),currentPrefix));
}

function namlocExportOld()
{
	local i=0;
	local text="";
	local floor=core_GetNode("mainFloor");
	for(i=0;i<allUsedNL.len();i++)
	{
		local dot=core_GetNode(allUsedNL[i]._id);
		local line=format("Location%i: x%f y%f w%f h%f @%s\n",i,2*dot._x/floor._w,2*dot._y/floor._h,2*dot._w/floor._w,2*dot._h/floor._h,allUsedNL[i]._name);
		text+=line;
	}

	for(i=0;i<allUsedCn.len();i++)
	{
		local line=format("Connection%i: f[%s] t[%s]\n",i,allUsedCn[i]._from,allUsedCn[i]._to);
		text+=line;
	}
	core_SaveFile(currentPrefix,text);
	core_Alert(format("Saved to %s",currentPrefix));
}
