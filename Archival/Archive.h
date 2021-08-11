#pragma once

#include "Archive-old.h"

#include <typeinfo>
#include <cxxabi.h>

namespace future
{


struct ArchiveExample
{
    template <typename T>
    bool Serialize(const String& name, T&& val);

    bool isInput;
    bool IsInput() const { return isInput; }

    /// Allows binary to bake conditional choices and text to skip optional entries.
    bool WriteConditional(bool value) { return isInput || value; }
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
bool ArchiveExample::Serialize(const String& name, T&& val)
{
//    const auto& tname = DemangledName_v<T>;
//    URHO3D_LOGDEBUG("Serializing "+ tname);
    return Archiver<T>::ArchiveValue(*this,name,std::forward<T>(val));
}


}



