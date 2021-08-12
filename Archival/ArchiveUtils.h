#pragma once

#include <functional>

#include "Archive.h"

namespace Archival {


/// Holder to specialize ArchiveValue on to handle the case of a get and set function instead of a variable reference.
template<typename Getter, typename Setter, class T = typename std::result_of<Getter()>::type>
struct GetSetHolder
{
    Getter getter;
    Setter setter;
    using Type = T;

    // Operators to allow the WithDefault behavior
    bool operator == (const T& rhs) { return getter() == rhs; }
    bool operator != (const T& rhs) { return getter() != rhs; }
    void operator=(const T& rhs) { setter(rhs); }

    /// An easter egg: Go() returns itself. It's solely so that you can have GetSet(...).Go()
    GetSetHolder& Go() { return *this; }

    template<class U>
    bool CallSetter(U&& value)
    {
        return detail::CallReturningBool(setter, std::forward<U>(value));
    }

    bool ArchiveValue(Archive& ar, const String& name)
    {
//        using GS = GetSetHolder<Getter, Setter, T>;
        if (ar.IsInput())
        {
            //TODO: remove const reference

            //TODO: add a specific check for Backend::RequiresInitializedGet()
            T val = getter();

            return ar.Serialize(name, val) && CallSetter(val);
        }
        else
        {
            //TODO: remove const reference
            auto val = getter();
            return ar.Serialize(name, val);
        }
    }
};

/// Convenience function to deduce the type for a GetSetHolder based on the getter and setter.
template<typename Getter, typename Setter, class T = typename std::result_of<Getter()>::type>
GetSetHolder<Getter,Setter,T> GetSet(Getter&&g, Setter&&s) { return {g,s}; }

/// Convenience function to deduce the type for a GetSetHolder based on the getter and setter.
template<class Object, typename Getter, typename Setter>
auto ObjectGetSet(Object&& o, Getter&&g, Setter&&s) {
//    auto getter = std::bind(std::forward<Getter>(g),std::forward<Object>(o));
//    auto setter = std::bind(std::forward<Setter>(s),std::forward<Object>(o),std::placeholders::_1);
//    return GetSet(getter,setter);
    auto res = GetSet(std::bind(std::forward<Getter>(g),std::forward<Object>(o)),std::bind(std::forward<Setter>(s),std::forward<Object>(o),std::placeholders::_1));
    return res;
}


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


/// Overload to ArchiveValue that uses the provided enum names to store the enum based on the Backend's PrefersBinaryValue().
template<typename Enum, typename Strings, bool CASE_SENSITIVE>
struct Archiver<EnumNamesHolder<Enum, Strings, CASE_SENSITIVE>>
{
    using T = EnumNamesHolder<Enum, Strings, CASE_SENSITIVE>;
    static bool ArchiveValue(Archive& ar, const String& name, T&& enumNames)
    {
        using intType = typename std::underlying_type<Enum>::type;

        // Coerce the value into an integer reference.
        intType* intPointer = reinterpret_cast<intType*>(&enumNames.value);
        intType& val = *intPointer;

        /// TODO: this call should go ahead and add the ALLOWED_OPTIONS hint.
        Urho3D::VariantVector names = Urho3D::ConvertToVariantVector(enumNames.enumNames);
//        ar.Hint(Detail::Hint::SUGGESTED_OPTIONS, names);

        bool good = true;
//        if (ar.GetBackend().PrefersBinaryData())
        if constexpr (false)
        {
            good = ar.Serialize(name, val)
                    .Else(name, GetSet([&](){return enumNames.EnumToString();},
            [&](const String& name){ return enumNames.StringToEnum(name);}));
        }
        else {
            good = ar.Serialize(name, GetSet([&](){return enumNames.EnumToString();},
            [&](const String& name){ return enumNames.StringToEnum(name);}))
//                    .Else(name, val);
                    ;
        }
//        ar.UnHint(Detail::Hint::SUGGESTED_OPTIONS);
//        return {ar, good, enumNames};
        return good;
    }
};

/// Holder for a default value. It need not be the same type as the value being archived, but it must be assignable.
template<class T, class Default>
struct WithDefaultHolder
{
    T& archiveValue;
    Default defaultValue;

    bool ArchiveValue(Archive& ar, const String& name)
    {
        // Write the conditional check for binary output / only output the value when it is not the default for the text value.
        if (/*TODO: ar.GetBackend()->AlwaysOutputDefault() || */ar.WriteConditional(name + "-not-default", archiveValue != defaultValue))
        {
            URHO3D_LOGINFO("WriteCond Match - input or not default.");
            if ((!ar.Serialize(name,archiveValue) && ar.IsInput()))
            {
                URHO3D_LOGINFO("Failed to serialize and we are input - assigning.");
                archiveValue = defaultValue;
            }
            else URHO3D_LOGINFO("Successful serialization and we are input - not assigning default.");

            // TODO: if (ar.GetBackend()->GetHint(RESET_TO_DEFAULT)) { archiveValue = defaultValue; ar.GetBackend()->ClearHint(RESET_TO_DEFAULT) }
        }
        else URHO3D_LOGINFO("WriteCond failed - output and default.");

        return true;
    }
};


template<typename T, typename Default>
WithDefaultHolder<T,Default> WithDefault(T&& val, Default&& def) { return {val, std::forward<Default>(def)}; }



}
