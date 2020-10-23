
#define private public
#include <Urho3D/Graphics/Material.h>

#include "ArchiveUrhoTypes.h"

namespace Urho3D
{

ArchiveResult<Archive, IntVector2> ArchiveValue(Archive &archive, const String &name, IntVector2 &self)
{
    if (Archival::ArchiveValue<Archive, IntVector2>(archive, name, self))
        return {archive, true, self};

    bool good = true;
    auto ar = archive.CreateGroup(name);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.x_).Else("x",self.x_))
        ar.Serialize("x", self.x_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.y_).Else("y",self.y_))
        ar.Serialize("y", self.y_);
    return {archive, good, self};
}

ArchiveResult<Archive, Urho3D::IntVector3> ArchiveValue(Archive &archive, const String &name, IntVector3 &self)
{
    if (Archival::ArchiveValue<Archive, IntVector3>(archive, name, self))
        return {archive, true, self};

    bool good = true;
    auto ar = archive.CreateGroup(name);
//    unsigned sz = 3;
//    ar.SerializeSeriesSize("value", sz);
//    if (sz != 3)
//        URHO3D_LOGERROR("Failures");
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.x_).Else("x",self.x_))
        ar.Serialize("x", self.x_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.y_).Else("y",self.y_))
        ar.Serialize("y", self.y_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.z_).Else("z",self.z_))
        ar.Serialize("z", self.z_);
    return {archive, good, self};
}

ArchiveResult<Archive, Vector2> ArchiveValue(Archive &archive, const String &name, Vector2 &self)
{
    if (Archival::ArchiveValue<Archive, Vector2>(archive, name, self))
        return {archive, true, self};

    bool good = true;
    auto ar = archive.CreateGroup(name);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.x_).Else("x",self.x_))
        ar.Serialize("x", self.x_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.y_).Else("y",self.y_))
        ar.Serialize("y", self.y_);
    return {archive, good, self};
}

ArchiveResult<Archive, Urho3D::Vector3> ArchiveValue(Archive &archive, const String &name, Vector3 &self)
{
    if (Archival::ArchiveValue<Archive, Vector3>(archive, name, self))
        return {archive, true, self};

    bool good = true;
    auto ar = archive.CreateGroup(name);
//    unsigned sz = 3;
//    ar.SerializeSeriesSize("value", sz);
//    if (sz != 3)
//        URHO3D_LOGERROR("Failures");
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.x_).Else("x",self.x_))
        ar.Serialize("x", self.x_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.y_).Else("y",self.y_))
        ar.Serialize("y", self.y_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.z_).Else("z",self.z_))
        ar.Serialize("z", self.z_);
    return {archive, good, self};
}

ArchiveResult<Archive, Vector4> ArchiveValue(Archive &archive, const String &name, Vector4 &self)
{
    if (Archival::ArchiveValue<Archive, Vector4>(archive, name, self))
        return {archive, true, self};

    bool good = true;
    auto ar = archive.CreateGroup(name);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.x_).Else("x",self.x_))
        ar.Serialize("x", self.x_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.y_).Else("y",self.y_))
        ar.Serialize("y", self.y_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.z_).Else("z",self.z_))
        ar.Serialize("z", self.z_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.z_).Else("w",self.z_))
        ar.Serialize("w", self.z_);
    return {archive, good, self};
}

ArchiveResult<Archive, Quaternion> ArchiveValue(Archive& archive, const String& name, Quaternion& self)
{

    if (Archival::ArchiveValue<Archive, Quaternion>(archive, name, self))
        return {archive, true, self};

    bool good = true;
    auto ar = archive.CreateGroup(name);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.z_).Else("w",self.z_))
        ar.Serialize("w", self.z_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.x_).Else("x",self.x_))
        ar.Serialize("x", self.x_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.y_).Else("y",self.y_))
        ar.Serialize("y", self.y_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.z_).Else("z",self.z_))
        ar.Serialize("z", self.z_);
    return {archive, good, self};
}

