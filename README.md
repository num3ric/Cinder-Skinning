ModelSource Block
================================

This block aims to facilitate game-ready asset loading in Cinder. 

Installation
-------------------------
1. Clone inside the Cinder block folder:

        cd [Cinder]/blocks/
        git clone https://github.com/num3ric/ModelSource.git


2. Run the `INSTALL.sh` script, which downloads and extracts the compiled Assimp static libraries (for Mac, Windows & iOS) into a `lib/` folder.

-------------------------
Alternatively, you can compile Assimp from source by using the latest version on available here: https://github.com/assimp/assimp.git On Mac, we used the following command to compile it: 

```
cmake -G "Unix Makefiles" -DBOOST_ROOT="../cinder_master/boost" -DCMAKE_CXX_FLAGS="-stdlib=libc++ -fvisibility-inlines-hidden" -DCMAKE_BUILD_TYPE=Release -DASSIMP_BUILD_STATIC_LIB=true -DCMAKE_OSX_ARCHITECTURES="i386;x86_64"
```
-------------------------
To create a new project which uses **ModelSource**, use TinderBox to link your project against this block.

Samples Overview
-------------------------
**SeymourDemo**: This is the basic demo. It showcases the standard collada *seymour.dae* file. The animation is played by moving the mouse left and right. Furthermore, any other assets can be tested/loaded by drag-and-dropping them onto the app window.

![01](https://dl.dropboxusercontent.com/u/29102565/block_img/seymour.png "SeymourDemo")
-------------------------
**CustomShaderDemo**: illustrates how it is possible to use custom shaders instead of calling *model::Renderer::draw*. Note however that any custom shader must maintain some skinning code (especially the vertex shader).

![02](https://dl.dropboxusercontent.com/u/29102565/block_img/customShader.png "CustomShaderDemo")
-------------------------
**ArmyDemo**: shows how it is possible to instantiate multiple instances of an asset, where each skeletal mesh can individually have different animation poses.

![03](https://dl.dropboxusercontent.com/u/29102565/block_img/army.png "ArmyDemo")
-------------------------
**ProceduralAnim**: demontrates that it is possible to animate skeletons procedurally by interacting directly with transformations at the bone/node level.

![04](https://dl.dropboxusercontent.com/u/29102565/block_img/proceduralAnim.png "ProceduralAnim")
-------------------------
**ProceduralAnim**: illustrates the functionalities related to an asset with multiple animation cycles, and how these animations can be blended using blend weights.

![05](https://dl.dropboxusercontent.com/u/29102565/block_img/multipleAnim.png "MultipleAnimationsDemo")
-------------------------
~~**KinectDemo**~~: This demo needs to be rewritten.

Common Use
-------------------------

For example, storing and loading a `model::SkeletalMeshRef` called `mCharacter` can be done with the following assignment : `mCharacter = model::SkeletalMesh::create( model::AssimpLoader( loadAsset( ... ) ) );`.

Use the `model::SkeletalTriMesh` class instead of `model::SkeletalMesh` to hold the mesh data on the CPU with `ci::TriMesh` section(s).

The animation is done at the skeleton level, for example: `mCharacter->getSkeleton()->setPose( time );`

However, animating the skeleton doesn't automatically animate a skeletal mesh containing it (for the moment at least) so a `mCharacter->update();` is necessary.

Rendering can be done via our default renderer implementation: `model::Renderer::draw( mCharacter )`

Architecture [OUT OF DATE]
-------------------------
![06](https://dl.dropboxusercontent.com/u/29102565/block_img/architecture.png "Architecture diagram")

TODOs
-------------------------

With the graphics cinder rewrite:
* Reinforcing the loader, fail elegantly
* Asynchronous loading (shared OpenGL context required)

In general:
* Progress on animation API (timeline, animation blending)
* Finish Kinect sample (skeleton remapping?)
* Samples are great, but tests would help, especially for the anim curves code
* Assimp independance so that models can be saved and loaded (via a standard format) without it

