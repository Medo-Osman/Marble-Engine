#ifndef PCH_H
#define PCH_H

#ifndef UNICODE
#define UNICODE
#endif

// Standard
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <comdef.h>
#include <assert.h>
#include <memory>
#include <wrl/client.h>
#include <windows.applicationmodel.core.h>
#include <Shobjidl.h>
//#define NOMINMAX // std::min/std::max wont work without this
#include <algorithm>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <ctime>

#include <math.h>
#include <fstream>
#include <stack>
#include <random>
#include <algorithm>
#include <iterator>
#include <utility>
#include <codecvt>
#include <locale>
#include <sys/types.h>

// Utilities
#include "Timer.h"
#include "KeyCodes.h"
#include "dirent.h"
#include "StringUtilities.h"
#include "MathUtilities.h"
#include "MapFileStructs.h"

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/pbrmaterial.h>

// ImGUI
#include <imGUI/imgui.h>
#include <imGUI/imgui_impl_win32.h>
#include <imGUI/imgui_impl_dx11.h>
#include <imGUI/imgui_internal.h>
#include <imGUI/imfilebrowser.h>

// DirectX Helpers
#include "VertexTypeList.h"
#include "ConstantBufferStructs.h"
#include "TextureHelper.h"
#include "ShaderHelper.h"
#include "Buffer.h"

// DirectX
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <SimpleMath.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <DirectXTex.h>
#include <DDSTextureLoader.h>
#include <CommonStates.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <PrimitiveBatch.h>
#include <Effects.h>
#include <VertexTypes.h>
#include <DirectXColors.h>
#include "DebugDraw.h"

using namespace DirectX;
using namespace Microsoft::WRL;

// DirectX 11 Linking
#pragma comment (lib, "user32")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "gdi32")
#pragma comment (lib, "d3dcompiler.lib")

#endif //!PCH_H