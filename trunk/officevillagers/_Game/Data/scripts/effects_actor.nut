function actor_AddIconDelayed(param)
{
	actor_AddItem(param._actor,"fly_icon","icon="+param._icon);
}

function actor_AddIcon(thisActor,iconName)
{
	if(game_IsCutscene()){
		// No one should see this in cutscene
		return false;
	}
	//actor_AddItem(thisActor,"fly_icon","icon="+iconName);
	//if(core_KeyPressed(16)){core_Warning(format("adding %s",iconName));}
	core_SetTimeoutLocal("main_office_level",core_Rnd(500,5000),"actor_AddIconDelayed",{_actor=thisActor,_icon=iconName});
	return;
}

function effect_ActorProgress(particle)
{
	if(particle._doInit)
	{
		// Расставляем по местам
		particle.nodeFull <- core_GetNode(particle);
		local actorId=core_GetNodeAttribute(particle.nodeFull,"ACTORID");//core_GetNode(particle.nodeFull._parentid);
		particle.actor <- actor_GetActor(actorId);
		//core_Warning("progress on actor "+particle.actor.Name);
	}
	{
		//particle._alpha=1;
		local atlasData=core_GetNodeAtlas(particle.nodeFull);
		local prc=actor_GetAttributeN(particle.actor,particle._parameter)/100.0;
		atlasData._x=(atlasData._sizex*prc).tointeger();
		core_SetNodeAtlas(particle.nodeFull,atlasData);
	}
	return;
}  