#pragma once

#include "ExpressionValue.h"
#include <functional>

class UObject;
class ExpressionValue;

typedef std::function<void(UObject* self, ExpressionValue* Args)> NativeFuncHandler;

class NativeFunctions
{
public:
	static std::vector<NativeFuncHandler> NativeByIndex;
	static std::map<std::pair<std::string, std::string>, NativeFuncHandler> NativeByName;

	static void RegisterHandler(const std::string& className, const std::string& funcName, int nativeIndex, NativeFuncHandler handler);
};

// Static native functions:

inline void RegisterVMNativeFunc_0(const std::string& className, const std::string& funcName, void(*func)(), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func();
	});
}

template<typename Arg1>
void RegisterVMNativeFunc_1(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(Args[0].ToType<Arg1>());
	});
}

template<typename Arg1, typename Arg2>
void RegisterVMNativeFunc_2(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3>
void RegisterVMNativeFunc_3(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2, Arg3 arg3), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4>
void RegisterVMNativeFunc_4(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
void RegisterVMNativeFunc_5(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
void RegisterVMNativeFunc_6(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>(), Args[5].ToType<Arg6>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
void RegisterVMNativeFunc_7(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>(), Args[5].ToType<Arg6>(), Args[6].ToType<Arg7>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
void RegisterVMNativeFunc_8(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>(), Args[5].ToType<Arg6>(), Args[6].ToType<Arg7>(), Args[7].ToType<Arg8>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
void RegisterVMNativeFunc_9(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>(), Args[5].ToType<Arg6>(), Args[6].ToType<Arg7>(), Args[7].ToType<Arg8>(), Args[8].ToType<Arg9>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
void RegisterVMNativeFunc_10(const std::string& className, const std::string& funcName, void(*func)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>(), Args[5].ToType<Arg6>(), Args[6].ToType<Arg7>(), Args[7].ToType<Arg8>(), Args[8].ToType<Arg9>(), Args[9].ToType<Arg10>());
	});
}

// Instance native functions:

inline void RegisterVMNativeFunc_0(const std::string& className, const std::string& funcName, void(*func)(UObject* self), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(self);
	});
}

template<typename Arg1>
void RegisterVMNativeFunc_1(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(self, Args[0].ToType<Arg1>());
	});
}

template<typename Arg1, typename Arg2>
void RegisterVMNativeFunc_2(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1, Arg2 arg2), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(self, Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3>
void RegisterVMNativeFunc_3(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(self, Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4>
void RegisterVMNativeFunc_4(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(self, Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
void RegisterVMNativeFunc_5(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(self, Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
void RegisterVMNativeFunc_6(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(self, Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>(), Args[5].ToType<Arg6>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
void RegisterVMNativeFunc_7(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(self, Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>(), Args[5].ToType<Arg6>(), Args[6].ToType<Arg7>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
void RegisterVMNativeFunc_8(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(self, Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>(), Args[5].ToType<Arg6>(), Args[6].ToType<Arg7>(), Args[7].ToType<Arg8>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
void RegisterVMNativeFunc_9(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(self, Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>(), Args[5].ToType<Arg6>(), Args[6].ToType<Arg7>(), Args[7].ToType<Arg8>(), Args[8].ToType<Arg9>());
	});
}

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
void RegisterVMNativeFunc_10(const std::string& className, const std::string& funcName, void(*func)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10), int nativeIndex)
{
	NativeFunctions::RegisterHandler(className, funcName, nativeIndex, [=](UObject* self, ExpressionValue* Args)
	{
		func(self, Args[0].ToType<Arg1>(), Args[1].ToType<Arg2>(), Args[2].ToType<Arg3>(), Args[3].ToType<Arg4>(), Args[4].ToType<Arg5>(), Args[5].ToType<Arg6>(), Args[6].ToType<Arg7>(), Args[7].ToType<Arg8>(), Args[8].ToType<Arg9>(), Args[9].ToType<Arg10>());
	});
}
