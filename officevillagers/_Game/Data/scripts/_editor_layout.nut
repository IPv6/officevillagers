__inputTypeInit <- false;
__isEditorEnabled <- false;
__isEditorActive <- false;
__isEditorActiveSpr <- null;
__isEditorActiveSprHL <- null;
function HandleLayoutEditor(key)
{
	if(!__inputTypeInit){
		__inputTypeInit=true;
		core_SetKeyHandlerMode(2);
		if(core_GetIniParameter("EnableIngameEditor")!="0"){
			__isEditorEnabled=true;
		}
	}
	if(!__isEditorEnabled){
		return false;
	}
	// F6 - выбор
	local handled=0;
	if(key._pressed){
		if(key._key==117)//F6
		{
			if(__isEditorActive){
				__isEditorActive=false;
				core_Warning("SprLayoutEditor disabled");
				if(__isEditorActiveSprHL){
					core_DeleteNode(__isEditorActiveSprHL);
					__isEditorActiveSprHL=null;
				}
				return true;
			}
			//TODO добавить mouse picking
			local aNamedSprs=core_GetNodesNames();
			local choice=core_AskChoice(aNamedSprs,"Sprite&");
			if(choice<0){
				return;
			}
			__isEditorActiveSpr=core_GetNode(aNamedSprs[choice]);
			if(__isEditorActiveSpr){
				__isEditorActiveSpr._name<-aNamedSprs[choice];
				core_Warning("SprLayoutEditor enabled on "+aNamedSprs[choice]);
				__isEditorActive=true;
				__isEditorActiveSprHL=core_CreateNode("\\gui\\black.png",__isEditorActiveSpr,{z=0.000001,w=__isEditorActiveSpr._w,h=__isEditorActiveSpr._h});
				handled++;//return true;
			}else{
				core_Warning("SprLayoutEditor disabled: node unaccessible");
				return true;
			}
		}
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
			core_SetNode(__isEditorActiveSprHL,__isEditorActiveSprHL);
		}
		if(key._key==32){
			handled++;
			local data="Sprite ["__isEditorActiveSpr._name+"]:\n";
			data+="x: "+__isEditorActiveSpr._x.tostring()+"\n";
			data+="y: "+__isEditorActiveSpr._y.tostring()+"\n";
			data+="z: "+__isEditorActiveSpr._z.tostring()+"\n";
			data+="w: "+__isEditorActiveSpr._w.tostring()+"\n";
			data+="h: "+__isEditorActiveSpr._h.tostring()+"\n";
			data+="r: "+__isEditorActiveSpr._rotation.tostring()+"\n";
			core_Alert(data);
		}
		return handled>0?true:false;
	}
	return false;
}
