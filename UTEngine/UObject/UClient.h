#pragma once

#include "UObject.h"

class UClient : public UObject { using UObject::UObject; };
class UCanvas : public UObject { using UObject::UObject; };
class UConsole : public UObject { using UObject::UObject; };

class UPlayer : public UObject { using UObject::UObject; };
class UViewport : public UPlayer { using UPlayer::UPlayer; };
class UNetConnection : public UPlayer { using UPlayer::UPlayer; };
class UDemoRecConnection : public UNetConnection { using UNetConnection::UNetConnection; };

class UPendingLevel : public UObject { using UObject::UObject; };
class UNetPendingLevel : public UPendingLevel { using UPendingLevel::UPendingLevel; };
class UDemoPlayPendingLevel : public UPendingLevel { using UPendingLevel::UPendingLevel; };

class UChannel : public UObject { using UObject::UObject; };
class UControlChannel : public UChannel { using UChannel::UChannel; };
class UActorChannel : public UChannel { using UChannel::UChannel; };
class UFileChannel : public UChannel { using UChannel::UChannel; };
