function CalcDeadlineDate()
{
     local timeToNewIsuue=level_GetTimeToNewIssue()*game_GetGameSpeed();
     local dateNow=core_GetDate();
     dateNow=core_AddTime2Date(dateNow,timeToNewIsuue);
     local dateFormat = core_Translate("DEADLINE_DATE_FROMAT");
     local result=format(dateFormat,format("%i",dateNow._day),core_Translate(format("MONTH_OF%i",dateNow._month)),format("%i",dateNow._hour),format("%i",dateNow._min));
     return result;
}

function RecalcOfficeParameters()
{
    actor_SetAttribute("Office","DEADLINE_DATE",CalcDeadlineDate());
    actor_SetAttribute("Office","FinancierNeed",actor_GetAttributeS("Office", "FinansierMax"));
    // Число финансистов
    local objectsArray = actor_GetActors({_byProfession="*FINANCIER*"});
    local FinancierCur = objectsArray.len();
    if (actor_GetActorProfession(actor_GetActor("ACTOR.AGATA"))=="FINANCIER"){
        FinancierCur = FinancierCur - 1;
    }
    actor_SetAttribute("Office","FinancierCur",FinancierCur);
}

function RecalcOfficeOnLevelChange()
{
     local ProgrMax=getGlobal(format("IssueProgressMax%i", actor_GetAttributeN("Office", "Level")));
     gui_SetIssueProgressMax(ProgrMax);
     actor_SetAttribute("Office","IssueProgressMax", ProgrMax);
     actor_SetAttribute("Office", "FinansierMax", getGlobal(format("FinansierMax%i", actor_GetAttributeN("Office", "Level"))));
     actor_SetAttribute("Office","IssueProgressCur", gui_GetIssueProgressCur());
}