//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#ifndef __commandhandler_hh__
#define __commandhandler_hh__
#include "packagesocket.hh"
#include <string>

class CommandHandler {
public:
    virtual std::string name() const = 0;
    virtual void handle(PackageSocket & s) = 0;
};

CommandHandler * produceVersionHandler();
CommandHandler * producePingHandler();
CommandHandler * produceListHandler();
CommandHandler * produceImportHandler();
CommandHandler * produceDestroyHandler();
CommandHandler * produceJudgeHandler();
#endif //__commandhandler_hh__
