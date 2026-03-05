
#include"Logger.h"
int main() {

    Log::Logger::getInstance().open("./log_1.txt");
    LOG_INFO("s");
    Log::Logger::getInstance().close();

}