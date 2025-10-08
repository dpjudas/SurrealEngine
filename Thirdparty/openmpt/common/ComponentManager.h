/*
 * ComponentManager.h
 * ------------------
 * Purpose: Manages loading of optional components.
 * Notes  : (currently none)
 * Authors: Joern Heusipp
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/mutex/mutex.hpp"

#include <map>
#include <vector>
#include "../common/misc_util.h"
#if defined(MODPLUG_TRACKER)
#include "../misc/mptLibrary.h"
#endif


OPENMPT_NAMESPACE_BEGIN


enum ComponentType
{
	ComponentTypeUnknown = 0,
	ComponentTypeBuiltin,            // PortAudio
	ComponentTypeSystem,             // mf.dll
	ComponentTypeSystemInstallable,  // acm mp3 codec
	ComponentTypeBundled,            // libsoundtouch
	ComponentTypeForeign,            // libmp3lame
};


class ComponentFactoryBase;


class IComponent
{

	friend class ComponentFactoryBase;

protected:

	IComponent() = default;

public:

	virtual ~IComponent() = default;

public:

	virtual ComponentType GetType() const = 0;
	
	virtual bool IsInitialized() const = 0;  // Initialize() has been called
	virtual bool IsAvailable() const = 0;  // Initialize() has been successfull
	virtual mpt::ustring GetVersion() const = 0;

	virtual void Initialize() = 0;  // try to load the component

};


class ComponentBase
	: public IComponent
{

private:

	ComponentType m_Type;

	bool m_Initialized;
	bool m_Available;

protected:

	ComponentBase(ComponentType type);

public:

	~ComponentBase() override;

protected:

	void SetInitialized();
	void SetAvailable();

public:

	ComponentType GetType() const override;
	bool IsInitialized() const override;
	bool IsAvailable() const override;

	mpt::ustring GetVersion() const override;

public:

	void Initialize() override;

protected:

	virtual bool DoInitialize() = 0;

};


class ComponentBuiltin : public ComponentBase
{
public:
	ComponentBuiltin()
		: ComponentBase(ComponentTypeBuiltin)
	{
		return;
	}
	bool DoInitialize() override
	{
		return true;
	}
};


#define MPT_GLOBAL_BIND(lib, name) name = &::name;


#if defined(MODPLUG_TRACKER)


class ComponentLibrary
	: public ComponentBase
{

private:
	
	using TLibraryMap = std::map<std::string, mpt::Library>;
	TLibraryMap m_Libraries;
	
	bool m_BindFailed;

protected:

	ComponentLibrary(ComponentType type);

public:

	virtual ~ComponentLibrary();

protected:

	bool AddLibrary(const std::string &libName, const mpt::LibraryPath &libPath);
	void ClearLibraries();
	void SetBindFailed();
	void ClearBindFailed();
	bool HasBindFailed() const;

public:
	
	virtual mpt::Library GetLibrary(const std::string &libName) const;
	
	template <typename Tfunc>
	bool Bind(Tfunc * & f, const std::string &libName, const std::string &symbol) const
	{
		return GetLibrary(libName).Bind(f, symbol);
	}

protected:

	bool DoInitialize() override = 0;

};


#define MPT_COMPONENT_BIND(libName, func) do { if(!Bind( func , libName , #func )) { SetBindFailed(); } } while(0)
#define MPT_COMPONENT_BIND_OPTIONAL(libName, func) Bind( func , libName , #func )
#define MPT_COMPONENT_BIND_SYMBOL(libName, symbol, func) do { if(!Bind( func , libName , symbol )) { SetBindFailed(); } } while(0)
#define MPT_COMPONENT_BIND_SYMBOL_OPTIONAL(libName, symbol, func) Bind( func , libName , symbol )

#if MPT_OS_WINDOWS
#ifdef UNICODE
#define MPT_COMPONENT_BINDWIN_SUFFIX "W"
#else
#define MPT_COMPONENT_BINDWIN_SUFFIX "A"
#endif
#define MPT_COMPONENT_BINDWIN(libName, func) do { if(!Bind( func , libName , #func MPT_COMPONENT_BINDWIN_SUFFIX )) { SetBindFailed(); } } while(0)
#define MPT_COMPONENT_BINDWIN_OPTIONAL(libName, func) Bind( func , libName , #func MPT_COMPONENT_BINDWIN_SUFFIX )
#define MPT_COMPONENT_BINDWIN_SYMBOL(libName, symbol, func) do { if(!Bind( func , libName , symbol MPT_COMPONENT_BINDWIN_SUFFIX )) { SetBindFailed(); } } while(0)
#define MPT_COMPONENT_BINDWIN_SYMBOL_OPTIONAL(libName, symbol, func) Bind( func , libName , symbol MPT_COMPONENT_BINDWIN_SUFFIX )
#endif


class ComponentSystemDLL : public ComponentLibrary
{
private:
	mpt::PathString m_BaseName;
public:
	ComponentSystemDLL(const mpt::PathString &baseName)
		: ComponentLibrary(ComponentTypeSystem)
		, m_BaseName(baseName)
	{
		return;
	}
	bool DoInitialize() override
	{
		AddLibrary(m_BaseName.ToUTF8(), mpt::LibraryPath::System(m_BaseName));
		return GetLibrary(m_BaseName.ToUTF8()).IsValid();
	}
};


class ComponentBundledDLL : public ComponentLibrary
{
private:
	mpt::PathString m_FullName;
public:
	ComponentBundledDLL(const mpt::PathString &fullName)
		: ComponentLibrary(ComponentTypeBundled)
		, m_FullName(fullName)
	{
		return;
	}
	bool DoInitialize() override
	{
		AddLibrary(m_FullName.ToUTF8(), mpt::LibraryPath::AppFullName(m_FullName));
		return GetLibrary(m_FullName.ToUTF8()).IsValid();
	}
};


#endif // MODPLUG_TRACKER


#if MPT_COMPONENT_MANAGER


class ComponentManager;

using ComponentFactoryMethod = std::shared_ptr<IComponent> (*)(ComponentManager &componentManager);


class IComponentFactory
{
protected:
	IComponentFactory() = default;
public:
	virtual ~IComponentFactory() = default;
public:
	virtual std::string GetID() const = 0;
	virtual std::string GetSettingsKey() const = 0;
	virtual std::shared_ptr<IComponent> Construct(ComponentManager &componentManager) const = 0;
	virtual ComponentFactoryMethod GetStaticConstructor() const = 0;
};


class ComponentFactoryBase
	: public IComponentFactory
{
private:
	std::string m_ID;
	std::string m_SettingsKey;
protected:
	ComponentFactoryBase(const std::string &id, const std::string &settingsKey);
	void PreConstruct() const;
	void Initialize(ComponentManager &componentManager, std::shared_ptr<IComponent> component) const;
public:
	virtual ~ComponentFactoryBase();
	std::string GetID() const override;
	std::string GetSettingsKey() const override;
	std::shared_ptr<IComponent> Construct(ComponentManager &componentManager) const override = 0;
	ComponentFactoryMethod GetStaticConstructor() const override = 0;
};


template <typename T>
class ComponentFactory
	: public ComponentFactoryBase
{
public:
	ComponentFactory()
		: ComponentFactoryBase(T::g_ID, T::g_SettingsKey)
	{
		return;
	}
public:
	std::shared_ptr<IComponent> Construct(ComponentManager &componentManager) const override
	{
		PreConstruct();
		std::shared_ptr<IComponent> component = std::make_shared<T>();
		Initialize(componentManager, component);
		return component;
	}
	static std::shared_ptr<IComponent> StaticConstruct(ComponentManager &componentManager)
	{
		return ComponentFactory().Construct(componentManager);
	}
	virtual ComponentFactoryMethod GetStaticConstructor() const override
	{
		return &StaticConstruct;
	}
};


class IComponentManagerSettings
{
public:
	virtual bool LoadOnStartup() const = 0;
	virtual bool KeepLoaded() const = 0;
	virtual bool IsBlocked(const std::string &key) const = 0;
protected:
	virtual ~IComponentManagerSettings() = default;
};


class ComponentManagerSettingsDefault
	: public IComponentManagerSettings
{
public:
	bool LoadOnStartup() const override { return false; }
	bool KeepLoaded() const override { return true; }
	bool IsBlocked(const std::string & /*key*/ ) const override { return false; }
};


