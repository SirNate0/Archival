#include "ArchiveTest.h"


namespace OtherNamespace {

bool Example::ArchiveValue(future::ArchiveExample& ar, const String& name)
{
    URHO3D_LOGINFO("ONS Successful Method: " + name);
    ar.Serialize("f",a);
    ar.Serialize("i",b);
    ar.Serialize("s",c);
    return true;
}

bool ArchiveValueEx(future::ArchiveExample& ar, const String& name, ExampleEx& ex)
{
    URHO3D_LOGINFO("ONS Successful External Function: " + name);
    ar.Serialize("f",ex.a);
    ar.Serialize("i",ex.b);
    ar.Serialize("s",ex.c);
    return true;
}

}

namespace future {
//template<>
bool future::Archiver<OtherNamespace::ExampleS&>::ArchiveValue(ArchiveExample& ar, const String& name, future::Archiver<OtherNamespace::ExampleS&>::T& ex)
{
    URHO3D_LOGINFO("Successful Specialization: " + name);
    ar.Serialize("f",ex.a);
    ar.Serialize("i",ex.b);
    ar.Serialize("s",ex.c);
    return true;
}
}
