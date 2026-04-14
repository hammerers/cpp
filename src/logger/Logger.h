

#ifndef LOGGER_H
#define LOGGER_H

#include<iostream>
#include<string>
#include<fstream>
#include<stdexcept>
#include<string.h>
#include<memory>
#include<mutex>
#include <format>  
#include <chrono>
#include<type_traits>
namespace Log {
    //log level
    enum class Level{
        debug = 0,
        info = 1,
        warning = 2,
        error = 3,
        fatal = 4,
        level_count = 5
    };

    //type trait check
    template<typename T,typename = void>
    struct is_streamable:std::false_type{};
    template<typename T>
    struct is_streamable<T,std::void_t<decltype(std::declval<std::ofstream &>()<<std::declval<T>())>>:std::true_type{};
    template<typename T>
    static inline constexpr bool is_streamable_v = is_streamable<T>::value;

    class Logger {
    public:
        static Logger& getInstance();
        void open(const std::string & file_name);
        void close();
        template<typename T, typename ...Args,
            typename sfinae = std::enable_if_t<is_streamable_v<T>&&(is_streamable_v<Args>&& ...)>
        >
        void log(const Log::Level & level,const char*file,int line,T&& t,Args &&...args) {
            if (m_fout_.fail()) {
                throw std::logic_error("Error opening file" + m_filename_);
            }
            std::lock_guard<std::mutex> lock(m_mutex_);
            m_fout_<< "[\n" ;
            _log_normal(level,file,line);
            m_fout_ << t;
            if constexpr (sizeof...(Args) > 0) {
                ((m_fout_ << " "<<args), ...);
            }
            m_fout_<<"\n";
            m_fout_<< "]\n";
            m_fout_.flush();
        }

        private:
        void _log_normal(Log::Level level,const char*file,int line);
        explicit Logger() noexcept = default;
        Logger(Logger&logger) = delete;
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        ~Logger();
    private:

        std::string m_filename_;
        std::ofstream m_fout_;
        static const char* s_level_[static_cast<int>(Level::level_count)];
        std::mutex m_mutex_;
    };

}


inline const char * Log::Logger::s_level_[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL"

};
inline Log::Logger& Log::Logger::getInstance() {
    static Logger logger;
    return logger;
}
#define LOG_DEBUG(...)  Log::Logger::getInstance().log(Log::Level::debug, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  Log::Logger::getInstance().log(Log::Level::info, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARNING(...)  Log::Logger::getInstance().log(Log::Level::warning, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...)  Log::Logger::getInstance().log(Log::Level::error, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...)  Log::Logger::getInstance().log(Log::Level::fatal, __FILE__, __LINE__, __VA_ARGS__)
inline void Log::Logger::open(const std::string & file_name) {
    std::lock_guard<std::mutex> lock(m_mutex_);
    if (m_fout_.is_open()) {
        m_fout_.close();
    }
    m_filename_ = file_name;
    m_fout_.open(file_name,std::ios::app);



}

inline void Log::Logger::close() {
    std::lock_guard<std::mutex> lock(m_mutex_);
    if (m_fout_.is_open()) {
        m_fout_.close();
    }
}

inline void Log::Logger::_log_normal(Log::Level level, const char *file, int line) {
    // time_t now = time(NULL);
    // struct tm now_tm;
    // #ifdef _WIN32
    //     localtime_s(&now_tm,&now);
    // #else
    //     localtime_r(&now, &now_tm);
    // #endif
    // char timestamp[32]={0};
    // strftime(timestamp,sizeof(timestamp),"%Y-%m-%d %H:%M:%S",&now_tm);
    // m_fout_ <<"time: "<<timestamp <<"\n"<<"level: "<< level<<"\n" <<"file: "<<file<<"\n"<<"line: "<< line <<"\n"<<"MSG: ";

    //auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    // m_fout_ << std::format(
    //     "time: {:%Y-%m-%d %H:%M:%S}\n"
    //     "level: {}\n"
    //     "file: {}\n"
    //     "line: {}\n"
    //     "MSG: ",
    //     now,s_level_[static_cast<int>(level)],file,line
    // );
    auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    std::print(m_fout_, 
    "time: {:%Y-%m-%d %H:%M:%S}\n"
    "level: {}\n"
    "file: {}\n"
    "line: {}\n"
    "MSG: ", 
    now, s_level_[static_cast<int>(level)], file, line
);
}


inline Log::Logger::~Logger() {
}

#endif