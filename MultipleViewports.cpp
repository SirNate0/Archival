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

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Drawable.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/IK/IKEffector.h>
#include <Urho3D/IK/IKSolver.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/Constraint.h>

#include "MultipleViewports.h"


#include "Utils.h"

#include <bitset>

#include <Urho3D/DebugNew.h>

#include "Archival/ImGuiBackend.h"

#include "Archival/Archive.h"

URHO3D_DEFINE_APPLICATION_MAIN(MultipleViewports)

MultipleViewports::MultipleViewports(Context* context) :
    Sample(context),
    drawDebug_(true)
{
}

void MultipleViewports::Setup()
{
    Sample::Setup();

    // Add the Added path
    engineParameters_["ResourcePaths"] = "CoreData;Data;Added;Added2";

    // Smaller window for recording
    if (CheckSSR())
    {
        engineParameters_["WindowWidth"] = 848;
        engineParameters_["WindowHeight"] = 480;

        uiStrings["GL Library SSR"] = "SSR";
    }
}

void MultipleViewports::Start()
{
    Detail::ImGuiBackend::RegisterBackend(context_);


//    TestBezierCurves();
//    GetSubsystem<Engine>()->Exit();
//    return;

    // Execute base class startup
    Sample::Start();

    // Create the scene content
    CreateScene();

    // Create our character
    SetupCharacter();

    // Create the UI content
    CreateInstructions();

    // Setup the viewports for displaying the scene
    SetupViewports();

    // Hook up to the frame update and render post-update events
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
//    Sample::InitMouseMode(MM_ABSOLUTE);
   Sample::InitMouseMode(MM_FREE);
}

void MultipleViewports::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create octree, use default volume (-1000, -1000, -1000) to (1000, 1000, 1000)
    // Also create a DebugRenderer component so that we can draw debug geometry
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();
    auto pw = scene_->CreateComponent<PhysicsWorld>();
//    pw->SetFps(10);

    {

        Node* planeNode = scene_->CreateChild("Sphere");
        planeNode->SetPosition(Vector3::FORWARD * 10 + Vector3::RIGHT * 18 + Vector3::UP * 4);
        StaticModel* planeObject = planeNode->CreateComponent<StaticModel>();
        planeObject->SetModel(cache->GetResource<Model>("Chunk.mdl"));
        planeObject->SetMaterial(cache->GetResource<Material>("Materials/Normal.xml"));

    }
    {

        // Create scene node & StaticModel component for showing a static plane
        Node* planeNode = scene_->CreateChild("Sphere");
        planeNode->Scale(3.0);
        planeNode->SetPosition(Vector3::FORWARD * 10 + Vector3::LEFT * 8 + Vector3::UP * 4);
        StaticModel* planeObject = planeNode->CreateComponent<StaticModel>();
        planeObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
        planeObject->SetMaterial(cache->GetResource<Material>("Materials/Normal.xml"));

        Node* parent = planeNode;
        // Create child node & StaticModel component for showing a static plane
        Node* child = parent->CreateChild("Sphere2");
        {
            child->SetPosition(Vector3::UP * 1);
            child->SetScale(0.5f);
            StaticModel* model = child->CreateComponent<StaticModel>();
            model->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
            model->SetMaterial(cache->GetResource<Material>("Materials/Normal.xml"));
        }
//        parent->SetEnabled(false);
    }

    // Create scene node & StaticModel component for showing a static plane
    Node* floorNode = scene_->CreateChild("Plane");
    floorNode->SetScale(Vector3(100.0f, 1.0f, 100.0f));
    StaticModel* planeObject = floorNode->CreateComponent<StaticModel>();
    planeObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
    planeObject->SetMaterial(cache->GetResource<Material>("Materials/StoneTiled.xml"));

    {
        floorViewNode_ = scene_->CreateChild("ViewPlane");
        floorViewNode_->SetScale(Vector3(100.0f, 1.0f, 100.0f));
        floorViewNode_->Translate(Vector3::UP * 0.01f);
        StaticModel* planeObject = floorViewNode_->CreateComponent<StaticModel>();
        planeObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
        planeObject->SetMaterial(cache->GetResource<Material>("Materials/Translucent.xml"));
    }

    // Create scene node & StaticModel component for showing a static plane
