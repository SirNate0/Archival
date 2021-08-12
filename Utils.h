#pragma once

#include <Urho3D/Graphics/Skeleton.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Math/MathDefs.h>

#include <exception>

#include "Archival/Utils.h"

/// Unfinished code exception to be thrown.
struct Unfinished: public std::exception
{
};

inline void PrintBone(Urho3D::Skeleton& sk, Urho3D::Bone* bone, Urho3D::String indent = Urho3D::String::EMPTY)
{
    using namespace Urho3D;

    if (!bone)
        return;

    URHO3D_LOGRAW(indent + String(bone->name_) + "\n");

    if (!bone->node_)
        return;

    for (auto child : bone->node_->GetChildren())
    {
        auto id = sk.GetBoneIndex(child->GetNameHash());
        if (id != M_MAX_UNSIGNED)
            PrintBone(sk,sk.GetBone(id),indent+"-");
    }
    return;
}

inline void PrintSkeleton(Urho3D::Skeleton sk)
{
    using namespace Urho3D;

//    if (!sk)
//    {
//        URHO3D_LOGERROR("Cannot print null skeleton.");
//        return;
//    }

    PrintBone(sk, sk.GetRootBone());
}

/// Rescales the value x at time t lerping from min1 to max1 to a value between min2 and max2
inline float Rescale(float t1, float min1, float max1, float min2 = 0, float max2 = 1)
{
    float t0 = (t1-min1)/(max1-min1);
    float t2 = t0*(max2-min2) + min2;
    return t2;
}

/// Returns an increasing Sawtooth |/|/| wave with period starting at t=0 to t=1 (basically a Modulus clamp on the interval).
inline float SawtoothWave(float t, float min = 0, float max = 1)
{
    return min + (max - min) * Urho3D::AbsMod(t,1.f);
}

/// Emits a triangle wave where t = 0,1 corresponds to min and t = mid corresponds to max
inline float TriangleWave(float t, float min = 0, float max = 1)
{
    // From Wikipedia, 2 * the absolute value of the specified sawtooth.
    return min + (max - min) * 2 * Urho3D::Abs(t - Urho3D::Floor(t + 0.5f));
}

/// Emits a sort-of triangle wave that will go through the min to max point instead of through the excluded midle. Starts at exMin, goes to min, jumps to max, goes to exMax, reverses.
inline float OuterTriangleWave(float t, float excludedMin, float excludedMax, float min = 0, float max = 1)
{
    using Urho3D::Abs;
    auto excluded = Abs(excludedMax - excludedMin);
//    auto excluded = Urho3D::Abs(excludedMax - excludedMin);
//    if (excludedMax < excludedMin)
//        Urho3D::Swap(min,max);
    auto full = Abs(max - min);
    auto region = full - excluded;
//    region *= Urho3D::Sign(excluded);
    region = Urho3D::Abs(region) * Urho3D::Sign(excludedMax - excludedMin);
    return SawtoothWave(TriangleWave(t, excludedMin, excludedMin - region),min,max);
}

/// Smart (periodically) bounded triangle wave that will go from min to max to min through the middle if exmin < max and through the outside if min >= max. Reversing the periodic bounds will reverse which side the wave starts at.
inline float PeriodicTriangleWave(float t, float min, float max, float boundMin = 0, float boundMax = 1)
{
    if (min < max)
        if (boundMin < boundMax)
            return TriangleWave(t, min, max);
        else
            return TriangleWave(t, max, min);
    else
        return OuterTriangleWave(t, min, max, boundMin, boundMax);
}

/// Returns 0 < min, 1 > min, and linear between the two.
inline float ClampRamp(float t, float min, float max)
{
    if (t <= min)
        return 0;
    if (t >= max)
        return 1;
    return (t-min)/(max-min);
}

#include <cmath>

/// Hyperbolic Tangent based sigmoid step. Basically a smooth ClampRamp() that extends to +/- infinity.
inline float Sigmoid(float t, float min, float max)
{
    return Rescale(tanh(Rescale(t,min,max,-1,1)),-1,1,min,max);
}

