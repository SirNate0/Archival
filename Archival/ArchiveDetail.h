#pragma once

#include <Urho3D/Core/Context.h>

#include <Urho3D/Resource/JSONValue.h>
#include <Urho3D/IO/Log.h>

#include "Utils.h"

inline namespace Archival {

class Archive;

namespace Detail {

using namespace Urho3D;

template<typename T>
struct IsBasicType
{
    static constexpr bool value{false};
};
template<> struct IsBasicType<bool> { static constexpr bool value{true}; };
template<> struct IsBasicType<unsigned char> { static constexpr bool value{true}; };
template<> struct IsBasicType<signed char> { static constexpr bool value{true}; };
template<> struct IsBasicType<unsigned short> { static constexpr bool value{true}; };
template<> struct IsBasicType<signed short> { static constexpr bool value{true}; };
template<> struct IsBasicType<unsigned int> { static constexpr bool value{true}; };
template<> struct IsBasicType<signed int> { static constexpr bool value{true}; };
template<> struct IsBasicType<unsigned long long> { static constexpr bool value{true}; };
template<> struct IsBasicType<signed long long> { static constexpr bool value{true}; };

template<> struct IsBasicType<float> { static constexpr bool value{true}; };
template<> struct IsBasicType<double> { static constexpr bool value{true}; };

template<> struct IsBasicType<String> { static constexpr bool value{true}; };
#define EXTENDED_ARCHIVE_TYPES
#ifdef EXTENDED_ARCHIVE_TYPES
template<> struct IsBasicType<Urho3D::IntVector2> { static constexpr bool value{true}; };
template<> struct IsBasicType<Urho3D::IntVector3> { static constexpr bool value{true}; };
//template<> struct IsBasicType<Urho3D::IntVector4> { static constexpr bool value{true}; };

template<> struct IsBasicType<Urho3D::Vector2> { static constexpr bool value{true}; };
template<> struct IsBasicType<Urho3D::Vector3> { static constexpr bool value{true}; };
template<> struct IsBasicType<Urho3D::Vector4> { static constexpr bool value{true}; };
template<> struct IsBasicType<Urho3D::Quaternion> { static constexpr bool value{true}; };
template<> struct IsBasicType<Urho3D::Color> { static constexpr bool value{true}; };

template<> struct IsBasicType<Urho3D::Matrix3> { static constexpr bool value{true}; };
template<> struct IsBasicType<Urho3D::Matrix3x4> { static constexpr bool value{true}; };
template<> struct IsBasicType<Urho3D::Matrix4> { static constexpr bool value{true}; };
#endif


/// A hint to the backend about how to treat a value.
struct Hint
{
    enum HINT
    {
        /// A description of what the value is. Basically, the documentation. Primary: String.
        DESCRIPTION,
        /// The min and max bounds for the value. Min,Max = Primary,Secondary: matches value type or VAR_NONE for unbounded.
        BOUNDS,
        /// The "scale" of a drag input. Primary: matches value type or VAR_NONE for unbounded.
        RESOLUTION_SCALE,
        /// Strictly limit to these values (e.g. for enums). Primary: VAR_VECTOR of value type.
        ALLOWED_OPTIONS,
        /// Softer limit to these values (e.g. for node names). Primary: VAR_VECTOR of value type.
        SUGGESTED_OPTIONS,


        /// A hint that tells the input to remove a pair of legs. Primary: bool.
        OUTPUT_SHOULD_CLOSE,
        /// None present.
        OUTPUT_NONE,
    };

    /// The kind of the hint.
    HINT kind;

    /// The primary value of the hint.
    Variant value;
    /// The secondary value of the hint. Used, e.g., for the upper bound.
    Variant secondary;

    operator bool() const { return kind != OUTPUT_NONE; }

    static const Hint EMPTY_HINT;
};

/// Archival backend that actually implements the saving and loading for a specific set of types.
class Backend
{
    /// Sentinel value to indicate that we are getting/setting an inline value.
    String inlineValueName_{"value"};
public:

    virtual ~Backend()=default;

    /// Returns the current inline value sentinel string.
    const String& InlineName() const { return inlineValueName_; }
    /// Sets the current inline value sentinel string. Restores to default "value" with no arguments.
    void ResetInlineName(const String& name = "value") { inlineValueName_ = name; }

    /// Returns the name of the backend
    virtual const String& GetBackendName()=0;

    ///------------------------
    /// Group/Series Functions

    /// Creates/Finds a group with the specified name and returns a new Backend* that will use it. Returns nullptr if not found.
    /// - May overwrite previous groups of the same name on output.
    /// - May return the current group if the inline group is requested.
    virtual Backend* CreateGroup(const String& name, bool isInput)=0;

    /// Creates/Finds a series entry with the specified name and returns a new Backend that will use it. Returns nullptr if not found.
    virtual Backend* CreateSeriesEntry(const String& name, bool isInput)=0;
    /// Retrieves the size of the series with the specified name, if the series exists. Use for serializing dynamic length series.
    virtual bool GetSeriesSize(const String& name, unsigned& size)=0;
    /// Sets the size of the series with the specified name. Use for serializing dynamic length series.
    virtual bool SetSeriesSize(const String& name, const unsigned& size)=0;
    /// Retrieves the list of entries names in the backend. Will clear the vector before adding new names first. Names are not required to be ordered.
    /// Use for serializing dynamically named entries like a Map (with key names appropriately restricted)
    virtual bool GetEntryNames(StringVector& names)=0;
    /// Sets the list of entries names in the backend from the names in the vector. May be a no-op if advanced serializtion is not required (in which case it should return true anyways).
    /// Use for serializing dynamically named entries like a Map (with key names appropriately restricted).
    virtual bool SetEntryNames(const StringVector& names)=0;



    /// Estimate of how verbose an inline series is.
    /// E.g. Binary gets a 0: no separators are needed between entries.
    /// E.g. JSON and the like get a 10: a single character separates entries, potentially very terse, though Urho will write it with newlines as well.
    /// E.g. XML gets a 100 because an inline series will create a child element <value/> for every element as attributes cannot be duplicated.
    virtual unsigned char InlineSeriesVerbosity() const =0;

    /// Simpler flag to use to check how verbose the
    bool UsesVerboseInlineSeries() const { return InlineSeriesVerbosity() < 50; }

    /// True if the Backend prefers binary data. False to prefer text data. Primarily used to determine prefered way to serialize an enum. Defaults to false as that is more human readable.
    virtual bool PrefersBinaryData() const { return false; }

    ///------------------------
    /// Control Functions

    /// Returns true if the condition is allowed. Used so that binary backend can "bake" the result of a conditional write.
    virtual bool WriteConditional(bool condition, bool isInput)
    {
        if (isInput)
            return true;
        return condition;
    }

    ///------------------------
    /// Value Get/Set Functions

    /// Template method to allow unsupported types by just failing to get them. Comment out while testing new types.
    template<class Unsupported> bool Get(Unsupported&) { return false; }

    /// Template method to allow unsupported types by just failing to get them. Comment out while testing new types.
    template<class Unsupported> bool Set(const Unsupported&) { return false; }

    /// Support for checking if there is a null value?
    virtual bool Get(const String& name, const std::nullptr_t&) = 0;

    /// Get the value with the specified name from the archive if it exists and set it to the passed in reference.
    virtual bool Get(const String& name, bool& val) = 0;

    virtual bool Get(const String& name, unsigned char& val) = 0;
    virtual bool Get(const String& name, signed char& val) = 0;
    virtual bool Get(const String& name, unsigned short& val) = 0;
    virtual bool Get(const String& name, signed short& val) = 0;
    virtual bool Get(const String& name, unsigned int& val) = 0;
    virtual bool Get(const String& name, signed int& val) = 0;
    virtual bool Get(const String& name, unsigned long long& val) = 0;
    virtual bool Get(const String& name, signed long long& val) = 0;

    virtual bool Get(const String& name, float& val) = 0;
    virtual bool Get(const String& name, double& val) = 0;

    virtual bool Get(const String& name, String& val) = 0;
#define EXTENDED_ARCHIVE_TYPES
#ifdef EXTENDED_ARCHIVE_TYPES

    /// Extended backend types should only be used if the backend has a special way of treating them.
    /// For example: ImGui has a ColorPicker and 3D gizmos. CSS has many special ways to specify color. Etc.
    virtual bool Get(const String& name, Urho3D::IntVector2& val) { return false; }
    virtual bool Get(const String& name, Urho3D::IntVector3& val) { return false; }
//    virtual bool Get(const String& name, Urho3D::IntVector4& val) { return false; }

    virtual bool Get(const String& name, Urho3D::Vector2& val) { return false; }
    virtual bool Get(const String& name, Urho3D::Vector3& val) { return false; }
    virtual bool Get(const String& name, Urho3D::Vector4& val) { return false; }
    virtual bool Get(const String& name, Urho3D::Quaternion& val) { return false; }
    virtual bool Get(const String& name, Urho3D::Color& val) { return false; }

    virtual bool Get(const String& name, Urho3D::Matrix3& val) { return false; }
    virtual bool Get(const String& name, Urho3D::Matrix3x4& val) { return false; }
    virtual bool Get(const String& name, Urho3D::Matrix4& val) { return false; }

#endif

#ifdef EXTENDED_ARCHIVE_TYPES
    /// True if the backend supports extended types like Vector3 and Color. The overload for such types then just has to call ArchiveValue<Color> internally.
    constexpr bool ExtendedTypeSupport() const { return true; }
    /// Note: may not use this vvv
    /// True if the backend supports directly setting container types like Vector and HashMap
    constexpr bool ContainerTypeSupport() const { return true; }
#else
    /// True if the backend supports extended types like Vector3 and Color. The overload for such types then just has to call ArchiveValue<Color> internally.
    constexpr bool ExtendedTypeSupport() const { return false; }
    /// Note: may not use this vvv
    /// True if the backend supports directly setting container types like Vector and HashMap
    constexpr bool ContainerTypeSupport() const { return false; }
#endif



    /// Support for checking if there is a null value?
    virtual bool Set(const String& name, const std::nullptr_t&) = 0;

    /// Set the value with the specified name from the archive if it exists and set it to the passed in reference.
    virtual bool Set(const String& name, const bool& val) = 0;

    virtual bool Set(const String& name, const unsigned char& val) = 0;
    virtual bool Set(const String& name, const signed char& val) = 0;
    virtual bool Set(const String& name, const unsigned short& val) = 0;
    virtual bool Set(const String& name, const signed short& val) = 0;
    virtual bool Set(const String& name, const unsigned int& val) = 0;
    virtual bool Set(const String& name, const signed int& val) = 0;
    virtual bool Set(const String& name, const unsigned long long& val) = 0;
    virtual bool Set(const String& name, const signed long long& val) = 0;

    virtual bool Set(const String& name, const float& val) = 0;
    virtual bool Set(const String& name, const double& val) = 0;

    virtual bool Set(const String& name, const String& val) = 0;
#ifdef EXTENDED_ARCHIVE_TYPES

    /// Extended backend types should only be used if the backend has a special way of treating them.
    /// For example: ImGui has a ColorPicker and 3D gizmos. CSS has many special ways to specify color. Etc.
    virtual bool Set(const String& name, const Urho3D::IntVector2& val) { return false; }
    virtual bool Set(const String& name, const Urho3D::IntVector3& val) { return false; }
//    virtual bool Set(const String& name, const Urho3D::IntVector4& val) { return false; }

    virtual bool Set(const String& name, const Urho3D::Vector2& val) { return false; }
    virtual bool Set(const String& name, const Urho3D::Vector3& val) { return false; }
    virtual bool Set(const String& name, const Urho3D::Vector4& val) { return false; }
    virtual bool Set(const String& name, const Urho3D::Quaternion& val) { return false; }
    virtual bool Set(const String& name, const Urho3D::Color& val) { return false; }

    virtual bool Set(const String& name, const Urho3D::Matrix3& val) { return false; }
    virtual bool Set(const String& name, const Urho3D::Matrix3x4& val) { return false; }
    virtual bool Set(const String& name, const Urho3D::Matrix4& val) { return false; }

    /// Add rect classes, resource ref, and maybe a few more.
#endif

#define TRY_EXTENDED(archive, name, val) ArchiveValue<decltype(val)>(archive, name, value)
#define TRY_EXTENDED_RETURN(archive, name, val) if(auto res = TRY_EXTENDED(archive, name, val)) return res;

    ///TODO: Redefine the above based on whether we have set EXTENDED_ARCHIVE_TYPES

    ///-----------------------
    /// Backend Hint functions

    struct BoundsHint {} BOUNDS;

    /// Somehow allow passing hints to the backend. Maybe as a hashmap of parameters? Maybe like OpenGL with enums and such.
    /// Hints allow things like specifying a min,max value for numbers and such. They are not enforced, merely hints at how to display it.
    /// Also things like an enumerated list of strings. Hints must be explicitly cleared as well.
//    virtual bool HintBounds(Urho3D::Variant min, Urho3D::Variant max) { return false; }

    /// Adds a hint to the stack. Returns true if the hint kind is recognized.
    virtual bool AddHint(const Hint& hint) { return false; }
    bool AddHint(Hint::HINT kind, const Variant& primary, const Variant& secondary = Variant()) { return AddHint(Hint{kind, primary, secondary}); }

    /// Returns true if a hint of this kind is present.
    virtual bool HasHint(Hint::HINT kind) { return false; }

    /// Returns a pointer to a hint with the requested kind or nullptr if the hint has not been set or is unsupported.
    virtual const Hint& GetHint(Hint::HINT kind) { return Hint::EMPTY_HINT; }

    /// Removes the requested hint kind. Returns true if it found such a hint.
    virtual bool RemoveHint(Hint::HINT kind) { return false; }

    /// Clears set hints. Returns true if succeeded.
    virtual bool ClearHints() { return false; }
};


/// Backend that simply fails at every operation. Use for returning non-existent groups/series for reading.
class NoOpBackend: public Backend
{
public:

    const String& GetBackendName() override { static const String name("NOOP"); return name; }
    Backend* CreateGroup(const String&, bool) override { return nullptr; }
    Backend* CreateSeriesEntry(const String&, bool) override { return nullptr; }
    bool GetSeriesSize(const String&, unsigned&) override { return false; }
    bool SetSeriesSize(const String&, const unsigned&) override { return false; }
    bool GetEntryNames(StringVector &) override { return false; }
    bool SetEntryNames(const StringVector &) override { return false; }
    unsigned char InlineSeriesVerbosity() const override { return 0; }
    bool Get(const String&, const std::nullptr_t&) override { return false; }
    bool Get(const String&, bool&) override { return false; }
    bool Get(const String&, unsigned char&) override { return false; }
    bool Get(const String&, signed char&) override { return false; }
    bool Get(const String&, unsigned short&) override { return false; }
    bool Get(const String&, signed short&) override { return false; }
    bool Get(const String&, unsigned int&) override { return false; }
    bool Get(const String&, signed int&) override { return false; }
    bool Get(const String&, unsigned long long&) override { return false; }
    bool Get(const String&, signed long long&) override { return false; }
    bool Get(const String&, float&) override { return false; }
    bool Get(const String&, double&) override { return false; }
    bool Get(const String&, String&) override { return false; }
    bool Set(const String&, const std::nullptr_t &) override { return false; }
    bool Set(const String&, const bool&) override { return false; }
    bool Set(const String&, const unsigned char&) override { return false; }
    bool Set(const String&, const signed char&) override { return false; }
    bool Set(const String&, const unsigned short&) override { return false; }
    bool Set(const String&, const signed short&) override { return false; }
    bool Set(const String&, const unsigned int&) override { return false; }
    bool Set(const String&, const signed int&) override { return false; }
    bool Set(const String&, const unsigned long long&) override { return false; }
    bool Set(const String&, const signed long long&) override { return false; }
    bool Set(const String&, const float&) override { return false; }
    bool Set(const String&, const double&) override { return false; }
    bool Set(const String&, const String&) override { return false; }
};


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


///TODO: XML Backend. Inline serialization will be <parent>Value</parent> maybe Inline series has to be faked entirely though

///TODO: Interface to iterate arbitrary named archive entries (for building a VariantMap, for example0


}

}
#include "Utils.h"

