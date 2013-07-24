#pragma once

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "kdlib/typeinfo.h"
#include "kdlib/variant.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class TypedVar;
typedef boost::shared_ptr<TypedVar>  TypedVarPtr;
typedef std::vector<TypedVarPtr> TypedVarList;

TypedVarPtr loadTypedVar( const std::wstring &varName );

TypedVarPtr loadTypedVar( const std::wstring &typeName, MEMOFFSET_64 addr, const SymbolPtr &symVar );

TypedVarPtr loadTypedVar( const TypeInfoPtr &typeInfo, MEMOFFSET_64 addr, const SymbolPtr &symVar );

TypedVarPtr containingRecord( MEMOFFSET_64 addr, const std::wstring &typeName, const std::wstring &fieldName );

TypedVarPtr containingRecord( MEMOFFSET_64 addr, TypeInfoPtr &typeInfo, const std::wstring &fieldName );

TypedVarList loadTypedVarList( MEMOFFSET_64 addr, const std::wstring &typeName, const std::wstring &fieldName );

TypedVarList loadTypedVarList( MEMOFFSET_64 addr, TypeInfoPtr &typeInfo, const std::wstring &fieldName );

TypedVarList loadTypedVarArray( MEMOFFSET_64 addr, const std::wstring &typeName, size_t count );

TypedVarList loadTypedVarArray( MEMOFFSET_64 addr, TypeInfoPtr &typeInfo, size_t count );


class TypedVar : private boost::noncopyable, public NumBehavior {

    friend TypedVarPtr loadTypedVar( const std::wstring &varName );

    friend TypedVarPtr loadTypedVar( const std::wstring &typeName, MEMOFFSET_64 addr, const SymbolPtr &symVa );

    friend TypedVarPtr loadTypedVar( const TypeInfoPtr &typeInfo, MEMOFFSET_64 addr, const SymbolPtr &symVa );

public:
    
    virtual MEMOFFSET_64 getAddress() const = 0;
    virtual size_t getSize() const = 0;
    virtual TypedVarPtr getElement( const std::wstring& fieldName ) = 0;
    virtual TypedVarPtr getElement( size_t index ) = 0;
    virtual MEMOFFSET_32 getElementOffset(const std::wstring& fieldName ) = 0;
    virtual MEMOFFSET_32 getElementOffset(size_t index ) = 0;
    virtual size_t getElementCount() = 0;
    virtual std::wstring getElementName( size_t index ) = 0;
    virtual TypeInfoPtr getType() const = 0;
    virtual NumVariant getValue() const = 0;
    virtual TypedVarPtr deref() = 0;

protected:


    TypedVar() 
    {}

    virtual ~TypedVar() 
    {}
};

///////////////////////////////////////////////////////////////////////////////

} // end kdlib namespace
