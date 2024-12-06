#ifndef _SERVER_REGISTRY_H_
#define _SERVER_REGISTRY_H_

#include "RegistryKey.h"
#include "config.h"
#include "Registry.h"

RegistryKey * getServerRegistry(int updateMode=0);

int initServerRegistry();

int saveServerRegistry();

RegistryKey * getUserData(ULONGLONG accountId);
RegistryKey * getUserDataByChatId(DWORD id);

void setUpdating(int updating);
#endif