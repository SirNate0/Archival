#include "ArchiveTest.h"

namespace Urho3D {

bool Example::ArchiveValue(future::ArchiveExample& ar, const Urho3D::String& name)
{
    URHO3D_LOGINFO("Successful Method: " + name);
    ar.Serialize("f",a);
    ar.Serialize("i",b);
    ar.Serialize("s",c);
    return true;
}

String ExampleGS::getC() const
{
    return c;
}

void ExampleGS::setC(const String& value)
{
    c = value;
}

float ExampleGS::getA() const
{
    URHO3D_LOGINFO("Got A");
    return a;
}

void ExampleGS::setA(float value)
{
    URHO3D_LOGINFO("Set A");
    a = value;
}

}

namespace future {

//template<>
bool Archiver<Urho3D::ExampleS&>::ArchiveValue(future::ArchiveExample& ar, const String& name, future::Archiver<Urho3D::ExampleS&>::T& ex)
{
    URHO3D_LOGINFO("Successful Specialization: " + name);
    ar.Serialize("f",ex.a);
    ar.Serialize("i",ex.b);
    ar.Serialize("s",ex.c);
    return true;
}

}


namespace Urho3D
{
void TestArchive()
{
    static bool input = false;
    input = !input;
    future::ArchiveExample archive;
    archive.isInput = input;

    // Test in this namespace
    {
        Example ex;
        ex.a = 1;
        ExampleEx exex;
        ExampleS exs;
        archive.Serialize("method",ex);
        archive.Serialize("function",exex);
        archive.Serialize("specialization",exs);
    }
    // Test in another namespace
    {
        OtherNamespace::Example ex;
        OtherNamespace::ExampleEx exex;
        OtherNamespace::ExampleS exs;
        archive.Serialize("ONS.method",ex);
        archive.Serialize("ONS.function",exex);
        archive.Serialize("ONS.specialization",exs);
    }

    // Test the Utils
    {
        ExampleGS exgs;
        archive.Serialize("function+GS",exgs);

        ExampleEnum exenum{ExampleEnum::AAAAH};
        archive.Serialize("specialization+enum",exenum);

    }
}
}
