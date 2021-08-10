#pragma once

#include "Archive.h"


namespace Urho3D
{
class Material;

using Archival::ArchiveResult;
ArchiveResult<Archive, IntVector2> ArchiveValue(Archive& archive, const String& name, IntVector2& self);
ArchiveResult<Archive, IntVector3> ArchiveValue(Archive& archive, const String& name, IntVector3& self);
ArchiveResult<Archive, Vector2> ArchiveValue(Archive& archive, const String& name, Vector2& self);
/// Archives a Vector3 type
ArchiveResult<Archive, Vector3> ArchiveValue(Archive& archive, const String& name, Vector3& self);
ArchiveResult<Archive, Vector4> ArchiveValue(Archive& archive, const String& name, Vector4& self);
ArchiveResult<Archive, Quaternion> ArchiveValue(Archive& archive, const String& name, Quaternion& self);
ArchiveResult<Archive, Color> ArchiveValue(Archive& archive, const String& name, Color& self);

ArchiveResult<Archive, Matrix3> ArchiveValue(Archive& archive, const String& name, Matrix3& self);
ArchiveResult<Archive, Matrix3x4> ArchiveValue(Archive& archive, const String& name, Matrix3x4& self);
ArchiveResult<Archive, Matrix4> ArchiveValue(Archive& archive, const String& name, Matrix4& self);

ArchiveResult<Archive, Material> ArchiveValue(Archive& archive, const String& name, Material& self);

ArchiveResult<Archive, Rect> ArchiveValue(Archive& archive, const String& name, Rect& self);


/// Overload to ArchiveValue for Vector. Erases elements that fail to serialize on input.
template<typename T>
ArchiveResult<Archive, Vector<T>> ArchiveValue(Archive& ar, const String& name, Vector<T>&& vector)
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
