#include "ImGuiBackend.h"

#include "Archive.h"

#include "imgui_UrhoString.h"

#include "SystemUI/SystemUI.h"

#include "virtualGizmo3D/imGuIZMOquat.h"

#include "ImPlot/implot.h"

inline namespace Archival {

namespace Detail {

struct ImGuiID
{
    ImGuiID(int id) { ImGui::PushID(id); }
    ImGuiID(unsigned id) { ImGui::PushID(static_cast<int>(id)); }
    ImGuiID(const char* id) { ImGui::PushID(id); }
    ~ImGuiID() { ImGui::PopID(); }
};

int ImGuiBackend::globalTreeDepth_ = -1;
Vector<String> ImGuiBackend::lastTreeNames_;
Vector<int> ImGuiBackend::lastTreeSeriesEntries_;

static StringVector ids;

ImGuiBackend::ImGuiBackend(const String &groupName, unsigned treeDepth, int seriesEntry)
    : seriesEntry_(seriesEntry), myTreeDepth_(treeDepth), myTreeName_(groupName)
{
    if (myTreeDepth_ == 0)//!windowName_.Empty())
    {
//        if (lastTreeNames_.Empty())
//        {
//            ImGui::PushID(myTreeName_.CString());
//            lastTreeNames_.Push(myTreeName_);
//        }
        ImGui::Begin(myTreeName_.CString());
    }
    BeginValue();
    EndValue();
}

ImGuiBackend::ImGuiBackend(const String &windowName): ImGuiBackend(windowName, 0, INVALID_SERIES_ENTRY)//windowName_(windowName)
{
//    if (!windowName_.Empty())
//        ImGui::Begin(windowName_.CString());
}

ImGuiBackend::~ImGuiBackend()
{
    // A bit of cleaning up.
    if (myTreeDepth_ == 0)//!windowName_.Empty())
    {
        while (lastTreeNames_.Size()) {
            if (lastTreeSeriesEntries_.Back() != INVALID_SERIES_ENTRY)
                PopID();
            lastTreeSeriesEntries_.Pop();
            lastTreeNames_.Pop();
            PopID();
        }
        ImGui::End();
    }
}

void ImGuiBackend::RegisterBackend(Urho3D::Context *context)
{
    using namespace Urho3D;
    // SystemUI subsytem need to be registered after engine initialization
    if (!context->GetSubsystem<SystemUI>())
    {
        context->RegisterSubsystem(new SystemUI(context));
        context->GetSubsystem<SystemUI>()->Start();
    }

}

bool ImGuiBackend::SaveButton(String &filename, const String &button, bool withSeparator)
{
//    ImGuiCol_Text,
//    ImGuiCol_TextDisabled,
//    ImGuiCol_FrameBg,               // Background of checkbox, radio button, plot, slider, text input
//    ImGuiCol_FrameBgHovered,
//    ImGuiCol_FrameBgActive,
    ImGui::PushID("Button-FileName");
    if (withSeparator)
        ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(0.0f, 0.2f, 0.2f));
    ImGui::InputText("",&filename);
    ImGui::PopStyleColor();
    ImGui::PopID();
    ImGui::SameLine();
    auto hue = 0.0f;
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(hue, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 0.8f, 0.8f));
    bool pushed = ImGui::Button(button.CString());
    ImGui::PopStyleColor(3);
    return pushed;
}

bool ImGuiBackend::Graph(const String& name, const float *pts, unsigned count, unsigned dataStride)
{
    ImGui::PlotLines(name.CString(),pts,static_cast<int>(count),0, NULL,FLT_MAX, FLT_MAX, ImVec2(0, 0), dataStride);

    return true;
}

bool ImGuiBackend::Graph(const String &name, const float *x_data, const float *y_data, unsigned count, unsigned dataStride)
{
    if (ImPlot::BeginPlot(name.CString())) {
//        ImPlot::PlotBars("My Bar Plot", bar_data, 11);
        ImPlot::PlotLine(name.CString(), x_data, y_data, count, 0, dataStride);
        ImPlot::EndPlot();
    }

    return true;
}

bool ImGuiBackend::GraphMulti(const String &graphName, const StringVector &lineLabels, const Vector<PODVector<Vector2>> &pts)
{
    if (ImPlot::BeginPlot(graphName.CString())) {
        unsigned i = 0;
        for (auto&& pts : pts)
        {
            String line = lineLabels.Size() > i ? lineLabels[i] : String(i);

            ImPlot::PlotLine(line.CString(), (ImVec2*)&pts[0], pts.Size());
        }
        ImPlot::EndPlot();
    }
    return true;
}