//    auto f = cache->GetFile("Nodes/DefaultBattleField.xml");
    auto f = cache->GetFile("Nodes/ground.xml");
    auto floorNodeCurved = scene_->InstantiateXML(*f, {},{});
    floorNodeCurved->Translate({20,-5,0});
    floorNodeCurved->Scale(0.5);


    auto* body = floorNode->CreateComponent<RigidBody>();
    // Use collision layer bit 2 to mark world scenery. This is what we will raycast against to prevent camera from going
    // inside geometry
    body->SetCollisionLayer(2);
    auto* shape = floorNode->CreateComponent<CollisionShape>();
//    shape->SetBox(Vector3::ONE);
    shape->SetStaticPlane();


    // Create movable boxes. Let them fall from the sky at first
    SetRandomSeed(100);
    const unsigned NUM_BOXES = 100;
    for (unsigned i = 0; i < NUM_BOXES; ++i)
    {
//        float scale = Random(2.0f) + 0.5f;
        float scale = 1+ (Rand() % 7);

        Node* objectNode = scene_->CreateChild("Box");
        objectNode->SetPosition(Vector3(Random(180.0f) - 90.0f, Random(10.0f) + 1.0f, Random(180.0f) - 90.0f));
//        objectNode->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
        objectNode->SetScale(scale * Vector3(1,0.5,1));
        auto* object = objectNode->CreateComponent<StaticModel>();
        object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
//        object->SetModel(cache->GetResource<Model>("BlenderOrientation.mdl"));
        object->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
        object->SetCastShadows(true);

        auto* body = objectNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2);
        // Bigger boxes will be heavier and harder to move
        body->SetMass(scale * 2.0f);
        auto* shape = objectNode->CreateComponent<CollisionShape>();
        shape->SetBox(Vector3::ONE);
    }

    for (unsigned i = 0; i < NUM_BOXES; ++i)
    {
//        float scale = Random(2.0f) + 0.5f;
        float scale = 1+ (Rand() % 7);

        Node* objectNode = scene_->CreateChild("Cone");
        objectNode->SetPosition(Vector3(Random(180.0f) - 90.0f, Random(10.0f) + 15.0f, Random(180.0f) - 90.0f));
//        objectNode->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
        float r = Random(5.5f)+0.5f;
        float h = Random(1.5f) + 0.5f;
        objectNode->SetScale(Vector3(r,h,r));
        auto* object = objectNode->CreateComponent<StaticModel>();
        object->SetModel(cache->GetResource<Model>("Models/Cone.mdl"));
//        object->SetModel(cache->GetResource<Model>("BlenderOrientation.mdl"));
        object->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
        object->SetCastShadows(true);

        auto* body = objectNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2);
        // Bigger boxes will be heavier and harder to move
        body->SetMass(scale * 2.0f);
        auto* shape = objectNode->CreateComponent<CollisionShape>();
        shape->SetCone(1,1);
    }


    // Create a Zone component for ambient lighting & fog control
    Node* zoneNode = scene_->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.35f, 0.35f, 0.35f));
//    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
    zone->SetFogColor(Color::WHITE*0.1);
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);
    zone->SetViewMask(0x1);

    // Create a directional light to the world. Enable cascaded shadows on it
    Node* lightNode2 = scene_->CreateChild("DirectionalLight");
    lightNode2->SetDirection(Vector3(-0.6f, -1.0f, -0.8f));
    Light* light2 = lightNode2->CreateComponent<Light>();
    light2->SetLightType(LIGHT_DIRECTIONAL);
    light2->SetCastShadows(true);
    light2->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    // Set cascade splits at 10, 50 and 200 world units, fade shadows out at 80% of maximum shadow distance
    light2->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));

    // Create a directional light to the world. Enable cascaded shadows on it
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    // Set cascade splits at 10, 50 and 200 world units, fade shadows out at 80% of maximum shadow distance
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));

