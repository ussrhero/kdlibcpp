#include "stdafx.h"

#include "dbgsession.h"
#include "dbgext.h"

DbgExt      dbgGlobalSession = { 0 };

bool        dbgSessionStarted = false;

void
dbgCreateSession()
{
    IDebugClient4     *client = NULL;
    DebugCreate( __uuidof(IDebugClient4), (void **)&client );  
        
    SetupDebugEngine( client, &dbgGlobalSession );
    dbgExt = &dbgGlobalSession;
    
    dbgSessionStarted = true;
}

bool
dbgIsSessionStart()
{
    return dbgSessionStarted;
}