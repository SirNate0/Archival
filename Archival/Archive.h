#pragma once

#include "Archive-old.h"

#include <typeinfo>
#include <cxxabi.h>

namespace future
{

struct ArchiveExample;

/// "Magic" class that allows conditional serialization through providing Then() and Else() that will serialize based on the result of the previous Serialization/WriteConditional call.
/// Provides an operator bool() overload so Serialization can still be checked in a boolean success/fail manner. The originating Archive must live as long as the results.
template<class... T>
class ArchiveResult2
{
    /// Non-owning pointer to the originating archive.
    ArchiveExample* archive;
    /// True if the last call was a success, false if it failed.
    bool succeeded;
    /// Store the values that made the call so they can be serialized with a different name (most useful in reading [1,2,3] or [{x:1},{y:2},{z:3}] type situations.
    std::tuple<T&...> vals;

public:
    /// Construct the ArchiveResult from the supplying archive with a last successful call result of success called with the provided values.
    ArchiveResult2(ArchiveExample& archive, bool success, T&... values): archive{&archive}, succeeded{success}, vals{values...}
    {}


    /// Will serialize the passed value only if the last call was a failure.
    template<class... Args>
    auto Else(const String& name, Args&&... params);

    /// Will serialize the passed value only if the last call was a success.
    template<class... Args>
    auto Then(const String& name, Args&&... params);

    /// Utility method that returns a new result with different parameters (as if the last call had been made with those instead).
    template<class... U>
    ArchiveResult<Archive, U...> ReplaceParams(U... params)
    {
        return {*archive, Succeeded(), params...};
    }


    /// If the last call failed to serialize retry serialization to the same value with a new name.
    ArchiveResult2 Else(const String& name)
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
    ArchiveResult2 ElseSeq(const String& name, detail::seq<S...>)
    {
        return Else(name, std::get<S>(vals) ...);
    }
};

struct ArchiveExample
{
    /// Stores the backend for the archive. The backend handles the actual saving and loading of the "basic" types, allowing us to serialize classes simply by overloading the ArchiveValue function.
    Urho3D::UniquePtr<Archival::Detail::Backend> backend_;
    /// Returns a reference to the current backend. Will return a reference to the NpOpBackend if no backend is present.
    Detail::Backend& GetBackend() { assert(backend_); return *backend_; }

    ArchiveExample(bool input): backend_{new Detail::NoOpBackend()}, isInput_(input) {}


    /// Construct an input/output archive as specified by moving the backend from another unique pointer.
    ArchiveExample(bool input, Urho3D::UniquePtr<Detail::Backend>&& backend): backend_(std::move(backend)), isInput_(input) {}
    /// Construct an input/output archive as specified by taking ownership of a raw backend pointer.
    ArchiveExample(bool input, Detail::Backend* backend): backend_(backend), isInput_(input) {}


//    /// Placeholders for the real backend.
//    struct BAK { String InlineName() const { return "value"; } } backend;
//    BAK& GetBackend() { return backend; }

    /// The core function. Dispatches serialization to teh Archiver.
    template <typename T>
    ArchiveResult2<T> Serialize(const String& name, T&& val);

    /// Create a group in the archive with the specified name.
    /// Will try to create the group inline if the Backend::InlineName() string is passed.
    /// Inline means {"old" : "val", **{ENTRY} } using python syntax instead of {"old" : "val", "value" : {ENTRY}}.
    ArchiveExample CreateGroup(const String& name)
    {
        URHO3D_LOGINFO("Creating Group Entry " + name);

        if (auto* b = GetBackend().CreateGroup(name, IsInput()))
            return {IsInput(), b};
        return {IsInput()};
    }

    /// Create a new series entry in the archive with the specified name.
    /// Will try to create the series inline if the Backend::InlineName() string is passed.
    /// Inline means "x" : [ {ENTRY} ] instead of "x" : { "value" : [ {ENTRY} ] }
    ArchiveExample CreateSeriesEntry(const String& name)
    {
//        if (Urho3D::UniquePtr<Detail::Backend> b = GetBackend().CreateSeriesEntry(name, IsInput()))
//            return Archive(IsInput(), std::move(b));
        URHO3D_LOGINFO("Creating Series Entry " + name);

        if (auto b = GetBackend().CreateSeriesEntry(name, IsInput()))
            return {IsInput(), b};
        return {IsInput()};
    }

    /// Extension of the core function that implicitly creates a series for the specified values.
    template <typename T, typename... Ts>
    ArchiveResult2<T,Ts...> Serialize(const String& name, T&& val, Ts&&... vals);

