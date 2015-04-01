#pragma once

#include <boost/shared_ptr.hpp>

#include "kdlib/dbgtypedef.h"
#include "kdlib/module.h"
#include "kdlib/breakpoint.h"
#include "kdlib/stack.h"
#include "kdlib/variant.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class TargetSystem;
typedef boost::shared_ptr<TargetSystem>  TargetSystemPtr;

class TargetProcess;
typedef boost::shared_ptr<TargetProcess>  TargetProcessPtr;

class TargetThread;
typedef boost::shared_ptr<TargetThread>  TargetThreadPtr;

///////////////////////////////////////////////////////////////////////////////

class TargetSystem
{
public: 

    static TargetSystemPtr  getCurrent();
    static TargetSystemPtr  getByIndex(unsigned long index);
    static unsigned long getNumber();

public:

    virtual std::wstring  getDescription() = 0;
    virtual bool isDumpAnalyzing() = 0;
    virtual bool isKernelDebugging() = 0;
    virtual bool is64bitSystem() = 0;

    virtual bool isCurrent() = 0;
    virtual void setCurrent() = 0;
    
    virtual unsigned long getNumberProcesses() = 0;
    virtual TargetProcessPtr getProcessByIndex(unsigned long index) = 0;
    virtual TargetProcessPtr getCurrentProcess() = 0;

};

///////////////////////////////////////////////////////////////////////////////

class TargetProcess
{
public:

    static TargetProcessPtr  getCurrent();
    static TargetProcessPtr  getByIndex(unsigned long index);
    static unsigned long getNumber();

public:

    virtual std::wstring getExecutableName() = 0;
    virtual PROCESS_ID getSystemId() = 0;
    virtual MEMOFFSET_64 getPebOffset() = 0;

    virtual bool isCurrent() = 0;
    virtual void setCurrent() = 0;

    virtual unsigned long getNumberThreads() = 0;
    virtual TargetThreadPtr getThreadByIndex(unsigned long index) = 0;
    virtual TargetThreadPtr getCurrentThread() = 0;

    virtual unsigned long getNumberModules() = 0;
    virtual ModulePtr getModuleByIndex(unsigned long index) = 0;

    virtual unsigned long getNumberBreakpoints() = 0;
    virtual BreakpointPtr getBreakpoint(unsigned long index) = 0;
};

///////////////////////////////////////////////////////////////////////////////

class TargetThread
{
public:

    static TargetThreadPtr getCurrent();
    static TargetThreadPtr getByIndex(unsigned long index);
    static unsigned long getNumber();

public:

    virtual THREAD_ID getSystemId() = 0;
    virtual MEMOFFSET_64 getTebOffset() = 0;
    virtual TargetProcessPtr getProcess() = 0;

    virtual bool isCurrent() = 0;
    virtual void setCurrent() = 0;

    virtual StackPtr getStack() = 0;

    virtual unsigned long getNumberRegisters() = 0;
    virtual NumVariant getRegisterByName(const std::wstring& regName) = 0;
    virtual NumVariant getRegisterByIndex(unsigned long regIndex) = 0;
};

///////////////////////////////////////////////////////////////////////////////

};