//    // Create some mushrooms
//    const unsigned NUM_MUSHROOMS = 240;
//    for (unsigned i = 0; i < NUM_MUSHROOMS; ++i)
//    {
//        Node* mushroomNode = scene_->CreateChild("Mushroom");
//        mushroomNode->SetPosition(Vector3(Random(90.0f) - 45.0f, 0.0f, Random(90.0f) - 45.0f));
//        mushroomNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
//        mushroomNode->SetScale(0.5f + Random(2.0f));
//        StaticModel* mushroomObject = mushroomNode->CreateComponent<StaticModel>();
//        mushroomObject->SetModel(cache->GetResource<Model>("Models/Mushroom.mdl"));
//        mushroomObject->SetMaterial(cache->GetResource<Material>("Materials/CrossHatching.xml"));//"Materials/Mushroom.xml"));
//        mushroomObject->SetCastShadows(true);
//    }

//    // Create randomly sized boxes. If boxes are big enough, make them occluders
//    const unsigned NUM_BOXES = 20;
//    for (unsigned i = 0; i < NUM_BOXES; ++i)
//    {
//        Node* boxNode = scene_->CreateChild("Box");
//        float size = 1.0f + Random(10.0f);
//        boxNode->SetPosition(Vector3(Random(80.0f) - 40.0f, size * 0.5f, Random(80.0f) - 40.0f));
//        boxNode->SetScale(size);
//        StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
//        boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
//        boxObject->SetMaterial(cache->GetResource<Material>("Materials/Edge.xml"));//"Materials/Stone.xml"));
//        boxObject->SetCastShadows(true);
//        if (size >= 3.0f)
//            boxObject->SetOccluder(true);
//    }

    // Create the cameras. Limit far clip distance to match the fog
    cameraNode_ = scene_->CreateChild("Camera");
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);

    // Parent the rear camera node to the front camera node and turn it 180 degrees to face backward
    // Here, we use the angle-axis constructor for Quaternion instead of the usual Euler angles
    rearCameraNode_ = cameraNode_->CreateChild("RearCamera");
    rearCameraNode_->Rotate(Quaternion(180.0f, Vector3::UP));
    Camera* rearCamera = rearCameraNode_->CreateComponent<Camera>();
    rearCamera->SetFarClip(300.0f);
    // Because the rear viewport is rather small, disable occlusion culling from it. Use the camera's
    // "view override flags" for this. We could also disable eg. shadows or force low material quality
    // if we wanted
    rearCamera->SetViewOverrideFlags(VO_DISABLE_OCCLUSION);

    // Set an initial position for the front camera scene node above the plane
    cameraNode_->SetPosition(Vector3(0.0f, 1.0f, -10.0f));
    cameraNode_->Rotate({30.0f,Vector3::RIGHT});

    cameraNode_->Translate(Vector3::FORWARD * 11.5f,TS_WORLD);


    {
        GetSubsystem<Input>()->SetMouseVisible(true);
    }
}

