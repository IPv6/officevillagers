function effect_SizeDrifting(particle)
{
	if(particle._doInit)
	{
		particle._initialSizeW <- particle._w;
		particle._initialSizeH <- particle._h;
		particle._initialSizeDrift <- particle._w*0.07;
		particle._initialSizeDriftSpeed <- 0.004;
	}else
	{
		particle._w=particle._initialSizeW+particle._initialSizeDrift*sin(particle._time*particle._initialSizeDriftSpeed);
		particle._h=particle._initialSizeH+particle._initialSizeDrift*sin(particle._time*particle._initialSizeDriftSpeed+3.1415);
	}
	return;
}

function effect_InitPRSAD(particle)
{
	particle._affectPosition=1;
	particle._affectRotation=1;
	particle._affectSize=1;
	particle._affectAlpha=1;
	particle._affectFade=0;
	particle._affectFlow=1;
	particle._affectColor=0;
	particle._sharedMode=0;
	particle._dieNow=0;
}

function effect_CursorDrift(particle)
{
	if(particle._doInit)
	{
		//core_Warning(particle._parameter);
		particle.initialOffset <- {_x=particle._x, _y=particle._y};
		particle.pathing <- split(particle._parameter,",");
		particle.pathingInfo <- [];
		local i=0,ttm=0;
		for(i=0;i<particle.pathing.len();i++)
		{
			ttm += 3;
			local pathPartInfo = {_name=particle.pathing[i],_time=ttm};
			particle.pathingInfo.append(pathPartInfo); 
			//core_Warning(particle.pathingInfo[i]._name);
		}
		local StartActor = actor_GetActorPos(particle.pathingInfo[0]._name);
		particle.pathingInfo[0]._xb <- StartActor._x;
		particle.pathingInfo[0]._yb <- StartActor._y;
		local EndActor = actor_GetActorPos(particle.pathingInfo[1]._name);
		particle.pathingInfo[0]._xe <- EndActor._x;
		particle.pathingInfo[0]._ye <- EndActor._y;
		if(particle.pathing.len())
		{
			particle._x = StartActor._x+particle.initialOffset._x;
			particle._y = StartActor._y+particle.initialOffset._y;
			local length = sqrt((StartActor._x-EndActor._x)*(StartActor._x-EndActor._x)+(StartActor._y-EndActor._y)*(StartActor._y-EndActor._y));
			particle._time2path <- (1000*length*0.2).tointeger();
		}
		// Расставляем по местам
	}
	// Устанавливаем координаты - даже при ините!
	{
		local needle = (particle._time%particle._time2path).tofloat()/particle._time2path.tofloat();
		particle._alpha=1;
		if(needle<0.1){
			particle._alpha=0;
		}else if(needle<0.3){
			particle._alpha=(needle-0.1)/0.2;
		}else if(needle>0.9){
			particle._alpha=0;
		}else if(needle>0.7){
			particle._alpha=1.0-(needle-0.7)/0.2;
		}
		//core_Warning(needle.tostring());
		local StartActor = {_x=particle.pathingInfo[0]._xb,_y=particle.pathingInfo[0]._yb};//actor_GetActorPos(particle.pathingInfo[0]._name);
		local EndActor = {_x=particle.pathingInfo[0]._xe,_y=particle.pathingInfo[0]._ye};//actor_GetActorPos(particle.pathingInfo[1]._name);
		particle._x = (EndActor._x*needle + StartActor._x*(1.0-needle))+particle.initialOffset._x;
		particle._y = (EndActor._y*needle + StartActor._y*(1.0-needle))+particle.initialOffset._y;
		//core_Alert(format("!!! %f %f",particle._x,particle._y));
	}
	return;
}

function effect_MatchPosL(particle)
{
	particle.type <- 1;
	effect_MatchPos(particle);
}

function effect_MatchPosR(particle)
{
	particle.type <- 2;
	effect_MatchPos(particle);
}

function effect_MatchPos(particle)
{
	if(particle._doInit)
	{
		local type=0;
		if("type" in particle){
			type = particle.type;
		}
		local pathing = split(particle._parameter,",");
		local StartActor = actor_GetActorPos(pathing[0]);
		local EndActor = actor_GetActorPos(pathing[1]);
		if(type==0){
			particle._w = sqrt((StartActor._x-EndActor._x)*(StartActor._x-EndActor._x)+(StartActor._y-EndActor._y)*(StartActor._y-EndActor._y));//fabs(StartActor._x-EndActor._x);
			particle._h = 2;//fabs(StartActor._y-EndActor._y);
			particle._x = (StartActor._x+EndActor._x)*0.5;
			particle._y = (StartActor._y+EndActor._y)*0.5;
			particle._rotation = atan2((EndActor._y-StartActor._y),(EndActor._x-StartActor._x));
		}
		if(type==1){
			particle._x = StartActor._x;
			particle._y = StartActor._y;
			particle._h = 3;
			particle._w = 3;
		}
		if(type==2){
			particle._x = EndActor._x;
			particle._y = EndActor._y;
			particle._h = 3;
			particle._w = 3;
		}
	}
	particle._cancel=1;
}

function initTalkBaloonSprite(baloon)
{
	local thisActorID = core_GetNodeAttribute(baloon,"ACTORID");
	//core_Alert("a1="+thisActorID.tostring());
	local partnerActorID = actor_GetAttributeS(thisActorID,"TalkPartner");
	//core_Alert("a2="+partnerActorID.tostring());
	local posThis=actor_GetActorPos(thisActorID);
	local posPartner=actor_GetActorPos(partnerActorID);
	if(posThis._x>posPartner._x){
		// Надо инвертировать позицию
		baloon._x=-baloon._x;
		baloon._w=-baloon._w;
		core_SetNode(baloon,baloon);
	}
}