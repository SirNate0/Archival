#pragma once

#include "Archive.h"
#include "ArchiveDetail.h"


namespace Archival {

class Archive;

namespace Detail {

using namespace Urho3D;

//template<class T>
//inline bool SetJSON(Urho3D::JSONValue& holder, const T& val);

// template<>
inline bool SetJSON(Urho3D::JSONValue& holder, const bool& val)
{
    holder = val;
    return true;
}
// template<>
inline bool SetJSON(Urho3D::JSONValue& holder, const int& val)
{

    holder = val;
    return true;
}
// template<>
inline bool SetJSON(Urho3D::JSONValue& holder, const unsigned& val)
{

    holder = val;
    return true;
}
// template<>
inline bool SetJSON(Urho3D::JSONValue& holder, const float& val)
{
    if (!IsNaN(val) && !IsInf(val))
        holder = val;
    else
        holder.SetType(JSONValueType::JSON_NULL);
    return true;
}
// template<>
inline bool SetJSON(Urho3D::JSONValue& holder, const double& val)
{
    if (!IsNaN(val) && !IsInf(val))
        holder = val;
    else
        holder.SetType(JSONValueType::JSON_NULL);
    return true;
}
// template<>
inline bool SetJSON(Urho3D::JSONValue& holder, const Urho3D::String& val)
{

    holder = val;
    return true;
}
// template<>
inline bool SetJSON(Urho3D::JSONValue& holder, const Urho3D::JSONArray& val)
{

    holder = val;
    return true;
}
// template<>
inline bool SetJSON(Urho3D::JSONValue& holder, const Urho3D::JSONObject& val)
{

    holder = val;
    return true;
}


/// Archival Backend that serializes to/from JSON.
class JSONBackend: public Backend
{

public:

    /// Construct to serialize to the provide JSONValue. The value must have a lifetime as long as the backend. Set it to be an input or output archive based on the flag isInput.
    JSONBackend(Urho3D::JSONValue& val, bool isInput): object_(val)
    {
        if (!isInput && object_.IsNull())
            assert(false);//object_.SetType(Urho3D::JSON_OBJECT);
    }

    /// Destruct.
    ~JSONBackend()override=default;

    /// Returns the name of the backend
    const String& GetBackendName() override { static const String name("JSON"); return name; }

    /// Utility method to create an Archive with a JSONBackend from the provided value.
    static Archive MakeArchive(bool isInput, Urho3D::JSONValue& val);

    Backend* CreateGroup(const String &name, bool isInput) override;
    Backend* CreateSeriesEntry(const String &name, bool isInput) override;
    bool GetSeriesSize(const String &name, unsigned &size) override
    {
        auto& obj = GetSeriesObject(true);

            if (name == InlineName() && obj.IsArray())
                size = obj.Size();
            else if (obj.Contains(name))
                size = obj[name].Size();
            else
                return false;

        return true;
    }
    bool SetSeriesSize(const String &name, const unsigned &size) override;
    bool GetEntryNames(StringVector &names) override;
    bool SetEntryNames(const StringVector &names) override;
    unsigned char InlineSeriesVerbosity() const override { return 10; }

    bool Get(const String &name, const std::nullptr_t &) override
    {
        auto& obj = GetSeriesObject(true);
        if (obj.IsObject() && obj.Contains(name))
        {
            auto holder = obj[name];
            // flatten inline value tables.
            while (holder.IsObject() && holder.Contains(InlineName()))
                holder = obj[InlineName()];
            return holder.IsNull();
        }
        else if (name == InlineName())
            return obj.IsNull();
        return false;
    }
    bool Set(const String &name, const std::nullptr_t &) override
    {
        auto& obj = (object_.IsArray() ? object_[seriesEntry_] : object_);
        if (name == InlineName())
        {
            // need the null check anyways to avoid the else branch
            if (obj.IsNull() || (obj.IsObject() && obj.Size() == 0))
                obj.SetType(JSONValueType::JSON_NULL);
            else
                obj[name].SetType(JSONValueType::JSON_NULL);
        }
        else
        {
            // Convert back to an object if necessary
            if (!obj.IsObject())
            {
                Urho3D::JSONValue oldVal = obj;
                obj.Set(InlineName(), oldVal);
            }

            obj[name].SetType(JSONValueType::JSON_NULL);
        }
        return true;
    }
    bool Get(const String &name, bool &val) override { return GetInternal(name, val); }
    bool Get(const String &name, unsigned char &val) override
    {
        unsigned canGet;
        if (!GetInternal(name, canGet)) return false;
        val = static_cast<unsigned char>(canGet);
        return true;
    }
    bool Get(const String &name, signed char &val) override
    {
        unsigned canGet;
        if (!GetInternal(name, canGet)) return false;
        val = static_cast<signed char>(canGet);
        return true;
    }
    bool Get(const String &name, unsigned short &val) override
    {
        unsigned canGet;
        if (!GetInternal(name, canGet)) return false;
        val = static_cast<unsigned short>(canGet);
        return true;
    }
    bool Get(const String &name, signed short &val) override
    {
        unsigned canGet;
        if (!GetInternal(name, canGet)) return false;
        val = static_cast<signed short>(canGet);
        return true;
    }
    bool Get(const String &name, unsigned int &val) override { return GetInternal(name, val); }
    bool Get(const String &name, signed int &val) override { return GetInternal(name, val); }
    bool Get(const String &name, unsigned long long &val) override
    {
        unsigned canGet;
        if (!GetInternal(name, canGet)) return false;
        val = canGet;
        URHO3D_LOGWARNING("Extending unsigned to unsigned long long in JSON Archival.");
        return true;
    }
    bool Get(const String &name, signed long long &val) override
    {
        unsigned canGet;
        if (!GetInternal(name, canGet)) return false;
        val = canGet;
        URHO3D_LOGWARNING("Extending int to long long in JSON Archival.");
        return true;
    }
    bool Get(const String &name, float &val) override { return GetInternal(name, val); }
    bool Get(const String &name, double &val) override { return GetInternal(name, val); }
    bool Get(const String &name, String &val) override { return GetInternal(name, val); }

