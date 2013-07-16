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

    throw DbgException( L"unsupported registry type");
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end