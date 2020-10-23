#pragma once

#include <Urho3D/Container/Str.h>
#include <Urho3D/Core/StringUtils.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/IO/Log.h>

#include <Urho3D/Core/Context.h>
// Broken, so include Context before it
#include <Urho3D/Resource/JSONValue.h>


#include <tuple>
#include <functional>
#include <type_traits>

#include "Utils.h"

#include "ArchiveDetail.h"

inline namespace Archival
{

using Urho3D::String;

// Forward declaration of ArchiveResult and ArchiveValue
template<class Archive, class... T>
class ArchiveResult;
template<class Archive, typename T>
ArchiveResult<Archive, T> ArchiveValue(Archive& ar, const String& name, T& value);


template<typename T>
using base_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;


// From https://stackoverflow.com/questions/36612596/tuple-to-parameter-pack
namespace detail {

template<int ...> struct seq {};

template<int N, int ...S> struct gens : gens<N - 1, N - 1, S...> { };

template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };


//template<typename Fn, class T, bool RETURNS_VOID = std::is_same<typename std::result_of<Fn>::type,void>::value>
//struct CallReturningBool {};

//template<typename Fn, class T>
//struct CallReturningBool<Fn, T, true> { static bool Call(Fn& f, T&& val) { f(std::forward<T>(val)); return true; } };

//template<typename Fn, class T>
//struct CallReturningBool<Fn, T, false> { static bool Call(Fn& f, T&& val) { return f(std::forward<T>(val)); } };

template<class Fn, class T, class...Extra>
bool CallReturningBool(Fn& fn, T&& val, Extra...) { fn(std::forward<T>(val)); return true; }

template<class Fn, class T>
bool CallReturningBool(Fn& fn, typename std::enable_if<std::is_same<typename std::result_of<Fn(T)>::type, void>::value, T>::type&& val)
{
    return fn(std::forward<T>(val));
}

}


/// Holder to specialize ArchiveValue on to handle the case of a get and set function instead of a variable reference.
template<typename Getter, typename Setter, class T = typename std::result_of<Getter()>::type>
struct GetSetHolder
{
    Getter& getter;
    Setter& setter;

    /// An easter egg: Go() returns itself. It's solely so that you can have GetSet(...).Go()
    GetSetHolder& Go() { return *this; }

    template<class U>
    bool CallSetter(U&& value)
    {
        return detail::CallReturningBool(setter, std::forward<U>(value));
    }
};

/// Convenience function to deduce the type for a GetSetHolder based on the getter and setter.
template<typename Getter, typename Setter, class T = typename std::result_of<Getter()>::type>
GetSetHolder<Getter,Setter,T> GetSet(Getter&&g, Setter&&s) { return {g,s}; }

/// Class that holds an enum value and associated string names.
template<typename Enum, typename StringsContainer = const char **, bool CASE_SENSITIVE = true>
struct EnumNamesHolder
{
    Enum& value;
    StringsContainer enumNames;

    /// Returns the string representation for the enum value.
    String EnumToString()
    {
        return enumNames[static_cast<unsigned>(value)];
    }

    /// Attempts to look up the value in the provided strings, case sensitively based on the bool template parameter.
    bool StringToEnum(const String& val)
    {
        typename std::underlying_type<Enum>::type i = 0;
        for (auto s : enumNames)
            if (CASE_SENSITIVE ? val == s : (val.Compare(s, false) == 0))
            {
                value = static_cast<Enum>(i);
                return true;
            }
            else
                i += 1;
        return false;
    }
};

/// Class that holds an enum value and associated string names. Specialized for an array of const char *'s with a {0} sentinel for the end.
template<typename Enum, bool CASE_SENSITIVE>
struct EnumNamesHolder<Enum, const char **, CASE_SENSITIVE>
{
    Enum& value;
    const char ** enumNames;


    /// Returns the string representation for the enum value.
    String EnumToString()
    {
        return enumNames[static_cast<unsigned>(value)];
    }

    /// Attempts to look up the value in the provided strings, case sensitively based on the bool template parameter.
    bool StringToEnum(const String& val)
    {
        typename std::underlying_type<Enum>::type i = 0;
        for (const char** name = enumNames; *name[0] != 0; ++name)
        {
            if (CASE_SENSITIVE ? val == *name : (val.Compare(*name, false) == 0))
            {
                value = static_cast<Enum>(i);
                return true;
            }
            else
                i += 1;
        }
        return false;
    }
};

