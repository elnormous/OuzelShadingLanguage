//
//  OSL
//

#pragma once

class TypeDeclaration;

class QualifiedType
{
public:
    bool operator<(const QualifiedType& other) const
    {
        if (typeDeclaration != other.typeDeclaration)
            return typeDeclaration < other.typeDeclaration;
        else if (isConst != other.isConst)
            return isConst < other.isConst;
        else if (isVolatile != other.isVolatile)
            return isVolatile < other.isVolatile;
        else return true;
    }

    TypeDeclaration* typeDeclaration = nullptr;
    bool isConst = false;
    bool isVolatile = false;
};
