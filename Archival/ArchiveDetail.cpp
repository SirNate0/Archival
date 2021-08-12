#include <Urho3D/Urho3D.h>

#warning FIXING /home/nathan/Projects/Urho3D/Build/include/Urho3D/Resource/JSONValue.h:257: er ror: ‘Context’ has not been declared void SetVariant(const Variant& variant, Context* context = nullptr);
#include <Urho3D/Core/Context.h>

#include "ArchiveDetail.h"

#include "Archive.h"

namespace Archival
{

namespace Detail {

const Hint Hint::EMPTY_HINT{OUTPUT_NONE, {}, {}};



}

}