template<typename Enum, typename StringsContainer>
EnumNamesHolder<Enum, StringsContainer, false> EnumNames(Enum& val, StringsContainer&& names) { return {val, names}; }

template<typename Enum, typename StringsContainer>
EnumNamesHolder<Enum, StringsContainer, true> EnumNamesCaseSensitive(Enum& val, StringsContainer&& names) { return {val, names}; }

///TODO: Default Holder
template<class T>
struct WithDefaultHolder
{
    T& archiveValue;
    T defaultValue;
};

/// "Magic" class that allows conditional serialization through providing Then() and Else() that will serialize based on the result of the previous Serialization/WriteConditional call.
/// Provides an operator bool() overload so Serialization can still be checked in a boolean success/fail manner. The originating Archive must live as long as the results.
template<class Archive, class... T>
class ArchiveResult
{
    /// Non-owning pointer to the originating archive.
    Archive* archive;
    /// True if the last call was a success, false if it failed.
    bool succeeded;
    /// Store the values that made the call so they can be serialized with a different name (most useful in reading [1,2,3] or [{x:1},{y:2},{z:3}] type situations.
    std::tuple<T&...> vals;

public:
    /// Construct the ArchiveResult from the supplying archive with a last successful call result of success called with the provided values.
    ArchiveResult(Archive& archive, bool success, T&... values): archive{&archive}, succeeded{success}, vals{values...}
    {}


    /// Will serialize the passed value only if the last call was a failure.
    template<class... Args>
    auto Else(const String& name, Args&&... params)
    {
        typedef decltype (archive->Serialize(name, std::forward<Args>(params)...)) ret;
        if (!Succeeded())
            return archive->Serialize(name, std::forward<Args>(params)...);
        return ret{*archive, Succeeded(), params...};
    }

    /// Will serialize the passed value only if the last call was a success.
    template<class... Args>
    auto Then(const String& name, Args&&... params)
    {
        typedef decltype (archive->Serialize(name, std::forward<Args>(params)...)) ret;
        if (Succeeded())
            return archive->Serialize(name, std::forward<Args>(params)...);
        return ret{*archive, Succeeded(), params...};
    }

    /// Utility method that returns a new result with different parameters (as if the last call had been made with those instead).
    template<class... U>
    ArchiveResult<Archive, U...> ReplaceParams(U... params)
    {
        return {*archive, Succeeded(), params...};
    }


    /// If the last call failed to serialize retry serialization to the same value with a new name.
    ArchiveResult Else(const String& name)
    {
        using namespace detail;
        return ElseSeq(name, typename gens<sizeof...(T)>::type()); // Item #1
    }

    /// True if the last call succeded.
    bool Succeeded() const { return succeeded; }
    operator bool() const { return Succeeded(); }

private:

    /// Internal method to convert the tuple to a parameter pack for the Else("newName") call.
    template<int ...S>
    ArchiveResult ElseSeq(const String& name, detail::seq<S...>)
    {
        return Else(name, std::get<S>(vals) ...);
    }
};

/// Overload to ArchiveValue that uses a provided getter and setter function stored in the GetSetHolder.
template<class Archive, typename Getter, typename Setter, class T = typename std::result_of<Getter()>::type>
ArchiveResult<Archive, GetSetHolder<Getter, Setter, T>> ArchiveValue(Archive& ar, const String& name, GetSetHolder<Getter,Setter,T>&& getset)
{
    using GS = GetSetHolder<Getter, Setter, T>;
    if (ar.IsInput())
    {
        //TODO: remove const reference
        T val = getset.getter();
        //TODO: add a specific check for Backend::RequiresInitializedGet()
        if (auto res = ArchiveValue(ar, name, val))
        {
            return ArchiveResult<Archive, GS>{ar, getset.CallSetter(val), getset};
        }
        else
        {
            return ArchiveResult<Archive, GS>{ar, false, getset};
        }
    }
    else
    {
        //TODO: remove const reference
        auto val = getset.getter();
        auto success = ArchiveValue(ar, name, val);
        return ArchiveResult<Archive, GS>{ar, success, getset};
    }
}

/// Archives have the ability to serialize values to specified
class Archive
{
    Archive(bool input): backend_{new Detail::NoOpBackend()}, isInput_(input) {}
public:

    /// Construct an input/output archive as specified by moving the backend from another unique pointer.
    Archive(bool input, Urho3D::UniquePtr<Detail::Backend>&& backend): backend_(std::move(backend)), isInput_(input) {}
    /// Construct an input/output archive as specified by taking ownership of a raw backend pointer.
    Archive(bool input, Detail::Backend* backend): backend_(backend), isInput_(input) {}