/// Sigmoid that is min at t=0 and max at t=inf, but close to max at t=max and close to min at t=min.
inline float PositiveSigmoid(float t, float min, float max)
{
    float invT = 1/t;
    float invMax = 1/min;
    float invMin = 1/max;
    float smoothed = Sigmoid(invT, invMin, invMax);
    return 1/smoothed;
}



extern Urho3D::HashMap<Urho3D::String, Urho3D::String> uiStrings;

#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/ResourceCache.h>

namespace Urho3D
{


/// Samples values between min and max with even spacing.
inline PODVector<float> SampleLinspace(unsigned count, float min = 0, float max = 1)
{
    PODVector<float> vals(count);
    auto delta = (max - min) / count;
    for (unsigned i = 0; i < count; ++i)
        vals[i] = min + i*delta;
    return vals;
}

/// Applies the function with specified extra arguments to the provided vector and returns the results
template <class F, class... T>
inline PODVector<float> ApplyTo(const PODVector<float>& pts, const F& fn, T&&... args)
{
    PODVector<float> out(pts.Size());
    for (unsigned i = 0; i < pts.Size(); ++i)
        out[i] = fn(pts[i],std::forward(args)...);
    return out;
}

/// Approximate equals for float based on provided tolerance
inline bool ApproximatelyEqual(float a, float b, float tolerance = 0.001f)
{
    return Abs(a - b) <= tolerance;
}

inline float SmoothStep(float t, float min, float max)
{
    // Scale, bias and saturate x to 0..1 range
    t = Clamp((t - min) / (max - min), 0.0f, 1.0f);
    // Evaluate polynomial
    return t * t * (3 - 2 * t);
}

inline float InverseSmoothStep(float x) {
  return 0.5 - Sin(Asin(1.0 - 2.0 * x) / 3.0);
}


// Returns binomial coefficient without explicit use of factorials,
// which can't be used with negative integers
inline int PascalTriangle(int a, int b) {
  int result = 1;
  for (int i = 0; i < b; ++i)
    result *= (a - i) / (i + 1);
  return result;
}

// Generalized smoothstep
inline float GeneralSmoothStep(int N, float x) {
  x = Clamp(x, 0.f, 1.f); // x must be equal to or between 0 and 1
  float result = 0;
  for (int n = 0; n <= N; ++n)
    result += PascalTriangle(-N - 1, n) *
              PascalTriangle(2 * N + 1, N - n) *
              pow(x, N + n + 1);
  return result;
}



inline void AddLabel(DebugRenderer* dr, const Vector3& position, const String& name, const String& text, const Color& color, bool dynamicText){
    auto* ui=dr->GetSubsystem<UI>();

    auto labels = ui->GetRoot()->GetChild("LABELS",false);
    if (!labels)
    {
        labels = ui->GetRoot()->CreateChild<UIElement>("LABELS");
        labels->SetSize(ui->GetRoot()->GetSize());
    }
    Text* oldLabel = (Text*)labels->GetChild(name,false);
    if(oldLabel)
    {
        auto* vp = dr->GetSubsystem<Renderer>()->GetViewport(0);
        oldLabel->SetPosition(vp->WorldToScreenPoint(position));
        oldLabel->SetColor(color);
        if(!dynamicText)
            return;
        else
            oldLabel->SetText(text);
    }else{
        Text* t = new Text(dr->GetContext());
        t->SetText(text);
        t->SetName(name);
//        t->SetStyleAuto();
        ResourceCache* cache = dr->GetSubsystem<ResourceCache>();
        t->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 8);
        auto* vp = dr->GetSubsystem<Renderer>()->GetViewport(0);
        t->SetPosition(vp->WorldToScreenPoint(position));
        t->SetColor(color);
        t->SetTextAlignment(HA_CENTER);


        labels->AddChild(t);
    }

}

/// Removes all added label UI elements.
inline void ClearLabels(DebugRenderer* dr)
{
    auto* ui=dr->GetSubsystem<UI>();
    auto labels = ui->GetRoot()->GetChild("LABELS",false);
    if (labels)
        labels->Remove();
}