    bool Set(const String &name, const bool &val) override { return SetInternal(name, val); }
    bool Set(const String &name, const unsigned char &val) override { return SetInternal(name, val); }
    bool Set(const String &name, const signed char &val) override { return SetInternal(name, val); }
    bool Set(const String &name, const unsigned short &val) override { return SetInternal(name, val); }
    bool Set(const String &name, const signed short &val) override { return SetInternal(name, val); }
    bool Set(const String &name, const unsigned int &val) override { return SetInternal(name, val); }
    bool Set(const String &name, const signed int &val) override { return SetInternal(name, val); }
    bool Set(const String &name, const unsigned long long &val) override
    {
        URHO3D_LOGWARNING("Truncating unsigned long long to unsigned in JSON Archival.");
        return SetInternal(name, static_cast<unsigned>(val));
    }
    bool Set(const String &name, const signed long long &val) override
    {
        URHO3D_LOGWARNING("Truncating long long to int in JSON Archival.");
        return SetInternal(name, static_cast<int>(val));
    }
    bool Set(const String &name, const float &val) override { return SetInternal(name, val); }
    bool Set(const String &name, const double &val) override { return SetInternal(name, val); }
    bool Set(const String &name, const String &val) override { return SetInternal(name, val); }

//    bool HintBounds(Urho3D::Variant min, Urho3D::Variant max) override {}
//    bool ClearHints() override {}


    Urho3D::JSONValue& GetSeriesObject(bool isInput)
    {
        if (seriesEntry_ != INVALID_SERIES_ENTRY)
        {
//            assert(object_.IsArray());
            if (!isInput || (object_.IsArray() && seriesEntry_ < object_.Size()))
                return object_[seriesEntry_];
            else
                return empty;
        }
        return object_;
    }

private:

    template<class T>
    bool GetInternal(const String& name, T& val)
    {
        auto& obj = GetSeriesObject(true);
        if (obj.IsObject() && obj.Contains(name))
        {
            auto holder = obj[name];
            // flatten inline value tables.
            while (holder.IsObject() && holder.Contains(InlineName()))
                holder = obj[InlineName()];
            return Urho3D::GetJSON<T>(holder, val);
        }
        else if (name == InlineName())
            return Urho3D::GetJSON<T>(obj, val);
        return false;
    }

    template<class T>
    inline bool SetInternal(const String& name, const T& val)
    {
        auto& obj = (object_.IsArray() ? object_[seriesEntry_] : object_);
        if (name == InlineName())
        {
            if (obj.IsNull() || (obj.IsObject() && obj.Size() == 0))
                Detail::SetJSON(obj, val); // obj = val;
            else
                Detail::SetJSON(obj[name], val); // obj[name] = val;
        }
        else
        {
            // Convert back to an object if necessary
            if (!obj.IsObject())
            {
                Urho3D::JSONValue oldVal = obj;
                obj.Set(InlineName(), oldVal);
            }

            Detail::SetJSON(obj[name], val); // obj[name] = val;
        }
        return true;
    }

    JSONValue& object_;
    Urho3D::HashMap<String, unsigned> entries_;
    static constexpr unsigned INVALID_SERIES_ENTRY{0xFFFFFFFF};
    unsigned seriesEntry_{INVALID_SERIES_ENTRY};
    static Urho3D::JSONValue empty;

    /// Get/Create and return a reference to a JSONValue Array with the specified size. Must be an output operation.
    JSONValue &MakeSeriesEntryInternal(const String &name, unsigned size);
};


} // namespace Detail

} // namespace Archival
