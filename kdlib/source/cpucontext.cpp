#include "stdafx.h"

#include "kdlib/cpucontext.h"
#include "kdlib/dbgengine.h"
#include "kdlib/exceptions.h"

namespace kdlib {

/////////////////////////////////////////////////////////////

CPUContext::CPUContext( size_t index )
{
    if ( index == -1 )
    {
        index = getCurrentThreadId();
    }
    else
    {
        if ( index >= getNumberThreads() )
            throw IndexException(index);
    }

    m_contextIndex = index;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 CPUContext::getIP()
{
    return  getInstructionOffset( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 CPUContext::getSP()
{
     return  getStackOffset( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 CPUContext::getFP()
{
    return  getFrameOffset( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

NumVariant CPUContext::getRegisterByName( const std::wstring &name)
{
    size_t index = getRegsiterIndex( m_contextIndex, name );
    return getRegisterByIndex( index );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring CPUContext::getRegisterName( size_t index )
{
    return kdlib::getRegisterName( m_contextIndex, index );
}

///////////////////////////////////////////////////////////////////////////////

NumVariant CPUContext::getRegisterByIndex( size_t index )
{
    CPURegType  regType = getRegisterType(m_contextIndex, index);

    switch( regType )
    {
    case  RegInt8:
        {
            char  val;
            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;

    case  RegInt16:
        {
            short  val;
            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;

    case  RegInt32:
        {
            long  val;
            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;


    case  RegInt64:
        {
            long long  val;
            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;

    case  RegFloat32:
        {
            float  val;
            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;


    case  RegFloat64:
        {
            double  val;
            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;

    case RegFloat80:
    case RegFloat128:
    case RegVector64:
    case RegVector128:
        return NumVariant();
    }

    throw DbgException( "unsupported registry type");
}

///////////////////////////////////////////////////////////////////////////////

unsigned long long CPUContext::loadMSR( size_t msrIndex )
{
    return kdlib::loadMSR(m_contextIndex, msrIndex );
}

///////////////////////////////////////////////////////////////////////////////

void CPUContext::setMSR( size_t msrIndex, unsigned long long value )
{
    return kdlib::setMSR( m_contextIndex, msrIndex, value );
}

///////////////////////////////////////////////////////////////////////////////

CPUType CPUContext::getCPUType()
{
    return kdlib::getCPUType( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

CPUType CPUContext::getCPUMode()
{
    return kdlib::getCPUMode( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

size_t CPUContext::getStackLength()
{
    return kdlib::getNumberFrames( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

void CPUContext::getStackFrame( size_t frameIndex, MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp )
{
    kdlib::getStackFrame( m_contextIndex, frameIndex, ip, ret, fp, sp );
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end