    /// Saves/Loads a value inline (if possible) to/from the archive based on IsInput(). Inline means [VAL] instead of [{"value" : VAL}].
    template<class...T>
    auto SerializeInline(T&&...args)
    {
        return Serialize(GetBackend().InlineName(), std::forward<T>(args)...);
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

    bool isInput_;
    bool IsInput() const { return isInput_; }

    /// Allows binary to bake conditional choices and text to skip optional entries.
    bool WriteConditional(bool value) { return isInput_ || value; }
};

// https://dev.krzaq.cc/post/checking-whether-a-class-has-a-member-function-with-a-given-signature/
template<typename T>
struct has_ArchiveValue_method
{
private:
    typedef std::true_type yes;
    typedef std::false_type no;

    template<typename U> static auto test(int) -> decltype(std::declval<U>().ArchiveValue(std::declval<ArchiveExample&>(), std::declval<const String&>()) == 1, yes());

    template<typename> static no test(...);

public:

    static constexpr bool value = std::is_same<decltype(test<T>(0)),yes>::value;
};

// https://www.cppstories.com/2019/07/detect-overload-from-chars/
template <typename T, typename = void>
struct is_ArchiveValueEx_available : std::false_type {};
template <typename T>
struct is_ArchiveValueEx_available<T,
           std::void_t<
        decltype(ArchiveValueEx(std::declval<ArchiveExample&>(), std::declval<const String&>(), std::declval<T&>())
                 ) >> : std::true_type {};
// helper variable template
template< typename T> inline constexpr bool is_ArchiveValueEx_available_v =
          is_ArchiveValueEx_available<T>::value;

template< class T> class false_on_instantiation : public std::false_type {};

template <typename T>
struct Archiver
{
    static bool ArchiveValue(ArchiveExample& a, const String& name, T&& val)
    {
//        static_assert(Archival::Detail::IsBasicType<std::remove_reference_t<T>>::value,"Check True");
//        static_assert(!Archival::Detail::IsBasicType<std::remove_reference_t<T>>::value,"Check False");
        if constexpr (Archival::Detail::IsBasicType<std::remove_reference_t<T>>::value)
        {
            // Call Get or Set
            if (a.IsInput())
            {
                URHO3D_LOGINFO("Reading " + name + ": " + String(val));
            }
            else
            {
                URHO3D_LOGINFO("Writing " + name + ": " + String(val));
            }
            return true;
        }
        else if constexpr (has_ArchiveValue_method<std::remove_reference_t<T>>::value)
        {
            return val.ArchiveValue(a,name);
        }
        else if constexpr (is_ArchiveValueEx_available_v<std::remove_reference_t<T>>)
        {
            return ArchiveValueEx(a,name,val);
        }
        else
        {
            // Use the _on_instantiation so the compiler won't
            static_assert(false_on_instantiation<T>::value,"User should add an ArchiveValue method to the class or specialize Archiver class.");
        }
    }
};


template <typename T>
inline String DemangledName()
{
    auto name = typeid (T).name();
    auto demangled = __cxxabiv1::__cxa_demangle(name,nullptr,nullptr,nullptr);
    String s(demangled);
    free(demangled);
    return s;
}

template<typename T>
const String DemangledName_v = DemangledName<T>();

template <typename T>
ArchiveResult2<T> ArchiveExample::Serialize(const String& name, T&& val)
{
//    const auto& tname = DemangledName_v<T>;
//    URHO3D_LOGDEBUG("Serializing "+ tname);
//    return Archiver<T>::ArchiveValue(*this,name,std::forward<T>(val));
    return {*this, Archiver<T>::ArchiveValue(*this,name,std::forward<T>(val)), val};
}

template <typename T, typename... Ts>
ArchiveResult2<T,Ts...> ArchiveExample::Serialize(const String& name, T&& val, Ts&&... vals)
{
//    const auto& tname = DemangledName_v<T>;
//    URHO3D_LOGDEBUG("Serializing "+ tname);
//    return Archiver<T>::ArchiveValue(*this,name,std::forward<T>(val));
//    return {*this, Archiver<T>::ArchiveValue(*this,name,std::forward<T>(val)), val};

//    bool success = (Archiver<T>::ArchiveValue(*this,name,std::forward<T>(val))
//            && ... && Archiver<Ts>::ArchiveValue(*this,name,std::forward<Ts>(vals)));

    bool success = (CreateSeriesEntry(name).SerializeInline(val)
            && ... && CreateSeriesEntry(name).SerializeInline(vals));
    return {*this,success,val,vals...};

}

template<class... T>
template<class... Args>
auto ArchiveResult2<T...>::Else(const String& name, Args&&... params)
{
    typedef decltype (archive->Serialize(name, std::forward<Args>(params)...)) ret;
    if (!Succeeded())
        return archive->Serialize(name, std::forward<Args>(params)...);
    return ret{*archive, Succeeded(), params...};
}

template<class... T>
template<class... Args>
auto ArchiveResult2<T...>::Then(const String& name, Args&&... params)
{
    typedef decltype (archive->Serialize(name, std::forward<Args>(params)...)) ret;
    if (Succeeded())
        return archive->Serialize(name, std::forward<Args>(params)...);
    return ret{*archive, Succeeded(), params...};
}


}