/// Add lines connecting the specified points
inline void AddLinesBetween(DebugRenderer* dr, const Matrix3x4& transform, const PODVector<Vector3>& points, const Color& color, bool depthTest)
{
    for (unsigned i = 0; i + 1 < points.Size(); ++i)
    {
        dr->AddLine(transform*points[i],transform*points[i+1],color,depthTest);
    }
}

/// Add an RGB XYZ axes matching the world transform.
inline void AddWorldTransform(DebugRenderer* dr, const Matrix3x4& transform, bool depthTest)
{
    dr->AddLine(transform*Vector3::ZERO,transform*Vector3::RIGHT,Color::RED,depthTest);
    dr->AddLine(transform*Vector3::ZERO,transform*Vector3::UP,Color::GREEN,depthTest);
    dr->AddLine(transform*Vector3::ZERO,transform*Vector3::FORWARD,Color::BLUE,depthTest);
}

inline void ShowSkeleton(DebugRenderer* dr, const Skeleton& skeleton, const Color& color, bool depthTest)
{
    const Vector<Bone>& bones = skeleton.GetBones();
    if (!bones.Size())
        return;

    unsigned uintColor = color.ToUInt();

    for (unsigned i = 0; i < bones.Size(); ++i)
    {
        // Skip if bone contains no skinned geometry
        if (bones[i].radius_ < M_EPSILON && bones[i].boundingBox_.Size().LengthSquared() < M_EPSILON)
            continue;

        Node* boneNode = bones[i].node_;
        if (!boneNode)
            continue;

        Vector3 start = boneNode->GetWorldPosition();
        Vector3 end;

        unsigned j = bones[i].parentIndex_;
        Node* parentNode = boneNode->GetParent();

        // If bone has a parent defined, and it also skins geometry, draw a line to it. Else draw the bone as a point
        if (parentNode && (bones[j].radius_ >= M_EPSILON || bones[j].boundingBox_.Size().LengthSquared() >= M_EPSILON))
            end = parentNode->GetWorldPosition();
        else
            end = start;

        dr->AddLine(start, end, uintColor, depthTest);
        dr->AddCross(start,0.02f,color,depthTest);
        AddLabel(dr, start, ToString("Bone %d",i),bones[i].name_, Color(color.r_*0.75f,color.g_*0.75f,color.b_*0.75f,color.a_), false);
    }
}

/// Utility to compute x to a given power while preserving it's sign.
inline float SignedPow(float x, float power)
{
    return Sign(x) * Pow(Abs(x), power);
}

inline float SignedAngle(Vector3 from, Vector3 to, Vector3 axis)
{
    float unsignedAngle = from.Angle(to);
    float sign = axis.DotProduct(from.CrossProduct(to));
    if(sign<0)
        unsignedAngle = -unsignedAngle;
    return unsignedAngle;
}

inline Vector3 VectorClamp(Vector3 vec, float maxLength)
{
    float length = vec.Length();
    if (Equals(length,0.f))
        return vec;
    return vec / length * Clamp(length,0.f,maxLength);
}

/// Returns the manhattan length of the vector (vec.DotProduct(ONE)).
inline float VectorSumComponents(const Vector2& vec)
{
    return vec.x_ + vec.y_;
}
/// Returns the manhattan length of the vector (vec.DotProduct(ONE)).
inline float VectorSumComponents(const Vector3& vec)
{
    return vec.x_ + vec.y_ + vec.z_;
}


template<class V>
inline bool FuzzyEqual(V lhs, V rhs)
{
    V diff = rhs - lhs;
    for (int i = 0; i < sizeof(V)/sizeof(float); ++i)
        if (Abs(diff.Data()[i]) > M_EPSILON)
            return false;
    return true;
}

inline bool FuzzyEqual(Quaternion lhs, Quaternion rhs, float fuzzy = 0.0001f)
{
    auto diff = rhs - lhs;
    return Abs(diff.x_) <= fuzzy && Abs(diff.y_) <= fuzzy && Abs(diff.z_) <= fuzzy && Abs(diff.w_) <= fuzzy;
}

inline Vector3 operator%(const Vector3& lhs, float rhs)
{
    return {Mod(lhs.x_, rhs), Mod(lhs.y_, rhs), Mod(lhs.z_, rhs)};
}

