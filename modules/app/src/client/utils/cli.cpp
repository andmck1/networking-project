#include "cli.h"

#include "argh/argh.h"
#include <spdlog/spdlog.h>

bool Arguments::init(char *argv[])
{
    argh::parser cmdl(argv);
    if (!cmdl(2))
    {
        this->printHelpString();
        return false;
    }
    else if (cmdl(3))
    {
        this->printHelpString();
        return false;
    }

    cmdl(1) >> this->handle;
    spdlog::info("Handle: {}", this->handle);
    cmdl(2) >> this->skinIndex;
    if (this->skinIndex < 0 || this->skinIndex > 4)
    {
        this->printHelpString();
        return false;
    }
    spdlog::info("Skin Index: {}", this->skinIndex);
    return true;
}

void Arguments::printHelpString()
{
    spdlog::error(this->helpString);
}