void MultipleViewports::SetupCharacter()
{
    auto* cache = GetSubsystem<ResourceCache>();

    context_->RegisterSubsystem(scene_->GetComponent<DebugRenderer>());
    context_->RegisterSubsystem(cameraNode_->GetComponent<Camera>());


    // Load Jack model
//    jackNode_ = scene_->CreateChild("Jack");
//    jackNode_->SetRotation(Quaternion(0.0f, 270.0f, 0.0f));
//    auto* jack = jackNode_->CreateComponent<AnimatedModel>();
//    jack->SetModel(cache->GetResource<Model>("Models/Jack.mdl"));
//    jack->SetMaterial(cache->GetResource<Material>("Materials/Jack.xml"));
//    jack->SetCastShadows(true);

    jackNode_ = scene_->CreateChild("Dragon");
//    jackNode_->SetRotation(Quaternion(0.0f, 270.0f, 0.0f));
    auto* jack = jackNode_->CreateComponent<AnimatedModel>();
    jack->SetModel(cache->GetResource<Model>("A/Dragon1.mdl"));
    jack->SetMaterial(cache->GetResource<Material>("A/Material.xml"));
    jack->SetCastShadows(true);

    jackNode_->SetPosition({5,0,0});

//    PrintSkeleton(jack->GetSkeleton());

    {
        SharedPtr<File> f;
        f = cache->GetFile("New-Exports/Scene.xml");

        gv = scene_->InstantiateXML(*f, {},{});



        String rightToe = "RToe";
        String leftToe = "LToe";

        String rightFoot = "RFoot"; // Armature > ... > Origin > Waist > Hips > _Thigh > _Leg > _Foot > _ToeC
        String rightHand = "RHand"; // Armature > ... > Origin > Waist > Spine1 > Spine2 > _Shoulder > _Arm > _ForeArm > _Hand > _Finger{A|B}{1..3}
        String leftFoot = "LFoot";
        String leftHand = "LHand";
        String rightHips = "Hips";
        String leftHips = "Hips";
        String centerOfMass = "Waist";
        String root = "Origin";
        String head = "Head";

        gv->Translate(Vector3::RIGHT*10);



        // Create rigidbody, and set non-zero mass so that the body becomes dynamic
        auto* body = gv->CreateComponent<RigidBody>();
        body->SetCollisionLayer(1);
        body->SetMass(1.0f);
        body->SetFriction(0);
        body->SetFriction(0.5);

        // Set zero angular factor so that physics doesn't turn the character on its own.
        // Instead we will control the character yaw manually
        body->SetAngularFactor(Vector3::UP);

        body->SetLinearFactor(Vector3::ONE * 0.999f);

        // Set the rigidbody to signal collision also when in rest, so that we get ground collisions properly
        body->SetCollisionEventMode(COLLISION_ALWAYS);

        // Set a capsule shape for collision
        auto* shape = gv->CreateComponent<CollisionShape>();
//        shape->SetCapsule(0.7f, 1.8f, Vector3(0.0f, 0.98f, 0.0f));
        shape->SetCapsule(0.7f, 1.8f, Vector3(0.0f, 0.9f, 0.0f));
//        shape->SetCylinder(0.7f, 1.8f, Vector3(0.0f, 0.9f, 0.0f));


        ResetCamera();

//        rearCameraNode_->SetPosition(Vector3{-1,0.2,0});
//        rearCameraNode_->SetRotation({90,Vector3::UP});
//        rearCameraNode_->SetPosition(Vector3{0,1.2,-1});
    }

    // We need to attach two inverse kinematic effectors to Jack's feet to
    // control the grounding.
//    leftFoot_  = jackNode_->GetChild("Bip01_L_Foot", true);
//    rightFoot_ = jackNode_->GetChild("Bip01_R_Foot", true);
    leftFoot_  = jackNode_->GetChild("foot.L", true);
    rightFoot_ = jackNode_->GetChild("foot.R", true);
    leftEffector_  = leftFoot_->CreateComponent<IKEffector>();
    rightEffector_ = rightFoot_->CreateComponent<IKEffector>();
    // Control 2 segments up to the hips
    leftEffector_->SetChainLength(2);
    rightEffector_->SetChainLength(2);
}

extern "C"
{
extern void glXCreateWindow();
}

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

const char *my_fname(void) {
    Dl_info dl_info;
    dladdr((void*)glXCreateWindow, &dl_info);
    return(dl_info.dli_fname);
}

#include <iostream>
#include <fstream>
#include <string>

using namespace std;


bool CheckSSR()
{

    const char* filename ="/proc/self/maps";
    const char* search = "libssr-glinject.so";

    int offset;
    string line;
    ifstream Myfile;
    Myfile.open (filename);

    if(Myfile.is_open())
    {
        while(!Myfile.eof())
        {
            getline(Myfile,line);
            if ((offset = line.find(search, 0)) != string::npos)
            {
             cout << "found '" << search << " \n\n"<< line  <<endl;
             return true;
            }
            else
            {

                cout << "Not found \n\n";

            }
        }
        Myfile.close();
    }
    else
    cout<<"Unable to open this file."<<endl;

    return false;
}