    /// Returns true if the archive is an input archive (Get's from the Backend).
    bool IsInput() const { return isInput_; }

    /// Create a group in the archive with the specified name.
    /// Will try to create the group inline if the Backend::InlineName() string is passed.
    /// Inline means {"old" : "val", **{ENTRY} } using python syntax instead of {"old" : "val", "value" : {ENTRY}}.
    Archive CreateGroup(const String& name)
    {
        if (auto* b = GetBackend().CreateGroup(name, IsInput()))
            return Archive(IsInput(), b);
        return Archive(IsInput());
    }

    /// Create a new series entry in the archive with the specified name.
    /// Will try to create the series inline if the Backend::InlineName() string is passed.
    /// Inline means "x" : [ {ENTRY} ] instead of "x" : { "value" : [ {ENTRY} ] }
    Archive CreateSeriesEntry(const String& name)
    {
//        if (Urho3D::UniquePtr<Detail::Backend> b = GetBackend().CreateSeriesEntry(name, IsInput()))
//            return Archive(IsInput(), std::move(b));
        if (auto b = GetBackend().CreateSeriesEntry(name, IsInput()))
            return Archive(IsInput(), b);
        return Archive(IsInput());
    }

    /// Utility method to create a series with the sentinel inline name.
    Archive CreateSeriesEntryInline()
    {
        return CreateSeriesEntry(GetBackend().InlineName());
    }

    /// Serializes a dynamically sized series size. Required to support the BinaryBackend. Recommended regardless to allow resizing the user's container regardless.
    /// Calls resize on the passed object to generate a series of that size. Specialized to supply an int as well.
    template<class Resizable, class... ExtraArgs>
    bool SerializeSeriesSize(const String& name, Resizable& v, ExtraArgs&&...args)
    {
        if (IsInput())
        {
            unsigned size = v.Size();
            if (GetBackend().GetSeriesSize(name, size))
            {
                v.Resize(size, std::forward<ExtraArgs>(args)...);
                return true;
            }
            else
                return false;
        }
        else
        {
            return GetBackend().SetSeriesSize(name, v.Size());
        }
        return true;
    }

    /// Serializes dynamically named elements, such as the entries of a HashMap. May or may not perform actual serialization with all Backends.
    /// Pushes names of serialized elements to the vector on input or (possibly) reads from it on output.
    /// Will clear the vector before inserting entries into it.
    bool SerializeEntryNames(Urho3D::StringVector& names)
    {
        if (IsInput())
        {
            return GetBackend().GetEntryNames(names);
        }
        else
        {
            return GetBackend().SetEntryNames(names);
        }
    }

    /// Magic function to allow skipping writing of values if appropriate. Follow with .Then(...).
    /// Condition may be saved to the file (e.g. BinaryBackend) to allow matching brancing on load.
    ArchiveResult<Archive> WriteConditional(bool value)
    {
        return ArchiveResult<Archive>(*this, GetBackend().WriteConditional(value, IsInput()));
    }

    /// Saves/Loads a value to/from the archive based on IsInput().
    template<class T>
    auto Serialize(const String& name, T&& val)
    {
        return ArchiveValue(*this, name, std::forward<T>(val));
    }

    /// Saves/Loads a value inline (if possible) to/from the archive based on IsInput(). Inline means [VAL] instead of [{"value" : VAL}].
    template<class...T>
    auto SerializeInline(T&&...args)
    {
        return Serialize(GetBackend().InlineName(), std::forward<T>(args)...);
    }

    /// Adds a hint to the backend. Returns *this so that you can perform the call inline (e.g. CreateGroup(...).Hint(...).Serialize(...);
    Archive& Hint(const Detail::Hint& hint) { GetBackend().AddHint(hint); return *this; }
    Archive& Hint(Detail::Hint::HINT kind, const Urho3D::Variant& primary, const Urho3D::Variant& secondary = Urho3D::Variant()) { GetBackend().AddHint(kind, primary, secondary); return *this; }

    /// Removes a hint from the backend, if present. Returns *this so that you can perform the call inline (e.g. ar.UnHint(...).Serialize(...);
    Archive& UnHint(const Detail::Hint::HINT kind) { GetBackend().RemoveHint(kind); return *this; }
    /// Clears all hints in the backend. Returns *this so that you can perform the call inline (e.g. ar.ClearHints().Serialize(...);
    Archive& ClearHints() { GetBackend().ClearHints(); return *this; }

