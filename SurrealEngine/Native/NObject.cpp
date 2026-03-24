
#include "Precomp.h"
#include "NObject.h"
#include "VM/NativeFunc.h"
#include "VM/Frame.h"
#include "Package/PackageManager.h"
#include "Engine.h"
#include "Math/quaternion.h"
#include "Utils/StrCompare.h"
#include "Utils/Random.h"
#include "../Package/ObjectFlags.h"
#include <cmath>

#ifdef _MSC_VER
#pragma warning(disable: 4244) // warning C4244: '/=': conversion from 'float' to 'int', possible loss of data
#endif

// TODO: assign native indices based on game, not hardcoded
void NObject::RegisterFunctions()
{
	RegisterVMNativeFunc_2("Object", "Abs", &NObject::Abs, 186);
	RegisterVMNativeFunc_2("Object", "AddAdd_Byte", &NObject::AddAdd_Byte, 139);
	RegisterVMNativeFunc_2("Object", "AddAdd_Int", &NObject::AddAdd_Int, 165);
	RegisterVMNativeFunc_2("Object", "AddAdd_PreByte", &NObject::AddAdd_PreByte, 137);
	RegisterVMNativeFunc_2("Object", "AddAdd_PreInt", &NObject::AddAdd_PreInt, 163);
	RegisterVMNativeFunc_3("Object", "AddEqual_ByteByte", &NObject::AddEqual_ByteByte, 135);
	RegisterVMNativeFunc_3("Object", "AddEqual_FloatFloat", &NObject::AddEqual_FloatFloat, 184);
	RegisterVMNativeFunc_3("Object", "AddEqual_IntInt", &NObject::AddEqual_IntInt, 161);
	RegisterVMNativeFunc_3("Object", "AddEqual_RotatorRotator", &NObject::AddEqual_RotatorRotator, 318);
	RegisterVMNativeFunc_3("Object", "AddEqual_VectorVector", &NObject::AddEqual_VectorVector, 223);
	RegisterVMNativeFunc_3("Object", "Add_FloatFloat", &NObject::Add_FloatFloat, 174);
	RegisterVMNativeFunc_3("Object", "Add_IntInt", &NObject::Add_IntInt, 146);
	RegisterVMNativeFunc_3("Object", "Add_RotatorRotator", &NObject::Add_RotatorRotator, 316);
	RegisterVMNativeFunc_3("Object", "Add_VectorVector", &NObject::Add_VectorVector, 215);
	RegisterVMNativeFunc_3("Object", "AndAnd_BoolBool", &NObject::AndAnd_BoolBool, 130);
	RegisterVMNativeFunc_3("Object", "And_IntInt", &NObject::And_IntInt, 156);
	if (engine->LaunchInfo.engineVersion > 219)
		RegisterVMNativeFunc_2("Object", "Asc", &NObject::Asc, 237);
	else
		RegisterVMNativeFunc_2("Object", "Asc", &NObject::Asc, 0);
	RegisterVMNativeFunc_3("Object", "At_StrStr", &NObject::At_StrStr, 168);
	RegisterVMNativeFunc_2("Object", "Atan", &NObject::Atan, 190);
	RegisterVMNativeFunc_2("Object", "Caps", &NObject::Caps, 235);
	if (engine->LaunchInfo.engineVersion > 219)
		RegisterVMNativeFunc_2("Object", "Chr", &NObject::Chr, 236);
	else
		RegisterVMNativeFunc_2("Object", "Chr", &NObject::Chr, 0);
	RegisterVMNativeFunc_4("Object", "Clamp", &NObject::Clamp, 251);
	RegisterVMNativeFunc_3("Object", "ClassIsChildOf", &NObject::ClassIsChildOf, 258);
	RegisterVMNativeFunc_3("Object", "ComplementEqual_FloatFloat", &NObject::ComplementEqual_FloatFloat, 210);
	RegisterVMNativeFunc_3("Object", "ComplementEqual_StrStr", &NObject::ComplementEqual_StrStr, 124);
	RegisterVMNativeFunc_2("Object", "Complement_PreInt", &NObject::Complement_PreInt, 141);
	RegisterVMNativeFunc_3("Object", "Concat_StrStr", &NObject::Concat_StrStr, 112);
	RegisterVMNativeFunc_2("Object", "Cos", &NObject::Cos, 188);
	RegisterVMNativeFunc_3("Object", "Cross_VectorVector", &NObject::Cross_VectorVector, 220);
	RegisterVMNativeFunc_1("Object", "Disable", &NObject::Disable, 118);
	RegisterVMNativeFunc_3("Object", "DivideEqual_ByteByte", &NObject::DivideEqual_ByteByte, 134);
	RegisterVMNativeFunc_3("Object", "DivideEqual_FloatFloat", &NObject::DivideEqual_FloatFloat, 183);
	RegisterVMNativeFunc_3("Object", "DivideEqual_IntFloat", &NObject::DivideEqual_IntFloat, 160);
	RegisterVMNativeFunc_3("Object", "DivideEqual_RotatorFloat", &NObject::DivideEqual_RotatorFloat, 291);
	RegisterVMNativeFunc_3("Object", "DivideEqual_VectorFloat", &NObject::DivideEqual_VectorFloat, 222);
	RegisterVMNativeFunc_3("Object", "Divide_FloatFloat", &NObject::Divide_FloatFloat, 172);
	RegisterVMNativeFunc_3("Object", "Divide_IntInt", &NObject::Divide_IntInt, 145);
	RegisterVMNativeFunc_3("Object", "Divide_RotatorFloat", &NObject::Divide_RotatorFloat, 289);
	RegisterVMNativeFunc_3("Object", "Divide_VectorFloat", &NObject::Divide_VectorFloat, 214);
	RegisterVMNativeFunc_3("Object", "Dot_VectorVector", &NObject::Dot_VectorVector, 219);
	if (engine->LaunchInfo.engineVersion > 219)
		RegisterVMNativeFunc_4("Object", "DynamicLoadObject", &NObject::DynamicLoadObject, 0);
	else
		RegisterVMNativeFunc_3("Object", "DynamicLoadObject", &NObject::DynamicLoadObject_219, 0);
	RegisterVMNativeFunc_1("Object", "Enable", &NObject::Enable, 117);
	RegisterVMNativeFunc_3("Object", "EqualEqual_BoolBool", &NObject::EqualEqual_BoolBool, 242);
	RegisterVMNativeFunc_3("Object", "EqualEqual_FloatFloat", &NObject::EqualEqual_FloatFloat, 180);
	RegisterVMNativeFunc_3("Object", "EqualEqual_IntInt", &NObject::EqualEqual_IntInt, 154);
	RegisterVMNativeFunc_3("Object", "EqualEqual_NameName", &NObject::EqualEqual_NameName, 254);
	RegisterVMNativeFunc_3("Object", "EqualEqual_ObjectObject", &NObject::EqualEqual_ObjectObject, 114);
	RegisterVMNativeFunc_3("Object", "EqualEqual_RotatorRotator", &NObject::EqualEqual_RotatorRotator, 142);
	RegisterVMNativeFunc_3("Object", "EqualEqual_StrStr", &NObject::EqualEqual_StrStr, 122);
	RegisterVMNativeFunc_3("Object", "EqualEqual_VectorVector", &NObject::EqualEqual_VectorVector, 217);
	RegisterVMNativeFunc_2("Object", "Exp", &NObject::Exp, 191);
	RegisterVMNativeFunc_4("Object", "FClamp", &NObject::FClamp, 246);
	RegisterVMNativeFunc_3("Object", "FMax", &NObject::FMax, 245);
	RegisterVMNativeFunc_3("Object", "FMin", &NObject::FMin, 244);
	RegisterVMNativeFunc_1("Object", "FRand", &NObject::FRand, 195);
	RegisterVMNativeFunc_4("Object", "GetAxes", &NObject::GetAxes, 229);
	RegisterVMNativeFunc_3("Object", "GetEnum", &NObject::GetEnum, 0);
	RegisterVMNativeFunc_2("Object", "GetPropertyText", &NObject::GetPropertyText, 0);
	RegisterVMNativeFunc_1("Object", "GetStateName", &NObject::GetStateName, 284);
	RegisterVMNativeFunc_4("Object", "GetUnAxes", &NObject::GetUnAxes, 230);
	RegisterVMNativeFunc_2("Object", "GotoState", &NObject::GotoState, 113);
	RegisterVMNativeFunc_3("Object", "GreaterEqual_FloatFloat", &NObject::GreaterEqual_FloatFloat, 179);
	RegisterVMNativeFunc_3("Object", "GreaterEqual_IntInt", &NObject::GreaterEqual_IntInt, 153);
	RegisterVMNativeFunc_3("Object", "GreaterEqual_StrStr", &NObject::GreaterEqual_StrStr, 121);
	RegisterVMNativeFunc_3("Object", "GreaterGreaterGreater_IntInt", &NObject::GreaterGreaterGreater_IntInt, 196);
	RegisterVMNativeFunc_3("Object", "GreaterGreater_IntInt", &NObject::GreaterGreater_IntInt, 149);
	RegisterVMNativeFunc_3("Object", "GreaterGreater_VectorRotator", &NObject::GreaterGreater_VectorRotator, 276);
	RegisterVMNativeFunc_3("Object", "Greater_FloatFloat", &NObject::Greater_FloatFloat, 177);
	RegisterVMNativeFunc_3("Object", "Greater_IntInt", &NObject::Greater_IntInt, 151);
	RegisterVMNativeFunc_3("Object", "Greater_StrStr", &NObject::Greater_StrStr, 116);
	RegisterVMNativeFunc_3("Object", "InStr", &NObject::InStr, 126);
	RegisterVMNativeFunc_3("Object", "Invert", &NObject::Invert, 227);
	RegisterVMNativeFunc_2("Object", "IsA", &NObject::IsA, 303);
	RegisterVMNativeFunc_2("Object", "IsInState", &NObject::IsInState, 281);
	RegisterVMNativeFunc_3("Object", "Left", &NObject::Left, 128);
	RegisterVMNativeFunc_2("Object", "Len", &NObject::Len, 125);
	RegisterVMNativeFunc_4("Object", "Lerp", &NObject::Lerp, 247);
	RegisterVMNativeFunc_3("Object", "LessEqual_FloatFloat", &NObject::LessEqual_FloatFloat, 178);
	RegisterVMNativeFunc_3("Object", "LessEqual_IntInt", &NObject::LessEqual_IntInt, 152);
	RegisterVMNativeFunc_3("Object", "LessEqual_StrStr", &NObject::LessEqual_StrStr, 120);
	RegisterVMNativeFunc_3("Object", "LessLess_IntInt", &NObject::LessLess_IntInt, 148);
	RegisterVMNativeFunc_3("Object", "LessLess_VectorRotator", &NObject::LessLess_VectorRotator, 275);
	RegisterVMNativeFunc_3("Object", "Less_FloatFloat", &NObject::Less_FloatFloat, 176);
	RegisterVMNativeFunc_3("Object", "Less_IntInt", &NObject::Less_IntInt, 150);
	RegisterVMNativeFunc_3("Object", "Less_StrStr", &NObject::Less_StrStr, 115);
	RegisterVMNativeFunc_4("Object", "Localize", &NObject::Localize, 0);
	RegisterVMNativeFunc_2("Object", "Log", &NObject::Log, 231);
	RegisterVMNativeFunc_2("Object", "Loge", &NObject::Loge, 192);
	RegisterVMNativeFunc_3("Object", "Max", &NObject::Max, 250);
	RegisterVMNativeFunc_4("Object", "Mid", &NObject::Mid, 127);
	RegisterVMNativeFunc_3("Object", "Min", &NObject::Min, 249);
	RegisterVMNativeFunc_3("Object", "MirrorVectorByNormal", &NObject::MirrorVectorByNormal, 300);
	RegisterVMNativeFunc_3("Object", "MultiplyEqual_ByteByte", &NObject::MultiplyEqual_ByteByte, 133);
	RegisterVMNativeFunc_3("Object", "MultiplyEqual_FloatFloat", &NObject::MultiplyEqual_FloatFloat, 182);
	RegisterVMNativeFunc_3("Object", "MultiplyEqual_IntFloat", &NObject::MultiplyEqual_IntFloat, 159);
	RegisterVMNativeFunc_3("Object", "MultiplyEqual_RotatorFloat", &NObject::MultiplyEqual_RotatorFloat, 290);
	RegisterVMNativeFunc_3("Object", "MultiplyEqual_VectorFloat", &NObject::MultiplyEqual_VectorFloat, 221);
	RegisterVMNativeFunc_3("Object", "MultiplyEqual_VectorVector", &NObject::MultiplyEqual_VectorVector, 297);
	RegisterVMNativeFunc_3("Object", "MultiplyMultiply_FloatFloat", &NObject::MultiplyMultiply_FloatFloat, 170);
	RegisterVMNativeFunc_3("Object", "Multiply_FloatFloat", &NObject::Multiply_FloatFloat, 171);
	RegisterVMNativeFunc_3("Object", "Multiply_FloatRotator", &NObject::Multiply_FloatRotator, 288);
	RegisterVMNativeFunc_3("Object", "Multiply_FloatVector", &NObject::Multiply_FloatVector, 213);
	RegisterVMNativeFunc_3("Object", "Multiply_IntInt", &NObject::Multiply_IntInt, 144);
	RegisterVMNativeFunc_3("Object", "Multiply_RotatorFloat", &NObject::Multiply_RotatorFloat, 287);
	RegisterVMNativeFunc_3("Object", "Multiply_VectorFloat", &NObject::Multiply_VectorFloat, 212);
	RegisterVMNativeFunc_3("Object", "Multiply_VectorVector", &NObject::Multiply_VectorVector, 296);
	RegisterVMNativeFunc_2("Object", "Normal", &NObject::Normal, 226);
	RegisterVMNativeFunc_2("Object", "Normalize", &NObject::Normalize, 0);
	RegisterVMNativeFunc_3("Object", "NotEqual_BoolBool", &NObject::NotEqual_BoolBool, 243);
	RegisterVMNativeFunc_3("Object", "NotEqual_FloatFloat", &NObject::NotEqual_FloatFloat, 181);
	RegisterVMNativeFunc_3("Object", "NotEqual_IntInt", &NObject::NotEqual_IntInt, 155);
	RegisterVMNativeFunc_3("Object", "NotEqual_NameName", &NObject::NotEqual_NameName, 255);
	RegisterVMNativeFunc_3("Object", "NotEqual_ObjectObject", &NObject::NotEqual_ObjectObject, 119);
	RegisterVMNativeFunc_3("Object", "NotEqual_RotatorRotator", &NObject::NotEqual_RotatorRotator, 203);
	RegisterVMNativeFunc_3("Object", "NotEqual_StrStr", &NObject::NotEqual_StrStr, 123);
	RegisterVMNativeFunc_3("Object", "NotEqual_VectorVector", &NObject::NotEqual_VectorVector, 218);
	RegisterVMNativeFunc_2("Object", "Not_PreBool", &NObject::Not_PreBool, 129);
	RegisterVMNativeFunc_3("Object", "OrOr_BoolBool", &NObject::OrOr_BoolBool, 132);
	RegisterVMNativeFunc_3("Object", "Or_IntInt", &NObject::Or_IntInt, 158);
	RegisterVMNativeFunc_4("Object", "OrthoRotation", &NObject::OrthoRotation, 0);
	RegisterVMNativeFunc_3("Object", "Percent_FloatFloat", &NObject::Percent_FloatFloat, 173);
	RegisterVMNativeFunc_2("Object", "Rand", &NObject::Rand, 167);
	RegisterVMNativeFunc_3("Object", "RandRange", &NObject::RandRange, 1033);
	RegisterVMNativeFunc_0("Object", "ResetConfig", &NObject::ResetConfig, 0);
	RegisterVMNativeFunc_3("Object", "Right", &NObject::Right, 234);
	RegisterVMNativeFunc_2("Object", "RotRand", &NObject::RotRand, 320);
	if (engine->LaunchInfo.IsUnreal1_227())
		RegisterVMNativeFunc_1("Object", "SaveConfig", &NObject::SaveConfig_U227, 536);
	else
		RegisterVMNativeFunc_0("Object", "SaveConfig", &NObject::SaveConfig, 536);
	RegisterVMNativeFunc_2("Object", "SetPropertyText", &NObject::SetPropertyText, 0);
	RegisterVMNativeFunc_2("Object", "Sin", &NObject::Sin, 187);
	RegisterVMNativeFunc_4("Object", "Smerp", &NObject::Smerp, 248);
	RegisterVMNativeFunc_2("Object", "Sqrt", &NObject::Sqrt, 193);
	RegisterVMNativeFunc_2("Object", "Square", &NObject::Square, 194);
	RegisterVMNativeFunc_0("Object", "StaticSaveConfig", &NObject::StaticSaveConfig, 0);
	RegisterVMNativeFunc_3("Object", "SubtractEqual_ByteByte", &NObject::SubtractEqual_ByteByte, 136);
	RegisterVMNativeFunc_3("Object", "SubtractEqual_FloatFloat", &NObject::SubtractEqual_FloatFloat, 185);
	RegisterVMNativeFunc_3("Object", "SubtractEqual_IntInt", &NObject::SubtractEqual_IntInt, 162);
	RegisterVMNativeFunc_3("Object", "SubtractEqual_RotatorRotator", &NObject::SubtractEqual_RotatorRotator, 319);
	RegisterVMNativeFunc_3("Object", "SubtractEqual_VectorVector", &NObject::SubtractEqual_VectorVector, 224);
	RegisterVMNativeFunc_2("Object", "SubtractSubtract_Byte", &NObject::SubtractSubtract_Byte, 140);
	RegisterVMNativeFunc_2("Object", "SubtractSubtract_Int", &NObject::SubtractSubtract_Int, 166);
	RegisterVMNativeFunc_2("Object", "SubtractSubtract_PreByte", &NObject::SubtractSubtract_PreByte, 138);
	RegisterVMNativeFunc_2("Object", "SubtractSubtract_PreInt", &NObject::SubtractSubtract_PreInt, 164);
	RegisterVMNativeFunc_3("Object", "Subtract_FloatFloat", &NObject::Subtract_FloatFloat, 175);
	RegisterVMNativeFunc_3("Object", "Subtract_IntInt", &NObject::Subtract_IntInt, 147);
	RegisterVMNativeFunc_2("Object", "Subtract_PreFloat", &NObject::Subtract_PreFloat, 169);
	RegisterVMNativeFunc_2("Object", "Subtract_PreInt", &NObject::Subtract_PreInt, 143);
	RegisterVMNativeFunc_2("Object", "Subtract_PreVector", &NObject::Subtract_PreVector, 211);
	RegisterVMNativeFunc_3("Object", "Subtract_RotatorRotator", &NObject::Subtract_RotatorRotator, 317);
	RegisterVMNativeFunc_3("Object", "Subtract_VectorVector", &NObject::Subtract_VectorVector, 216);
	RegisterVMNativeFunc_2("Object", "Tan", &NObject::Tan, 189);
	RegisterVMNativeFunc_1("Object", "VRand", &NObject::VRand, 252);
	RegisterVMNativeFunc_2("Object", "VSize", &NObject::VSize, 225);
	RegisterVMNativeFunc_1("Object", "Warn", &NObject::Warn, 232);
	RegisterVMNativeFunc_3("Object", "XorXor_BoolBool", &NObject::XorXor_BoolBool, 131);
	RegisterVMNativeFunc_3("Object", "Xor_IntInt", &NObject::Xor_IntInt, 157);

	// Unreal Gold 227 exclusive functions
	if (engine->LaunchInfo.IsUnreal1_227())
	{
		RegisterVMNativeFunc_3("Object", "Add_QuatQuat", &NObject::Add_QuatQuat_U227, 613);
		RegisterVMNativeFunc_3("Object", "AddEqual_QuatQuat", &NObject::AddEqual_QuatQuat_U227, 615);
		RegisterVMNativeFunc_3("Object", "AlignQuatWith", &NObject::AlignQuatWith_U227, 634);
		RegisterVMNativeFunc_3("Object", "AllFiles", &NObject::AllFiles_U227, 603);
		RegisterVMNativeFunc_7("Object", "AllLinkers", &NObject::AllLinkers_U227, 636);
		RegisterVMNativeFunc_3("Object", "AllObjects", &NObject::AllObjects_U227, 304);
		RegisterVMNativeFunc_3("Object", "And_RotatorRotator", &NObject::And_RotatorRotator_U227, 607);
		RegisterVMNativeFunc_1("Object", "AppSeconds", &NObject::AppSeconds_U227, 643);
		RegisterVMNativeFunc_1("Object", "AppInEditor", &NObject::AppInEditor_U227, 645);
		RegisterVMNativeFunc_2("Object", "Ceil", &NObject::Ceil_U227, 635);
		RegisterVMNativeFunc_2("Object", "ClearConfig", &NObject::ClearConfig_U227, 537);
		RegisterVMNativeFunc_2("Object", "Complement_PreQuat", &NObject::Complement_PreQuat_U227, 619);
		RegisterVMNativeFunc_2("Object", "CoordsToQuat", &NObject::CoordsToQuat_U227, 620);
		RegisterVMNativeFunc_5("Object", "Divide", &NObject::Divide_U227, 638);
		RegisterVMNativeFunc_3("Object", "DivideEqual_CoordsRotator", &NObject::DivideEqual_CoordsRotator_U227, 333);
		RegisterVMNativeFunc_3("Object", "DivideEqual_QuatFloat", &NObject::DivideEqual_QuatFloat_U227, 618);
		RegisterVMNativeFunc_3("Object", "Dot_QuatQuat", &NObject::Dot_QuatQuat_U227, 622);
		RegisterVMNativeFunc_3("Object", "ExecFunctionStr", &NObject::ExecFunctionStr_U227, 390);
		RegisterVMNativeFunc_2("Object", "ExportFullProperties", &NObject::ExportFullProperties_U227, 0);
		RegisterVMNativeFunc_6("Object", "ExtractString", &NObject::ExtractString_U227, 639);
		RegisterVMNativeFunc_3("Object", "FindFunction", &NObject::FindFunction_U227, 646);
		RegisterVMNativeFunc_3("Object", "FindObject", &NObject::FindObject_U227, 600);
		RegisterVMNativeFunc_2("Object", "FindObjectIndex", &NObject::FindObjectIndex_U227, 644);
		RegisterVMNativeFunc_2("Object", "GetClassFlags", &NObject::GetClassFlags_U227, 649);
		RegisterVMNativeFunc_2("Object", "GetDefaultObject", &NObject::GetDefaultObject_U227, 637);
		RegisterVMNativeFunc_2("Object", "GetParentClass", &NObject::GetParentClass_U227, 601);
		// RegisterVMNativeFunc_4("Object", "GetPerObjectNames", &NObject::GetPerObjectNames_U227, 0);
		RegisterVMNativeFunc_1("Object", "GetUnitCoords", &NObject::GetUnitCoords_U227, 335);
		RegisterVMNativeFunc_1("Object", "ImportFullProperties", &NObject::ImportFullProperties_U227, 0);
		// RegisterVMNativeFunc_3("Object", "InterpCurveEval", &NObject::InterpCurveEval_U227, 0); // Necessary structs not implemented yet
		RegisterVMNativeFunc_3("Object", "IntToStr", &NObject::IntToStr_U227, 325);
		RegisterVMNativeFunc_4("Object", "LerpRotation", &NObject::LerpRotation_U227, 0);
		RegisterVMNativeFunc_4("Object", "LerpVector", &NObject::LerpVector_U227, 0);
		// RegisterVMNativeFunc_4("Object", "LoadPackageContents", &NObject::LoadPackageContents_U227, 257);
		RegisterVMNativeFunc_2("Object", "Locs", &NObject::Locs_U227, 238);
		RegisterVMNativeFunc_5("Object", "MakeColor", &NObject::MakeColor_U227, 198);
		RegisterVMNativeFunc_5("Object", "MakeQuat", &NObject::MakeQuat_U227, 606);
		RegisterVMNativeFunc_3("Object", "MultiplyEqual_CoordsCoords", &NObject::MultiplyEqual_CoordsCoords_U227, 331);
		RegisterVMNativeFunc_3("Object", "MultiplyEqual_CoordsRotator", &NObject::MultiplyEqual_CoordsRotator_U227, 332);
		RegisterVMNativeFunc_3("Object", "MultiplyEqual_QuatFloat", &NObject::MultiplyEqual_QuatFloat_U227, 617);
		RegisterVMNativeFunc_3("Object", "MultiplyEqual_VectorCoords", &NObject::MultiplyEqual_VectorCoords_U227, 334);
		RegisterVMNativeFunc_3("Object", "Multiply_CoordsCoords", &NObject::Multiply_CoordsCoords_U227, 330);
		RegisterVMNativeFunc_3("Object", "Multiply_FloatQuat", &NObject::Multiply_FloatQuat_U227, 611);
		RegisterVMNativeFunc_3("Object", "Multiply_QuatFloat", &NObject::Multiply_QuatFloat_U227, 610);
		RegisterVMNativeFunc_3("Object", "Multiply_QuatQuat", &NObject::Multiply_QuatQuat_U227, 612);
		RegisterVMNativeFunc_2("Object", "Normal2D", &NObject::Normal2D_U227, 274);
		RegisterVMNativeFunc_3("Object", "QuatError", &NObject::QuatError_U227, 633);
		RegisterVMNativeFunc_3("Object", "QuatFromAxisAndAngle", &NObject::QuatFromAxisAndAngle_U227, 632);
		RegisterVMNativeFunc_2("Object", "QuatGetAngle", &NObject::QuatGetAngle_U227, 627);
		RegisterVMNativeFunc_2("Object", "QuatGetAxis", &NObject::QuatGetAxis_U227, 628);
		RegisterVMNativeFunc_2("Object", "QuatGetVect", &NObject::QuatGetVect_U227, 623);
		RegisterVMNativeFunc_3("Object", "QuatRotate", &NObject::QuatRotate_U227, 629);
		RegisterVMNativeFunc_2("Object", "QuatSizeSquared", &NObject::QuatSizeSquared_U227, 624);
		RegisterVMNativeFunc_2("Object", "QuatSize", &NObject::QuatSize_U227, 625);
		RegisterVMNativeFunc_5("Object", "QuatSlerp", &NObject::QuatSlerp_U227, 631);
		RegisterVMNativeFunc_3("Object", "QuatToCoords", &NObject::QuatToCoords_U227, 621);
		RegisterVMNativeFunc_2("Object", "QuatToRotation", &NObject::QuatToRotation_U227, 605);
		RegisterVMNativeFunc_3("Object", "QuatVRotate", &NObject::QuatVRotate_U227, 630);
		RegisterVMNativeFunc_3("Object", "RandIntRange", &NObject::RandIntRange_U227, 0);
		RegisterVMNativeFunc_5("Object", "ReplaceStr", &NObject::ReplaceStr_U227, 239);
		RegisterVMNativeFunc_2("Object", "RotationToQuat", &NObject::RotationToQuat_U227, 604);
		RegisterVMNativeFunc_4("Object", "SlerpRotation", &NObject::SlerpRotation_U227, 0);
		RegisterVMNativeFunc_3("Object", "SortArray", &NObject::SortArray_U227, 240);
		RegisterVMNativeFunc_4("Object", "SortStaticArray", &NObject::SortStaticArray_U227, 241);
		RegisterVMNativeFunc_4("Object", "StartsWith", &NObject::StartsWith_U227, 392);
		RegisterVMNativeFunc_3("Object", "StringToName", &NObject::StringToName_U227, 391);
		RegisterVMNativeFunc_3("Object", "Subtract_QuatQuat", &NObject::Subtract_QuatQuat_U227, 614);
		RegisterVMNativeFunc_3("Object", "SubtractEqual_QuatQuat", &NObject::SubtractEqual_QuatQuat_U227, 616);
		RegisterVMNativeFunc_3("Object", "TransformCoordsByNormal", &NObject::TransformCoordsByNormal_U227, 337);
		RegisterVMNativeFunc_3("Object", "TransformRotatorByNormal", &NObject::TransformRotatorByNormal_U227, 336);
		RegisterVMNativeFunc_2("Object", "VSizeSq", &NObject::VSizeSq_U227, 253);
		RegisterVMNativeFunc_2("Object", "VSize2D", &NObject::VSize2D_U227, 271);
		RegisterVMNativeFunc_2("Object", "VSize2DSq", &NObject::VSize2DSq_U227, 273);
	}

	if (engine->LaunchInfo.IsDeusEx())
	{
		RegisterVMNativeFunc_2("Object", "AllObjects", &NObject::AllObjects_DeusEx, 1001);
		RegisterVMNativeFunc_1("Object", "CriticalDelete", &NObject::CriticalDelete, 751);
	}

	// Package 61 stuff
	if (engine->LaunchInfo.engineVersion <= 219)
	{
		RegisterVMNativeFunc_3("Object", "Concat_StrStr", &NObject::Concat_StrStr, 228);
		RegisterVMNativeFunc_3("Object", "Less_StrStr", &NObject::Less_StrStr, 197);
		RegisterVMNativeFunc_3("Object", "Greater_StrStr", &NObject::Greater_StrStr, 198);
		RegisterVMNativeFunc_3("Object", "LessEqual_StrStr", &NObject::LessEqual_StrStr, 199);
		RegisterVMNativeFunc_3("Object", "GreaterEqual_StrStr", &NObject::GreaterEqual_StrStr, 200);
		RegisterVMNativeFunc_3("Object", "EqualEqual_StrStr", &NObject::EqualEqual_StrStr, 201);
		RegisterVMNativeFunc_3("Object", "NotEqual_StrStr", &NObject::NotEqual_StrStr, 202);
		RegisterVMNativeFunc_3("Object", "ComplementEqual_StrStr", &NObject::ComplementEqual_StrStr, 168);
		RegisterVMNativeFunc_2("Object", "Len", &NObject::Len, 204);
		RegisterVMNativeFunc_3("Object", "InStr", &NObject::InStr, 205);
		RegisterVMNativeFunc_4("Object", "Mid", &NObject::Mid, 206);
		RegisterVMNativeFunc_3("Object", "Left", &NObject::Left, 207);
		RegisterVMNativeFunc_3("Object", "Right", &NObject::Right, 208);
		RegisterVMNativeFunc_2("Object", "Caps", &NObject::Caps, 209);
	}
}