//bool isScreenRecorded(Context* context)
//{
//    String ssr = "libssr-glinject.so";
//    File maps(context,"/proc/self/maps");
//    maps.Read();
//}


void MultipleViewports::CreateInstructions()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
//    instructionText_ = ui->GetRoot()->CreateChild<Text>();
//    instructionText_->SetText(
//        "Use WASD keys and mouse/touch to move\n"
//        "B to toggle bloom, F to toggle FXAA\n"
//        "Space to toggle debug geometry\n"
//    );
//    instructionText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
//    // The text has multiple rows. Center them in relation to each other
//    instructionText_->SetTextAlignment(HA_CENTER);

//    // Position the text relative to the screen center
//    instructionText_->SetHorizontalAlignment(HA_CENTER);
//    instructionText_->SetVerticalAlignment(VA_CENTER);
//    instructionText_->SetPosition(0, 0.85 * ui->GetRoot()->GetHeight() );
//    instructionText_->SetColor(Color::RED);

    {
        // Construct new Text object, set string to display and font to use
        updateText_ = ui->GetRoot()->CreateChild<Text>();
        updateText_->SetText("");
        updateText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 16);
//        updateText_->SetFont(cache->GetResource<Font>("Fonts/Arial.ttf"), 16);
        // The text has multiple rows. Center them in relation to each other
        updateText_->SetTextAlignment(HA_LEFT);

        // Position the text relative to the screen center
        updateText_->SetHorizontalAlignment(HA_LEFT);
        updateText_->SetVerticalAlignment(VA_TOP);
        updateText_->SetPosition(20, 20);
        updateText_->SetColor(Color::WHITE);
    }
}

void MultipleViewports::SetupViewports()
{
    Graphics* graphics = GetSubsystem<Graphics>();
    Renderer* renderer = GetSubsystem<Renderer>();

    renderer->SetNumViewports(2);

    // Set up the front camera viewport
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, rearCameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);

    // Clone the default render path so that we do not interfere with the other viewport, then add
    // bloom and FXAA post process effects to the front viewport. Render path commands can be tagged
    // for example with the effect name to allow easy toggling on and off. We start with the effects
    // disabled.
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));
    // Make the bloom mixing parameter more pronounced
    effectRenderPath->SetShaderParameter("BloomMix", Vector2(0.9f, 0.6f));
    effectRenderPath->SetEnabled("Bloom", false);
    effectRenderPath->SetEnabled("FXAA2", false);
    viewport->SetRenderPath(effectRenderPath);

    // Set up the rear camera viewport on top of the front view ("rear view mirror")
    // The viewport index must be greater in that case, otherwise the view would be left behind
    SharedPtr<Viewport> rearViewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>(),
        IntRect(graphics->GetWidth() * 2 / 3, 32, graphics->GetWidth() - 32, graphics->GetHeight() / 3)));
    renderer->SetViewport(1, rearViewport);
}

void MultipleViewports::SubscribeToEvents()
{
    // Subscribe HandleUpdate() method for processing update events
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MultipleViewports, HandleUpdate));

    // Subscribe HandlePostRenderUpdate() method for processing the post-render update event, during which we request
    // debug geometry
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(MultipleViewports, HandlePostRenderUpdate));
}

void MultipleViewports::MoveCamera(float timeStep)
{
    // Do not move if the UI has a focused element (the console)
    if (GetSubsystem<UI>()->GetFocusElement())
        return;

    Input* input = GetSubsystem<Input>();

    // Movement speed as world units per second
    const float MOVE_SPEED = 20.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input->GetMouseMove();
    yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    pitch_ = Clamp(pitch_, -90.0f, 90.0f);

    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    if (input->GetKeyDown(KEY_W))
        cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(KEY_S))
        cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(KEY_A))
        cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(KEY_D))
        cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(KEY_PAGEUP))
        cameraNode_->Translate(Vector3::UP * 0.5 * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(KEY_PAGEDOWN))
        cameraNode_->Translate(Vector3::DOWN * 0.5 * MOVE_SPEED * timeStep);