    /// Retrieves a hint from the backend, or the EMPTY_HINT if not present.
    const auto& GetHint(const Detail::Hint::HINT kind) { return GetBackend().GetHint(kind); }

    /// Returns a reference to the current backend. Will return a reference to the NpOpBackend if no backend is present.
    Detail::Backend& GetBackend() { assert(backend_); return *backend_; }

private:
    /// Stores the backend for the archive. The backend handles the actual saving and loading of the "basic" types, allowing us to serialize classes simply by overloading the ArchiveValue function.
    Urho3D::UniquePtr<Detail::Backend> backend_;

    /// True if this Archive sets values in the user supplied objects. Roughly this if true for read, false for write, but cases like the IMGUI backend may be read-write and require input access to set the values.
    bool isInput_;

/// Utility macro to define the appropriate friend function for the specified type. The Archive will have name archive, and the Value& will have the spefified name. Just do (Type, ) for no name.
#define FRIEND_ARCHIVE_VALUE(Value_Type, val) friend ArchiveResult< ::Archival::Archive, Value_Type> ArchiveValue(::Archival::Archive& archive, const String& name, Value_Type& val)

};


/// The default implementation of archiving a type. Defers to Get<T>/Set<T>.
/// Get/Set must exist for the types bool, int, unsigned, float, and String.
///         Maybe also for null, (unsigned) short, (unsigned) long, and double.
/// Values may possibly be cast between different types by the underlying implementation, e.g. XML stores all as a String.
/// The name "value" is reserved. It is used to handle the case of inline values (like JSON [1,2,3]).
template<class Archive, typename T>
ArchiveResult<Archive, T> ArchiveValue(Archive& ar, const String& name, T& value)
{
    if (ar.IsInput())
        return ArchiveResult<Archive, T>(ar, ar.GetBackend().Get(name, value), value);
    else
        return ArchiveResult<Archive, T>(ar, ar.GetBackend().Set(name, value), value);
}


/// Overload to ArchiveValue that uses the provided enum names to store the enum based on the Backend's PrefersBinaryValue().
template<typename Enum, typename Strings, bool CASE_SENSITIVE>
ArchiveResult<Archive, EnumNamesHolder<Enum, Strings, CASE_SENSITIVE>> ArchiveValue(Archive& ar, const String& name, EnumNamesHolder<Enum, Strings, CASE_SENSITIVE>&& enumNames)
{
    using intType = typename std::underlying_type<Enum>::type;

    // Coerce the value into an integer reference.
    intType* intPointer = reinterpret_cast<intType*>(&enumNames.value);
    intType& val = *intPointer;

    /// TODO: this call should go ahead and add the ALLOWED_OPTIONS hint.
    Urho3D::VariantVector names = Urho3D::ConvertToVariantVector(enumNames.enumNames);
    ar.Hint(Detail::Hint::SUGGESTED_OPTIONS, names);
//    if (ar.IsInput())
//    {
        bool good = true;
        if (ar.GetBackend().PrefersBinaryData())
        {
            good = ar.Serialize(name, val)
                    .Else(name, GetSet([&](){return enumNames.EnumToString();},
            [&](const String& name){ return enumNames.StringToEnum(name);}));
        }
        else {
            good = ar.Serialize(name, GetSet([&](){return enumNames.EnumToString();},
            [&](const String& name){ return enumNames.StringToEnum(name);}))
                    .Else(name, val);
        }
        ar.UnHint(Detail::Hint::SUGGESTED_OPTIONS);
        return {ar, good, enumNames};
//    }
//    else
//    {
//        /// If it's outputting the values we don't need the else clause, but there's not much point in writing it seperately.
//        bool good = true;
//        if (ar.GetBackend().PrefersBinaryData())
//        {
//            good = ar.Serialize(name, static_cast<intType&>(enumNames.value));
//        }
//        else {
//            good = ar.Serialize(name, GetSet([&](){return enumNames.EnumToString();},
//            [&](const String& name){ return enumNames.StringToEnum(name);}));
//        }
//        return {ar, good, enumNames};
//    }
}

/// Calls resize on the passed object to generate a series of that size. Specialized to supply an int as well.
template<>
inline bool Archive::SerializeSeriesSize(const String& name, unsigned& size)
{
    if (IsInput())
    {
        return GetBackend().GetSeriesSize(name, size);
    }
    else
    {
        return GetBackend().SetSeriesSize(name, size);
    }
}

}
