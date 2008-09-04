function CalcDeadlineDate()
{
	local timeToNewIsuue=level_GetTimeToNewIssue()*game_GetGameSpeed();
	local dateNow=core_GetDate();
	dateNow=core_AddTime2Date(dateNow,timeToNewIsuue);
	local result = core_Translate("DEADLINE_DATE_FROMAT");
	result = core_StrReplace(result,"@DAY",format("%i",dateNow._day));
	result = core_StrReplace(result,"@MON",core_Translate(format("MONTH_OF%i",dateNow._month)));
	result = core_StrReplace(result,"@HOUR",format("%02i",dateNow._hour));
	result = core_StrReplace(result,"@MIN",format("%02i",dateNow._min));
	return result;
}

function RecalcOfficeParameters()
{
    actor_SetAttribute("Office","DEADLINE_DATE",CalcDeadlineDate());
    actor_SetAttribute("Office","FinancierNeed",actor_GetAttributeS("Office", "FinansierMax"));
    // Число финансистов
    //local objectsArray = actor_GetActors({_byProfession="*FINANCIER*"});
    local FinancierCur = actor_GetProfessionStats("FINANCIER").ActorsCount+actor_GetProfessionStats("FINANCIER-EXPERT").ActorsCount;
	local agataProf = actor_GetActorProfession(actor_GetActor("ACTOR.AGATA"));
    if (agataProf=="FINANCIER" || agataProf=="FINANCIER-EXPERT"){
        FinancierCur = FinancierCur - 1;
    }
    actor_SetAttribute("Office","FinancierCur",FinancierCur);
}

function RecalcOfficeOnLevelChange()
{
	if(actor_GetAttributeN("Office", "Level")<5){
		actor_SetAttribute("Office", "Level", 5);
	}
	local ProgrMax=getGlobal(format("IssueProgressMax%i", actor_GetAttributeN("Office", "Level")));
	gui_SetIssueProgressMax(ProgrMax);
	actor_SetAttribute("Office", "FinansierMax", getGlobal(format("FinansierMax%i", actor_GetAttributeN("Office", "Level"))));
	actor_SetAttribute("Office","IssueProgressCur", gui_GetIssueProgressCur());
	RecalcOfficeParameters();
}

function office_CalcSkipTime()
{
	return 4.0;
}