//    // Toggle post processing effects on the front viewport. Note that the rear viewport is unaffected
//    RenderPath* effectRenderPath = GetSubsystem<Renderer>()->GetViewport(0)->GetRenderPath();
//    if (input->GetKeyPress(KEY_B))
//        effectRenderPath->ToggleEnabled("Bloom");
//    if (input->GetKeyPress(KEY_F))
//        effectRenderPath->ToggleEnabled("FXAA2");

    // Toggle debug geometry with space
    if (input->GetKeyPress(KEY_F6))
        drawDebug_ = !drawDebug_;

    if (input->GetKeyPress(KEY_RIGHT))
        AdvanceFrames(1+9*input->GetKeyDown(KEY_SHIFT));
    if (input->GetKeyPress(KEY_LEFT))
        AdvanceFrames(-1-9*input->GetKeyDown(KEY_SHIFT));
}

void MultipleViewports::ResetCamera()
{
    gv_offset = Vector3{0,2,0};
    rearCameraNode_->SetPosition(Vector3{0,4,-6});
//    rearCameraNode_->SetPosition(Vector3{0,0,-1.5});
    rearCameraNode_->SetRotation({0,Vector3::UP});
    rearCameraNode_->Rotate({20,Vector3::RIGHT});

    rearCameraNode_->LookAt(gv->GetPosition() + gv_offset);
}

Urho3D::HashMap<Urho3D::String, Urho3D::String> uiStrings;