void NObject::Abs(float A, float& ReturnValue)
{
	ReturnValue = std::abs(A);
}

void NObject::AddAdd_Byte(uint8_t& A, uint8_t& ReturnValue)
{
	ReturnValue = A++;
}

void NObject::AddAdd_Int(int& A, int& ReturnValue)
{
	ReturnValue = A++;
}

void NObject::AddAdd_PreByte(uint8_t& A, uint8_t& ReturnValue)
{
	ReturnValue = ++A;
}

void NObject::AddAdd_PreInt(int& A, int& ReturnValue)
{
	ReturnValue = ++A;
}

void NObject::AddEqual_ByteByte(uint8_t& A, uint8_t B, uint8_t& ReturnValue)
{
	ReturnValue = A += B;
}

void NObject::AddEqual_FloatFloat(float& A, float B, float& ReturnValue)
{
	ReturnValue = A += B;
}

void NObject::AddEqual_IntInt(int& A, int B, int& ReturnValue)
{
	ReturnValue = A += B;
}

void NObject::AddEqual_RotatorRotator(Rotator& A, const Rotator& B, Rotator& ReturnValue)
{
	ReturnValue = A += B;
}

void NObject::AddEqual_VectorVector(vec3& A, const vec3& B, vec3& ReturnValue)
{
	ReturnValue = A += B;
}