void ImGuiBackend::TestImGuiUpdate()
{
    // Show ImGui test window
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ImGui::ShowDemoWindow();/*
return;*/
    // Create some ImGui controls to manage cube rotation
    static float axis[] = { 0.0f, 10.0f, 0.0f };
    ImGui::SetNextWindowSize(ImVec2(200,200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Cube rotation speed");
    ImGui::SliderFloat3("Axis", axis, -200.0f, 200.0f);
    ImGui::End();


    ImGui::SliderFloat3("Axis-no begin end", axis, -200.0f, 200.0f);


    ImGui::Begin("Begin 1");
    ImGui::Begin("Begin 1");
    static vgm::Quat q{1,0,0,0};
    vgm::Quat tmp = normalize(q);
//    ImGui::gizmo3D("quaternion", tmp);
//    q = tmp;
//    ImGui::SameLine();
    if(ImGui::gizmo3D("##aaa", tmp, 50))  {
//        tLight = -lL;
        q = tmp;
    }
    ImGui::SliderFloat3("Axis", axis, -200.0f, 200.0f);
    ImGui::End();
    ImGui::End();
}

void ImGuiBackend::TestBroken()
{
    ImGui::Begin("Test Strides 3");

    static Vector3 v;
    for (int i = 0; i < 3; ++i)
    {
        auto name = ToString("Group%d",i);
        ImGui::CollapsingHeader(name.CString(), ImGuiTreeNodeFlags_DefaultOpen);
        float& val = (&v.x_)[i];
        ImGui::PushID(name.CString());
        ImGui::DragFloat("test-val", &val, 0.1f);
        ImGui::PopID();
    }
    ImGui::End();
}

Backend *ImGuiBackend::CreateGroup(const String &name, bool isInput)
{
    if (!isInput)
        return nullptr;

    if (name == InlineName())
        return new ImGuiBackend(name, myTreeDepth_+1, seriesEntry_);
    else
    {
        // THIS DOES NOT WORK WITH HOW I'M DOING IT NOW WITH THE VALUES
//        ImGuiID raii(seriesEntry_);
        BeginValue(); // So we'll try this instead.
        bool open = ImGui::CollapsingHeader(name.CString(), ImGuiTreeNodeFlags_DefaultOpen);
        EndValue();
        if (open)
            return new ImGuiBackend(name, myTreeDepth_+1, seriesEntry_);
        else
            return new NoOpBackend();
    }

}

Backend *ImGuiBackend::CreateSeriesEntry(const String &name, bool isInput)
{
    if (!entries_.Contains(name))
        entries_[name] = 0;
    else
        entries_[name] += 1;

//    if (name == InlineName())
//        return Archive(isInput, new ImGuiBackend(String::EMPTY, entries_[name]));
//    else if (entries_[name] > 0 || ImGui::CollapsingHeader(name.CString()))
//        return Archive(isInput, new ImGuiBackend(String::EMPTY, entries_[name]));
//    else
//        return Archive(isInput, new NoOpBackend());

//    if (name == InlineName())
//        return Archive(isInput, new ImGuiBackend(String::EMPTY, entries_[name]));

    if (lastSeriesName_ != name)
    {
        lastSeriesName_ = name;
        lastSeriesOpen_ = ImGui::CollapsingHeader(name.CString(), ImGuiTreeNodeFlags_DefaultOpen);
    }

    if (lastSeriesOpen_)
    {
        ImGui::TextColored({1,0,0,1}, "%s[%d]",name.CString(),entries_[name]);

        ImGui::SameLine();

        auto hue = 0.0f;
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(hue, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 0.8f, 0.8f));
        BeginValue();
        ImGui::PushID(name.CString());
        ImGui::PushID(entries_[name]);
        bool shouldClose = ImGui::Button("X");
        if (shouldClose)
            AddHint(Hint::OUTPUT_SHOULD_CLOSE,true);
        else
            RemoveHint(Hint::OUTPUT_SHOULD_CLOSE);
        ImGui::PopID();
        ImGui::PopID();
        EndValue();
        ImGui::PopStyleColor(3);

        if (shouldClose)
            return nullptr;
        return new ImGuiBackend(name, myTreeDepth_+1, entries_[name]);
    }
    else
        return nullptr;
}

bool ImGuiBackend::GetSeriesSize(const String &name, unsigned &size)
{
    if (lastSeriesName_ != name)
    {
        lastSeriesName_ = name;
        lastSeriesOpen_ = ImGui::CollapsingHeader(ToString("%s <%d>",name.CString(), size).CString(), ImGuiTreeNodeFlags_DefaultOpen);

        if (lastSeriesOpen_)
        {
            BeginValue();
            ImGui::PushID(name.CString());
//            ImGui::PushID(seriesEntry_);

            float hue = 1.f/3;
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(hue, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 0.8f, 0.8f));
            if (ImGui::Button("+"))
                size += 1;
            ImGui::PopStyleColor(3);

            if (size > 0)
            {
                ImGui::SameLine();

                hue = 0;
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(hue, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 0.8f, 0.8f));
                if (ImGui::Button("Delete Last"))
                    if (size > 0)
                        size -= 1;
                ImGui::SameLine(0,25);
                if (ImGui::Button("Clear All"))
                    size = 0;
                ImGui::PopStyleColor(3);
            }

//            ImGui::PopID();
            ImGui::PopID();
            EndValue();
        }
    }

    return lastSeriesOpen_;
}