void MultipleViewports::HandleUpdate(StringHash eventType, VariantMap& eventData)
{

    TestArchive();

    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move the camera, scale movement with time step
    MoveCamera(timeStep);

    Archival::Detail::ImGuiBackend::TestImGuiUpdate();

//    using Archival::Detail::ImGuiBackend;
//    ImGuiBackend back("VINDER");
//    static float v = 1.0f;
//    static String text("Hello World.");
//    static Vector3 pos;
//    back.Get("Testing",v);
//    back.Get("Testing 2",v);
//    back.Get("String test", text);
//    back.Get("Vect test", pos);
//    back.CreateSeriesEntry("series", true).Serialize("Serialized",v);
//    back.CreateSeriesEntry("series", true).Serialize("Serialized",v);
//    back.CreateSeriesEntry("series", true).Serialize("Serialized",v);
//    back.CreateSeriesEntry("series", true).Serialize("Serialized",v);
//    back.CreateGroup("group", true).Serialize("Serialized",v).Then("Vector",pos);


    // Add IMGUI
    {
        using Archival::Detail::ImGuiBackend;
        using Archival::Archive;
        Archive ar(true, new ImGuiBackend("Application"));
        ar.Serialize("Draw Debug", drawDebug_);
        ar.Serialize("Draw Skeleton", drawSkeleton_);
        ar.Serialize("Draw Physics", drawPhysics_);
        ar.Serialize("Dragon", GetSet([&](){return jackNode_->IsEnabled();}, [&](bool e){jackNode_->SetEnabled(e);}));
        ar.Serialize("Joystick Index", joystickIndex_);
        ar.Serialize("Plane", GetSet([&](){return floorViewNode_->IsEnabled();}, [&](bool e){floorViewNode_->SetEnabled(e);}))
                .Then(">Height",GetSet([&](){return floorViewNode_->GetPosition().y_;}, [&](float val){floorViewNode_->SetPosition({0,val,0});}));
//        gv->GetComponent<CharacterController>()->SetJoystickIndex(joystickIndex_);
    }

    // Update the text
    String currentText;
    for (auto labelText : uiStrings)
    {
        currentText += "\n" + labelText.first_  + ": " + labelText.second_;
    }
    if (currentText.Empty())
        currentText = "No-Values";

    updateText_->SetText(currentText);

    if (false)
    {
        float r = 3.0f;
        float circ = 2*M_PI*r;
        float omega = 2*M_PI*v/circ; // rad/s
        Quaternion rot;
        rot.FromAngleAxis(s*omega*timeStep*180/M_PI,Vector3::UP);
        s = s + timeStep*3;//(0.01*RandStandardNormal()+0.5 + s)/2;
        if (s > 5)
            s = -5.f;

        jackNode_->SetWorldPosition(rot*jackNode_->GetWorldPosition());
        jackNode_->SetWorldRotation(rot*jackNode_->GetWorldRotation());
    }
    else
    {
        Input* in = GetSubsystem<Input>();
        bool f = in->GetKeyDown(KEY_UP) + in->GetKeyDown(KEY_W);
        bool d = in->GetKeyDown(KEY_DOWN) + in->GetKeyDown(KEY_S);
        int turn = in->GetKeyDown(KEY_RIGHT) + in->GetKeyDown(KEY_D);
        turn -= in->GetKeyDown(KEY_LEFT) + in->GetKeyDown(KEY_A);
        jackNode_->Translate(Vector3::FORWARD*v*timeStep*(int)(f-d)*(3+7*in->GetKeyDown(KEY_SHIFT)));

        jackNode_->Rotate({turn * timeStep * 45,Vector3::UP});

        if (JoystickState* js = in->GetJoystickByIndex(joystickIndex_))
        {
//            instructionText_->SetPosition(VectorRoundToInt(64*Vector2(js->axes_[0], js->axes_[1])));
//            instructionText_->SetPosition(VectorRoundToInt(64*Vector2(js->axes_[2], js->axes_[3])) + IntVector2{0,300});

            jackNode_->Translate(-3*Vector3::FORWARD*v*timeStep*js->axes_[1]);
            jackNode_->Translate(Vector3::RIGHT*v*timeStep*js->axes_[2]);


            jackNode_->Rotate({js->axes_[0] * timeStep * 45,Vector3::UP});

            String buttonString;
            for (unsigned i = 0; i < js->buttons_.Size(); ++i)
            {
                buttonString += js->buttonPress_[i] ? "_" : "-";
                buttonString += js->buttons_[i] ? String(i) : &("  "[1-i/10]);
            }
            buttonString += " && ";
            for (unsigned i = 0; i < js->axes_.Size(); ++i)
            {
                buttonString += ToString("%d %f|",i,Round(js->axes_[i]*100)/100);
            }
            uiStrings["Buttons"] = buttonString;
//            instructionText_->SetText(instructionText_->GetText() + "\n" + buttonString);

            auto costheta = rearCameraNode_->GetWorldDirection().ProjectOntoPlane(Vector3::ZERO,Vector3::UP).Length();
            float deadzone = 0.1f;
            float x = js->axes_[2];
            x -= Clamp(x, -deadzone, deadzone);
            float y = js->axes_[3];
            y -= Clamp(y, -deadzone, deadzone);
            rearCameraNode_->Translate(Vector3{ x * costheta * 2, -y, 0} * timeStep * 10);
            rearCameraNode_->LookAt(gv->GetPosition() + gv_offset);

            if (js->buttonPress_[8])
            {
                ResetCamera();
            }
        }

//        else
        {
            float tiltUp = in->GetKeyDown(KEY_KP_8) - in->GetKeyDown(KEY_KP_2);
            float tiltRight = in->GetKeyDown(KEY_KP_6) - in->GetKeyDown(KEY_KP_4);
            float inwards = in->GetKeyDown(KEY_KP_7) - in->GetKeyDown(KEY_KP_1);

            float offsetUp = in->GetKeyDown(KEY_KP_9) - in->GetKeyDown(KEY_KP_3);

            auto diff = Vector3{0, offsetUp, 0} * timeStep * 1;
            gv_offset += diff;
            rearCameraNode_->Translate(diff);

            rearCameraNode_->Translate(Vector3{tiltRight, tiltUp, inwards} * timeStep * 10);
            rearCameraNode_->LookAt(gv->GetPosition() + gv_offset);

            if (in->GetKeyDown(KEY_KP_5))
            {
                ResetCamera();
            }
        }

        {
            float tiltUp = in->GetKeyDown(KEY_0) - in->GetKeyDown(KEY_P);
            float tiltRight = in->GetKeyDown(KEY_RIGHTBRACKET) - in->GetKeyDown(KEY_LEFTBRACKET);
            float inwards = in->GetKeyDown(KEY_MINUS) - in->GetKeyDown(KEY_EQUALS);

            float offsetUp = in->GetKeyDown(KEY_PAGEUP) - in->GetKeyDown(KEY_PAGEDOWN);

            auto diff = Vector3{0, offsetUp, 0} * timeStep * 1;
            gv_offset += diff;
            rearCameraNode_->Translate(diff);

            rearCameraNode_->Translate(Vector3{tiltRight, tiltUp, inwards} * timeStep * 10);
            rearCameraNode_->LookAt(gv->GetPosition() + gv_offset);

            if (in->GetKeyDown(KEY_HOME))
            {
                ResetCamera();
            }
        }
    }



}

