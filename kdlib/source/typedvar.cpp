#include "stdafx.h"

#include <iomanip>

#include <boost/regex.hpp>

#include "kdlib/typedvar.h"
#include "kdlib/module.h"
#include "kdlib/stack.h"

#include "typedvarimp.h"

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

std::wstring printFieldValue( const kdlib::TypeInfoPtr& fieldType, const kdlib::TypedVarPtr& fieldVar )
{
    std::wstringstream  sstr;

    if ( fieldType->isBase() )
    {
        return dynamic_cast<const kdlib::TypedVarBase*>( fieldVar.get() )->printValue();
    }

    if ( fieldType->isPointer() )
    {
        return dynamic_cast<const kdlib::TypedVarPointer*>( fieldVar.get() )->printValue();
    }

    if (fieldType->isBitField())
    {
        return dynamic_cast<const kdlib::TypedVarBitField*>(fieldVar.get())->printValue();
    }

    if (fieldType->isEnum())
    {
        return dynamic_cast<const kdlib::TypedVarEnum*>(fieldVar.get())->printValue();
    }

    return L"";
}


std::wstring getSymbolName( kdlib::SymbolPtr& symbol )
{
    try {
        return symbol->getName();
    }
    catch( kdlib::SymbolException& )
    {}

    return L"";

}

