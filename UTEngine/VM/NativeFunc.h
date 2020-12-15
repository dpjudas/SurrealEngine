#pragma once

// Static native functions:

inline void RegisterVMNativeFunc_0(const std::string& className, const std::string& funcName, void(*)(), int nativeIndex) { }

template<typename Arg1>
void RegisterVMNativeFunc_1(const std::string& className, const std::string& funcName, void(*)(Arg1 arg1), int nativeIndex) { }

template<typename Arg1, typename Arg2>
void RegisterVMNativeFunc_2(const std::string& className, const std::string& funcName, void(*)(Arg1 arg1, Arg2 arg2), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3>
void RegisterVMNativeFunc_3(const std::string& className, const std::string& funcName, void(*)(Arg1 arg1, Arg2 arg2, Arg3 arg3), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4>
void RegisterVMNativeFunc_4(const std::string& className, const std::string& funcName, void(*)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
void RegisterVMNativeFunc_5(const std::string& className, const std::string& funcName, void(*)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
void RegisterVMNativeFunc_6(const std::string& className, const std::string& funcName, void(*)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
void RegisterVMNativeFunc_7(const std::string& className, const std::string& funcName, void(*)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
void RegisterVMNativeFunc_8(const std::string& className, const std::string& funcName, void(*)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
void RegisterVMNativeFunc_9(const std::string& className, const std::string& funcName, void(*)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
void RegisterVMNativeFunc_10(const std::string& className, const std::string& funcName, void(*)(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10), int nativeIndex) { }

// Instance native functions:

inline void RegisterVMNativeFunc_0(const std::string& className, const std::string& funcName, void(*)(UObject* self), int nativeIndex) { }

template<typename Arg1>
void RegisterVMNativeFunc_1(const std::string& className, const std::string& funcName, void(*)(UObject* self, Arg1 arg1), int nativeIndex) { }

template<typename Arg1, typename Arg2>
void RegisterVMNativeFunc_2(const std::string& className, const std::string& funcName, void(*)(UObject* self, Arg1 arg1, Arg2 arg2), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3>
void RegisterVMNativeFunc_3(const std::string& className, const std::string& funcName, void(*)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4>
void RegisterVMNativeFunc_4(const std::string& className, const std::string& funcName, void(*)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
void RegisterVMNativeFunc_5(const std::string& className, const std::string& funcName, void(*)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
void RegisterVMNativeFunc_6(const std::string& className, const std::string& funcName, void(*)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
void RegisterVMNativeFunc_7(const std::string& className, const std::string& funcName, void(*)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
void RegisterVMNativeFunc_8(const std::string& className, const std::string& funcName, void(*)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9>
void RegisterVMNativeFunc_9(const std::string& className, const std::string& funcName, void(*)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9), int nativeIndex) { }

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9, typename Arg10>
void RegisterVMNativeFunc_10(const std::string& className, const std::string& funcName, void(*)(UObject* self, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10), int nativeIndex) { }
