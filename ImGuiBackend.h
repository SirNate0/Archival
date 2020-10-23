#pragma once

#include <Urho3D/Container/Str.h>

#include "ArchiveDetail.h"


#include <Urhox/Urhox.h>
#include <ImGui/imgui.h>

inline namespace Archival {
namespace Detail {

using namespace Urho3D;

/// Archival Backend that uses Dear IMGUI to display and edit the values in a GUI.
class ImGuiBackend: public Backend
{
    /// The default implementation of archiving a type. Defers to Get<T>/Set<T>.
    /// Get/Set must exist for the types bool, int, unsigned, float, and String.
    ///         Maybe also for null, (unsigned) short, (unsigned) long, and double.
    /// Values may possibly be cast between different types by the underlying implementation, e.g. XML stores all as a String.
    /// The name "value" is reserved. It is used to handle the case of inline values (like JSON [1,2,3]).


    /// Internal constructor that is used for CreateGroup/SeriesEntry for non-root groups in the tree. Takes the name of the node, the depth in the tree, and the series entry if it was an entry in a series element rather than a group.
    ImGuiBackend(const String& groupName, unsigned treeDepth, int seriesEntry);

public:

    /// Creates the backend that will use the provided window name.
    ImGuiBackend(const String& windowName);

    /// Destructor. Closes the window if the backend was for the top-level (as opposed to an element deeper in the tree).
    virtual ~ImGuiBackend() override;

    /// Registers the backend with the Context, setting up the IMGUI interface.
    static void RegisterBackend(Context* context);

    /// Utility method to create a save button with input text. Returns true on button press. Optionally creates a separator beforehand.
    static bool SaveButton(String& filename, const String& button = "Save", bool withSeparator = true);

    /// Utility method to create a graph from the provided points.
    static bool Graph(const String &name, const float *pts, unsigned count, unsigned dataStride = sizeof(float));

    /// Test IMGUI method to be called per-frame.
    static void TestImGuiUpdate();


    Backend* CreateGroup(const String &name, bool isInput) override;
    Backend* CreateSeriesEntry(const String &name, bool isInput) override;
    bool GetSeriesSize(const String &name, unsigned &size) override;
    bool SetSeriesSize(const String &, const unsigned &) override { return false; }
    bool GetEntryNames(StringVector &) override { return true; /* we don't need to pre-serialize names */}
    bool SetEntryNames(const StringVector &) override { return false; /* we do not support setting values */ }
    unsigned char InlineSeriesVerbosity() const override { return 20; }

    bool Get(const String &name, const std::nullptr_t &) override;
    bool Get(const String &name, bool &val) override;
    bool Get(const String &name, unsigned char &val) override;
    bool Get(const String &name, signed char &val) override;
    bool Get(const String &name, unsigned short &val) override;
    bool Get(const String &name, signed short &val) override;
    bool Get(const String &name, unsigned int &val) override;
    bool Get(const String &name, signed int &val) override;
    bool Get(const String &name, unsigned long long &val) override;
    bool Get(const String &name, signed long long &val) override;
    bool Get(const String &name, float &val) override;
    bool Get(const String &name, double &val) override;
    bool Get(const String &name, String &val) override;

#ifdef EXTENDED_ARCHIVE_TYPES
    bool Get(const String &name, Urho3D::IntVector2 &val) override;
    bool Get(const String &name, Urho3D::IntVector3 &val) override;
    bool Get(const String &name, Urho3D::Vector2 &val) override;
    bool Get(const String &name, Urho3D::Vector3 &val) override;
    bool Get(const String &name, Urho3D::Vector4 &val) override;
    bool Get(const String &name, Urho3D::Quaternion &val) override;
    bool Get(const String &name, Urho3D::Color &val) override;
    bool Get(const String &name, Urho3D::Matrix3 &val) override;
    bool Get(const String &name, Urho3D::Matrix3x4 &val) override;
    bool Get(const String &name, Urho3D::Matrix4 &val) override;
#endif

    /// ImGui backend should be treated only as an Input backend. It will update it's values based on what was present when Get is called.
    bool Set(const String &, const std::nullptr_t &) override { return false; }
    bool Set(const String &, const bool &) override { return false; }
    bool Set(const String &, const unsigned char &) override { return false; }
    bool Set(const String &, const signed char &) override { return false; }
    bool Set(const String &, const unsigned short &) override { return false; }
    bool Set(const String &, const signed short &) override { return false; }
    bool Set(const String &, const unsigned int &) override { return false; }
    bool Set(const String &, const signed int &) override { return false; }
    bool Set(const String &, const unsigned long long &) override { return false; }
    bool Set(const String &, const signed long long &) override { return false; }
    bool Set(const String &, const float &) override { return false; }
    bool Set(const String &, const double &) override { return false; }
    bool Set(const String &, const String &) override { return false; }

    using Backend::AddHint;
    /// Adds a hint to the stack. Returns true if the hint kind is recognized.
    bool AddHint(const Hint& hint) override { hints_.Push(hint); return true; }

    /// Returns true if a hint of this kind is present.
    virtual bool HasHint(Hint::HINT kind) override { for (Hint& h: hints_) if (h.kind == kind) return true; return false; }

    /// Returns a pointer to a hint with the requested kind or nullptr if the hint has not been set or is unsupported.
    const Hint& GetHint(Hint::HINT kind) override { for (Hint& h: hints_) if (h.kind == kind) return h; return Hint::EMPTY_HINT; }

    /// Removes the requested hint kind. Returns true if it found such a hint.
    bool RemoveHint(Hint::HINT kind) override
    {
        bool found = false;
        for (unsigned i = 0; i < hints_.Size(); ++i)
            if (hints_[i].kind == kind)
            {
                hints_.Erase(i);
                --i;
                found = true;
            }
        return found;
    }

    /// Clears set hints. Returns true if succeeded.
    bool ClearHints() override { hints_.Clear(); return true; }

    /// Returns true if the condition is allowed. Used so that binary backend can "bake" the result of a conditional write.
    bool WriteConditional(bool condition, bool isInput) override;

    /// Returns the name of the backend
    const String& GetBackendName() override { static const String name("IMGUI"); return name; }


private:
    /// Sentinal value to indicate we aren't a first-level element in a series (so we don't need to push to the id stack)
    static constexpr int INVALID_SERIES_ENTRY{-1};
    /// Holds the entry in the parent series or the invalid value if the parent is a group.
    int seriesEntry_{INVALID_SERIES_ENTRY};

    /// List of the number of elements in each entry child.
    Urho3D::HashMap<String, unsigned> entries_;
    /// True if the last written series entry was an open value.
    bool lastSeriesOpen_{true};
    /// Stores the last created series entry's name so we know when we have to create a new header.
    String lastSeriesName_;
    /// The hints that we have set.
    Vector<Hint> hints_;

    /// Integer representing the depth in the tree of this instance of the backend. 0 represents windows, >0 is somewhere in the tree.
    unsigned myTreeDepth_;
    /// String that is the name of the element (node for the tree or window title).
    String myTreeName_;

    /// Holds the current global tree depth so we know how many times to PopID from the stack.
    static int globalTreeDepth_;
    /// Holds the current tree-name stack so we know what to pop from the tree.
    static StringVector lastTreeNames_;

    /// Method to be called before a value is written to IMGUI to set up the necessary IDs on the stack and such.
    void BeginValue();
    /// Method to be called after a value is written to IMGUI to clean up the necessary IDs on the stack and such.
    void EndValue();

    /// Returns the speed to use for drag inputs. Based on the hint, defaults to 0.1f;
    float GetSpeedHint();
};

}
}