void NObject::AddEqual_QuatQuat_U227(quaternion& A, quaternion& B, quaternion& ReturnValue)
{
	ReturnValue = A += B;
}

void NObject::Add_FloatFloat(float A, float B, float& ReturnValue)
{
	ReturnValue = A + B;
}

void NObject::Add_IntInt(int A, int B, int& ReturnValue)
{
	ReturnValue = A + B;
}

void NObject::Add_RotatorRotator(const Rotator& A, const Rotator& B, Rotator& ReturnValue)
{
	ReturnValue = A + B;
}

void NObject::Add_VectorVector(const vec3& A, const vec3& B, vec3& ReturnValue)
{
	ReturnValue = A + B;
}

void NObject::Add_QuatQuat_U227(quaternion& A, quaternion& B, quaternion& ReturnValue)
{
	ReturnValue = A + B;
}

void NObject::OrOr_BoolBool(bool A, std::optional<bool> B, BitfieldBool& ReturnValue)
{
	ReturnValue = A || *B;
}

void NObject::AndAnd_BoolBool(bool A, std::optional<bool> B, BitfieldBool& ReturnValue)
{
	ReturnValue = A && *B;
}

void NObject::And_IntInt(int A, int B, int& ReturnValue)
{
	ReturnValue = A & B;
}