ArchiveResult<Archive, Color> ArchiveValue(Archive &archive, const String &name, Color &self)
{
    if (Archival::ArchiveValue<Archive, Color>(archive, name, self))
        return {archive, true, self};

    bool good = true;
    auto ar = archive.CreateGroup(name);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.r_).Else("r",self.r_))
        ar.Serialize("r", self.r_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.g_).Else("g",self.g_))
        ar.Serialize("g", self.g_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.b_).Else("b",self.b_))
        ar.Serialize("b", self.b_);
    if (!ar.CreateSeriesEntryInline().SerializeInline(self.a_).Else("a",self.a_))
        ar.Serialize("a", self.a_);
    return {archive, good, self};
}

ArchiveResult<Archive, Matrix3> ArchiveValue(Archive &archive, const String &name, Matrix3 &self)
{
    if (Archival::ArchiveValue<Archive, Matrix3>(archive, name, self))
        return {archive, true, self};

    const char* names[] = {
            "m00","m01","m02"
            "m10","m11","m12"
            "m20","m21","m22"};

    bool good = true;
    auto ar = archive.CreateGroup(name);
    constexpr auto ROWS = 3;
    constexpr auto COLUMNS = 3;
    for (unsigned r = 0; r < ROWS; ++r)
    {
        auto row = ar.CreateSeriesEntryInline();

        for (unsigned c = 0; c < COLUMNS; ++c)
        {
            auto i = r * ROWS + c;
            auto name = names[i];
            float& val = (&self.m00_)[i];
            if (!row.CreateSeriesEntryInline().SerializeInline(val).Else(name,val))
                if (!row.Serialize(name, val))
                    ar.Serialize(name, val);
        }

    }
    return {archive, good, self};
}

ArchiveResult<Archive, Matrix3x4> ArchiveValue(Archive &archive, const String &name, Matrix3x4 &self)
{
    if (Archival::ArchiveValue<Archive, Matrix3x4>(archive, name, self))
        return {archive, true, self};

    const char* names[] = {
            "m00","m01","m02","m03",
            "m10","m11","m12","m13",
            "m20","m21","m22","m23"};

    bool good = true;
    auto ar = archive.CreateGroup(name);
    constexpr auto ROWS = 3;
    constexpr auto COLUMNS = 4;
    for (unsigned r = 0; r < ROWS; ++r)
    {
        auto row = ar.CreateSeriesEntryInline();

        for (unsigned c = 0; c < COLUMNS; ++c)
        {
            auto i = r * ROWS + c;
            auto name = names[i];
            float& val = (&self.m00_)[i];
            if (!row.CreateSeriesEntryInline().SerializeInline(val).Else(name,val))
                if (!row.Serialize(name, val))
                    ar.Serialize(name, val);
        }

    }
    return {archive, good, self};
}

ArchiveResult<Archive, Matrix4> ArchiveValue(Archive &archive, const String &name, Matrix4 &self)
{
    if (Archival::ArchiveValue<Archive, Matrix4>(archive, name, self))
        return {archive, true, self};

    const char* names[] = {
            "m00","m01","m02","m03",
            "m10","m11","m12","m13",
            "m20","m21","m22","m23",
            "m30","m31","m32","m33"};

    bool good = true;
    auto ar = archive.CreateGroup(name);
    constexpr auto ROWS = 4;
    constexpr auto COLUMNS = 4;
    for (unsigned r = 0; r < ROWS; ++r)
    {
        auto row = ar.CreateSeriesEntryInline();

        for (unsigned c = 0; c < COLUMNS; ++c)
        {
            auto i = r * ROWS + c;
            auto name = names[i];
            float& val = (&self.m00_)[i];
            if (!row.CreateSeriesEntryInline().SerializeInline(val).Else(name,val))
                if (!row.Serialize(name, val))
                    ar.Serialize(name, val);
        }

    }
    return {archive, good, self};
}