bool ImGuiBackend::Get(const String &name, const std::nullptr_t &)
{
    BeginValue();
    ImGui::TextDisabled("%s (null)",name.CString());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, bool &val)
{
    BeginValue();
    ImGui::Checkbox(name.CString(), &val);
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, unsigned char &val)
{
    BeginValue();
    unsigned extended = val;
    unsigned char min = 0, max = 0xff;
    ImGui::DragScalar(name.CString(), ImGuiDataType_U32, &extended, GetSpeedHint(), &min, &max);
    val = static_cast<unsigned char>(extended);
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, signed char &val)
{
    BeginValue();
    static int extended;
    extended = val;
    using type = std::decay<decltype(val)>::type;
    type min = std::numeric_limits<type>::min(), max = std::numeric_limits<type>::max();
    ImGui::DragScalar(name.CString(), ImGuiDataType_S32, &extended, GetSpeedHint(), &min, &max);
    val = static_cast<signed char>(extended);
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, unsigned short &val)
{
    BeginValue();
    static unsigned extended;
    extended = val;
    unsigned short min = 0, max = 0xffff;
    ImGui::DragScalar(name.CString(), ImGuiDataType_U32, &extended, GetSpeedHint(), &min, &max);
    val = static_cast<unsigned short>(extended);
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, signed short &val)
{
    BeginValue();
    static int extended;
    extended = val;
    using type = std::decay<decltype(val)>::type;
    type min = std::numeric_limits<type>::min(), max = std::numeric_limits<type>::max();
    ImGui::DragScalar(name.CString(), ImGuiDataType_S32, &extended, GetSpeedHint(), &min, &max);
    val = static_cast<short>(extended);
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, unsigned int &val)
{
    BeginValue();
    ImGui::DragScalar(name.CString(), ImGuiDataType_U32, &val, GetSpeedHint());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, signed int &val)
{
    BeginValue();
    ImGui::DragScalar(name.CString(), ImGuiDataType_S32, &val, GetSpeedHint());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, unsigned long long &val)
{
    BeginValue();
    ImGui::DragScalar(name.CString(), ImGuiDataType_U64, &val, GetSpeedHint());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, signed long long &val)
{
    BeginValue();
    ImGui::DragScalar(name.CString(), ImGuiDataType_S64, &val, GetSpeedHint());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, float &val)
{
    if (name.Contains("test-val"))
    {
        String s;
        for (auto i : ids)
            s += ", " + i;
        URHO3D_LOGERROR(s);
    }
    BeginValue();
    if (name.Contains("test-val"))
    {
        String s;
        for (auto i : ids)
            s += ", " + i;
        URHO3D_LOGERROR(s);
    }
    // Assert fails with infinite bounds.
//    ImGui::SliderFloat(name.CString(), &val, -M_INFINITY, M_INFINITY);

//    ImGui::SliderFloat(name.CString(), &val, -100, 100);
//    ImGui::DragFloat(name.CString(), &val, 0.1f, -100, 100);
    ImGui::DragFloat(name.CString(), &val, 0.1f);


//    ImGui::SetNextWindowSize(ImVec2(200,200), ImGuiCond_FirstUseEver);
//    ImGui::Begin("Cube rotation speed");
////    ImGui::SliderFloat3("Axis", axis, -200.0f, 200.0f);

//    ImGui::DragFloat(name.CString(), &val, 0.1f);
    //    ImGui::End();

    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, double &val)
{
    BeginValue();
    ImGui::DragScalar(name.CString(), ImGuiDataType_Double, &val, GetSpeedHint());
//    ImGui::InputDouble(name.CString(), &val);
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, String &val)
{
    BeginValue();

    do // so we can break out of the loop we'll do {} while (false)
    {
        if (const Hint& h = GetHint(Hint::SUGGESTED_OPTIONS))
        {
            if (h.value.GetType() != VAR_VARIANTVECTOR)
                break;
            auto& values = h.value.GetVariantVector();
            int current = 0;
            Variant varCurrentVal{val};
//            for (auto& var : *values)
//                if (varCurrentVal == var)
//                    break;
//                else
//                    ++current;

            /// TODO: handle the Suggested options case
            if (current == values.Size())
                break;

            if (ImGui::BeginCombo(name.CString(), val.CString(), 0)) // The second parameter is the label previewed before opening the combo.
            {
                for (unsigned n = 0; n < values.Size(); n++)
                {
                    bool is_selected = (varCurrentVal == values[n]);
                    const String& item = values[n].GetString();
                    if (ImGui::Selectable(item.CString(), is_selected))
                        val = item;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                }
                ImGui::EndCombo();
            }

            EndValue();
            return true;
        }
    } while (false);

    ImGui::InputText(name.CString(), &val);
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, IntVector2 &val)
{
    BeginValue();
    ImGui::DragInt2(name.CString(), &val.x_, GetSpeedHint());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, IntVector3 &val)
{
    BeginValue();
    ImGui::DragInt3(name.CString(), &val.x_, GetSpeedHint());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, Vector2 &val)
{
    BeginValue();
    ImGui::DragFloat2(name.CString(), &val.x_, GetSpeedHint());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, Vector3 &val)
{
    BeginValue();
    ImGui::DragFloat3(name.CString(), &val.x_, GetSpeedHint());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, Vector4 &val)
{
    BeginValue();
    ImGui::DragFloat4(name.CString(), &val.x_, GetSpeedHint());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, Quaternion &val)
{
    BeginValue();

    // Create 3d gizmo to edit the quaternion
    vgm::Quat tmp(val.w_,val.x_, val.y_, val.z_);
    tmp = normalize(tmp);
    if(ImGui::gizmo3D((name + "-Gizmo-" + name).CString(), tmp))  {
        val.w_ = tmp.w;
        val.x_ = tmp.x;
        val.y_ = tmp.y;
        val.z_ = tmp.z;
    }

//    // Add raw edits to the right of the edit gizmo
//    ImGui::SameLine();
//    ImGui::BeginGroup();

//    // Angle axis view and XYZW value view
//    Vector4 angleAxes(Equals(val.w_ * val.w_, 1.f) ? Vector3::UP : val.Axis(), val.Angle());
//    ImGui::DragFloat4((name + "-Axis Angle").CString(), &angleAxes.x_, GetSpeedHint());
//    val.FromAngleAxis(val.w_, {val.x_,val.y_,val.z_});

//    ImGui::DragFloat4((name + "-XYZW").CString(), &val.w_, GetSpeedHint());
//    val.Normalize();

//    // Right hand side end
//    ImGui::EndGroup();

    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, Color &val)
{
    BeginValue();
    ImGui::ColorEdit4(name.CString(), &val.r_, ImGuiColorEditFlags_RGB
                      | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, Matrix3 &val)
{
    BeginValue();
    ImGui::DragFloat3((name + ":R0").CString(), &val.m00_, GetSpeedHint());
    ImGui::DragFloat3((name + ":R1").CString(), &val.m10_, GetSpeedHint());
    ImGui::DragFloat3((name + ":R2").CString(), &val.m20_, GetSpeedHint());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, Matrix3x4 &val)
{
    BeginValue();
    ImGui::DragFloat4((name + ":R0").CString(), &val.m00_, GetSpeedHint());
    ImGui::DragFloat4((name + ":R1").CString(), &val.m10_, GetSpeedHint());
    ImGui::DragFloat4((name + ":R2").CString(), &val.m20_, GetSpeedHint());
    EndValue();
    return true;
}

bool ImGuiBackend::Get(const String &name, Matrix4 &val)
{
    BeginValue();
    ImGui::DragFloat4((name + ":R0").CString(), &val.m00_, GetSpeedHint());
    ImGui::DragFloat4((name + ":R1").CString(), &val.m10_, GetSpeedHint());
    ImGui::DragFloat4((name + ":R2").CString(), &val.m20_, GetSpeedHint());
    ImGui::DragFloat4((name + ":R3").CString(), &val.m30_, GetSpeedHint());
    EndValue();
    return true;
}

bool Archival::Detail::ImGuiBackend::WriteConditional(bool condition, bool isInput)
{
    return condition;
}

void ImGuiBackend::PushID(const String &id)
{
    ids.Push(id);
    ImGui::PushID(id.CString());
}

void ImGuiBackend::PushID(unsigned id)
{
    ids.Push(String(id));
    ImGui::PushID(id);
}

void ImGuiBackend::PopID()
{
    ids.Pop();
    ImGui::PopID();
}

void ImGuiBackend::BeginValue()
{
    if (myTreeDepth_+1 < (int)lastTreeNames_.Size())
    {
        while (myTreeDepth_+1 < (int)lastTreeNames_.Size())
        {
            if (lastTreeSeriesEntries_.Back() != INVALID_SERIES_ENTRY)
                PopID();
            PopID();
            globalTreeDepth_--;
            lastTreeSeriesEntries_.Pop();
            lastTreeNames_.Pop();
        }
    }

    if (myTreeDepth_+1 == (int)lastTreeNames_.Size())
    {
        if (myTreeName_ != lastTreeNames_[myTreeDepth_])
        {
            if (lastTreeSeriesEntries_[myTreeDepth_] != INVALID_SERIES_ENTRY)
                PopID();
            PopID();
            PushID(myTreeName_);
            if (seriesEntry_ != INVALID_SERIES_ENTRY)
                PushID(seriesEntry_);
            lastTreeNames_[myTreeDepth_] = myTreeName_;
            lastTreeSeriesEntries_[myTreeDepth_] = seriesEntry_;
        }
        else if (seriesEntry_ != lastTreeSeriesEntries_[myTreeDepth_])
        {
            PopID();
            if (seriesEntry_ != INVALID_SERIES_ENTRY)
                PushID(seriesEntry_);
        }
    }
    else if (myTreeDepth_+1 > (int)lastTreeNames_.Size())
    {
        assert (myTreeDepth_ == (int)lastTreeNames_.Size());
        PushID(myTreeName_);
        lastTreeNames_.Push(myTreeName_);
        globalTreeDepth_++;
        if (seriesEntry_ != INVALID_SERIES_ENTRY)
            PushID(seriesEntry_);
        lastTreeSeriesEntries_.Push(seriesEntry_);
    }

    assert(myTreeDepth_+1 == (int)lastTreeNames_.Size());

//#warning: This is broken for 2D series sort of arrangements like the Hair Strands Nodes.
//    if (seriesEntry_ != INVALID_SERIES_ENTRY)
//        ImGui::PushID(seriesEntry_);
}

/*
{
    if (myTreeDepth_ < globalTreeDepth_)
    {
        while (myTreeDepth_ < globalTreeDepth_)
        {
            ImGui::PopID();
            globalTreeDepth_--;
            lastTreeNames_.Pop();
        }
    }

    if (myTreeDepth_ == globalTreeDepth_)
    {
        if (myTreeName_ != lastTreeNames_[myTreeDepth_])
        {
            ImGui::PopID();
            ImGui::PushID(myTreeName_.CString());
            lastTreeNames_[myTreeDepth_] = myTreeName_;
        }
    }
    else if (myTreeDepth_ > globalTreeDepth_)
    {
        while (myTreeDepth_ > globalTreeDepth_ + 1)
        {
            ImGui::PushID(lastTreeNames_[globalTreeDepth_+1].CString());
            globalTreeDepth_++;
        }
        ImGui::PushID(myTreeName_.CString());
        lastTreeNames_.Push(myTreeName_);
        globalTreeDepth_++;
    }

    assert(myTreeDepth_+1 == (int)lastTreeNames_.Size());

    if (seriesEntry_ != INVALID_SERIES_ENTRY)
        ImGui::PushID(seriesEntry_);
}*/

void ImGuiBackend::EndValue()
{
//    if (seriesEntry_ != INVALID_SERIES_ENTRY)
//        ImGui::PopID();
}

float ImGuiBackend::GetSpeedHint()
{
    if (auto& h = GetHint(Hint::RESOLUTION_SCALE))
    {
        float speed = h.value.GetFloat() * 0.1f;
        if (speed <= M_EPSILON)
            speed = 0.1f;
        return speed;
    }
    return 0.1f;
}


}

}