///////////////////////////////////////////////////////////////////////////////

} // end noname namespace

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr getTypedVar( const TypeInfoPtr& typeInfo, DataAccessorPtr &dataSource, const std::wstring& name = L"" )
{
    if ( typeInfo->isBase() )
        return TypedVarPtr( new TypedVarBase( typeInfo, dataSource, name ) );

    if ( typeInfo->isUserDefined() )
        return TypedVarPtr( new TypedVarUdt( typeInfo, dataSource, name ) );

    if ( typeInfo->isPointer() )
        return TypedVarPtr( new TypedVarPointer( typeInfo, dataSource, name ) );

    if ( typeInfo->isArray() )
        return TypedVarPtr( new TypedVarArray( typeInfo, dataSource, name ) );

    if ( typeInfo->isBitField() )
        return TypedVarPtr( new TypedVarBitField( typeInfo, dataSource, name ) );

    if ( typeInfo->isEnum() )
        return TypedVarPtr( new TypedVarEnum( typeInfo, dataSource, name ) );

    if ( typeInfo->isFunction() )
        return TypedVarPtr( new TypedVarFunction( typeInfo, dataSource, name ) );

    if (typeInfo->isVtbl() )
        return TypedVarPtr( new TypedVarVtbl( typeInfo, dataSource, name ) );

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr loadTypedVar( SymbolPtr &symbol )
{
    if ( SymTagFunction == symbol->getSymTag() )
    {
        return TypedVarPtr( new SymbolFunction( symbol ) );
    }

    if ( LocIsConstant != symbol->getLocType() )
    {
        MEMOFFSET_64 offset = symbol->getVa();

        TypeInfoPtr varType = loadType( symbol );

        return getTypedVar( varType, getMemoryAccessor(offset,varType->getSize()), ::getSymbolName(symbol) );
    }

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr loadTypedVar( const std::wstring &varName )
{
    std::wstring     moduleName;
    std::wstring     symName;

    splitSymName( varName, moduleName, symName );

    ModulePtr  module;

    if ( moduleName.empty() )
    {
        StackFramePtr  frame = getCurrentStackFrame();
        if (frame->findLocalVar(varName))
            return frame->getLocalVar(varName);

        if (frame->findParam(varName))
            return frame->getTypedParam(varName);

        if (frame->findStaticVar(varName))
            return frame->getStaticVar(varName);

        MEMOFFSET_64 moduleOffset = findModuleBySymbol( symName );
        module = loadModule( moduleOffset );
    }
    else
    {
        module = loadModule( moduleName );
    }

    SymbolPtr  symVar = module->getSymbolScope()->getChildByName( symName );

    return loadTypedVar( symVar );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr loadTypedVar( const std::wstring &typeName, MEMOFFSET_64 offset )
{
    TypeInfoPtr varType = loadType( typeName );

    return getTypedVar( varType, getMemoryAccessor(offset,varType->getSize()) );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr loadTypedVar( const TypeInfoPtr &varType, MEMOFFSET_64 offset )
{
    if ( !varType )
        throw DbgException( "type info is null");

    return getTypedVar( varType, getMemoryAccessor(offset,varType->getSize()) );
}


///////////////////////////////////////////////////////////////////////////////

TypedVarPtr loadTypedVar( const std::wstring &typeName, DataAccessorPtr& dataSource)
{
    TypeInfoPtr varType = loadType( typeName );

    return getTypedVar( varType, dataSource );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr loadTypedVar(const TypeInfoPtr &varType, DataAccessorPtr& dataSource)
{
    if (!varType)
        throw DbgException("type info is null");

    return getTypedVar(varType, dataSource);
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr containingRecord( MEMOFFSET_64 offset, const std::wstring &typeName, const std::wstring &fieldName )
{
    std::wstring     moduleName;
    std::wstring     symName;

    splitSymName( typeName, moduleName, symName );

     ModulePtr  module;

    if ( moduleName.empty() )
    {
        MEMOFFSET_64 moduleOffset = findModuleBySymbol( symName );
        module = loadModule( moduleOffset );
    }
    else
    {
        module = loadModule( moduleName );
    }

    TypeInfoPtr typeInfo = module->getTypeByName( symName );

    return containingRecord( offset, typeInfo, fieldName );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr containingRecord( MEMOFFSET_64 offset, TypeInfoPtr &typeInfo, const std::wstring &fieldName )
{
    if ( !typeInfo )
        throw DbgException( "type info is null");

    offset = addr64( offset );

    return loadTypedVar( typeInfo, offset - typeInfo->getElementOffset( fieldName ) );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarList loadTypedVarList( MEMOFFSET_64 offset, const std::wstring &typeName, const std::wstring &fieldName )
{
    std::wstring     moduleName;
    std::wstring     symName;

    splitSymName( typeName, moduleName, symName );

     ModulePtr  module;

    if ( moduleName.empty() )
    {
        MEMOFFSET_64 moduleOffset = findModuleBySymbol( symName );
        module = loadModule( moduleOffset );
    }
    else
    {
        module = loadModule( moduleName );
    }

    TypeInfoPtr typeInfo = module->getTypeByName( symName );

    return loadTypedVarList( offset, typeInfo, fieldName );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarList loadTypedVarList( MEMOFFSET_64 offset, TypeInfoPtr &typeInfo, const std::wstring &fieldName )
{
    if ( !typeInfo )
        throw DbgException( "type info is null" );

    offset = addr64(offset);

    TypedVarList  lst;
    lst.reserve(100);

    MEMOFFSET_64  entryAddress = 0;
    TypeInfoPtr  fieldTypeInfo = typeInfo->getElement( fieldName );
    size_t  psize = fieldTypeInfo->getPtrSize();

    if ( fieldTypeInfo->getName() == ( typeInfo->getName() + L"*" ) )
    {
        for( entryAddress = ptrPtr(offset, psize); addr64(entryAddress) != offset && entryAddress != NULL; entryAddress = ptrPtr( entryAddress + typeInfo->getElementOffset(fieldName), psize ) )
            lst.push_back( loadTypedVar( typeInfo, entryAddress ) );
    }
    else
    {
        for( entryAddress = ptrPtr( offset, psize ); addr64(entryAddress) != offset && entryAddress != NULL; entryAddress = ptrPtr( entryAddress, psize ) )
            lst.push_back( containingRecord( entryAddress, typeInfo, fieldName ) );
    }

    return lst;
}

///////////////////////////////////////////////////////////////////////////////

TypedVarList loadTypedVarArray( MEMOFFSET_64 offset, const std::wstring &typeName, size_t number )
{
    std::wstring     moduleName;
    std::wstring     symName;

    splitSymName( typeName, moduleName, symName );

     ModulePtr  module;

    if ( moduleName.empty() )
    {
        MEMOFFSET_64 moduleOffset = findModuleBySymbol( symName );
        module = loadModule( moduleOffset );
    }
    else
    {
        module = loadModule( moduleName );
    }

    TypeInfoPtr typeInfo = module->getTypeByName( symName );

    return loadTypedVarArray( offset, typeInfo, number );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarList loadTypedVarArray( MEMOFFSET_64 offset, TypeInfoPtr &typeInfo, size_t number )
{
   if ( !typeInfo )
        throw DbgException( "type info is null" );

    offset = addr64(offset); 

    TypedVarList  lst;
    lst.reserve(100);

    for( size_t i = 0; i < number; ++i )
        lst.push_back( loadTypedVar( typeInfo, offset + i * typeInfo->getSize() ) );
   
    return lst;
}
///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypedVarImp::castTo(const std::wstring& typeName)
{
    return loadTypedVar(typeName, m_varData);
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypedVarImp::castTo(const TypeInfoPtr &typeInfo)
{
    return loadTypedVar(typeInfo, m_varData);
}

///////////////////////////////////////////////////////////////////////////////

NumVariant TypedVarBase::getValue() const
{
    if ( m_typeInfo->getName() == L"Char" )
        return NumVariant( m_varData->readSignByte() );

    if ( m_typeInfo->getName() == L"WChar" )
        return NumVariant( m_varData->readSignByte() );

    if ( m_typeInfo->getName() == L"Int1B" )
        return NumVariant( m_varData->readByte() );

    if ( m_typeInfo->getName() == L"UInt1B" )
        return NumVariant( m_varData->readByte() );

    if ( m_typeInfo->getName() == L"Int2B" )
        return NumVariant( m_varData->readSignWord() );

    if ( m_typeInfo->getName() == L"UInt2B" )
        return NumVariant( m_varData->readWord() );

    if ( m_typeInfo->getName() == L"Int4B" )
        return NumVariant( m_varData->readSignDWord() );

    if ( m_typeInfo->getName() == L"UInt4B" )
        return NumVariant( m_varData->readDWord() );

    if ( m_typeInfo->getName() == L"Int8B" )
        return NumVariant( m_varData->readSignQWord() );

    if ( m_typeInfo->getName() == L"UInt8B" )
        return NumVariant( m_varData->readQWord() );

    if ( m_typeInfo->getName() == L"Float" )
        return NumVariant( m_varData->readFloat() );

    if ( m_typeInfo->getName() == L"Double" )
        return NumVariant( m_varData->readDouble() );

    if ( m_typeInfo->getName() == L"Bool" )
        return NumVariant( m_varData->readByte() );

    if ( m_typeInfo->getName() == L"Hresult" )
        return NumVariant( m_varData->readDWord() );

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypedVarBase::str()
{
    std::wstringstream  sstr;

    sstr << m_typeInfo->getName() << L" at 0x" << std::hex << m_varData->getAddress();
    sstr << " Value: ";
    try
    {
        sstr << L"0x" << getValue().asHex() <<  L" (" << getValue().asStr() <<  L")";
    }
    catch (const MemoryException &)
    {
        sstr << L"????";
    }

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypedVarBase::printValue() const
{
    std::wstringstream  sstr;

    try {
        sstr << L"0x" << getValue().asHex() <<  L" (" << getValue().asStr() <<  L")";
        return sstr.str();
    } catch(MemoryException& )
    {}

    return L"Invalid memory";
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypedVarUdt::getElement( const std::wstring& fieldName )
{
    TypeInfoPtr fieldType = m_typeInfo->getElement( fieldName );

    if ( m_typeInfo->isStaticMember(fieldName) )
    {
        MEMOFFSET_64  staticOffset = m_typeInfo->getElementVa(fieldName);

        //if ( staticOffset == 0 )
        //   NOT_IMPLEMENTED();

        return  loadTypedVar( fieldType, staticOffset );
    }

    MEMOFFSET_32   fieldOffset = m_typeInfo->getElementOffset(fieldName);

    if ( m_typeInfo->isVirtualMember( fieldName ) )
    {
        fieldOffset += getVirtualBaseDisplacement( fieldName );
    }

    return  loadTypedVar( fieldType, m_varData->getAddress() + fieldOffset );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypedVarUdt::getElement( size_t index )
{
    TypeInfoPtr fieldType = m_typeInfo->getElement( index );

    if ( m_typeInfo->isStaticMember(index) )
    {
        MEMOFFSET_64  staticOffset = m_typeInfo->getElementVa(index);

        //if ( staticOffset == 0 )
        //   NOT_IMPLEMENTED();

        return  loadTypedVar( fieldType, staticOffset );
    }

    MEMOFFSET_32   fieldOffset = m_typeInfo->getElementOffset(index);

    if ( m_typeInfo->isVirtualMember( index ) )
    {
        fieldOffset += getVirtualBaseDisplacement( index );
    }

    return  loadTypedVar( fieldType, m_varData->getAddress() + fieldOffset );
}

///////////////////////////////////////////////////////////////////////////////

MEMDISPLACEMENT TypedVarUdt::getVirtualBaseDisplacement( const std::wstring &fieldName )
{
    MEMOFFSET_32 virtualBasePtr = 0;
    size_t  virtualDispIndex = 0;
    size_t  virtualDispSize = 0;
    m_typeInfo->getVirtualDisplacement( fieldName, virtualBasePtr, virtualDispIndex, virtualDispSize );

    MEMOFFSET_64 vfnptr = m_varData->getAddress() + virtualBasePtr;
    MEMOFFSET_64 vtbl = m_typeInfo->getPtrSize() == 4 ? ptrDWord( vfnptr ) : ptrQWord(vfnptr);

    MEMDISPLACEMENT     displacement =  ptrSignDWord( vtbl + virtualDispIndex*virtualDispSize );

    return virtualBasePtr + displacement;
}

///////////////////////////////////////////////////////////////////////////////

MEMDISPLACEMENT TypedVarUdt::getVirtualBaseDisplacement( size_t index )
{
    MEMOFFSET_32 virtualBasePtr = 0;
    size_t  virtualDispIndex = 0;
    size_t  virtualDispSize = 0;
    m_typeInfo->getVirtualDisplacement( index, virtualBasePtr, virtualDispIndex, virtualDispSize );

    MEMOFFSET_64 vfnptr = m_varData->getAddress() + virtualBasePtr;

    if ( !vfnptr )
        return 0; // ������ ����� ���� ��� �� �����������������  ��� ��������� � �������� ��������

    MEMOFFSET_64 vtbl = m_typeInfo->getPtrSize() == 4 ? ptrDWord( vfnptr ) : ptrQWord(vfnptr);

    MEMDISPLACEMENT     displacement =  ptrSignDWord( vtbl + virtualDispIndex*virtualDispSize );
    return virtualBasePtr + displacement;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypedVarUdt::str()
{
    std::wstringstream  sstr;

    sstr << L"struct/class: " << m_typeInfo->getName() << " at 0x" << std::hex << m_varData->getAddress() << std::endl;
    
    for ( size_t i = 0; i < m_typeInfo->getElementCount(); ++i )
    {
        TypeInfoPtr     fieldType = m_typeInfo->getElement(i);
        TypedVarPtr     fieldVar;

        if ( m_typeInfo->isStaticMember(i) )
        {
            MEMOFFSET_64  staticOffset = m_typeInfo->getElementVa(i);
            if ( staticOffset != 0 )
                fieldVar = loadTypedVar( fieldType, staticOffset );

            sstr << L"   =" << std::right << std::setw(10) << std::setfill(L'0') << std::hex << staticOffset;
            sstr << L" " << std::left << std::setw(18) << std::setfill(L' ') << m_typeInfo->getElementName(i) << ':';
        }
        else
        {
            MEMOFFSET_32   fieldOffset = m_typeInfo->getElementOffset(i);

            if ( m_typeInfo->isVirtualMember(i) )
            {
                fieldOffset += getVirtualBaseDisplacement(i);
            }

            fieldVar = loadTypedVar( fieldType, getAddress() + fieldOffset );
            sstr << L"   +" << std::right << std::setw(4) << std::setfill(L'0') << std::hex << fieldOffset;
            sstr << L" " << std::left << std::setw(24) << std::setfill(L' ') << m_typeInfo->getElementName(i) << ':';
        }

        sstr << " " << std::left << fieldType->getName();

        if ( fieldVar )
            sstr << L"   " << ::printFieldValue( fieldType, fieldVar );
        else
            sstr << L"   " << L"failed to get value";



        //try {

        //    if ( fieldType->isBase() )
        //    {
        //        sstr << L"   ";

        //        if( fieldVar )
        //            sstr << L"0x" << fieldVar->getValue().asHex() <<  L" (" << fieldVar->getValue().asStr() <<  L")";
        //        else
        //            sstr << L"failed to get value";
        //    }
        //    else if ( fieldType->isPointer() )
        //    {
        //        sstr << L"   ";

        //        if( fieldVar )
        //            sstr << L"0x" << fieldVar->getValue().asHex();
        //        else
        //            sstr << L"failed to get value";
        //    }

        //} catch( MemoryException& )
        //{
        //    sstr << L"Invalid memory";
        //}

        sstr << std::endl;
    }

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypedVarPointer::deref()
{
    return loadTypedVar( 
                m_typeInfo->deref(), 
                m_typeInfo->getPtrSize() == 4 ? m_varData->readDWord() : m_varData->readQWord());
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypedVarPointer::str()
{
    std::wstringstream   sstr;

    sstr << L"Ptr " << m_typeInfo->getName() << L" at 0x" << std::hex << m_varData->getAddress();
    sstr << L" Value: " <<  printValue();

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypedVarPointer::printValue() const
{
    std::wstringstream   sstr;

    try {

         sstr << L"0x" << getValue().asHex() <<  L" (" << getValue().asStr() <<  L")";
         return sstr.str();

    }
    catch(MemoryException&)
    {}

    return L"Invalid memory";
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypedVarPointer::getElement(size_t index)
{
    TypeInfoPtr     elementType = m_typeInfo->deref();

    return loadTypedVar(
        elementType,
        m_typeInfo->getPtrSize() == 4 ? m_varData->readDWord() : m_varData->readQWord() +
        elementType->getSize()*index);
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypedVarArray::getElement( size_t index )
{
    //if ( index >= m_typeInfo->getElementCount() )
    //    throw IndexException( index );

    TypeInfoPtr     elementType = m_typeInfo->getElement(0);

    return loadTypedVar( elementType, m_varData->getAddress() + elementType->getSize()*index );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypedVarArray::str()
{
    std::wstringstream   sstr;

    sstr << m_typeInfo->getName() << L" at 0x" << std::hex << m_varData->getAddress();

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

NumVariant TypedVarBitField::getValue() const
{
    NumVariant  var = *loadTypedVar( m_typeInfo->getBitType(), m_varData->getAddress() );

    var >>=  m_typeInfo->getBitOffset();

    if ( var.isSigned() )
    {
        BITOFFSET width = m_typeInfo->getBitWidth();

        if ( ( var & ( 1ULL << ( width -1 ) ) ) != 0 )
        {
            var |= ~( ( 1ULL << width ) - 1 );
        }
        else
        {
            var &= ( 1ULL << width ) - 1;
        }

        var = var.asLongLong();
    }
    else
    {     

        var &= ( 1 << m_typeInfo->getBitWidth() ) - 1;
    }

    return var;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypedVarBitField::printValue() const
{
    std::wstringstream  sstr;

    try {
        sstr << L"0x" << getValue().asHex() << L" (" << getValue().asStr() << L")";
        return sstr.str();
    }
    catch (MemoryException&)
    {
    }

    return L"Invalid memory";
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypedVarEnum::str()
{
    std::wstringstream       sstr;

    sstr << L"enum: " << m_typeInfo->getName() << L" at 0x" << std::hex << m_varData->getAddress();
    sstr << L" Value: " << printValue();

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypedVarEnum::printValue() const
{
    std::wstringstream   sstr;

    try {

        unsigned long   ulongVal1 = getValue().asULong();

        for ( size_t i = 0; i < m_typeInfo->getElementCount(); ++i )
        {
            unsigned long   ulongVal2 = m_typeInfo->getElement(i)->getValue().asULong();
            if ( ulongVal1 == ulongVal2 )
            {
                sstr << m_typeInfo->getElementName(i);
                sstr << L" (" << getValue().asHex() << L")";

                return sstr.str();
            }
        }

        sstr << getValue().asHex();
        sstr << L" ( No matching name )";
    }
    catch( MemoryException& )
    {
        sstr << L"????";
    }

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypedVarFunction::str()
{
    return m_typeInfo->getName();
}

///////////////////////////////////////////////////////////////////////////////

SymbolFunction::SymbolFunction( SymbolPtr& symbol ) :
    TypedVarFunction( loadType( symbol ), getMemoryAccessor( symbol->getVa(), 0 ), ::getSymbolName(symbol) ),
    m_symbol( symbol )
{
}

///////////////////////////////////////////////////////////////////////////////

SymbolPtr SymbolFunction::getChildSymbol(size_t index)
{
    SymbolPtrList  paramLst = m_symbol->findChildren(SymTagData);
    if (paramLst.size() < index)
        throw IndexException(index);

    SymbolPtrList::iterator itVar = paramLst.begin();
    for (size_t i = 0; itVar != paramLst.end(); ++itVar)
    {
        unsigned long  dataKind = (*itVar)->getDataKind();

        if (dataKind == DataIsParam || dataKind == DataIsObjectPtr)
        {
            if (i == index)
            {
                return *itVar;
            }

            i++;
        }
    }

    throw IndexException(index);
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_REL SymbolFunction::getElementOffset( size_t index )
{
    SymbolPtr  paramSym = getChildSymbol(index);

    return paramSym->getOffset();
}

///////////////////////////////////////////////////////////////////////////////

RELREG_ID SymbolFunction::getElementOffsetRelativeReg(size_t index )
{
    SymbolPtr  paramSym = getChildSymbol(index);

    return paramSym->getRegRealativeId();
}

///////////////////////////////////////////////////////////////////////////////

VarStorage SymbolFunction::getElementStorage(const std::wstring& fieldName)
{
    SymbolPtr  paramSym = m_symbol->getChildByName(fieldName);
    if (paramSym->getLocType() == LocIsEnregistered)
        return RegisterVar;
    return MemoryVar;
}

///////////////////////////////////////////////////////////////////////////////

VarStorage SymbolFunction::getElementStorage(size_t index)
{
    SymbolPtr  paramSym = getChildSymbol(index);
    if (paramSym->getLocType() == LocIsEnregistered)
        return RegisterVar;
    return MemoryVar;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_REL SymbolFunction::getElementOffset( const std::wstring& paramName )
{
    SymbolPtr  paramSym = m_symbol->getChildByName(paramName);

    return paramSym->getOffset();
}
 
///////////////////////////////////////////////////////////////////////////////

RELREG_ID SymbolFunction::getElementOffsetRelativeReg(const std::wstring& paramName )
{
    SymbolPtr  paramSym = m_symbol->getChildByName(paramName);

    return paramSym->getRegRealativeId();
}

///////////////////////////////////////////////////////////////////////////////

size_t SymbolFunction::getElementIndex(const std::wstring& paramName )
{
    SymbolPtrList  paramLst = m_symbol->findChildren( SymTagData );

    SymbolPtrList::iterator itVar = paramLst.begin();

    for ( size_t i = 0; itVar != paramLst.end(); ++itVar )
    {
        unsigned long  dataKind = (*itVar)->getDataKind();

        if (dataKind == DataIsParam || dataKind == DataIsObjectPtr)
        {
            if (  (*itVar)->getName() == paramName )
                return i;
            i++;
        }
    }

    throw DbgException("parameter is not found");
}

///////////////////////////////////////////////////////////////////////////////

std::wstring SymbolFunction::getElementName( size_t index )
{
    SymbolPtr  paramSym = getChildSymbol(index);
    return  paramSym->getName();
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 SymbolFunction::getDebugXImpl(SymTags symTag) const
{
    SymbolPtrList resLst = m_symbol->findChildren(symTag);
    switch (resLst.size())
    {
    case 0:
        throw DbgException("child symbol is not found");
    case 1:
        return (*resLst.begin())->getVa();
    }
    throw DbgException("unexpected count of child symbols");
}

///////////////////////////////////////////////////////////////////////////////

unsigned long SymbolFunction::getElementReg(const std::wstring& fieldName)
{
    SymbolPtr  paramSym = m_symbol->getChildByName(fieldName);

    return paramSym->getRegRealativeId();
}

///////////////////////////////////////////////////////////////////////////////

unsigned long SymbolFunction::getElementReg(size_t index)
{
    SymbolPtr  paramSym = getChildSymbol(index);
    return paramSym->getRegisterId();
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypedVarVtbl::getElement(size_t index)
{
    if (index >= m_typeInfo->getElementCount())
        throw IndexException(index);

    TypeInfoPtr     elementType = loadType(L"Void*");

    return loadTypedVar(elementType, m_varData->getAddress() + elementType->getSize()*index);
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypedVarVtbl::str()
{
    std::wstringstream   sstr;

    sstr << m_typeInfo->getName() << L" at 0x" << std::hex << m_varData->getAddress() << std::endl;

    TypeInfoPtr  elementType = loadType(L"Void*");
    size_t  elementSize = elementType->getSize();
    MEMOFFSET_64  beginOffset = m_varData->getAddress();

    for (size_t i = 0; i < m_typeInfo->getElementCount(); ++i)
    {
        MEMOFFSET_64  val = ptrPtr(beginOffset + elementSize*i, elementSize);
        sstr << "   [" << std::dec << i << "] 0x" << std::hex << val << " (" << findSymbol(val) << ')' << std::endl;
    }

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

} // end kdlib namesapce
