function CalcDeadlineDate()
{
     local timeToNewIsuue=level_GetTimeToNewIssue()*game_GetGameSpeed();
     local dateNow=core_GetDate();
     dateNow=core_AddTime2Date(dateNow,timeToNewIsuue);
     local dateFormat = core_Translate("DEADLINE_DATE_FROMAT");
     local result=format(dateFormat,format("%i",dateNow._day), core_Translate(format("MONTH_OF%i",dateNow._month), format("%i",dateNow._hour), format("%i",dateNow._min));
     return result;
}

function RecalcOfficeParameters()
{
    actor_SetAttribute("Office","DEADLINE_DATE",CalcDeadlineDate());
}