#if 0

static_assert (sizeof(SharedPtr<Material>)==sizeof(Material*), "Shared Ptr Size differs");

static const char* textureUnitNames[10];
TextureUnit ParseTextureUnitName(String name);
ArchiveResult<Archive, Material> ArchiveValue(Archive &archive, const String &name, Material &self)
{
    auto ar = archive.CreateGroup(name);
    auto shader = ar.CreateGroup("shader");
    shader.Serialize("vsdefines", self.vertexShaderDefines_);
    shader.Serialize("psdefines", self.pixelShaderDefines_);

    auto techniquesGroup = ar.GetBackend().GetBackendName() == "XML" ? "technique" : "techniques";

    ar.SerializeSeriesSize(techniquesGroup, self.techniques_);
    for (unsigned i = 0; i < self.techniques_.Size(); ++i)
    {
        auto techGroup = ar.CreateSeriesEntry(techniquesGroup);
        auto techEntry = self.techniques_[i];

        techGroup.Serialize("name", GetSet([&](){return techEntry.technique_->GetName();},
        [&](const String& name){
            auto technique = self.GetSubsystem<ResourceCache>()->GetResource(name);
            techEntry.technique_ = techEntry.original_ = technique;
        }));
        if (techEntry.technique_)
        {
            techGroup.Serialize("quality", techEntry.qualityLevel_);
            techGroup.Serialize("loddistance", techEntry.lodDistance_);
        }
        else
        {
            // We have already logged the error through the resource cache about not finding the technique.
            self.techniques_.Erase(i);
            --i;
        }
    }
    if (ar.IsInput())
    {
        self.SortTechniques();
        self.ApplyShaderDefines();
    }
    auto texturesGroup = ar.GetBackend().GetBackendName() == "XML" ? "texture" : "textures";


    if (ar.GetBackend().GetBackendName() == "XML")
    {
        unsigned size = self.textures_.Size();
        ar.SerializeSeriesSize(techniquesGroup, size);
        auto iterator = self.textures_.Begin();
        for (unsigned i = 0; i < size; ++i)
        {
            auto texGroup = ar.CreateSeriesEntry(texturesGroup);
            TextureUnit unit = ar.IsInput() ? iterator->first_ : TU_DIFFUSE;
            Texture* tex = ar.IsInput() ? iterator->second_ : nullptr;
            String unitName = textureUnitNames[(int)unit];
            // Cannot serialize as a normal enum because of the shorthand names
            VariantVector options;
            textureUnitNames;
            //TODO: add texture names to the options. Do it statically.
            ar.Hint(Detail::Hint::SUGGESTED_OPTIONS, options);
            ar.Serialize("unit", unitName);
            ar.ClearHints();
            if (ar.IsInput())
            {
                unit = ParseTextureUnitName(unitName);
            }
            if (unit < MAX_TEXTURE_UNITS)
            {
                ar.Serialize("name", GetSet([&](){return tex->GetName();},[&](const String& name){
                    auto cache = self.GetSubsystem<ResourceCache>();

                    // Detect cube maps and arrays by file extension: they are defined by an XML file
                    if (GetExtension(name) == ".xml")
                    {
#ifdef DESKTOP_GRAPHICS
                        StringHash type = ParseTextureTypeXml(cache, name);
                        if (!type && unit == TU_VOLUMEMAP)
                            type = Texture3D::GetTypeStatic();

                        if (type == Texture3D::GetTypeStatic())
                            SetTexture(unit, cache->GetResource<Texture3D>(name));
                        else if (type == Texture2DArray::GetTypeStatic())
                            SetTexture(unit, cache->GetResource<Texture2DArray>(name));
                        else
#endif
                            SetTexture(unit, cache->GetResource<TextureCube>(name));
                    }
                    else
                        SetTexture(unit, cache->GetResource<Texture2D>(name));
                }));
            }
        }
    }


}
#endif
}
