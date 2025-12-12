#include <string>

struct Arguments
{
    std::string handle;
    int32_t skinIndex;

    bool init(char *argv[]);

    private:
    std::string helpString =
        "To start the application you must run client.exe "
        "<HANDLE> <SKIN_INDEX>, where <HANDLE> is a string and <SKIN_INDEX> is "
        "an integer between 0-4";
    void printHelpString();
};