inline namespace Archival
{
    template<class Archive, class... T>
    class ArchiveResult;
}

namespace OLD {

using namespace Urho3D;

///// Archives have the ability to serialize values either by keys or by sequential order.
//class Archive
//{
//    bool IsInput() const;

//public:

//    template<class T>
//    ArchiveResult<Archive, T> Serialize(const String& name, T& val)
//    {
//        if (IsInput())
//            return ArchiveResult<Archive, T>(*this, Get<T>(name, val), val);
//        else
//            return ArchiveResult<Archive, T>(*this, Set<T>(name, val), val);
//    }

//    Archive CreateGroup(const String& name);

//private:
//    template<class T>
//    bool Get(const String& name, T& val);

//    template<class T>
//    bool Set(const String& name, const T& val);
//};



/// Archives have the ability to serialize values to specified
class StringArchive
{
    bool IsInput() const;
    typedef StringArchive Archive;

    unsigned indent_{0};
public:

    template<class T>
    ArchiveResult<Archive, T> Serialize(const String& name, T& val)
    {
        if (IsInput())
            return ArchiveResult<Archive, T>(*this, Get<T>(name, val), val);
        else
            return ArchiveResult<Archive, T>(*this, Set<T>(name, val), val);
    }

    Urho3D::Vector<String>& data_;

private:
    template<class T>
    bool Get(const String& name, T& val)
    {
        if (data_.Empty())
            return false;

        val = Urho3D::FromString<T>(data_.Front().CString());
        /// TODO: could check if it was possible, but Urho doesn't support it.
        return true;
    }


