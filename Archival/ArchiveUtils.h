#pragma once

#include "Archive.h"

namespace future {

namespace detail {

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
struct GetSet2Holder
{
    Getter& getter;
    Setter& setter;
    using Type = T;

    /// An easter egg: Go() returns itself. It's solely so that you can have GetSet(...).Go()
    GetSet2Holder& Go() { return *this; }

    template<class U>
    bool CallSetter(U&& value)
    {
        return detail::CallReturningBool(setter, std::forward<U>(value));
    }

    bool ArchiveValue(ArchiveExample& ar, const String& name)
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
GetSet2Holder<Getter,Setter,T> GetSet2(Getter&&g, Setter&&s) { return {g,s}; }

}
