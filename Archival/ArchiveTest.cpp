#include "ArchiveTest.h"

namespace Urho3D {

bool Example::ArchiveValue(Archival::Archive& ar, const Urho3D::String& name)
{
    URHO3D_LOGINFO("Successful Method: " + name);
    ar.Serialize("f",a);
    ar.Serialize("i",b);
    ar.Serialize("s",c);
    return true;
}

float ExampleGS::getA() const
{
    URHO3D_LOGINFO("Got A");
    return a;
}

void ExampleGS::setA(float value)
{
    URHO3D_LOGINFO("Set A = " + String(value));
    a = value;
}

int ExampleGS::getB() const
{
    URHO3D_LOGINFO("Got B");
    return b;
}

void ExampleGS::setB(int value)
{
    URHO3D_LOGINFO("Set B = " + String(value));
    b = value;
}

String ExampleGS::getC() const
{
    URHO3D_LOGINFO("Got C");
    return c;
}

void ExampleGS::setC(const String& value)
{
    URHO3D_LOGINFO("Set C = " + value);
    c = value;
}

}

namespace Archival {

//template<>
bool Archiver<Urho3D::ExampleS&>::ArchiveValue(Archival::Archive& ar, const String& name, Archival::Archiver<Urho3D::ExampleS&>::T& ex)
{
    URHO3D_LOGINFO("Successful Specialization: " + name);
    ar.Serialize("f",ex.a).Else("f2");
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
    Archival::Archive archive{input};

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
        using namespace Archival;
        ExampleGS exgs;
        archive.Serialize("function+GS",exgs);

        ExampleEnum exenum{ExampleEnum::AAAAH};
        archive.Serialize("specialization+enum",exenum);

        archive.Serialize("method+enum+DEFAULT",WithDefault(exenum,ExampleEnum::AAAAH));

    }

    // Test extended features (groups and series)
    {
        Vector3 vec{1,2,3};
        String FIN = "FIN";
        archive.Serialize("vec",vec.x_,vec.y_,vec.z_,FIN);
    }
}
}