void NObject::And_RotatorRotator_U227(Rotator& R1, Rotator& R2, Rotator& ReturnValue)
{
	LogUnimplemented("Object.And_RotatorRotator() [U227]");
	ReturnValue = Rotator();
}

void NObject::AlignQuatWith_U227(quaternion& A, quaternion& B, quaternion& ReturnValue)
{
	LogUnimplemented("Object.AlignQuatWith() [U227]");
	ReturnValue = quaternion();
}

void NObject::AllLinkers_U227(NameString& PackageName, std::string& FileName, std::string& GUID, int& NameCount, int& ImportCount, int& ExportCount, int& FileSize)
{
	LogUnimplemented("Object.AllLinkersIterator [U227]");
}

void NObject::AppSeconds_U227(float& ReturnValue)
{
	ReturnValue = static_cast<float>(engine->TotalTime);
}

void NObject::AppInEditor_U227(BitfieldBool& ReturnValue)
{
	ReturnValue = engine->getEditorMode();
}

void NObject::AllFiles_U227(const std::string& FileExtension, const std::string& FilePrefix, std::string& outFileName)
{
	Frame::CreatedIterator = std::make_unique<AllFilesIterator>(FileExtension, FilePrefix, outFileName);
}

void NObject::Asc(const std::string& S, int& ReturnValue)
{
	ReturnValue = !S.empty() ? S.front() : '\0';
}

void NObject::At_StrStr(const std::string& A, const std::string& B, std::string& ReturnValue)
{
	ReturnValue = A + " " + B;
}

void NObject::Atan(float A, float& ReturnValue)
{
	ReturnValue = std::atan(A);
}