inline Vector3& operator %= (Vector3& lhs, float rhs)
{
    return lhs = lhs % rhs;
}

template<template<class, typename...> class V, class T, typename... extra>
T Sum(const V<T,extra...>& container, const T& initial = {})
{
    T sum = initial;
    for (const T& t : container)
        sum += t;
    return sum;
}

template<class T>
T Sum(const Vector<T>& container, const T& initial = {})
{
    T sum = initial;
    for (const T& t : container)
        sum += t;
    return sum;
}
template<class T>
T Sum(const PODVector<T>& container, const T& initial = {})
{
    T sum = initial;
    for (const T& t : container)
        sum += t;
    return sum;
}

template<class V, class T>
T Prod(const V& container)
{
    T sum{};
    for (const T& t : container)
        sum += t;
    return sum;
}


template<class T>
Vector<T> Reversed(const Vector<T>& v)
{
    Vector<T> out;
    out.Reserve(v.Size());
    for (unsigned i = v.Size(); i > 0; --i)
        out.Push(v[i-1]);
    return out;
}

namespace DebugRendererUtils
{
//inline AddArc
}

}



namespace Tests {
#if 0

PODVector<float> pts = SampleLinspace(500,-1,2);

ImGuiBackend::Graph("graph",pts.Buffer(),pts.Size());
{
    auto tri = ApplyTo(pts,[](float pt){return TriangleWave(pt,0.25,0.75);});
    ImGuiBackend::Graph("tri lh",tri.Buffer(),tri.Size());
}
{
    auto tri = ApplyTo(pts,[](float pt){return TriangleWave(pt,0.75,0.25);});
    ImGuiBackend::Graph("tri hl",tri.Buffer(),tri.Size());
}
{
    auto tri = ApplyTo(pts,[](float pt){return OuterTriangleWave(pt,0.25,0.75);});
    ImGuiBackend::Graph("otri lh",tri.Buffer(),tri.Size());
}
{
    auto tri = ApplyTo(pts,[](float pt){return OuterTriangleWave(pt,0.75,0.25);});
    ImGuiBackend::Graph("otri hl",tri.Buffer(),tri.Size());
}
{
    auto tri = ApplyTo(pts,[](float pt){return OuterTriangleWave(pt,0.25,0.75,1,0);});
    ImGuiBackend::Graph("otri lh2",tri.Buffer(),tri.Size());
}
{
    auto tri = ApplyTo(pts,[](float pt){return OuterTriangleWave(pt,0.75,0.25,1,0);});
    ImGuiBackend::Graph("otri hl2",tri.Buffer(),tri.Size());
}
{
    auto tri = ApplyTo(pts,[](float pt){return PeriodicTriangleWave(pt,0.05,0.75);});
    ImGuiBackend::Graph("stri lh",tri.Buffer(),tri.Size());
}
{
    auto tri = ApplyTo(pts,[](float pt){return PeriodicTriangleWave(pt,0.75,0.05);});
    ImGuiBackend::Graph("stri hl",tri.Buffer(),tri.Size());
}
{
    auto tri = ApplyTo(pts,[](float pt){return PeriodicTriangleWave(pt,0.05,0.75,1,0);});
    ImGuiBackend::Graph("stri lh2",tri.Buffer(),tri.Size());
}
{
    auto tri = ApplyTo(pts,[](float pt){return PeriodicTriangleWave(pt,0.75,0.05,1,0);});
    ImGuiBackend::Graph("stri hl2",tri.Buffer(),tri.Size());
}
{
    auto tri = ApplyTo(pts,[](float pt){return SawtoothWave(pt,0.25,0.75);});
    ImGuiBackend::Graph("sawtooth lh",tri.Buffer(),tri.Size());
}
{
    auto tri = ApplyTo(pts,[](float pt){return SawtoothWave(pt,0.75,0.25);});
    ImGuiBackend::Graph("saw hl",tri.Buffer(),tri.Size());
}
#endif
}



template<class C, int N>
class DequeuingRingQueue
{
public:
    DequeuingRingQueue(): buffer_{} {}

