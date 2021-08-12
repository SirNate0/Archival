#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/JSONValue.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Core/Variant.h>

namespace Urho3D
{
/// Store the values in the container in a VariantVector
template<class container>
VariantVector ConvertToVariantVector(const container& v)
{
    VariantVector ret;
    for (auto& s : v)
        ret.EmplaceBack(s);
    return ret;
}

/// Store the values in the string T*[] terminated with a sentinel nullptr (not included in the output)
template<class T>
VariantVector ConvertToVariantVector(T** v)
{
    VariantVector ret;
    for (T** p = v; *p; ++p)
        ret.EmplaceBack(*p);
    return ret;
}


//--------------------------
// Value To String Template
//--------------------------
/// Create a string from a type.
/// The base form must not be a reference so we can overload it with an int and not a const int& later.
template<class T>
String ToString(T val)
{
    return val.ToString();
}

/// Create a string from a type.
template<class T>
String ToString(const T& val)
{
    return val.ToString();
}

/// Create a string from a type.
template<> inline String ToString(const char* val) { return String(val); }
template<> inline String ToString(const String& val) { return String(val); }
template<> inline String ToString(bool val) { return String(val); }
template<> inline String ToString(float val) { return String(val); }
template<> inline String ToString(double val) { return String(val); }
template<> inline String ToString(int val) { return String(val); }
template<> inline String ToString(unsigned val) { return String(val); }


//-------------------------
// Get JSON Value Template
//-------------------------
template<class T>
inline bool GetJSON(JSONValue& holder, T& val);

template<>
inline bool GetJSON(JSONValue& holder, bool& val)
{
    if (holder.IsBool())
    {
        val = holder.GetBool();
        return true;
    }
    return false;
}
template<>
inline bool GetJSON(JSONValue& holder, int& val)
{
    if (holder.IsNumber())
    {
        val = holder.GetInt();
        return true;
    }
    return false;
}
template<>
inline bool GetJSON(JSONValue& holder, unsigned& val)
{
    if (holder.IsNumber())
    {
        val = holder.GetUInt();
        return true;
    }
    return false;
}
template<>
inline bool GetJSON(JSONValue& holder, float& val)
{
    if (holder.IsNumber())
    {
        val = holder.GetFloat();
        return true;
    }
    else if (holder.IsNull())
    {
        val = NAN;
        return true;
    }
    return false;
}
template<>
inline bool GetJSON(JSONValue& holder, double& val)
{
    if (holder.IsNumber())
    {
        val = holder.GetDouble();
        return true;
    }
    else if (holder.IsNull())
    {
        val = NAN;
        return true;
    }
    return false;
}
template<>
inline bool GetJSON(JSONValue& holder, String& val)
{
    if (holder.IsString())
    {
        val = holder.GetString();
        return true;
    }
    return false;
}
template<>
inline bool GetJSON(JSONValue& holder, JSONArray& val)
{
    if (holder.IsString())
    {
        val = holder.GetArray();
        return true;
    }
    return false;
}
template<>
inline bool GetJSON(JSONValue& holder, JSONObject& val)
{
    if (holder.IsString())
    {
        val = holder.GetObject();
        return true;
    }
    return false;
}


}
