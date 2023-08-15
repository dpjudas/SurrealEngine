cmake_minimum_required(VERSION 3.15)

add_subdirectory( Thirdparty/ZVulkan )
if(WIN32)
add_subdirectory( Thirdparty/openal-soft )
endif()

project(SurrealEngine)

set(SURREALCOMMON_SOURCES
	SurrealEngine/Precomp.cpp
	SurrealEngine/Precomp.h
	SurrealEngine/GameApp.cpp
	SurrealEngine/GameApp.h
	SurrealEngine/EditorApp.cpp
	SurrealEngine/EditorApp.h
	SurrealEngine/DebuggerApp.cpp
	SurrealEngine/DebuggerApp.h
	SurrealEngine/Engine.cpp
	SurrealEngine/Engine.h
	SurrealEngine/File.cpp
	SurrealEngine/File.h
	SurrealEngine/UTF16.cpp
	SurrealEngine/UTF16.h
	SurrealEngine/UTF8Reader.cpp
	SurrealEngine/UTF8Reader.h
	SurrealEngine/JsonValue.cpp
	SurrealEngine/JsonValue.h
	SurrealEngine/GameFolder.cpp
	SurrealEngine/GameFolder.h
	SurrealEngine/CommandLine.cpp
	SurrealEngine/CommandLine.h
	SurrealEngine/Commandlet/Commandlet.cpp
	SurrealEngine/Commandlet/Commandlet.h
	SurrealEngine/Commandlet/ExtractCommandlet.cpp
	SurrealEngine/Commandlet/ExtractCommandlet.h
	SurrealEngine/Commandlet/QuitCommandlet.cpp
	SurrealEngine/Commandlet/QuitCommandlet.h
	SurrealEngine/Commandlet/RunCommandlet.cpp
	SurrealEngine/Commandlet/RunCommandlet.h
	SurrealEngine/Commandlet/Debug/CollisionCommandlet.cpp
	SurrealEngine/Commandlet/Debug/CollisionCommandlet.h
	SurrealEngine/Commandlet/VM/BreakpointCommandlet.cpp
	SurrealEngine/Commandlet/VM/BreakpointCommandlet.h
	SurrealEngine/Commandlet/VM/CallstackCommandlet.cpp
	SurrealEngine/Commandlet/VM/CallstackCommandlet.h
	SurrealEngine/Commandlet/VM/DisassemblyCommandlet.cpp
	SurrealEngine/Commandlet/VM/DisassemblyCommandlet.h
	SurrealEngine/Commandlet/VM/ListSourceCommandlet.cpp
	SurrealEngine/Commandlet/VM/ListSourceCommandlet.h
	SurrealEngine/Commandlet/VM/LocalsCommandlet.cpp
	SurrealEngine/Commandlet/VM/LocalsCommandlet.h
	SurrealEngine/Commandlet/VM/PrintCommandlet.cpp
	SurrealEngine/Commandlet/VM/PrintCommandlet.h
	SurrealEngine/Commandlet/VM/StepCommandlet.cpp
	SurrealEngine/Commandlet/VM/StepCommandlet.h
	SurrealEngine/Render/RenderSubsystem.cpp
	SurrealEngine/Render/RenderSubsystem.h
	SurrealEngine/Render/RenderCanvas.cpp
	SurrealEngine/Render/RenderMesh.cpp
	SurrealEngine/Render/RenderBrush.cpp
	SurrealEngine/Render/RenderSprite.cpp
	SurrealEngine/Render/RenderCorona.cpp
	SurrealEngine/Render/RenderDecal.cpp
	SurrealEngine/Render/RenderScene.cpp
	SurrealEngine/Render/RenderLight.cpp
	SurrealEngine/Render/RenderFog.cpp
	SurrealEngine/Render/Lightmap/LightEffect.cpp
	SurrealEngine/Render/Lightmap/LightEffect.h
	SurrealEngine/Render/Lightmap/LightmapBuilder.cpp
	SurrealEngine/Render/Lightmap/LightmapBuilder.h
	SurrealEngine/Render/Lightmap/Shadowmap.cpp
	SurrealEngine/Render/Lightmap/Shadowmap.h
	SurrealEngine/Render/Lightmap/FogmapBuilder.cpp
	SurrealEngine/Render/Lightmap/FogmapBuilder.h
	SurrealEngine/VM/NativeFuncExtractor.h
	SurrealEngine/VM/NativeFunc.cpp
	SurrealEngine/VM/NativeFuncExtractor.cpp
	SurrealEngine/VM/Frame.cpp
	SurrealEngine/VM/ExpressionValue.h
	SurrealEngine/VM/ScriptCall.cpp
	SurrealEngine/VM/Bytecode.cpp
	SurrealEngine/VM/Bytecode.h
	SurrealEngine/VM/ScriptCall.h
	SurrealEngine/VM/ExpressionEvaluator.h
	SurrealEngine/VM/Frame.h
	SurrealEngine/VM/NativeFunc.h
	SurrealEngine/VM/Expression.h
	SurrealEngine/VM/ExpressionEvaluator.cpp
	SurrealEngine/VM/ExpressionVisitor.h
	SurrealEngine/VM/Iterator.cpp
	SurrealEngine/VM/Iterator.h
	SurrealEngine/Audio/AudioSource.h
	SurrealEngine/Audio/AudioSource.cpp
	SurrealEngine/Audio/AudioDevice.cpp
	SurrealEngine/Audio/AudioDevice.h
	SurrealEngine/Audio/AudioSubsystem.cpp
	SurrealEngine/Audio/AudioSubsystem.h
	SurrealEngine/Native/NStatLog.h
	SurrealEngine/Native/NZoneInfo.cpp
	SurrealEngine/Native/NNavigationPoint.cpp
	SurrealEngine/Native/NLevelInfo.h
	SurrealEngine/Native/NScriptedTexture.cpp
	SurrealEngine/Native/NPawn.cpp
	SurrealEngine/Native/NGameInfo.h
	SurrealEngine/Native/NGameInfo.cpp
	SurrealEngine/Native/NStatLogFile.h
	SurrealEngine/Native/NWarpZoneInfo.cpp
	SurrealEngine/Native/NScriptedTexture.h
	SurrealEngine/Native/NObject.h
	SurrealEngine/Native/NActor.cpp
	SurrealEngine/Native/NPlayerPawn.h
	SurrealEngine/Native/NStatLog.cpp
	SurrealEngine/Native/NStatLogFile.cpp
	SurrealEngine/Native/NPawn.h
	SurrealEngine/Native/NConsole.h
	SurrealEngine/Native/NWarpZoneInfo.h
	SurrealEngine/Native/NLevelInfo.cpp
	SurrealEngine/Native/NCanvas.h
	SurrealEngine/Native/NCommandlet.h
	SurrealEngine/Native/NConsole.cpp
	SurrealEngine/Native/NNavigationPoint.h
	SurrealEngine/Native/NCanvas.cpp
	SurrealEngine/Native/NPlayerPawn.cpp
	SurrealEngine/Native/NCommandlet.cpp
	SurrealEngine/Native/NDecal.cpp
	SurrealEngine/Native/NActor.h
	SurrealEngine/Native/NObject.cpp
	SurrealEngine/Native/NDecal.h
	SurrealEngine/Native/NZoneInfo.h
	SurrealEngine/Native/NInternetLink.cpp
	SurrealEngine/Native/NInternetLink.h
	SurrealEngine/Native/NTcpLink.cpp
	SurrealEngine/Native/NTcpLink.h
	SurrealEngine/Native/NUdpLink.cpp
	SurrealEngine/Native/NUdpLink.h
	SurrealEngine/Native/NDebugInfo.cpp
	SurrealEngine/Native/NDebugInfo.h
	SurrealEngine/Native/NDeusExPlayer.cpp
	SurrealEngine/Native/NDeusExPlayer.h
	SurrealEngine/Native/NScriptedPawn.cpp
	SurrealEngine/Native/NScriptedPawn.h
	SurrealEngine/Native/NDeusExDecoration.cpp
	SurrealEngine/Native/NDeusExDecoration.h
	SurrealEngine/Native/NDumpLocation.cpp
	SurrealEngine/Native/NDumpLocation.h
	SurrealEngine/Native/NDeusExSaveInfo.cpp
	SurrealEngine/Native/NDeusExSaveInfo.h
	SurrealEngine/Native/NGameDirectory.cpp
	SurrealEngine/Native/NGameDirectory.h
	SurrealEngine/Native/NParticleIterator.cpp
	SurrealEngine/Native/NParticleIterator.h
	SurrealEngine/Native/NPlayerPawnExt.cpp
	SurrealEngine/Native/NPlayerPawnExt.h
	SurrealEngine/RenderDevice/RenderDevice.cpp
	SurrealEngine/RenderDevice/RenderDevice.h
	SurrealEngine/RenderDevice/Vulkan/BufferManager.cpp
	SurrealEngine/RenderDevice/Vulkan/BufferManager.h
	SurrealEngine/RenderDevice/Vulkan/CachedTexture.h
	SurrealEngine/RenderDevice/Vulkan/CommandBufferManager.cpp
	SurrealEngine/RenderDevice/Vulkan/CommandBufferManager.h
	SurrealEngine/RenderDevice/Vulkan/DescriptorSetManager.cpp
	SurrealEngine/RenderDevice/Vulkan/DescriptorSetManager.h
	SurrealEngine/RenderDevice/Vulkan/FileResource.cpp
	SurrealEngine/RenderDevice/Vulkan/FileResource.h
	SurrealEngine/RenderDevice/Vulkan/FramebufferManager.cpp
	SurrealEngine/RenderDevice/Vulkan/FramebufferManager.h
	SurrealEngine/RenderDevice/Vulkan/RenderPassManager.cpp
	SurrealEngine/RenderDevice/Vulkan/RenderPassManager.h
	SurrealEngine/RenderDevice/Vulkan/SamplerManager.cpp
	SurrealEngine/RenderDevice/Vulkan/SamplerManager.h
	SurrealEngine/RenderDevice/Vulkan/SceneTextures.cpp
	SurrealEngine/RenderDevice/Vulkan/SceneTextures.h
	SurrealEngine/RenderDevice/Vulkan/ShaderManager.cpp
	SurrealEngine/RenderDevice/Vulkan/ShaderManager.h
	SurrealEngine/RenderDevice/Vulkan/TextureManager.cpp
	SurrealEngine/RenderDevice/Vulkan/TextureManager.h
	SurrealEngine/RenderDevice/Vulkan/TextureUploader.cpp
	SurrealEngine/RenderDevice/Vulkan/TextureUploader.h
	SurrealEngine/RenderDevice/Vulkan/UploadManager.cpp
	SurrealEngine/RenderDevice/Vulkan/UploadManager.h
	SurrealEngine/RenderDevice/Vulkan/VulkanRenderDevice.cpp
	SurrealEngine/RenderDevice/Vulkan/VulkanRenderDevice.h
	SurrealEngine/Package/ObjectFlags.h
	SurrealEngine/Package/Package.h
	SurrealEngine/Package/ObjectStream.h
	SurrealEngine/Package/PackageManager.cpp
	SurrealEngine/Package/PackageFlags.h
	SurrealEngine/Package/Package.cpp
	SurrealEngine/Package/PackageManager.h
	SurrealEngine/Package/PackageStream.cpp
	SurrealEngine/Package/IniFile.h
	SurrealEngine/Package/PackageStream.h
	SurrealEngine/Package/IniFile.cpp
	SurrealEngine/Package/NameString.cpp
	SurrealEngine/Package/NameString.h
	SurrealEngine/Math/FrustumPlanes.h
	SurrealEngine/Math/quaternion.h
	SurrealEngine/Math/FrustumPlanes.cpp
	SurrealEngine/Math/halffloat.cpp
	SurrealEngine/Math/hsb.cpp
	SurrealEngine/Math/halffloat.h
	SurrealEngine/Math/mat.h
	SurrealEngine/Math/vec.h
	SurrealEngine/Math/rotator.h
	SurrealEngine/Math/mat.cpp
	SurrealEngine/Math/bbox.h
	SurrealEngine/Math/hsb.h
	SurrealEngine/Math/coords.h
	SurrealEngine/GC/GC.cpp
	SurrealEngine/GC/GC.h
	SurrealEngine/UObject/ULevel.cpp
	SurrealEngine/UObject/PropertyOffsets.cpp
	SurrealEngine/UObject/UMusic.cpp
	SurrealEngine/UObject/UClient.cpp
	SurrealEngine/UObject/UActor.cpp
	SurrealEngine/UObject/ULevel.h
	SurrealEngine/UObject/UClass.cpp
	SurrealEngine/UObject/UTexture.cpp
	SurrealEngine/UObject/UFont.cpp
	SurrealEngine/UObject/UObject.h
	SurrealEngine/UObject/NativeObjExtractor.cpp
	SurrealEngine/UObject/UProperty.h
	SurrealEngine/UObject/UMusic.h
	SurrealEngine/UObject/UTexture.h
	SurrealEngine/UObject/USound.h
	SurrealEngine/UObject/UClient.h
	SurrealEngine/UObject/NativeObjExtractor.h
	SurrealEngine/UObject/UMesh.cpp
	SurrealEngine/UObject/UMesh.h
	SurrealEngine/UObject/UProperty.cpp
	SurrealEngine/UObject/UActor.h
	SurrealEngine/UObject/USound.cpp
	SurrealEngine/UObject/UFont.h
	SurrealEngine/UObject/UTextBuffer.cpp
	SurrealEngine/UObject/UClass.h
	SurrealEngine/UObject/UObject.cpp
	SurrealEngine/UObject/UTextBuffer.h
	SurrealEngine/UObject/UInternetLink.cpp
	SurrealEngine/UObject/UInternetLink.h
	SurrealEngine/UObject/PropertyOffsets.h
	SurrealEngine/Collision/CollisionHash.cpp
	SurrealEngine/Collision/CollisionHash.h
	SurrealEngine/Collision/CollisionHit.h
	SurrealEngine/Collision/TraceRayLevel.cpp
	SurrealEngine/Collision/TraceRayLevel.h
	SurrealEngine/Collision/TraceRayModel.cpp
	SurrealEngine/Collision/TraceRayModel.h
	SurrealEngine/Collision/TraceCylinderLevel.cpp
	SurrealEngine/Collision/TraceCylinderLevel.h
	SurrealEngine/Collision/TraceAABBModel.cpp
	SurrealEngine/Collision/TraceAABBModel.h
	SurrealEngine/Collision/OverlapCylinderLevel.cpp
	SurrealEngine/Collision/OverlapCylinderLevel.h
	SurrealEngine/Collision/OverlapAABBModel.cpp
	SurrealEngine/Collision/OverlapAABBModel.h
	SurrealEngine/UI/Core/Widget.cpp
	SurrealEngine/UI/Core/Widget.h
	SurrealEngine/UI/Core/Canvas.cpp
	SurrealEngine/UI/Core/Canvas.h
	SurrealEngine/UI/Core/Rect.h
	SurrealEngine/UI/Core/Colorf.h
	SurrealEngine/UI/Widgets/MainWindow/MainWindow.cpp
	SurrealEngine/UI/Widgets/MainWindow/MainWindow.h
	SurrealEngine/UI/Widgets/Menubar/Menubar.cpp
	SurrealEngine/UI/Widgets/Menubar/Menubar.h
	SurrealEngine/UI/Widgets/Toolbar/Toolbar.cpp
	SurrealEngine/UI/Widgets/Toolbar/Toolbar.h
	SurrealEngine/UI/Widgets/Toolbar/ToolbarButton.cpp
	SurrealEngine/UI/Widgets/Toolbar/ToolbarButton.h
	SurrealEngine/UI/Widgets/Statusbar/Statusbar.cpp
	SurrealEngine/UI/Widgets/Statusbar/Statusbar.h
	SurrealEngine/UI/Editor/EditorMainWindow.cpp
	SurrealEngine/UI/Editor/EditorMainWindow.h
	SurrealEngine/UI/Editor/EditorWorkspace.cpp
	SurrealEngine/UI/Editor/EditorWorkspace.h
	SurrealEngine/UI/Editor/EditorViewport.cpp
	SurrealEngine/UI/Editor/EditorViewport.h
	SurrealEngine/Window/Window.cpp
	SurrealEngine/Window/Window.h
)

