#pragma once

enum class PackageFlags : uint32_t
{
	None = 0,
	AllowDownload = 0x0001, // Allow downloading package
	ClientOptional = 0x0002, // Purely optional for clients
	ServerSideOnly = 0x0004, // Only needed on the server side
	BrokenLinks = 0x0008, // Loaded from linker with broken import links
	Unsecure = 0x0010, // Not trusted
	Need = 0x8000 // Client needs to download this package
};