enum ComponentState
{
	ComponentStateUnregistered,
	ComponentStateBlocked,
	ComponentStateUnintialized,
	ComponentStateUnavailable,
	ComponentStateAvailable,
};


struct ComponentInfo
{
	std::string name;
	ComponentState state;
	std::string settingsKey;
	ComponentType type;
};


class ComponentManager
{
	friend class ComponentFactoryBase;
public:
	static void Init(const IComponentManagerSettings &settings);
	static void Release();
	static std::shared_ptr<ComponentManager> Instance();
private:
	ComponentManager(const IComponentManagerSettings &settings);
private:
	struct RegisteredComponent
	{
		std::string settingsKey;
		ComponentFactoryMethod factoryMethod;
		std::shared_ptr<IComponent> instance;
		std::weak_ptr<IComponent> weakInstance;
	};
	using TComponentMap = std::map<std::string, RegisteredComponent>;
	const IComponentManagerSettings &m_Settings;
	TComponentMap m_Components;
private:
	bool IsComponentBlocked(const std::string &settingsKey) const;
	void InitializeComponent(std::shared_ptr<IComponent> component) const;
public:
	void Register(const IComponentFactory &componentFactory);
	void Startup();
	std::shared_ptr<const IComponent> GetComponent(const IComponentFactory &componentFactory);
	std::shared_ptr<const IComponent> ReloadComponent(const IComponentFactory &componentFactory);
	std::vector<std::string> GetRegisteredComponents() const;
	ComponentInfo GetComponentInfo(std::string name) const;
};


