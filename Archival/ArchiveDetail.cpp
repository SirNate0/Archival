#include <Urho3D/Urho3D.h>

#warning FIXING /home/nathan/Projects/Urho3D/Build/include/Urho3D/Resource/JSONValue.h:257: er ror: ‘Context’ has not been declared void SetVariant(const Variant& variant, Context* context = nullptr);
#include <Urho3D/Core/Context.h>

#include "ArchiveDetail.h"

#include "Archive.h"

inline namespace Archival
{

namespace Detail {

const Hint Hint::EMPTY_HINT{OUTPUT_NONE, {}, {}};


Archive JSONBackend::MakeArchive(bool isInput, JSONValue &val)
{
    return Archive(isInput, new JSONBackend(val, isInput));
}

Backend *JSONBackend::CreateGroup(const String &name, bool isInput)
{
    auto& obj = GetSeriesObject(isInput);
    if (isInput)
    {
        if (name == InlineName()
                && !(obj.Contains(name) && obj[name].IsObject()))
            return new JSONBackend(obj, isInput);
        else if (!obj.Contains(name))
            return nullptr;
        else
            return new JSONBackend(obj[name], isInput);
    }
    else
    {
        if (name == InlineName())
            return new JSONBackend(obj = Urho3D::JSONObject(), isInput);
        else
            return new JSONBackend(obj[name] = Urho3D::JSONObject(), isInput);
    }
}


Backend *JSONBackend::CreateSeriesEntry(const String &name, bool isInput)
{
    if (!entries_.Contains(name))
        entries_[name] = 0;
    else
        entries_[name] += 1;

    if (isInput)
    {

        auto& obj = GetSeriesObject(isInput);
        if (name == InlineName() && obj.IsArray())
        {
            auto backend = new JSONBackend(obj, isInput);
            backend->seriesEntry_ = entries_[name];
            return backend;
        }

        if (obj.Contains(name))
        {
            unsigned e = entries_[name];
            auto backend = new JSONBackend(obj[name], isInput);
            backend->seriesEntry_ = e;
            return backend;
        }
        else
            return nullptr;
    }
    else
    {
        //            if (!object_.Contains(name))
        //                object_[name] = Urho3D::JSONArray();

        //            auto idx = entries_[name];

        //            auto& obj = object_[name];
        //            obj.Resize(Urho3D::Max(idx+1,obj.Size()));
        //            return JSONArchive(false, obj[idx] = Urho3D::JSONObject());

        auto idx = entries_[name];
        JSONValue& array = MakeSeriesEntryInternal(name, idx + 1);
        return new JSONBackend(array[idx] = Urho3D::JSONObject(),isInput);
    }
}

bool JSONBackend::SetSeriesSize(const String &name, const unsigned &size)
{
    // Just defer to the magic of MakeSeriesEntryInternal
    MakeSeriesEntryInternal(name, size);
    return true;
}

bool JSONBackend::GetEntryNames(StringVector &names)
{
    auto& obj = GetSeriesObject(true);
    if (obj.IsObject())
    {
        names.Reserve(names.Size() + obj.Size());
        for (auto p : obj.GetObject())
            names.Push(p.first_);
        return true;
    }
    else
    {
        names.Push(InlineName());
        return true;
    }
}

bool JSONBackend::SetEntryNames(const StringVector &names)
{
    return true;
}



Urho3D::JSONValue JSONBackend::empty;

Urho3D::JSONValue& JSONBackend::MakeSeriesEntryInternal(const String& name, unsigned size)
{
    auto& obj = GetSeriesObject(false);
    Urho3D::JSONValue* array = nullptr;
    if (!obj.Contains(name))
    {
        if (name == InlineName())
        {
            if (obj.IsArray())
                array = &obj;
            else if (obj.IsNull() || (obj.IsObject() && obj.Size() == 0))
                array = &(obj = Urho3D::JSONArray());
            else
                array = &(obj[name] = Urho3D::JSONArray());
        }
        else
        {
            // Convert back to an object if necessary
            if (!obj.IsObject())
            {
                Urho3D::JSONValue oldVal = obj;
                obj.Set(InlineName(), oldVal);
            }

            array = &(obj[name] = Urho3D::JSONArray());
        }
    }
    else
    {
        if (!obj[name].IsArray())
        {
            URHO3D_LOGERROR("Overwriting JSON Value with array in Archive::CreateSeriesEntry. Name="+name);
            obj[name] = Urho3D::JSONArray();
            throw -1;
        }
        array = &(obj[name]);
    }

    array->Resize(Urho3D::Max(size,array->Size()));
    return *array;
}

}

}
