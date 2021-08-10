#include "Archive.h"

inline namespace Archival {

}

namespace Urho3D
{
void TestArchive()
{
    static bool input = false;
    input = !input;
    future::ArchiveExample archive;
    archive.isInput = input;
    Example ex;
    ex.a = 1;
    ExampleEx exex;
    ExampleS exs;
    archive.Serialize("method",ex);
    archive.Serialize("function",exex);
    archive.Serialize("specialization",exs);
    {
        OtherNamespace::Example ex;
        OtherNamespace::ExampleEx exex;
        OtherNamespace::ExampleS exs;
        archive.Serialize("ONS.method",ex);
        archive.Serialize("ONS.function",exex);
        archive.Serialize("ONS.specialization",exs);
    }

}
}