struct ComponentListEntry
{
	ComponentListEntry *next;
	void (*reg)(ComponentManager &componentManager);
};

bool ComponentListPush(ComponentListEntry *entry);

template <typename TComponent>
struct ComponentRegisterer
{
	static inline void RegisterComponent(ComponentManager &componentManager)
	{
		componentManager.Register(ComponentFactory<TComponent>());
	}
	static inline ComponentListEntry &GetComponentListEntry()
	{
		static ComponentListEntry s_ComponentListEntry = {nullptr, &RegisterComponent};
		return s_ComponentListEntry;
	}
	static inline bool g_ComponentRegistered = ComponentListPush(&GetComponentListEntry());
};

#define MPT_DECLARE_COMPONENT_MEMBERS(name, settingsKey) \
	public: \
		static constexpr const char *g_ID = #name ; \
		static constexpr const char *g_SettingsKey = settingsKey ; \
		static inline ComponentRegisterer< name > s_ComponentRegisterer; \
/**/


template <typename type>
std::shared_ptr<const type> GetComponent()
{
	return std::dynamic_pointer_cast<const type>(ComponentManager::Instance()->GetComponent(ComponentFactory<type>()));
}


template <typename type>
std::shared_ptr<const type> ReloadComponent()
{
	return std::dynamic_pointer_cast<const type>(ComponentManager::Instance()->ReloadComponent(ComponentFactory<type>()));
}


#else // !MPT_COMPONENT_MANAGER


#define MPT_DECLARE_COMPONENT_MEMBERS(name, settingsKey)


template <typename type>
std::shared_ptr<const type> GetComponent()
{
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif // MPT_COMPILER_CLANG
	static std::weak_ptr<type> cache;
	static mpt::mutex m;
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG	mpt::lock_guard<mpt::mutex> l(m);
	mpt::lock_guard<mpt::mutex> l(m);
	std::shared_ptr<type> component = cache.lock();
	if(!component)
	{
		component = std::make_shared<type>();
		component->Initialize();
		cache = component;
	}
	return component;
}


#endif // MPT_COMPONENT_MANAGER


// Simple wrapper around std::shared_ptr<ComponentType> which automatically
// gets a reference to the component (or constructs it) on initialization.
template <typename T>
class ComponentHandle
{
private:
	std::shared_ptr<const T> component;
public:
	ComponentHandle()
		: component(GetComponent<T>())
	{
		return;
	}
	~ComponentHandle()
	{
		return;
	}
	bool IsAvailable() const
	{
		return component && component->IsAvailable();
	}
	const T *get() const
	{
		return component.get();
	}
	const T &operator*() const
	{
		return *component;
	}
	const T *operator->() const
	{
		return &*component;
	}
#if MPT_COMPONENT_MANAGER
	void Reload()
	{
		component = nullptr;
		component = ReloadComponent<T>();
	}
#endif
};


template <typename T>
bool IsComponentAvailable(const ComponentHandle<T> &handle)
{
	return handle.IsAvailable();
}


OPENMPT_NAMESPACE_END
