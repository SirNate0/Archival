#pragma once

#include "Archive.h"


#include <Urho3D/Container/Str.h>

namespace Urho3D {

struct Example
{
    float a;
    int b;
    String c;

    bool ArchiveValue(future::ArchiveExample& ar, const String& name);
};


struct ExampleEx
{
    float a;
    int b;
    String c;
};

inline bool ArchiveValueEx(future::ArchiveExample& ar, const String& name, ExampleEx& ex)
{
    URHO3D_LOGINFO("Successful External Function: " + name);
    ar.Serialize("f",ex.a);
    ar.Serialize("i",ex.b);
    ar.Serialize("s",ex.c);
    return true;
}


struct ExampleS
{
    float a;
    int b;
    String c;
};

}

namespace future
{

template <>
struct Archiver<Urho3D::ExampleS>
{
    using T = Urho3D::ExampleS;
    static bool ArchiveValue(ArchiveExample& ar, const String& name, T& ex);
};
}

namespace OtherNamespace
{

struct Example
{
    float a;
    int b;
    String c;

    bool ArchiveValue(future::ArchiveExample& ar, const String& name);
};


struct ExampleEx
{
    float a;
    int b;
    String c;
};

bool ArchiveValueEx(future::ArchiveExample& ar, const String& name, ExampleEx& ex);


struct ExampleS
{
    float a;
    int b;
    String c;
};
}


namespace future
{
template <>
struct Archiver<OtherNamespace::ExampleS>
{
    using T = OtherNamespace::ExampleS;
    static bool ArchiveValue(ArchiveExample& ar, const String& name, T& ex);
};
}

namespace Urho3D
{

void TestArchive();

}