set(SURREALCOMMON_WIN32_SOURCES
	SurrealEngine/Window/Win32/Win32Window.cpp
	SurrealEngine/Window/Win32/Win32Window.h
	SurrealEngine/SurrealEngine.natvis
)

set(SURREALCOMMON_UNIX_SOURCES
	SurrealEngine/Window/X11/X11Window.cpp
	SurrealEngine/Window/X11/X11Window.h
	SurrealEngine/Window/X11/X11Atoms.cpp
	SurrealEngine/Window/X11/X11Atoms.h
)

set(THIRDPARTY_SOURCES
	Thirdparty/dr_wav.h
	Thirdparty/minimp3_ex.h
	Thirdparty/minimp3.h
	Thirdparty/dumb/src/core/duhtag.c
	Thirdparty/dumb/src/core/rawsig.c
	Thirdparty/dumb/src/core/atexit.c
	Thirdparty/dumb/src/core/dumbfile.c
	Thirdparty/dumb/src/core/loadduh.c
	Thirdparty/dumb/src/core/unload.c
	Thirdparty/dumb/src/core/makeduh.c
	Thirdparty/dumb/src/core/readduh.c
	Thirdparty/dumb/src/core/rendsig.c
	Thirdparty/dumb/src/core/rendduh.c
	Thirdparty/dumb/src/core/register.c
	Thirdparty/dumb/src/core/duhlen.c
	Thirdparty/dumb/src/helpers/lpc.c
	Thirdparty/dumb/src/helpers/clickrem.c
	Thirdparty/dumb/src/helpers/memfile.c
	Thirdparty/dumb/src/helpers/silence.c
	Thirdparty/dumb/src/helpers/tarray.c
	Thirdparty/dumb/src/helpers/sampbuf.c
	Thirdparty/dumb/src/helpers/riff.c
	Thirdparty/dumb/src/helpers/barray.c
	Thirdparty/dumb/src/helpers/resample.c
	Thirdparty/dumb/src/helpers/resampler.c
	Thirdparty/dumb/src/helpers/stdfile.c
	Thirdparty/dumb/src/it/readoldpsm.c
	Thirdparty/dumb/src/it/readdsmf.c
	Thirdparty/dumb/src/it/loadokt.c
	Thirdparty/dumb/src/it/loadamf.c
	Thirdparty/dumb/src/it/itorder.c
	Thirdparty/dumb/src/it/loadxm2.c
	Thirdparty/dumb/src/it/xmeffect.c
	Thirdparty/dumb/src/it/loadxm.c
	Thirdparty/dumb/src/it/readmod.c
	Thirdparty/dumb/src/it/readxm2.c
	Thirdparty/dumb/src/it/reads3m2.c
	Thirdparty/dumb/src/it/loadokt2.c
	Thirdparty/dumb/src/it/read669.c
	Thirdparty/dumb/src/it/readokt2.c
	Thirdparty/dumb/src/it/loadany.c
	Thirdparty/dumb/src/it/ptmeffect.c
	Thirdparty/dumb/src/it/readstm2.c
	Thirdparty/dumb/src/it/loadoldpsm2.c
	Thirdparty/dumb/src/it/read6692.c
	Thirdparty/dumb/src/it/loads3m.c
	Thirdparty/dumb/src/it/loadmod.c
	Thirdparty/dumb/src/it/readriff.c
	Thirdparty/dumb/src/it/readam.c
	Thirdparty/dumb/src/it/loadmtm.c
	Thirdparty/dumb/src/it/itunload.c
	Thirdparty/dumb/src/it/loadpsm.c
	Thirdparty/dumb/src/it/itrender.c
	Thirdparty/dumb/src/it/readxm.c
	Thirdparty/dumb/src/it/readstm.c
	Thirdparty/dumb/src/it/itmisc.c
	Thirdparty/dumb/src/it/itload.c
	Thirdparty/dumb/src/it/loadasy2.c
	Thirdparty/dumb/src/it/loadriff.c
	Thirdparty/dumb/src/it/load669.c
	Thirdparty/dumb/src/it/loads3m2.c
	Thirdparty/dumb/src/it/loadpsm2.c
	Thirdparty/dumb/src/it/readany2.c
	Thirdparty/dumb/src/it/readamf.c
	Thirdparty/dumb/src/it/itload2.c
	Thirdparty/dumb/src/it/itread2.c
	Thirdparty/dumb/src/it/loadriff2.c
	Thirdparty/dumb/src/it/loadasy.c
	Thirdparty/dumb/src/it/loadany2.c
	Thirdparty/dumb/src/it/readany.c
	Thirdparty/dumb/src/it/readpsm.c
	Thirdparty/dumb/src/it/readamf2.c
	Thirdparty/dumb/src/it/loadamf2.c
	Thirdparty/dumb/src/it/loadptm.c
	Thirdparty/dumb/src/it/load6692.c
	Thirdparty/dumb/src/it/readptm.c
	Thirdparty/dumb/src/it/loadoldpsm.c
	Thirdparty/dumb/src/it/readmtm.c
	Thirdparty/dumb/src/it/loadmtm2.c
	Thirdparty/dumb/src/it/loadstm.c
	Thirdparty/dumb/src/it/readasy.c
	Thirdparty/dumb/src/it/loadptm2.c
	Thirdparty/dumb/src/it/reads3m.c
	Thirdparty/dumb/src/it/itread.c
	Thirdparty/dumb/src/it/readmod2.c
	Thirdparty/dumb/src/it/readokt.c
	Thirdparty/dumb/src/it/loadmod2.c
	Thirdparty/dumb/src/it/loadstm2.c
	Thirdparty/dumb/include/aldumb.h
	Thirdparty/dumb/include/internal/stack_alloc.h
	Thirdparty/dumb/include/internal/dumbfile.h
	Thirdparty/dumb/include/internal/resampler.h
	Thirdparty/dumb/include/internal/aldumb.h
	Thirdparty/dumb/include/internal/dumb.h
	Thirdparty/dumb/include/internal/barray.h
	Thirdparty/dumb/include/internal/tarray.h
	Thirdparty/dumb/include/internal/lpc.h
	Thirdparty/dumb/include/internal/it.h
	Thirdparty/dumb/include/internal/riff.h
	Thirdparty/dumb/include/dumb.h
	Thirdparty/resample/CDSPSincFilterGen.h
	Thirdparty/resample/CDSPHBDownsampler.h
	Thirdparty/resample/CDSPResampler.h
	Thirdparty/resample/CDSPRealFFT.h
	Thirdparty/resample/CDSPFIRFilter.h
	Thirdparty/resample/pffft.cpp
	Thirdparty/resample/r8butil.h
	Thirdparty/resample/CDSPHBUpsampler.h
	Thirdparty/resample/CDSPProcessor.h
	Thirdparty/resample/r8bbase.cpp
	Thirdparty/resample/pffft.h
	Thirdparty/resample/CDSPFracInterpolator.h
	Thirdparty/resample/CDSPBlockConvolver.h
	Thirdparty/resample/fft4g.h
	Thirdparty/resample/r8bconf.h
	Thirdparty/resample/r8bbase.h
	Thirdparty/dr_flac.h
	Thirdparty/stb_vorbis.h
	Thirdparty/schrift/schrift.cpp
	Thirdparty/schrift/schrift.h
)

