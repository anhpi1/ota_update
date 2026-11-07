#ifndef TESTLOG_H
#define TESTLOG_H

#include "main.h"
#include "cmsis_os.h"
#include "driver/uart/drvUart.h"
#include "service/logging/serLog.h"
#include "usersetting.h"

#define TESTLOG_ON SET_TESTLOG_ON

void testLogInit(void);

#if TESTLOG_ON
    #define TESTLOG() do { testLogInit(); } while(0)
#else   
    #define TESTLOG() do { (void)0; } while(0)
#endif



#endif /* TESTLOG_H */