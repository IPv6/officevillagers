//TODO добавить mouse picking
//TODO добавить пересчет поворота в градусы
__isEditorEnabled <- false;
__isEditorActive <- false;
__isEditorActiveSpr <- null;
__isEditorActiveSprHL <- null;
function ActivateF6Editor(key)
{
	local handled=0;
	//core_Alert(dump(key));
	if(("_Disabling" in key) && key._Disabling==1)//F6
	{
		key._Disabling=0;
		if(__isEditorActive){
			__isEditorActive=false;
			core_Warning("SprLayoutEditor disabled");
			if(__isEditorActiveSprHL){
				core_DeleteNode(__isEditorActiveSprHL);
				__isEditorActiveSprHL=null;
			}
			return true;
		}
	}
	
	if(("_Enabling" in key) && key._Enabling==1)//F6
	{
		key._Enabling=0;
		local i;
		__isEditorActive=false;
		local aNamedSprs=core_GetNodesNames();
		local preffered=[], supers=1;
		aNamedSprs.sort();
		for(i=0;i<aNamedSprs.len();i++){
			local nod=core_GetNode(aNamedSprs[i]);
			local pos=core_GetCurrentCursorPosition(aNamedSprs[i],true);
			if(fabs(pos._x)<nod._w && fabs(pos._y)<nod._h){
				preffered.append("-"+supers.tostring()+" "+aNamedSprs[i]);
				supers++;
			}
		}
		if(preffered.len()>0){
			local extendedNames=preffered;
			extendedNames.extend(aNamedSprs);
			aNamedSprs=extendedNames;
		}
		local choice=core_AskChoice(aNamedSprs,"Sprite");
		if(choice<0){
			return;
		}
		local choiceName=aNamedSprs[choice];
		if(choiceName.slice(0,1)=="-"){
			choiceName=choiceName.slice(3);
		}
		__isEditorActiveSpr=core_GetNode(choiceName);
		if(__isEditorActiveSpr){
			__isEditorActiveSpr._name<-choiceName;
			core_Warning("SprLayoutEditor enabled on "+choiceName);
			__isEditorActive=true;
			__isEditorActiveSprHL=core_CreateNode("\\gui\\black.png",__isEditorActiveSpr,{z=0.000001,w=__isEditorActiveSpr._w,h=__isEditorActiveSpr._h});
			handled++;//return true;
		}else{
			core_Warning("SprLayoutEditor disabled: node unaccessible");
			return true;
		}
	}

	if(key._pressed){
		if(!__isEditorActive){
			return false;
		}
		//core_Warning(format("LEditor Keypress: %i",key._key));
		local xShift=0.0;
		local yShift=0.0;
		local step=0.2;
		if(key._shift){
			step=0.05;
		}
		if(key._key==38){//Up
			yShift+=step;
		}
		if(key._key==40){//Down
			yShift-=step;
		}
		if(key._key==37){//Left
			xShift-=step;
		}
		if(key._key==39){//Right
			xShift+=step;
		}
		if(xShift!=0 || yShift!=0)
		{
			handled++;
			if(key._control){
				__isEditorActiveSpr._w+=xShift;
				__isEditorActiveSpr._h+=yShift;
			}else{
				__isEditorActiveSpr._x+=xShift;
				__isEditorActiveSpr._y+=yShift;
			}
		}
		if(key._key==33){//PageUp
			handled++;
			__isEditorActiveSpr._z-=step;
		}
		if(key._key==34){//PageDown
			handled++;
			__isEditorActiveSpr._z+=step;
		}
		if(key._key==36){//Home
			handled++;
			__isEditorActiveSpr._rotation-=step*0.5;
		}
		if(key._key==35){//End
			handled++;
			__isEditorActiveSpr._rotation+=step*0.5;
		}
		if(handled){
			core_SetNode(__isEditorActiveSpr,__isEditorActiveSpr);
			__isEditorActiveSprHL._fade=0.2;
			__isEditorActiveSprHL._w=__isEditorActiveSpr._w;
			__isEditorActiveSprHL._h=__isEditorActiveSpr._h;
			__isEditorActiveSprHL._rotation=__isEditorActiveSpr._rotation;
			if(__isEditorActiveSprHL._w<0.2){
				__isEditorActiveSprHL._w=0.2;
				__isEditorActiveSprHL._fade=1.0;
			}
			if(__isEditorActiveSprHL._h<0.2){
				__isEditorActiveSprHL._h=0.2;
				__isEditorActiveSprHL._fade=1.0;
			}
			core_SetNode(__isEditorActiveSprHL,__isEditorActiveSprHL);
		}
		if(key._key==32){
			handled++;
			local data="Sprite ["+__isEditorActiveSpr._name+"]:\n";
			data+="x: "+__isEditorActiveSpr._x.tostring()+"\n";
			data+="y: "+__isEditorActiveSpr._y.tostring()+"\n";
			data+="z: "+__isEditorActiveSpr._z.tostring()+"\n";
			data+="w: "+__isEditorActiveSpr._w.tostring()+"\n";
			data+="h: "+__isEditorActiveSpr._h.tostring()+"\n";
			data+="r: "+format("%.02f (rad %.04f)",__isEditorActiveSpr._rotation*180/3.1415,__isEditorActiveSpr._rotation)+"\n";
			core_Alert(data);
		}
		return handled>0?true:false;
	}
	return false;
}
