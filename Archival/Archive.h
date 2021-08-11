#pragma once

#include "Archive-old.h"


namespace future
{


struct ArchiveExample
{
    template <typename T>
    void Serialize(const String& name, T& val);

    bool isInput;
    bool IsInput() const { return isInput; }

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

template< class T> class false_on_instantiation : std::false_type {};

template <typename T>
struct Archiver
{
    static bool ArchiveValue(ArchiveExample& a, const String& name, T& val)
    {
        if constexpr (Archival::Detail::IsBasicType<T>::value)
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
        else if constexpr (has_ArchiveValue_method<T>::value)
        {
            return val.ArchiveValue(a,name);
        }
        else if constexpr (is_ArchiveValueEx_available_v<T>)
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
void ArchiveExample::Serialize(const String& name, T& val)
{
    Archiver<T>::ArchiveValue(*this,name,val);
}


}