void NObject::Caps(const std::string& S, std::string& ReturnValue)
{
	ReturnValue.clear();
	ReturnValue.reserve(S.size());
	for (char c : S)
		ReturnValue.push_back(std::toupper(c));
}

void NObject::Ceil_U227(const float f, float& ReturnValue)
{
	ReturnValue = std::ceil(f);
}

void NObject::Chr(int i, std::string& ReturnValue)
{
	ReturnValue = std::string(1, i);
}

void NObject::Clamp(int V, int A, int B, int& ReturnValue)
{
	ReturnValue = clamp(V, A, B);
}

void NObject::ClassIsChildOf(UObject* TestClass, UObject* ParentClass, BitfieldBool& ReturnValue)
{
	for (UClass* cls = UObject::Cast<UClass>(TestClass); cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
	{
		if (cls == ParentClass)
		{
			ReturnValue = true;
			return;
		}
	}
	ReturnValue = false;
}

void NObject::ClearConfig_U227(UObject* Class, std::optional<std::string> PropName)
{
	LogUnimplemented("Object.ClearConfig() [U227]");
}

void NObject::ComplementEqual_FloatFloat(float A, float B, BitfieldBool& ReturnValue)
{
	ReturnValue = std::abs(A - B) < 0.0001;
}

void NObject::ComplementEqual_StrStr(const std::string& A, const std::string& B, BitfieldBool& ReturnValue)
{
	ReturnValue = StrCompare::equals_ignore_case(A, B);
}

void NObject::Complement_PreInt(int A, int& ReturnValue)
{
	ReturnValue = ~A;
}

void NObject::Complement_PreQuat_U227(const quaternion A, quaternion& ReturnValue)
{
	ReturnValue = inverse(A);
}

void NObject::Concat_StrStr(const std::string& A, const std::string& B, std::string& ReturnValue)
{
	ReturnValue = A + B;
}

void NObject::CoordsToQuat_U227(Coords& C, quaternion& ReturnValue)
{
	LogUnimplemented("Object.CoordsToQuat() [U227]");
	ReturnValue = quaternion();
}

void NObject::Cos(float A, float& ReturnValue)
{
	ReturnValue = std::cos(A);
}

void NObject::Cross_VectorVector(const vec3& A, const vec3& B, vec3& ReturnValue)
{
	ReturnValue = cross(A, B);
}

void NObject::Disable(UObject* Self, const NameString& ProbeFunc)
{
	Self->DisableEvent(ProbeFunc);
}

void NObject::CriticalDelete(UObject* Self, UObject* myObject)
{
	LogUnimplemented("Object.CriticalDelete");
}

void NObject::Divide_U227(std::string& Src, std::string& Divider, std::string& LeftPart, std::string& RightPart, BitfieldBool& ReturnValue)
{
	auto pos = Src.find(Divider);

	if (pos == std::string::npos)
	{
		LeftPart = Src;
		RightPart = "";
		ReturnValue = false;
	}
	else
	{
		LeftPart = Src.substr(0, pos);
		RightPart = Src.substr(pos + 1);
		ReturnValue = true;
	}
}

void NObject::DivideEqual_ByteByte(uint8_t& A, uint8_t B, uint8_t& ReturnValue)
{
	ReturnValue = A /= B;
}

void NObject::DivideEqual_CoordsRotator_U227(Coords& A, Rotator& B, Coords& ReturnValue)
{
	ReturnValue = A * Coords::InverseRotation(B);
}

void NObject::DivideEqual_FloatFloat(float& A, float B, float& ReturnValue)
{
	ReturnValue = A /= B;
}

void NObject::DivideEqual_IntFloat(int& A, float B, int& ReturnValue)
{
	ReturnValue = A /= B;
}

void NObject::DivideEqual_RotatorFloat(Rotator& A, float B, Rotator& ReturnValue)
{
	ReturnValue = A /= B;
}

void NObject::DivideEqual_VectorFloat(vec3& A, float B, vec3& ReturnValue)
{
	ReturnValue = A /= B;
}

void NObject::DivideEqual_QuatFloat_U227(quaternion& A, float B, quaternion& ReturnValue)
{
	ReturnValue = A /= B;
}

void NObject::Divide_FloatFloat(float A, float B, float& ReturnValue)
{
	ReturnValue = A / B;
}

void NObject::Divide_IntInt(int A, int B, int& ReturnValue)
{
	ReturnValue = A / B;
}

void NObject::Divide_RotatorFloat(const Rotator& A, float B, Rotator& ReturnValue)
{
	ReturnValue = A / B;
}

void NObject::Divide_VectorFloat(const vec3& A, float B, vec3& ReturnValue)
{
	ReturnValue = A / B;
}

void NObject::Dot_VectorVector(const vec3& A, const vec3& B, float& ReturnValue)
{
	ReturnValue = dot(A, B);
}

void NObject::Dot_QuatQuat_U227(quaternion& A, quaternion& B, float& ReturnValue)
{
	ReturnValue = dot(vec3{A.x, A.y, A.z}, vec3{B.x, B.y, B.z});
}

void NObject::DynamicLoadObject(const std::string& ObjectName, UObject* ObjectClass, std::optional<bool> MayFail, UObject*& ReturnValue)
{
	ReturnValue = nullptr;

	if (!ObjectName.empty())
	{
		auto dotpos = ObjectName.find('.');
		if (dotpos != 0 && dotpos != std::string::npos)
		{
			std::string packageName = ObjectName.substr(0, dotpos);
			std::string objectName = ObjectName.substr(dotpos + 1);

			try
			{
				ReturnValue = engine->packages->GetPackage(packageName)->GetUObject(ObjectClass->Name, objectName);
			}
			catch (...)
			{
			}
		}
	}

	if (!ReturnValue && (!MayFail || *MayFail == false))
	{
		LogMessage("Object.DynamicLoadObject: could not load '" + ObjectName + "'");
	}
}

void NObject::DynamicLoadObject_219(const std::string& ObjectName, UObject* ObjectClass, UObject*& ReturnValue)
{
	ReturnValue = nullptr;

	if (!ObjectName.empty())
	{
		auto dotpos = ObjectName.find('.');
		if (dotpos != 0 && dotpos != std::string::npos)
		{
			std::string packageName = ObjectName.substr(0, dotpos);
			std::string objectName = ObjectName.substr(dotpos + 1);

			try
			{
				ReturnValue = engine->packages->GetPackage(packageName)->GetUObject(ObjectClass->Name, objectName);
			}
			catch (...)
			{
			}
		}
	}

	if (!ReturnValue)
	{
		LogMessage("Object.DynamicLoadObject: could not load '" + ObjectName + "'");
	}
}

void NObject::Enable(UObject* Self, const NameString& ProbeFunc)
{
	Self->EnableEvent(ProbeFunc);
}

void NObject::EqualEqual_BoolBool(bool A, bool B, BitfieldBool& ReturnValue)
{
	ReturnValue = (A == B);
}

void NObject::EqualEqual_FloatFloat(float A, float B, BitfieldBool& ReturnValue)
{
	ReturnValue = (Float::Equals(A, B));
}

void NObject::EqualEqual_IntInt(int A, int B, BitfieldBool& ReturnValue)
{
	ReturnValue = (A == B);
}

void NObject::EqualEqual_NameName(const NameString& A, const NameString& B, BitfieldBool& ReturnValue)
{
	ReturnValue = (A == B);
}

void NObject::EqualEqual_ObjectObject(UObject* A, UObject* B, BitfieldBool& ReturnValue)
{
	ReturnValue = (A == B);
}

void NObject::EqualEqual_RotatorRotator(const Rotator& A, const Rotator& B, BitfieldBool& ReturnValue)
{
	ReturnValue = (A == B);
}

void NObject::EqualEqual_StrStr(const std::string& A, const std::string& B, BitfieldBool& ReturnValue)
{
	ReturnValue = (A == B);
}

void NObject::EqualEqual_VectorVector(const vec3& A, const vec3& B, BitfieldBool& ReturnValue)
{
	ReturnValue = (A == B);
}

void NObject::ExecFunctionStr_U227(const NameString& FuncName, const std::string& Params, std::optional<std::string> ReturnVal)
{
	LogUnimplemented("Object.ExecFunctionStr() [U227]");
	ReturnVal = std::nullopt;
}

void NObject::Exp(float A, float& ReturnValue)
{
	ReturnValue = std::exp(A);
}

void NObject::ExportFullProperties_U227(std::optional<bool> bDelta, std::string& ReturnValue)
{
	LogUnimplemented("Object.ExportFullProperties() [U227]");
	ReturnValue = "";
}

void NObject::ExtractString_U227(std::string& Src, std::string& LeftDivider, std::string& RightDivider, std::string& MidString, std::optional<bool> bAdvanced, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Object.ExtractString() [U227]");
	ReturnValue = false;
}

void NObject::FClamp(float V, float A, float B, float& ReturnValue)
{
	ReturnValue = clamp(V, A, B);
}

void NObject::FindFunction_U227(NameString& FuncName, std::optional<NameString> TestState, UObject* ReturnValue)
{
	LogUnimplemented("Object.FindFunction() [U227]");
	ReturnValue = nullptr;
}

void NObject::FindObject_U227(UObject* ObjClass, std::string& ObjectName, UObject* ReturnValue)
{
	LogUnimplemented("Object.FindObject() [U227]");
	ReturnValue = nullptr;
}

void NObject::FindObjectIndex_U227(int Index, UObject* ReturnValue)
{
	LogUnimplemented("Object.FindObjectIndex() [U227]");
	ReturnValue = nullptr;
}

void NObject::FMax(float A, float B, float& ReturnValue)
{
	ReturnValue = std::max(A, B);
}

void NObject::FMin(float A, float B, float& ReturnValue)
{
	ReturnValue = std::min(A, B);
}

void NObject::FRand(float& ReturnValue)
{
	// ReturnValue = (float)(std::rand() / (double)RAND_MAX);
	ReturnValue = ::FRand();
}

void NObject::GetAxes(const Rotator& A, vec3& X, vec3& Y, vec3& Z)
{
	Coords::Rotation(A).GetAxes(X, Y, Z);
}

void NObject::GetClassFlags_U227(UObject* Class, int& ReturnValue)
{
	ReturnValue = static_cast<int>(Class->Class->ClsFlags);
}

void NObject::GetDefaultObject_U227(UObject* ObjClass, UObject* ReturnValue)
{
	LogUnimplemented("Object.GetDefaultObject() [U227]");
	ReturnValue = nullptr;
}

void NObject::GetEnum(UObject* E, int i, NameString& ReturnValue)
{
	UEnum* type = UObject::Cast<UEnum>(E);
	if (i > 0 && i < (int)type->ElementNames.size())
		ReturnValue = type->ElementNames[i];
	else
		ReturnValue = {};
}

void NObject::GetParentClass_U227(std::optional<UObject*> ObjClass, UObject* ReturnValue)
{
	LogUnimplemented("Object.GetParentClass() [U227]");
	ReturnValue = nullptr;
}

void NObject::GetPropertyText(UObject* Self, const std::string& PropName, std::string& ReturnValue)
{
	for (UProperty* prop : Self->PropertyData.Class->Properties)
	{
		if (prop->Name == PropName)
		{
			void* data = Self->PropertyData.Ptr(prop);
			ReturnValue = prop->PrintValue(data);
			return;
		}
	}
	ReturnValue = "";
}

void NObject::GetStateName(UObject* Self, NameString& ReturnValue)
{
	ReturnValue = Self->GetStateName();
}

void NObject::GetUnAxes(const Rotator& A, vec3& X, vec3& Y, vec3& Z)
{
	Coords::Rotation(A).GetUnAxes(X, Y, Z);
}

void NObject::GetUnitCoords_U227(Coords& ReturnValue)
{
	LogUnimplemented("Object.GetUnitCoords() [U227]");
	ReturnValue = Coords::Identity();
}

void NObject::GotoState(UObject* Self, std::optional<NameString> NewState, std::optional<NameString> Label)
{
	Self->GotoState(NewState ? *NewState : Self->GetStateName(), Label ? *Label : std::string());
}

void NObject::GreaterEqual_FloatFloat(float A, float B, BitfieldBool& ReturnValue)
{
	ReturnValue = A >= B;
}

void NObject::GreaterEqual_IntInt(int A, int B, BitfieldBool& ReturnValue)
{
	ReturnValue = A >= B;
}

void NObject::GreaterEqual_StrStr(const std::string& A, const std::string& B, BitfieldBool& ReturnValue)
{
	ReturnValue = A >= B;
}

void NObject::GreaterGreaterGreater_IntInt(int A, int B, int& ReturnValue)
{
	ReturnValue = static_cast<unsigned int>(A) >> B;
}

void NObject::GreaterGreater_IntInt(int A, int B, int& ReturnValue)
{
	ReturnValue = A >> B;
}

void NObject::GreaterGreater_VectorRotator(const vec3& A, const Rotator& B, vec3& ReturnValue)
{
	ReturnValue = Coords::Rotation(B).Inverse() * A;
}

void NObject::Greater_FloatFloat(float A, float B, BitfieldBool& ReturnValue)
{
	ReturnValue = A > B;
}

void NObject::Greater_IntInt(int A, int B, BitfieldBool& ReturnValue)
{
	ReturnValue = A > B;
}

void NObject::Greater_StrStr(const std::string& A, const std::string& B, BitfieldBool& ReturnValue)
{
	ReturnValue = A > B;
}

void NObject::ImportFullProperties_U227(const std::string& S)
{
	LogUnimplemented("Object.ImportFullProperties() [U227]");
}

void NObject::InStr(const std::string& S, const std::string& t, int& ReturnValue)
{
	auto pos = S.find(t);
	ReturnValue = (pos != std::string::npos) ? (int)pos : -1;
}

void NObject::IntToStr_U227(int value, int minWidth, std::string& ReturnValue)
{
	LogUnimplemented("Object.IntToStr() [U227]");
	ReturnValue = "";
}

void NObject::Invert(vec3& X, vec3& Y, vec3& Z)
{
	mat3 m;
	m[0] = X.x; m[1] = X.y; m[2] = X.z;
	m[3] = Y.x; m[4] = Y.y; m[5] = Y.z;
	m[6] = Z.x; m[7] = Z.y; m[8] = Z.z;
	m = mat3::inverse(m);
	X.x = m[0]; X.y = m[1]; X.z = m[2];
	Y.x = m[3]; Y.y = m[4]; Y.z = m[5];
	Z.x = m[6]; Z.y = m[7]; Z.z = m[8];
}

void NObject::IsA(UObject* Self, const NameString& ClassName, BitfieldBool& ReturnValue)
{
	ReturnValue = Self->IsA(ClassName);
}

void NObject::IsInState(UObject* Self, const NameString& TestState, BitfieldBool& ReturnValue)
{
	ReturnValue = Self->GetStateName() == TestState;
}

void NObject::Left(const std::string& S, int i, std::string& ReturnValue)
{
	ReturnValue = S.substr(0, clamp(i, 0, (int)S.size()));
}

void NObject::Len(const std::string& S, int& ReturnValue)
{
	ReturnValue = (int)S.size();
}

void NObject::Lerp(float Alpha, float A, float B, float& ReturnValue)
{
	ReturnValue = mix(A, B, Alpha);
}

void NObject::LerpRotation_U227(Rotator& Dest, Rotator& Src, float Alpha, Rotator& ReturnValue)
{
	LogUnimplemented("Object.LerpRotation() [U227]");
	ReturnValue = {};
}

void NObject::LerpVector_U227(vec3& Dest, vec3& Src, float Alpha, vec3& ReturnValue)
{
	LogUnimplemented("Object.LerpVector() [U227]");
	ReturnValue = {};
}

void NObject::LessEqual_FloatFloat(float A, float B, BitfieldBool& ReturnValue)
{
	ReturnValue = A <= B;
}

void NObject::LessEqual_IntInt(int A, int B, BitfieldBool& ReturnValue)
{
	ReturnValue = A <= B;
}

void NObject::LessEqual_StrStr(const std::string& A, const std::string& B, BitfieldBool& ReturnValue)
{
	ReturnValue = A <= B;
}

void NObject::LessLess_IntInt(int A, int B, int& ReturnValue)
{
	ReturnValue = A << B;
}

void NObject::LessLess_VectorRotator(const vec3& A, const Rotator& B, vec3& ReturnValue)
{
	ReturnValue = Coords::Rotation(B) * A;
}

void NObject::Less_FloatFloat(float A, float B, BitfieldBool& ReturnValue)
{
	ReturnValue = A < B;
}

void NObject::Less_IntInt(int A, int B, BitfieldBool& ReturnValue)
{
	ReturnValue = A < B;
}

void NObject::Less_StrStr(const std::string& A, const std::string& B, BitfieldBool& ReturnValue)
{
	ReturnValue = A < B;
}

void NObject::Localize(const std::string& SectionName, const std::string& KeyName, const std::string& PackageName, std::string& ReturnValue)
{
	ReturnValue = engine->packages->Localize(PackageName, SectionName, KeyName);
}

void NObject::Locs_U227(const std::string& S, std::string& ReturnValue)
{
	ReturnValue = "";

	for (const char c : S)
		ReturnValue += std::tolower(c);
}

void NObject::Log(const std::string& S, std::optional<NameString> Tag)
{
	if (Tag)
		LogMessage("[" + Tag->ToString() + "] " + S);
	else
		LogMessage(S);
}

void NObject::Loge(float A, float& ReturnValue)
{
	ReturnValue = std::log(A);
}

void NObject::MakeColor_U227(uint8_t R, uint8_t G, uint8_t B, std::optional<uint8_t> A, Color& ReturnValue)
{
	const uint8_t alpha = A ? *A : 255;
	ReturnValue = Color{R, G, B, alpha};
}

void NObject::MakeQuat_U227(float X, float Y, float Z, float W, quaternion& ReturnValue)
{
	ReturnValue = quaternion(X, Y, Z, W);
}

void NObject::Max(int A, int B, int& ReturnValue)
{
	ReturnValue = std::max(A, B);
}

void NObject::Mid(const std::string& S, int i, std::optional<int> j, std::string& ReturnValue)
{
	int size = (int)S.size();
	int start = i;
	int end = start + (j ? *j : size);
	start = clamp(start, 0, size);
	end = clamp(end, start, size);
	ReturnValue = S.substr(start, end - start);
}

void NObject::Min(int A, int B, int& ReturnValue)
{
	ReturnValue = std::min(A, B);
}

void NObject::MirrorVectorByNormal(const vec3& Vect, const vec3& Normal, vec3& ReturnValue)
{
	ReturnValue = reflect(Vect, Normal);
}

void NObject::MultiplyEqual_ByteByte(uint8_t& A, uint8_t B, uint8_t& ReturnValue)
{
	ReturnValue = A *= B;
}

void NObject::MultiplyEqual_CoordsCoords_U227(Coords& A, Coords& B, Coords& ReturnValue)
{
	Exception::Throw("Object.MultiplyEqual_CoordsRotator [U227]");
	ReturnValue = Coords::Identity();
}

void NObject::MultiplyEqual_CoordsRotator_U227(Coords& A, Rotator& B, Coords& ReturnValue)
{
	Exception::Throw("Object.MultiplyEqual_CoordsRotator [U227]");
	ReturnValue = Coords::Identity();
}

void NObject::MultiplyEqual_FloatFloat(float& A, float B, float& ReturnValue)
{
	ReturnValue = A *= B;
}

void NObject::MultiplyEqual_QuatFloat_U227(quaternion& A, float B, quaternion& ReturnValue)
{
	ReturnValue = A *= B;
}

void NObject::MultiplyEqual_IntFloat(int& A, float B, int& ReturnValue)
{
	ReturnValue = A *= B;
}

void NObject::MultiplyEqual_RotatorFloat(Rotator& A, float B, Rotator& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::MultiplyEqual_VectorCoords_U227(vec3& A, Coords& B, vec3& ReturnValue)
{
	Exception::Throw("Object.MultiplyEqual_VectorCoords [U227]");
	ReturnValue = vec3();
	// ReturnValue = A *= B;
}

void NObject::MultiplyEqual_VectorFloat(vec3& A, float B, vec3& ReturnValue)
{
	ReturnValue = A *= B;
}

void NObject::MultiplyEqual_VectorVector(vec3& A, const vec3& B, vec3& ReturnValue)
{
	ReturnValue = A *= B;
}

void NObject::MultiplyMultiply_FloatFloat(float A, float B, float& ReturnValue)
{
	ReturnValue = std::pow(A, B);
}

void NObject::Multiply_CoordsCoords_U227(Coords& A, Coords& B, Coords& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::Multiply_FloatFloat(float A, float B, float& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::Multiply_FloatQuat_U227(float A, quaternion& B, quaternion& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::Multiply_FloatRotator(float A, const Rotator& B, Rotator& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::Multiply_FloatVector(float A, const vec3& B, vec3& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::Multiply_IntInt(int A, int B, int& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::Multiply_QuatFloat_U227(quaternion& A, float B, quaternion& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::Multiply_QuatQuat_U227(quaternion& A, quaternion& B, quaternion& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::Multiply_RotatorFloat(const Rotator& A, float B, Rotator& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::Multiply_VectorFloat(const vec3& A, float B, vec3& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::Multiply_VectorVector(const vec3& A, const vec3& B, vec3& ReturnValue)
{
	ReturnValue = A * B;
}

void NObject::Normal(const vec3& A, vec3& ReturnValue)
{
	ReturnValue = normalize(A);
}

void NObject::Normal2D_U227(const vec3& A, vec3& ReturnValue)
{
	const auto v2d = normalize(A.xy());
	ReturnValue = vec3(v2d.x, v2d.y, A.z);
}

void NObject::Normalize(const Rotator& Rot, Rotator& ReturnValue)
{
	ReturnValue = normalize(Rot);
}

void NObject::NotEqual_BoolBool(bool A, bool B, BitfieldBool& ReturnValue)
{
	ReturnValue = A != B;
}

void NObject::NotEqual_FloatFloat(float A, float B, BitfieldBool& ReturnValue)
{
	ReturnValue = A != B;
}

void NObject::NotEqual_IntInt(int A, int B, BitfieldBool& ReturnValue)
{
	ReturnValue = A != B;
}

void NObject::NotEqual_NameName(const NameString& A, const NameString& B, BitfieldBool& ReturnValue)
{
	ReturnValue = A != B;
}

void NObject::NotEqual_ObjectObject(UObject* A, UObject* B, BitfieldBool& ReturnValue)
{
	ReturnValue = A != B;
}

void NObject::NotEqual_RotatorRotator(const Rotator& A, const Rotator& B, BitfieldBool& ReturnValue)
{
	ReturnValue = A != B;
}

void NObject::NotEqual_StrStr(const std::string& A, const std::string& B, BitfieldBool& ReturnValue)
{
	ReturnValue = A != B;
}

void NObject::NotEqual_VectorVector(const vec3& A, const vec3& B, BitfieldBool& ReturnValue)
{
	ReturnValue = A != B;
}

void NObject::Not_PreBool(bool A, BitfieldBool& ReturnValue)
{
	ReturnValue = !A;
}

void NObject::Or_IntInt(int A, int B, int& ReturnValue)
{
	ReturnValue = A | B;
}

void NObject::OrthoRotation(const vec3& X, const vec3& Y, const vec3& Z, Rotator& ReturnValue)
{
	Exception::Throw("Object.OrthoRotation");
	ReturnValue = {};
}

void NObject::Percent_FloatFloat(float A, float B, float& ReturnValue)
{
	ReturnValue = std::fmod(A, B);
}

void NObject::QuatError_U227(quaternion& A, quaternion& B, float& ReturnValue)
{
	LogUnimplemented("Object.QuatError() [U227]");
	ReturnValue = 0.0f;
}

void NObject::QuatFromAxisAndAngle_U227(const vec3& axis, float angle, quaternion& ReturnValue)
{
	ReturnValue = quaternion(angle, axis);
}

void NObject::QuatGetAngle_U227(quaternion& A, float& ReturnValue)
{
	LogUnimplemented("Object.QuatGetAngle() [U227]");
	ReturnValue = 0.0f;
}

void NObject::QuatGetAxis_U227(quaternion& A, vec3& ReturnValue)
{
	LogUnimplemented("Object.QuatGetAxis() [U227]");
	ReturnValue = vec3();
}

void NObject::QuatGetVect_U227(quaternion& A, vec3& ReturnValue)
{
	ReturnValue = normalize(vec3{A.x, A.y, A.z});
}

void NObject::QuatRotate_U227(quaternion& A, quaternion& B, quaternion& ReturnValue)
{
	LogUnimplemented("Object.QuatRotate() [U227]");
	ReturnValue = quaternion();
}

void NObject::QuatSizeSquared_U227(quaternion& A, float& ReturnValue)
{
	ReturnValue = magnitude_squared(A);
}

void NObject::QuatSize_U227(quaternion& A, float& ReturnValue)
{
	ReturnValue = magnitude(A);
}

void NObject::QuatSlerp_U227(quaternion& A, quaternion& B, float C, std::optional<bool> bAlign, quaternion& ReturnValue)
{
	// TODO: Unsure if this is correct
	bool align = bAlign ? *bAlign : false;

	if (align)
	{
		auto invA = inverse(A);
		auto delta = normalize(B * invA);

		ReturnValue = slerp(delta, B, C);
	}
	else
		ReturnValue = slerp(A, B, C);
}

void NObject::QuatToCoords_U227(quaternion& Q, std::optional<vec3> Origin, Coords& ReturnValue)
{
	LogUnimplemented("Object.QuatToCoords() [U227]");
	ReturnValue = Coords::Identity();
}

void NObject::QuatToRotation_U227(quaternion& Q, Rotator& ReturnValue)
{
	LogUnimplemented("Object.QuatToRotation() [U227]");
	ReturnValue = Rotator();
}

void NObject::QuatVRotate_U227(quaternion& A, vec3& B, vec3& ReturnValue)
{
	LogUnimplemented("Object.QuatVRotate() [U227]");
	ReturnValue = vec3();
}

void NObject::Rand(int Max, int& ReturnValue)
{
	ReturnValue = RandInt(std::min(Max, 32767));
}

void NObject::RandIntRange_U227(UObject* Self, int Min, int Max, int& ReturnValue)
{
	ReturnValue = RandInt(Min, Max);
}

void NObject::RandRange(UObject* Self, float Min, float Max, float& ReturnValue)
{
	ReturnValue = FRandRange(Min, Max);
}

void NObject::ReplaceStr_U227(std::string& text, std::string& findStr, std::string& replaceWith, std::optional<bool> bCaseInsensitive, std::string& ReturnValue)
{
	LogUnimplemented("Object.ReplaceStr() [U227]");
	ReturnValue = text;
}

void NObject::ResetConfig()
{
	LogUnimplemented("Object.ResetConfig");
}

void NObject::Right(const std::string& S, int i, std::string& ReturnValue)
{
	int count = clamp(i, 0, (int)S.size());
	ReturnValue = S.substr(S.size() - count);
}

void NObject::RotationToQuat_U227(Rotator& R, quaternion& ReturnValue)
{
	LogUnimplemented("Object.RotationToQuat() [U227]");
	ReturnValue = quaternion();
}

void NObject::RotRand(std::optional<bool> bRoll, Rotator& ReturnValue)
{
	ReturnValue.Yaw = (int)(std::rand() * 0xffffLL / RAND_MAX);
	ReturnValue.Pitch = (int)(std::rand() * 0xffffLL / RAND_MAX);
	ReturnValue.Roll = (bRoll && *bRoll) ? (int)(rand() * 65535LL / RAND_MAX) : 0;
}

void NObject::SaveConfig(UObject* Self)
{
	Self->SaveConfig();
}

void NObject::SaveConfig_U227(UObject* Self, std::optional<bool> bNoWriteINI)
{
	LogUnimplemented("Object.SaveConfig() [U227 - bNoWriteINI parameter not implemented]");
	Self->SaveConfig();
}

void NObject::SetPropertyText(UObject* Self, const std::string& PropName, const std::string& PropValue)
{
	Self->SetPropertyFromString(PropName, PropValue);
}

void NObject::Sin(float A, float& ReturnValue)
{
	ReturnValue = std::sin(A);
}

void NObject::SlerpRotation_U227(Rotator& Dest, Rotator& Src, float Alpha, Rotator& ReturnValue)
{
	LogUnimplemented("Object.SlerpRotation() [U227]");
	ReturnValue = Dest;
}

void NObject::Smerp(float Alpha, float A, float B, float& ReturnValue)
{
	ReturnValue = (float)(A + (3.0 * Alpha * Alpha - 2.0 * Alpha * Alpha * Alpha) * (B - A));
}

void NObject::SortArray_U227(UObject* ArrayProperty, UObject* SortCode, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Object.SortArray() [U227]");
	ReturnValue = false;
}

void NObject::SortStaticArray_U227(UObject* Prop, UObject* SortCode, std::optional<int> SortSize, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Object.SortStaticArray() [U227]");
	ReturnValue = false;
}

void NObject::Sqrt(float A, float& ReturnValue)
{
	ReturnValue = std::sqrt(A);
}

void NObject::Square(float A, float& ReturnValue)
{
	ReturnValue = A * A;
}

void NObject::StartsWith_U227(const std::string& Str, const std::string& SubStr, std::optional<bool> bCaseInsensitive, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Object.StartsWith() [U227]");
	ReturnValue = false;
}

void NObject::StaticSaveConfig(UObject* Class)
{
	Class->SaveConfig();
}

void NObject::StringToName_U227(const std::string& S, std::optional<bool> bFind, NameString& ReturnValue)
{
	LogUnimplemented("Object.StringToName() [U227]");
	ReturnValue = "";
}

void NObject::SubtractEqual_ByteByte(uint8_t& A, uint8_t B, uint8_t& ReturnValue)
{
	ReturnValue = A -= B;
}

void NObject::SubtractEqual_FloatFloat(float& A, float B, float& ReturnValue)
{
	ReturnValue = A -= B;
}

void NObject::SubtractEqual_IntInt(int& A, int B, int& ReturnValue)
{
	ReturnValue = A -= B;
}

void NObject::SubtractEqual_RotatorRotator(Rotator& A, const Rotator& B, Rotator& ReturnValue)
{
	ReturnValue = A -= B;
}

void NObject::SubtractEqual_VectorVector(vec3& A, const vec3& B, vec3& ReturnValue)
{
	ReturnValue = A -= B;
}

void NObject::SubtractEqual_QuatQuat_U227(quaternion& A, quaternion& B, quaternion& ReturnValue)
{
	ReturnValue = A -= B;
}

void NObject::SubtractSubtract_Byte(uint8_t& A, uint8_t& ReturnValue)
{
	ReturnValue = A--;
}

void NObject::SubtractSubtract_Int(int& A, int& ReturnValue)
{
	ReturnValue = A--;
}

void NObject::SubtractSubtract_PreByte(uint8_t& A, uint8_t& ReturnValue)
{
	ReturnValue = --A;
}

void NObject::SubtractSubtract_PreInt(int& A, int& ReturnValue)
{
	ReturnValue = --A;
}

void NObject::Subtract_FloatFloat(float A, float B, float& ReturnValue)
{
	ReturnValue = A - B;
}

void NObject::Subtract_IntInt(int A, int B, int& ReturnValue)
{
	ReturnValue = A - B;
}

void NObject::Subtract_PreFloat(float A, float& ReturnValue)
{
	ReturnValue = -A;
}

void NObject::Subtract_PreInt(int A, int& ReturnValue)
{
	ReturnValue = -A;
}

void NObject::Subtract_PreVector(const vec3& A, vec3& ReturnValue)
{
	ReturnValue = vec3(0.0f) - A;
}

void NObject::Subtract_RotatorRotator(const Rotator& A, const Rotator& B, Rotator& ReturnValue)
{
	ReturnValue = A - B;
}

void NObject::Subtract_VectorVector(const vec3& A, const vec3& B, vec3& ReturnValue)
{
	ReturnValue = A - B;
}

void NObject::Subtract_QuatQuat_U227(quaternion& A, quaternion& B, quaternion& ReturnValue)
{
	ReturnValue = A - B;
}

void NObject::Tan(float A, float& ReturnValue)
{
	ReturnValue = std::tan(A);
}

void NObject::TransformCoordsByNormal_U227(Coords& C, vec3& FloorNormal, Coords& ReturnValue)
{
	LogUnimplemented("Object.TransformCoordsByNormal() [U227]");
	ReturnValue = C;
}

void NObject::TransformRotatorByNormal_U227(Rotator& R, vec3& FloorNormal, Rotator& ReturnValue)
{
	LogUnimplemented("Object.TransformRotatorByNormal() [U227]");
	ReturnValue = R;
}

void NObject::VRand(vec3& ReturnValue)
{
	while (true)
	{
		vec3 randomVector = vec3(FRandRange(-1.0f, 1.0f), FRandRange(-1.0f, 1.0f), FRandRange(-1.0f, 1.0f));
		if (dot(randomVector, randomVector) >= 1.0f)
		{
			ReturnValue = normalize(randomVector);
			break;
		}
	}
}

void NObject::VSize(const vec3& A, float& ReturnValue)
{
	ReturnValue = length(A);
}

void NObject::VSizeSq_U227(const vec3& A, float& ReturnValue)
{
	ReturnValue = dot(A, A);
}

void NObject::VSize2D_U227(const vec3& A, float& ReturnValue)
{
	ReturnValue = length(A.xy());
}

void NObject::VSize2DSq_U227(const vec3& A, float& ReturnValue)
{
	ReturnValue = dot(A.xy(), A.xy());
}

void NObject::Warn(const std::string& S)
{
	LogMessage("Warning: " + S);
}

void NObject::XorXor_BoolBool(bool A, bool B, BitfieldBool& ReturnValue)
{
	ReturnValue = !A ^ !B;
}

void NObject::Xor_IntInt(int A, int B, int& ReturnValue)
{
	ReturnValue = A ^ B;
}

void NObject::AllObjects_U227(UObject* Self, UObject* BaseClass, UObject*& Actor, UObject* InOuter)
{
	Frame::CreatedIterator = std::make_unique<AllObjectsIterator>(BaseClass, &Actor, InOuter);
}

void NObject::AllObjects_DeusEx(UObject* Self, UObject* BaseClass, UObject*& Actor)
{
	Frame::CreatedIterator = std::make_unique<AllObjectsIterator>(BaseClass, &Actor, nullptr);
}
