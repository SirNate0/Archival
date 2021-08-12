#include "ArchiveTest.h"


namespace OtherNamespace {

bool Example::ArchiveValue(Archival::Archive& ar, const String& name)
{
    URHO3D_LOGINFO("ONS Successful Method: " + name);
    ar.Serialize("f",a);
    ar.Serialize("i",b);
    ar.Serialize("s",c);
    return true;
}

bool ArchiveValueEx(Archival::Archive& ar, const String& name, ExampleEx& ex)
{
    URHO3D_LOGINFO("ONS Successful External Function: " + name);
    ar.Serialize("f",ex.a);
    ar.Serialize("i",ex.b);
    ar.Serialize("s",ex.c);
    return true;
}

}

namespace Archival {
//template<>
bool Archival::Archiver<OtherNamespace::ExampleS&>::ArchiveValue(Archive& ar, const String& name, Archival::Archiver<OtherNamespace::ExampleS&>::T& ex)
{
    URHO3D_LOGINFO("Successful Specialization: " + name);
    ar.Serialize("f",ex.a);
    ar.Serialize("i",ex.b);
    ar.Serialize("s",ex.c);
    return true;
}
}
