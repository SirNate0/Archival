#pragma once

#include "Archive.h"


namespace Urho3D
{
class Material;

using Archival::ArchiveResult;
using Archival::Archive;
bool ArchiveValueEx(Archive& archive, const String& name, IntVector2& self);
bool ArchiveValueEx(Archive& archive, const String& name, IntVector3& self);
bool ArchiveValueEx(Archive& archive, const String& name, Vector2& self);
/// Archives a Vector3 type
bool ArchiveValueEx(Archive& archive, const String& name, Vector3& self);
bool ArchiveValueEx(Archive& archive, const String& name, Vector4& self);
bool ArchiveValueEx(Archive& archive, const String& name, Quaternion& self);
bool ArchiveValueEx(Archive& archive, const String& name, Color& self);

bool ArchiveValueEx(Archive& archive, const String& name, Matrix3& self);
bool ArchiveValueEx(Archive& archive, const String& name, Matrix3x4& self);
bool ArchiveValueEx(Archive& archive, const String& name, Matrix4& self);

bool ArchiveValueEx(Archive& archive, const String& name, Material& self);

bool ArchiveValueEx(Archive& archive, const String& name, Rect& self);


/// Overload to ArchiveValueEx for Vector. Erases elements that fail to serialize on input.
template<typename T>
bool ArchiveValueEx(Archive& ar, const String& name, Vector<T>&& vector)
{
    if (ar.SerializeSeriesSize(name,vector))
    {
        for (unsigned i = 0; i < vector.Size();)
        {
            auto& x = vector[i];
            auto a = ar.CreateSeriesEntry(name);
            if (!a.SerializeInline(x) && a.IsInput())
                vector.Erase(i);
            else
                ++i;
        }
        return {ar, true, vector};
    }
    return {ar, false, vector};
}

};
