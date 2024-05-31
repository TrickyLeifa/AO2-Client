#pragma once

#include <QString>

void call_error(QString message);
void call_notice(QString message);
void call_message_error(QString message, QtMsgType messageType);
