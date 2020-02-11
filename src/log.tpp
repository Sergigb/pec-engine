

template <typename T> void log_(std::ofstream &logfile, T t){
    logfile << t << std::endl;
}

template<typename T, typename... Args> void log_(std::ofstream &logfile, T t, Args... args){
    logfile << t;

    log_(logfile, args...) ;
}


template<typename T, typename... Args> int log(T t, Args... args){
    std::chrono::system_clock::time_point now;
    std::time_t now_tt;
    std::ofstream logfile;
    struct std::tm *now_tm;

    now = std::chrono::system_clock::now();
    now_tt = std::chrono::system_clock::to_time_t(now);
    now_tm = std::gmtime(&now_tt);

    logfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    try{
        logfile.open(LOG_FILE, std::ios_base::app);
        logfile << '[' << now_tm->tm_hour << ':' << now_tm->tm_min << ':' << now_tm->tm_sec << "] ";
        logfile << t;
        log_(logfile, args...);

        logfile.close();    
    }
    catch(const std::ios_base::failure& e){
        std::cerr << "Could not write log file: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}


template<typename T> int log(T t){
    std::chrono::system_clock::time_point now;
    std::time_t now_tt;
    std::ofstream logfile;
    struct std::tm *now_tm;

    now = std::chrono::system_clock::now();
    now_tt = std::chrono::system_clock::to_time_t(now);
    now_tm = std::gmtime(&now_tt);

    logfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    try{
        logfile.open(LOG_FILE, std::ios_base::app);
        logfile << '[' << now_tm->tm_hour << ':' << now_tm->tm_min << ':' << now_tm->tm_sec << "] ";
        logfile << t << std::endl;

        logfile.close();    
    }
    catch(const std::ios_base::failure& e){
        std::cerr << "Could not write log file: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
