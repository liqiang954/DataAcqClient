#pragma once

#include <qfile.h>
#include <qdebug.h>
#include <QMessageBox>
#include "config.h"
#include "threadsafe_queue.h"



extern threadsafe_queue<totil_bufferData> queue_buf_struct;