    const C& operator=(const C& insert) {
        buffer_[write_idx] = insert;
        ++write_idx;
        write_idx %= N;
        size += 1;

        if (size > N)
        {
            ++read_idx;
            read_idx %= N;
            size = N;
        }

        return insert;
    }

    operator C() {
        if (size)
        {
            ++read_idx;
            read_idx %= N;
            --size;

            return buffer_[read_idx];
        }
        else
        {
            assert(!"EMPTY BUFFER");
        }
    }

    int Size() const {return size;}


private:
    C buffer_[N];
    int read_idx{0};
    int write_idx{0};
    int size{0};
};


template<class C, int N>
class RingQueue
{
public:
    RingQueue(): buffer_{} {}

    const C& operator=(const C& insert) {
        buffer_[write_idx] = insert;
        ++write_idx;
        write_idx %= N;
        size += 1;

        if (size > N)
        {
            ++read_idx;
            read_idx %= N;
            size = N;
        }

        return insert;
    }

    operator C() {
        if (size)
        {
            auto old = read_idx;
            ++read_idx;
            read_idx %= N;

            return buffer_[old];
        }
        else
        {
            assert(!"EMPTY BUFFER");
        }
    }

    int Size() const {return size;}

    void SetReadOldest()
    {
        read_idx = (write_idx-size)%N;
    }


private:
    C buffer_[N];
    int read_idx{0};
    int write_idx{0};
    int size{0};
};


template <unsigned N, class T, class Weight=float>
class AverageOver
{
public:
    struct ValWeight
    {
        T value;
        Weight weight;

        ValWeight(const T& val, const Weight& w): value{val}, weight{w} {}
    };

    T operator=(const ValWeight& vw)
    {
        if (vw.weight > 1e-4)
        {
            values_[idx] = vw.value;
            weights_[idx] = vw.weight;
            idx = (idx+1)%N;
        }

        return Value();
    }

    void SetValue(const T& val, const Weight& w)
    {
        if (w > 1e-4)
        {
            values_[idx] = val;
            weights_[idx] = w;
            idx = (idx+1)%N;
        }
    }

    T Value() const {
        T val{};
        Weight total{};
        for (unsigned i = 0; i < N; ++i)
        {
            val += values_[i] * weights_[i];
            total += weights_[i];
        }

        if (total < 1e-4)
            return {};
        return val / total;
    }

    operator T() { return Value(); }

private:
    std::array<T,N> values_;
    std::array<Weight,N> weights_;
    unsigned idx = 0;
};


template <unsigned N, class T, class Weight=float>
class AverageOverWithMax
{
public:
    AverageOverWithMax(Weight maxTotal): max{maxTotal} {}

    struct ValWeight
    {
        T value;
        Weight weight;

        ValWeight(const T& val, const Weight& w): value{val}, weight{w} {}
    };

    T operator=(const ValWeight& vw)
    {
        if (vw.weight > 1e-4)
        {
            values_[idx] = vw.value;
            weights_[idx] = vw.weight;
            TrimMax();
            idx = (idx+1)%N;
        }


        return Value();
    }

    void SetValue(const T& val, const Weight& w)
    {
        if (w > 1e-4)
        {
            values_[idx] = val;
            weights_[idx] = w;
            TrimMax();
            idx = (idx+1)%N;
        }
    }


    T Value() const {
        T val{};
        Weight total{};
        for (unsigned i = 0; i < N; ++i)
        {
            val += values_[i] * weights_[i];
            total += weights_[i];
        }

        if (total < 1e-4)
            return {};
        return val / total;
    }

    operator T() { return Value(); }

private:
    std::array<T,N> values_;
    std::array<Weight,N> weights_;
    unsigned idx = 0;
    Weight max;

    void TrimMax()
    {
        Weight total{};
        // Only up to N-1 as we don't want to remove the just added term
        for (unsigned i = 0; i+1 < N; ++i)
        {
            unsigned ci = (N + idx - i) % N;
            total += weights_[ci];
            if (total >= max)
            {
                values_[ci] = {};
                weights_[ci] = 0;
            }
        }
    }
};