    template<class T>
    bool Set(const String&, T val)
    {
        data_.Push(val.ToString());
        return true;
    }


    bool ApplyIndent()
    {
        if (data_.Empty())
            return false;
        if (data_.Back()[0] == '\t')
            return false;
        data_.Back() = String('\t',indent_) + data_.Back();
    }
};


template<> inline bool StringArchive::Set<>(const String&, int v) { data_.EmplaceBack(v); return ApplyIndent(); }
template<> inline bool StringArchive::Set<>(const String&, unsigned v) { data_.EmplaceBack(v); return ApplyIndent(); }
template<> inline bool StringArchive::Set<>(const String&, short v) { data_.EmplaceBack(v); return ApplyIndent(); }
template<> inline bool StringArchive::Set<>(const String&, unsigned short v) { data_.EmplaceBack(v); return ApplyIndent(); }
template<> inline bool StringArchive::Set<>(const String&, float v) { data_.EmplaceBack(v); return ApplyIndent(); }
template<> inline bool StringArchive::Set<>(const String&, double v) { data_.EmplaceBack(v); return ApplyIndent(); }
template<> inline bool StringArchive::Set<>(const String&, const String& v) { data_.Push(v.Replaced("\"","\\\"")); return ApplyIndent(); }
template<> inline bool StringArchive::Set<>(const String&, const char* v) { data_.EmplaceBack(v); return ApplyIndent(); }


//template<class Archive, class... T>
//template<class...U>
//auto ArchiveResult<Archive, T...>::Else(const String& name, U... params)
//{
//    if (archiveOrNullIsSuccess)
//        return archiveOrNullIsSuccess->Serialize(name, params...);
//    return decltype (archiveOrNullIsSuccess->Serialize(name, params...)){*archiveOrNullIsSuccess, !this->archiveOrNullIsSuccess, params...};
//}


//template<typename T>
//using get_type = typename base_type<typename std::result_of<T>::type>::type;

//template<typename T>
//using set_type = typename base_type<typename std::result_of<T>::type>::type;

//using getset_type = std::enable_if<std::is_same<
}
