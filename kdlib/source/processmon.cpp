#include "stdafx.h"

#include <map>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/atomic.hpp>

#include "processmon.h"

namespace kdlib
{

///////////////////////////////////////////////////////////////////////////////

class ProcessInfo;
typedef boost::shared_ptr<ProcessInfo>  ProcessInfoPtr;

///////////////////////////////////////////////////////////////////////////////

class ProcessInfo {

public:

    ModulePtr getModule(MEMOFFSET_64  offset);
    void insertModule( ModulePtr& module);
    void removeModule(MEMOFFSET_64  offset );

    void insertBreakpoint(BreakpointPtr& breakpoint);
    void removeBreakpoint(BreakpointPtr& breakpoint);

    DebugCallbackResult breakpointHit(BreakpointPtr& breakpoint);

private:

    typedef std::map<MEMOFFSET_64, ModulePtr> ModuleMap;
    ModuleMap  m_moduleMap;
    boost::recursive_mutex  m_moduleLock;
    
     typedef std::map<BREAKPOINT_ID, BreakpointPtr>  BreakpointIdMap;
     BreakpointIdMap  m_breakpointMap;
     boost::recursive_mutex  m_breakpointLock;
};

///////////////////////////////////////////////////////////////////////////////

class ProcessMonitorImpl {

public:

    ProcessMonitorImpl() : m_bpUnique(0x80000000)
    {}

public:

    DebugCallbackResult processStart(PROCESS_DEBUG_ID id);
    DebugCallbackResult processStop(PROCESS_DEBUG_ID id, ProcessExitReason reason, unsigned int ExitCode);
    void processAllStop();
    unsigned int getNumberProcesses();

    DebugCallbackResult moduleLoad(PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, const std::wstring& moduleName);
    DebugCallbackResult moduleUnload(PROCESS_DEBUG_ID id, MEMOFFSET_64  offset, const std::wstring& moduleName);
    DebugCallbackResult breakpointHit(PROCESS_DEBUG_ID id, BreakpointPtr& breakpoint);
    void currentThreadChange(THREAD_DEBUG_ID threadid);
    void executionStatusChange(ExecutionStatus status);
    void localScopeChange();
    void breakpointsChange(PROCESS_DEBUG_ID id);
    DebugCallbackResult  exceptionHit(const ExceptionInfo& excinfo);
    void debugOutput(const std::wstring text);

    ModulePtr getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id );
    void insertModule( ModulePtr& module, PROCESS_DEBUG_ID id );

    void registerEventsCallback(DebugEventsCallback *callback);
    void removeEventsCallback(DebugEventsCallback *callback);

    void registerBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id = -1 );
    void removeBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id = -1 );

    

private:

    ProcessInfoPtr  getProcess( PROCESS_DEBUG_ID id );

    boost::recursive_mutex  m_lock;

    typedef std::map<PROCESS_DEBUG_ID, ProcessInfoPtr>  ProcessMap;
    ProcessMap  m_processMap;

    boost::atomic<unsigned long long>  m_bpUnique;

private:

    typedef std::list<DebugEventsCallback*>  EventsCallbackList;

    boost::recursive_mutex      m_callbacksLock;
    EventsCallbackList          m_callbacks;
};

std::auto_ptr<ProcessMonitorImpl>  g_procmon;

///////////////////////////////////////////////////////////////////////////////

class CurrentProcessGuard {

public:

    CurrentProcessGuard() :
        m_procId( getCurrentProcessId() )
        {}

    ~CurrentProcessGuard() {
        setCurrentProcessById(m_procId);
    }

private:

