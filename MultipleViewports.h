//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "Sample.h"
namespace Urho3D
{

class Node;
class Scene;
class Animation;
class IKEffector;
class IKSolver;
class AnimationController;

}

/// Checks if we are running through SimpleScreenRecorder by checking for the library in /proc/self/maps
bool CheckSSR();

/// Multiple viewports example.
/// This sample demonstrates:
///     - Setting up two viewports with two separate cameras
///     - Adding post processing effects to a viewport's render path and toggling them
class MultipleViewports : public Sample
{
    URHO3D_OBJECT(MultipleViewports, Sample)

public:
    /// Construct.
    MultipleViewports(Context* context);

    /// Setup before engine initialization. Modifies the engine parameters.
    void Setup() override;

    /// Setup after engine initialization and before running the main loop.
    void Start() override;

    void ResetCamera();
    
protected:
    /// Return XML patch instructions for screen joystick layout for a specific sample app, if any.
    String GetScreenJoystickPatchString() const override { return
        "<patch>"
        "    <add sel=\"/element\">"
        "        <element type=\"Button\">"
        "            <attribute name=\"Name\" value=\"Button3\" />"
        "            <attribute name=\"Position\" value=\"-120 -120\" />"
        "            <attribute name=\"Size\" value=\"96 96\" />"
        "            <attribute name=\"Horiz Alignment\" value=\"Right\" />"
        "            <attribute name=\"Vert Alignment\" value=\"Bottom\" />"
        "            <attribute name=\"Texture\" value=\"Texture2D;Textures/TouchInput.png\" />"
        "            <attribute name=\"Image Rect\" value=\"96 0 192 96\" />"
        "            <attribute name=\"Hover Image Offset\" value=\"0 0\" />"
        "            <attribute name=\"Pressed Image Offset\" value=\"0 0\" />"
        "            <element type=\"Text\">"
        "                <attribute name=\"Name\" value=\"Label\" />"
        "                <attribute name=\"Horiz Alignment\" value=\"Center\" />"
        "                <attribute name=\"Vert Alignment\" value=\"Center\" />"
        "                <attribute name=\"Color\" value=\"0 0 0 1\" />"
        "                <attribute name=\"Text\" value=\"FXAA\" />"
        "            </element>"
        "            <element type=\"Text\">"
        "                <attribute name=\"Name\" value=\"KeyBinding\" />"
        "                <attribute name=\"Text\" value=\"F\" />"
        "            </element>"
        "        </element>"
        "    </add>"
        "    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/attribute[@name='Is Visible']\" />"
        "    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">Bloom</replace>"
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]\">"
        "        <element type=\"Text\">"
        "            <attribute name=\"Name\" value=\"KeyBinding\" />"
        "            <attribute name=\"Text\" value=\"B\" />"
        "        </element>"
        "    </add>"
        "    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/attribute[@name='Is Visible']\" />"
        "    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">Debug</replace>"
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]\">"
        "        <element type=\"Text\">"
        "            <attribute name=\"Name\" value=\"KeyBinding\" />"
        "            <attribute name=\"Text\" value=\"SPACE\" />"
        "        </element>"
        "    </add>"
        "</patch>";
    }

private:
    /// Construct the scene content.
    void CreateScene();
    /// Setup character
    void SetupCharacter();

    /// Construct an instruction text to the UI.
    void CreateInstructions();
    /// Set up viewports.
    void SetupViewports();
    /// Subscribe to application-wide logic update and post-render update events.
    void SubscribeToEvents();
    /// Read input and moves the camera.
    void MoveCamera(float timeStep);
    /// Handle the logic update event.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle the post-render update event.
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);

    /// Advance animation by the given frame count
    void AdvanceFrames(int amount);

    /// Rear-facing camera scene node.
    SharedPtr<Node> rearCameraNode_;
    /// Flag for drawing debug geometry.
    bool drawDebug_{true};
    /// Flag for drawing the skeleton.
    bool drawSkeleton_{false};
    /// Flag for drawing the physics.
    bool drawPhysics_{false};

    ///#Character
    /// Animation controller of Jack.
    SharedPtr<Urho3D::AnimationController> jackAnimCtrl_;
    /// Inverse kinematic left effector.
    SharedPtr<Urho3D::IKEffector> leftEffector_;
    /// Inverse kinematic right effector.
    SharedPtr<Urho3D::IKEffector> rightEffector_;
    /// Inverse kinematic solver.
    SharedPtr<Urho3D::IKSolver> solver_;
    /// Need references to these nodes to calculate foot angles and offsets.
    SharedPtr<Urho3D::Node> leftFoot_;
    SharedPtr<Urho3D::Node> rightFoot_;
    SharedPtr<Urho3D::Node> jackNode_;
    /// So we can rotate the floor.
    SharedPtr<Urho3D::Node> floorNode_;
    /// So we can rotate the floor.
    SharedPtr<Urho3D::Node> floorViewNode_;
    float floorPitch_{};
    float floorRoll_{};

    // Info text
//    SharedPtr<Text> instructionText_;

    // Updating text
    SharedPtr<Text> updateText_;


    Node* gv{nullptr};

    // Jack velocity
    float v = 0.3f;//1.5f;//m/s
    float s = 0.5f;

    Vector3 gv_offset{0,2,0};

    unsigned joystickIndex_=0;
};
