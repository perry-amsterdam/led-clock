#pragma once
#include "globals.h"

String htmlWrap(const String& inner);
void handleRoot();
void handleSave();
void handleReset();
bool isIpLike(const String& host);
void handleNotFound();
void startPortal();
void stopPortal();
void handlePing();