set(THIRDPARTY_WIN32_SOURCES
)

set(THIRDPARTY_UNIX_SOURCES
)

set(SURREALCOMMON_LIBS
	zvulkan
)

set(SURREALCOMMON_WIN32_LIBS
	zvulkan
	OpenAL
)

set(SURREALCOMMON_UNIX_LIBS
	zvulkan
	openal
)

set(SURREALENGINE_SOURCES
	SurrealEngine/MainGame.cpp
)

set(SURREALEDITOR_SOURCES
	SurrealEngine/MainEditor.cpp
)

set(SURREALDEBUGGER_SOURCES
	SurrealEngine/MainDebugger.cpp
)

source_group("SurrealEngine" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/.+")
source_group("SurrealEngine\\Audio" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Audio/.+")
source_group("SurrealEngine\\Math" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Math/.+")
source_group("SurrealEngine\\Native" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Native/.+")
source_group("SurrealEngine\\Package" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Package/.+")
source_group("SurrealEngine\\RenderDevice" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/RenderDevice/.+")
source_group("SurrealEngine\\RenderDevice/Vulkan" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/RenderDevice/Vulkan/.+")
source_group("SurrealEngine\\Render" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Render/.+")
source_group("SurrealEngine\\Render\\Lightmap" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Render/Lightmap/.+")
source_group("SurrealEngine\\UI" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/UI/.+")
source_group("SurrealEngine\\UI\\Core" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/UI/Core/.+")
source_group("SurrealEngine\\UI\\Widgets" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/UI/Widgets/.+")
source_group("SurrealEngine\\UI\\Widgets\\MainWindow" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/UI/Widgets/MainWindow/.+")
source_group("SurrealEngine\\UI\\Widgets\\Menubar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/UI/Widgets/Menubar/.+")
source_group("SurrealEngine\\UI\\Widgets\\Statusbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/UI/Widgets/Statusbar/.+")
source_group("SurrealEngine\\UI\\Widgets\\Toolbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/UI/Widgets/Toolbar/.+")
source_group("SurrealEngine\\UI\\Editor" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/UI/Editor/.+")
source_group("SurrealEngine\\UObject" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/UObject/.+")
source_group("SurrealEngine\\Collision" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Collision/.+")
source_group("SurrealEngine\\Commandlet" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Commandlet/.+")
source_group("SurrealEngine\\Commandlet\\Debug" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Commandlet/Debug/.+")
source_group("SurrealEngine\\Commandlet\\VM" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Commandlet/VM/.+")
source_group("SurrealEngine\\GC" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/GC/.+")
source_group("SurrealEngine\\VM" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/VM/.+")
source_group("SurrealEngine\\Window" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Window/.+")
source_group("SurrealEngine\\Window\\Win32" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Window/Win32/.+")
source_group("SurrealEngine\\Window\\X11" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Window/X11/.+")
source_group("Thirdparty" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/.+")
source_group("Thirdparty\\resample" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/resample/.+")
source_group("Thirdparty\\dumb" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/dumb/.+")
source_group("Thirdparty\\dumb\\src" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/dumb/src/.+")
source_group("Thirdparty\\dumb\\src\\core" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/dumb/src/core.+")
source_group("Thirdparty\\dumb\\src\\helpers" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/dumb/src/helpers/.+")
source_group("Thirdparty\\dumb\\src\\it" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/dumb/src/it/.+")
source_group("Thirdparty\\dumb\\include" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/dumb/include/.+")
source_group("Thirdparty\\dumb\\include\\internal" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/dumb/include/internal/.+")
source_group("Thirdparty\\schrift" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/schrift/.+")

include_directories(SurrealEngine Thirdparty Thirdparty/ZVulkan/include Thirdparty/dumb/include Thirdparty/openal-soft/include)

if(WIN32)
	set(SURREALCOMMON_SOURCES ${SURREALCOMMON_SOURCES} ${SURREALCOMMON_WIN32_SOURCES})
	set(THIRDPARTY_SOURCES ${THIRDPARTY_SOURCES} ${THIRDPARTY_WIN32_SOURCES})
	set(SURREALCOMMON_LIBS ${SURREALCOMMON_LIBS} ${SURREALCOMMON_WIN32_LIBS})
	add_definitions(-DUNICODE -D_UNICODE)
elseif(APPLE)
	set(SURREALCOMMON_SOURCES ${SURREALCOMMON_SOURCES} ${SURREALCOMMON_UNIX_SOURCES})
	set(THIRDPARTY_SOURCES ${THIRDPARTY_SOURCES} ${THIRDPARTY_UNIX_SOURCES})
	set(SURREALCOMMON_LIBS ${SURREALCOMMON_LIBS} ${SURREALCOMMON_UNIX_LIBS} ${CMAKE_DL_LIBS})
	add_definitions(-DUNIX -D_UNIX)
	add_link_options(-pthread)
else()
	set(SURREALCOMMON_SOURCES ${SURREALCOMMON_SOURCES} ${SURREALCOMMON_UNIX_SOURCES})
	set(THIRDPARTY_SOURCES ${THIRDPARTY_SOURCES} ${THIRDPARTY_UNIX_SOURCES})
	set(SURREALCOMMON_LIBS ${SURREALCOMMON_LIBS} ${SURREALCOMMON_UNIX_LIBS} ${CMAKE_DL_LIBS} -lX11 -ldl -lasound)
	add_definitions(-DUNIX -D_UNIX)
	add_link_options(-pthread)
endif()

if(MSVC)
	# Use all cores for compilation
	set(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS} /D_ITERATOR_DEBUG_LEVEL=0")

	# Ignore warnings in third party code
	set_source_files_properties(${THIRDPARTY_SOURCES} PROPERTIES COMPILE_FLAGS "/wd4244 /wd4267 /wd4005 /wd4018 -D_CRT_SECURE_NO_WARNINGS")
endif()

add_library(SurrealCommon STATIC ${SURREALCOMMON_SOURCES} ${THIRDPARTY_SOURCES})
set_target_properties(SurrealCommon PROPERTIES CXX_STANDARD 17)
target_precompile_headers(SurrealCommon PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/SurrealEngine/Precomp.h>)

add_executable(SurrealEngine WIN32 MACOSX_BUNDLE ${SURREALENGINE_SOURCES})
target_link_libraries(SurrealEngine SurrealCommon ${SURREALCOMMON_LIBS})
set_target_properties(SurrealEngine PROPERTIES CXX_STANDARD 17)

add_executable(SurrealEditor WIN32 MACOSX_BUNDLE ${SURREALEDITOR_SOURCES})
target_link_libraries(SurrealEditor SurrealCommon ${SURREALCOMMON_LIBS})
set_target_properties(SurrealEditor PROPERTIES CXX_STANDARD 17)

add_executable(SurrealDebugger ${SURREALDEBUGGER_SOURCES})
target_link_libraries(SurrealDebugger SurrealCommon ${SURREALCOMMON_LIBS})
set_target_properties(SurrealDebugger PROPERTIES CXX_STANDARD 17)

if(WIN32)
add_custom_command(TARGET SurrealEngine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:OpenAL::OpenAL>" $<TARGET_FILE_DIR:SurrealEngine>)
endif()

install(TARGETS SurrealEngine RUNTIME DESTINATION bin BUNDLE DESTINATION app)
install(TARGETS SurrealEditor RUNTIME DESTINATION bin BUNDLE DESTINATION app)
install(TARGETS SurrealDebugger RUNTIME DESTINATION bin)