    PROCESS_DEBUG_ID  m_procId;
};

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::init()
{
    g_procmon.reset( new ProcessMonitorImpl );
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::deinit()
{
    g_procmon.reset(0);
}

/////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::registerEventsCallback(DebugEventsCallback *callback)
{
    g_procmon->registerEventsCallback(callback);
}

/////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::removeEventsCallback(DebugEventsCallback *callback)
{
    g_procmon->removeEventsCallback(callback);
}

/////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::registerBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id )
{
    if ( id == -1 )
        id = getCurrentProcessId();

    g_procmon->registerBreakpoint(breakpoint, id);
}


/////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::removeBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id )
{
    if ( id == -1 )
        id = getCurrentProcessId();

    g_procmon->removeBreakpoint(breakpoint, id);
}

/////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::processStart(PROCESS_DEBUG_ID id)
{
    return g_procmon->processStart(id);
}

/////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::processStop(PROCESS_DEBUG_ID id, ProcessExitReason reason, unsigned int exitCode)
{
    return g_procmon->processStop(id, reason, exitCode);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::processAllStop()
{
    g_procmon->processAllStop();
}

///////////////////////////////////////////////////////////////////////////////

unsigned int ProcessMonitor::getNumberProcesses()
{
    return g_procmon->getNumberProcesses();
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::moduleLoad(PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, const std::wstring& moduleName)
{
    return g_procmon->moduleLoad(id, offset, moduleName);
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::moduleUnload(PROCESS_DEBUG_ID id, MEMOFFSET_64  offset, const std::wstring& moduleName)
{
    return g_procmon->moduleUnload(id, offset, moduleName);
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::breakpointHit(PROCESS_DEBUG_ID id, BreakpointPtr& breakpoint)
{
    return g_procmon->breakpointHit(id, breakpoint);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::currentThreadChange(THREAD_DEBUG_ID id)
{
    g_procmon->currentThreadChange(id);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::executionStatusChange(ExecutionStatus status)
{
    g_procmon->executionStatusChange(status);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::localScopeChange()
{
    g_procmon->localScopeChange();
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::breakpointsChange(PROCESS_DEBUG_ID id)
{
    g_procmon->breakpointsChange(id);
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitor::exceptionHit(const ExceptionInfo& excinfo)
{
    return g_procmon->exceptionHit(excinfo);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::debugOutput(const std::wstring text)
{
    return g_procmon->debugOutput(text);
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ProcessMonitor::getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id )
{
    if ( id == -1 )
        id = getCurrentProcessId();

    return g_procmon->getModule(offset,id);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitor::insertModule( ModulePtr& module, PROCESS_DEBUG_ID id )
{
    if ( id == -1 )
        id = getCurrentProcessId();

    return g_procmon->insertModule(module, id);
}


///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitorImpl::processStart(PROCESS_DEBUG_ID id)
{
    {
        ProcessInfoPtr  proc = ProcessInfoPtr(new ProcessInfo());
        boost::recursive_mutex::scoped_lock l(m_lock);
        m_processMap[id] = proc;
    }

    DebugCallbackResult  result = DebugCallbackNoChange;

    return result;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitorImpl::processStop(PROCESS_DEBUG_ID id, ProcessExitReason reason, unsigned int exitCode)
{
    {
        boost::recursive_mutex::scoped_lock l(m_lock);
        m_processMap.erase(id);
    }

    DebugCallbackResult  result = DebugCallbackNoChange;

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it = m_callbacks.begin();

    for (; it != m_callbacks.end(); ++it)
    {
        DebugCallbackResult  ret = (*it)->onProcessExit(id, reason, exitCode);
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::processAllStop()
{
    boost::recursive_mutex::scoped_lock l(m_lock);
    m_processMap.clear();
}

///////////////////////////////////////////////////////////////////////////////

unsigned int ProcessMonitorImpl::getNumberProcesses()
{
    boost::recursive_mutex::scoped_lock l(m_lock);
    return m_processMap.size();
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ProcessMonitorImpl::getModule( MEMOFFSET_64  offset, PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  processInfo = getProcess(id);

    ModulePtr  module;

    if ( processInfo )
        module = processInfo->getModule(offset);

    return module;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitorImpl::moduleLoad(PROCESS_DEBUG_ID id, MEMOFFSET_64 offset, const std::wstring& moduleName)
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    ProcessInfoPtr  processInfo = getProcess(id);

    if ( processInfo )
    {
        processInfo->removeModule( offset );
        loadModule(offset);
    }

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for (it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        DebugCallbackResult  ret = (*it)->onModuleLoad(offset, moduleName);
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitorImpl::moduleUnload(PROCESS_DEBUG_ID id, MEMOFFSET_64  offset, const std::wstring &moduleName)
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    ProcessInfoPtr  processInfo = getProcess(id);

    if ( processInfo )
        processInfo->removeModule( offset );

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for ( it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
    {
        DebugCallbackResult  ret = (*it)->onModuleUnload(offset, moduleName );
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessMonitorImpl::breakpointHit(PROCESS_DEBUG_ID id, BreakpointPtr& breakpoint)
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    ProcessInfoPtr  processInfo = getProcess(id);
    if ( processInfo )
        result = processInfo->breakpointHit(breakpoint);

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for (it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        DebugCallbackResult  ret = (*it)->onBreakpoint(breakpoint->getId());
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::currentThreadChange(THREAD_DEBUG_ID threadid)
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it = m_callbacks.begin();

    for (; it != m_callbacks.end(); ++it)
    {
        (*it)->onCurrentThreadChange(threadid);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::executionStatusChange(ExecutionStatus status)
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it = m_callbacks.begin();

    for (; it != m_callbacks.end(); ++it)
    {
        (*it)->onExecutionStatusChange(status);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::localScopeChange()
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it = m_callbacks.begin();

    for (; it != m_callbacks.end(); ++it)
    {
        (*it)->onChangeLocalScope();
    }
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::breakpointsChange(PROCESS_DEBUG_ID id)
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it = m_callbacks.begin();

    for (; it != m_callbacks.end(); ++it)
    {
        (*it)->onChangeBreakpoints();
    }
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult  ProcessMonitorImpl::exceptionHit(const ExceptionInfo& excinfo)
{
    DebugCallbackResult  result = DebugCallbackNoChange;

    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for (it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        DebugCallbackResult  ret = (*it)->onException(excinfo);
        result = ret != DebugCallbackNoChange ? ret : result;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::debugOutput(const std::wstring text)
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);

    EventsCallbackList::iterator  it;
    for (it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        (*it)->onDebugOutput(text);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::insertModule( ModulePtr& module, PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  processInfo = getProcess(id);
    if ( processInfo )
        return processInfo->insertModule(module);
}

///////////////////////////////////////////////////////////////////////////////

ProcessInfoPtr ProcessMonitorImpl::getProcess( PROCESS_DEBUG_ID id )
{
    boost::recursive_mutex::scoped_lock l(m_lock);

    ProcessMap::iterator  it  = m_processMap.find(id);

    if ( it != m_processMap.end() )
        return it->second;

    ProcessInfoPtr  proc = ProcessInfoPtr( new ProcessInfo() );
    m_processMap[id] = proc;

    return proc;
}


///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::registerEventsCallback(DebugEventsCallback *callback)
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);
    m_callbacks.push_back(callback);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::removeEventsCallback(DebugEventsCallback *callback)
{
    boost::recursive_mutex::scoped_lock l(m_callbacksLock);
    m_callbacks.remove(callback);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::registerBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  processInfo = getProcess(id);

    if ( processInfo )
    {
        processInfo->insertBreakpoint(breakpoint);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ProcessMonitorImpl::removeBreakpoint( BreakpointPtr& breakpoint, PROCESS_DEBUG_ID id )
{
    ProcessInfoPtr  processInfo = getProcess(id);

    if ( processInfo )
    {
        processInfo->removeBreakpoint(breakpoint);
    }
}

///////////////////////////////////////////////////////////////////////////////

ModulePtr ProcessInfo::getModule(MEMOFFSET_64  offset)
{
    boost::recursive_mutex::scoped_lock l(m_moduleLock);

    ModuleMap::iterator it = m_moduleMap.find(offset);

    if ( it != m_moduleMap.end() )
        return it->second;

    for ( ModuleMap::iterator it = m_moduleMap.begin(); it != m_moduleMap.end(); ++it )
    {
        if ( it->second->getBase() <= offset && offset < it->second->getEnd() )
        {
           return it->second;
        }
    }

    return ModulePtr();
}

///////////////////////////////////////////////////////////////////////////////

void ProcessInfo::insertModule( ModulePtr& module)
{
    boost::recursive_mutex::scoped_lock l(m_moduleLock);
    m_moduleMap[ module->getBase() ] = module;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessInfo::removeModule(MEMOFFSET_64  offset )
{
    boost::recursive_mutex::scoped_lock l(m_moduleLock);
    m_moduleMap.erase(offset);
}

///////////////////////////////////////////////////////////////////////////////

void ProcessInfo::insertBreakpoint(BreakpointPtr& breakpoint)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);
    
    m_breakpointMap[breakpoint->getId()] = breakpoint;
}

///////////////////////////////////////////////////////////////////////////////

void ProcessInfo::removeBreakpoint(BreakpointPtr& breakpoint)
{
    BreakpointPtr origbp;

    {
        boost::recursive_mutex::scoped_lock l(m_breakpointLock);

        BreakpointIdMap::iterator   it = m_breakpointMap.find(breakpoint->getId());
        if (it == m_breakpointMap.end() )
            return;

        origbp = it->second;

        m_breakpointMap.erase(it);
    }

    BreakpointCallback*  callback = origbp->getCallback();
    if ( callback != 0 )
        callback->onRemove();

}


///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ProcessInfo::breakpointHit(BreakpointPtr& breakpoint)
{
    boost::recursive_mutex::scoped_lock l(m_breakpointLock);
    
    BreakpointIdMap::iterator  it =  m_breakpointMap.find( breakpoint->getId() );

    if ( it == m_breakpointMap.end() )
        return DebugCallbackNoChange;

    BreakpointPtr  origBp = it->second;

    BreakpointCallback*  callback = origBp->getCallback();
    if ( callback == 0 )
        return DebugCallbackBreak;

    return callback->onHit();
}

/////////////////////////////////////////////////////////////////////////////


} //namesapce kdlib