void MultipleViewports::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    auto dr = scene_->GetComponent<DebugRenderer>();

    if (drawSkeleton_)
        ShowSkeleton(dr,gv->GetComponent<AnimatedModel>(true)->GetSkeleton(),Color::CYAN, false);
    else
        ClearLabels(dr);

    if (drawPhysics_)
        scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(dr,true);

    // If draw debug mode is enabled, draw viewport debug geometry, which will show eg. drawable bounding boxes and skeleton
    // bones. Disable depth test so that we can see the effect of occlusion
    if (drawDebug_)
    {
//        GetSubsystem<Renderer>()->DrawDebugGeometry(true);

//        for (auto c : gv->GetComponents())
//            c->DrawDebugGeometry(dr,false);


        dr->AddQuad(Vector3::UP,10.f,10.f,Color::RED,true);
        dr->AddQuad(Vector3::UP*2,10.f,10.f,Color::RED,true);
        dr->AddQuad(Vector3::UP*0.0001f,10.f,10.f,Color::BLUE,true);

        dr->AddLine({},Vector3::RIGHT,Color::RED,false);
        dr->AddLine({},Vector3::UP,Color::GREEN,false);
        dr->AddLine({},Vector3::FORWARD,Color::BLUE,false);

//        auto v = Vector3::FORWARD;
//        for (int i = 0; i <= 300; ++i)
//        {
//            dr->AddLine({0,0,0.0},Quaternion(v,Vector3::UP).Slerp({Vector3::RIGHT,Vector3::BACK},0.01*i) * v,Color::GREEN,false);
//        }
//        dr->AddLine({0,0,0.0},Quaternion(v,Vector3::UP).Slerp({v,Vector3::RIGHT},0) * v * 2,Color::CYAN,false);
//        dr->AddLine({0,0,0.0},Quaternion(v,Vector3::UP).Slerp({v,Vector3::RIGHT},1) * v * 2,Color::MAGENTA,false);

//        auto v2 = Vector3::UP*0.1;
//        for (int i = 0; i <= 300; ++i)
//        {
//            auto q = Quaternion(v,Vector3::UP).Slerp({Vector3::RIGHT,Vector3::BACK},0.01*i);
//            dr->AddLine(q * v,q*(v+v2),Color::GREEN,false);
//        }
//        {
//            auto q = Quaternion(v,Vector3::UP).Slerp({v,Vector3::RIGHT},0);
//            dr->AddLine(q * v * 2, q*(v*2+v2),Color::CYAN,false);
//            q = Quaternion(v,Vector3::UP).Slerp({v,Vector3::RIGHT},1);
//            dr->AddLine(q * v * 2, q*(v*2+v2),Color::MAGENTA,false);
//        }

    }
}

void MultipleViewports::AdvanceFrames(int amount)
{

}
