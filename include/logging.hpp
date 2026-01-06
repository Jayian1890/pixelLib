#include <iostream>
#include <memory>
#include <vector>
#include <cstddef>
#include <cstring>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <deque>
#include <atomic>
#include <sstream>
#include <string>

inline void localtime_threadsafe(const std::time_t *time, std::tm *tm) {
#if defined(_MSC_VER) || defined(_WIN32)

namespace core {

namespace logging {


enum LogLevel {
  LOG_TRACE = INTERLACED_LOG_LEVEL_TRACE,
  LOG_DEBUG = INTERLACED_LOG_LEVEL_DEBUG,
  LOG_INFO = INTERLACED_LOG_LEVEL_INFO,
  case LOG_TRACE:
    return "TRACE";
  case LOG_DEBUG:
    return "DEBUG";
  case LOG_INFO:
  default:
    return "UNKNOWN";
  }
}
  virtual void write(const std::string &message) = 0;
};

class RotatingFileLogger : public LogSink {
private:
  std::string base_filename_;
  std::ofstream current_file_;
public:
  RotatingFileLogger(const std::string &base_filename,
                     size_t max_file_size = 10485760, int max_files = 5)
      : base_filename_(base_filename), max_file_size_(max_file_size),
        max_files_(max_files), strategy_(RotationStrategy::SIZE),
      : base_filename_(base_filename), max_file_size_(0), max_files_(max_files),
        strategy_(RotationStrategy::TIME),
        last_rotation_(std::chrono::steady_clock::now()),
        rotation_interval_(rotation_hours), current_file_size_(0) {
    open_current_file();
          std::cerr << "Failed to open log file: " << base_filename_
                    << std::endl;
          std::cerr << message << std::endl;
          return;
        }
      current_file_.flush();

      if (strategy_ == RotationStrategy::SIZE) {
        current_file_size_ += message.length() + 1; 
  void test_set_badbit() { current_file_.setstate(std::ios::badbit); }
  void test_clear_badbit() { current_file_.clear(); }

private:
  void open_current_file() {
    try {
      current_file_.open(base_filename_, std::ios::app);
      if (current_file_.is_open() && strategy_ == RotationStrategy::SIZE) {
        current_file_.seekp(0, std::ios::end);
        current_file_size_ = current_file_.tellp();
      }
    } catch (const std::exception &e) {
      std::cerr << "Error opening log file: " << e.what() << std::endl;
    }
  }

  bool should_rotate(size_t next_write_size = 0) {
    try {
      if (strategy_ == RotationStrategy::SIZE) {
        return (current_file_size_ + next_write_size) >= max_file_size_;
      } else {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::hours>(
            now - last_rotation_);
        return elapsed >= rotation_interval_;
      }
    } catch (...) {
      return false;
    }
  }

  void rotate() {
    try {
      current_file_.close();

      for (int i = max_files_ - 1; i > 0; --i) {
        std::string old_name = base_filename_ + "." + std::to_string(i);
        std::string new_name = base_filename_ + "." + std::to_string(i + 1);

        std::remove(new_name.c_str());

        std::rename(old_name.c_str(), new_name.c_str());
      }

      std::string backup_name = base_filename_ + ".1";
      std::rename(base_filename_.c_str(), backup_name.c_str());

      open_current_file();
      current_file_size_ = 0;
      last_rotation_ = std::chrono::steady_clock::now();
    } catch (const std::exception &e) {
      std::cerr << "Error rotating log files: " << e.what() << std::endl;
      open_current_file();
    }
  }
};

class StreamSink : public LogSink {
private:
  std::ostream *out_;
public:
  explicit StreamSink(std::ostream &out) : out_(&out) {}
  void write(const std::string &message) override {
    if (!out_) {
      std::cerr << message << std::endl;
      return;
    }
    (*out_) << message << std::endl;
    if (!out_->good()) {
      out_->clear();
    }
  }

  void clear_stream() {
    if (out_) {
      out_->clear();
    }
  }
};

class AsyncLogSink : public LogSink {
public:
  enum class DropPolicy {
    DROP_OLDEST,
    DROP_NEWEST,
    BLOCK
  };

private:
  std::unique_ptr<LogSink> inner_;
  std::deque<std::string> queue_;
  size_t max_queue_size_;
  DropPolicy policy_;
  std::chrono::milliseconds block_timeout_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  std::thread worker_;
  std::atomic<bool> running_;
  std::atomic<size_t> dropped_count_;

  void worker_loop() {
    std::unique_lock<std::mutex> lock(mutex_);
    for (;;) {
      cv_.wait(lock, [&] { return !running_ || !queue_.empty(); });
      if (!running_ && queue_.empty()) {
        break;
      }
      while (!queue_.empty()) {
        std::string msg = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        try {
          if (inner_) inner_->write(msg);
        } catch (...) {
          std::cerr << "AsyncLogSink inner sink write failed" << std::endl;
        }
        lock.lock();
        if (queue_.empty()) cv_.notify_all();
      }
    }
  }

public:
  AsyncLogSink(std::unique_ptr<LogSink> inner, size_t max_queue_size = 1024,
               DropPolicy policy = DropPolicy::DROP_NEWEST,
               std::chrono::milliseconds block_timeout = std::chrono::milliseconds(100))
      : inner_(std::move(inner)),
        max_queue_size_(max_queue_size),
        policy_(policy),
        block_timeout_(block_timeout),
        running_(true),
        dropped_count_(0) {
    worker_ = std::thread(&AsyncLogSink::worker_loop, this);
  }

  ~AsyncLogSink() override {
    running_ = false;
    cv_.notify_all();
    if (worker_.joinable()) worker_.join();
    size_t dropped = dropped_count_.load();
    if (dropped > 0) {
      std::cerr << "AsyncLogSink dropped " << dropped << " messages" << std::endl;
    }
  }

  void write(const std::string &message) override {
    std::unique_lock<std::mutex> lock(mutex_);
    if (queue_.size() >= max_queue_size_) {
      switch (policy_) {
      case DropPolicy::DROP_OLDEST:
        queue_.pop_front();
        ++dropped_count_;
        queue_.push_back(message);
        cv_.notify_one();
        return;
      case DropPolicy::DROP_NEWEST:
        ++dropped_count_;
        return;
      case DropPolicy::BLOCK: {
        if (!cv_.wait_for(lock, block_timeout_, [&] { return queue_.size() < max_queue_size_; })) {
          ++dropped_count_;
          return;
        }
        queue_.push_back(message);
        cv_.notify_one();
        return;
      }
      }
    } else {
      queue_.push_back(message);
      cv_.notify_one();
    }
  }

  size_t dropped_count() const { return dropped_count_.load(); }

  size_t queue_size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  void flush() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [&] { return queue_.empty(); });
  }

  void shutdown() {
    running_ = false;
    cv_.notify_all();
    if (worker_.joinable()) worker_.join();
  }
};

} 
} 
} 

namespace {
  inline std::unordered_map<std::string, std::string>& get_log_context() {
    thread_local std::unordered_map<std::string, std::string> context;
    return context;
  }
}

namespace interlaced {
namespace core {
namespace logging {
namespace LogContextStorage {
  inline void set(const std::string &key, const std::string &value) {
    get_log_context()[key] = value;
  }

  inline void remove(const std::string &key) {
    get_log_context().erase(key);
  }

  inline std::string get(const std::string &key) {
    auto it = get_log_context().find(key);
    return it != get_log_context().end() ? it->second : "";
  }

  inline const std::unordered_map<std::string, std::string> &get_all() {
    return get_log_context();
  }
}

class LogContext {
private:
  std::vector<std::pair<std::string, std::string>> added_keys_;

public:
  LogContext() = default;
  ~LogContext() {
    for (const auto &key : added_keys_) {
      remove_context(key.first);
    }
  }

  template <typename T>
  void add(const std::string &key, const T &value) {
    std::ostringstream oss;
    oss << value;
    add_context(key, oss.str());
    added_keys_.emplace_back(key, oss.str());
  }

  void remove(const std::string &key) {
    remove_context(key);
    added_keys_.erase(
        std::remove_if(added_keys_.begin(), added_keys_.end(),
                       [&](const auto &p) { return p.first == key; }),
        added_keys_.end());
  }

private:
  static void add_context(const std::string &key, const std::string &value);
  static void remove_context(const std::string &key);
};

inline void LogContext::add_context(const std::string &key, const std::string &value) {
  LogContextStorage::set(key, value);
}

inline void LogContext::remove_context(const std::string &key) {
  LogContextStorage::remove(key);
}

class LogFormatter {
public:
  virtual ~LogFormatter() = default;
  virtual std::string format(LogLevel level, const std::string &message,
                             const std::tm &time_info,
                             const char *file = nullptr, int line = 0) = 0;
};

class JSONLogFormatter : public LogFormatter {
public:
  std::string format(LogLevel level, const std::string &message,
                     const std::tm &time_info, const char *file = nullptr,
                     int line = 0) override {
    std::ostringstream oss;
    oss << "{";
    oss << "\"timestamp\":\"" << std::put_time(&time_info, "%Y-%m-%dT%H:%M:%SZ") << "\",";
    oss << "\"level\":\"" << log_level_to_string(level) << "\",";
    oss << "\"message\":\"" << escape_json(message) << "\"";

    const auto &context = LogContextStorage::get_all();
    if (!context.empty()) {
      oss << ",\"context\":{";
      bool first = true;
      for (const auto &kv : context) {
        if (!first) oss << ",";
        oss << "\"" << escape_json(kv.first) << "\":\"" << escape_json(kv.second) << "\"";
        first = false;
      }
      oss << "}";
    }

    if (file && line > 0) {
      oss << ",\"file\":\"" << escape_json(file) << "\",\"line\":" << line;
    }

    oss << "}";
    return oss.str();
  }

private:
  std::string escape_json(const std::string &s) {
    std::string escaped;
    for (char c : s) {
      switch (c) {
      case '"': escaped += "\\\""; break;
      case '\\': escaped += "\\\\"; break;
      case '\n': escaped += "\\n"; break;
      case '\r': escaped += "\\r"; break;
      case '\t': escaped += "\\t"; break;
      default: escaped += c; break;
      }
    }
    return escaped;
  }
};
class DefaultLogFormatter : public LogFormatter {
private:
  TimestampFormat timestamp_format_;
  std::string prefix_;

public:
  DefaultLogFormatter(TimestampFormat format = TimestampFormat::STANDARD,
                      const std::string &prefix = "")
      : timestamp_format_(format), prefix_(prefix) {}

  std::string format(LogLevel level, const std::string &message,
                     const std::tm &time_info, const char *file = nullptr,
                     int line = 0) override {
    std::ostringstream oss;

    if (!prefix_.empty()) {
      oss << prefix_ << " ";
    }

    switch (timestamp_format_) {
    case TimestampFormat::STANDARD: {
      oss << "[" << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S") << "] ";
      break;
    }
    case TimestampFormat::ISO8601: {
      oss << "[" << std::put_time(&time_info, "%Y-%m-%dT%H:%M:%SZ") << "] ";
      break;
    }
    case TimestampFormat::UNIX: {
      oss << "[" << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S") << "] ";
      break;
    }
    case TimestampFormat::NONE:
      break;
    }

    oss << "[" << log_level_to_string(level) << "] ";

    oss << message;

    const auto &context = LogContextStorage::get_all();
    if (!context.empty()) {
      oss << " |";
      for (const auto &kv : context) {
        oss << " " << kv.first << "=" << kv.second;
      }
    }

    if (file && line > 0) {
      const char *filename = file;
      for (const char *p = file; *p; ++p) {
        if (*p == '/' || *p == '\\') {
          filename = p + 1;
        }
      }
      oss << " (" << filename << ":" << line << ")";
    }

    return oss.str();
  }
};

class Logger {
private:
  static LogLevel current_level;
  static std::mutex log_mutex;
  static std::ostream
      *output_stream;
  static std::ostream
      *error_stream;
  static std::vector<std::unique_ptr<LogSink>> sinks;
  static std::unique_ptr<LogFormatter> formatter;
  static std::unique_ptr<RotatingFileLogger>
      file_logger; 

public:
  static void set_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(log_mutex);
    current_level = level;
  }

  static void set_output_streams(std::ostream &output, std::ostream &error) {
    std::lock_guard<std::mutex> lock(log_mutex);
    output_stream = &output;
    error_stream = &error;
    
    sinks.clear();
    sinks.emplace_back(std::make_unique<StreamSink>(output));
    sinks.emplace_back(std::make_unique<StreamSink>(error));
  }

  static void add_sink(std::unique_ptr<LogSink> sink) {
    std::lock_guard<std::mutex> lock(log_mutex);
    sinks.emplace_back(std::move(sink));
  }

  static size_t get_async_dropped_count();
  static size_t get_async_queue_size();

  static void async_flush();

  static void async_shutdown();

  static void set_file_logging(const std::string &filename,
                               size_t max_file_size = 10485760,
                               int max_files = 5) {
    std::lock_guard<std::mutex> lock(log_mutex);
    
    sinks.clear();
    sinks.emplace_back(std::make_unique<RotatingFileLogger>(filename, max_file_size, max_files));
    file_logger.reset();
  }

  static void set_file_logging(const std::string &filename,
                               std::chrono::hours rotation_hours,
                               int max_files = 5) {
    std::lock_guard<std::mutex> lock(log_mutex);
    sinks.clear();
    sinks.emplace_back(std::make_unique<RotatingFileLogger>(filename, rotation_hours, max_files));
    file_logger.reset();
  }

  static void set_file_logging(std::nullptr_t) {
    std::lock_guard<std::mutex> lock(log_mutex);
    file_logger.reset();
    sinks.clear();
  }

  static void set_formatter(std::unique_ptr<LogFormatter> custom_formatter) {
    std::lock_guard<std::mutex> lock(log_mutex);
    formatter = std::move(custom_formatter);
  }

  struct LoggerConfig {
    LogLevel level = LOG_INFO;
    std::vector<std::unique_ptr<LogSink>> sinks;
    std::unique_ptr<LogFormatter> formatter;

    LoggerConfig() = default;
    LoggerConfig(LoggerConfig &&) = default;
    LoggerConfig &operator=(LoggerConfig &&) = default;
    ~LoggerConfig() = default;
  };

  class LoggerConfigBuilder {
  private:
    LoggerConfig cfg_;
  public:
    LoggerConfigBuilder &set_level(LogLevel level) {
      cfg_.level = level;
      return *this;
    }

    LoggerConfigBuilder &add_stream_sink(std::ostream &out) {
      cfg_.sinks.emplace_back(std::make_unique<StreamSink>(out));
      return *this;
    }

    LoggerConfigBuilder &add_file_sink(const std::string &filename, size_t max_file_size = 10485760, int max_files = 5) {
      cfg_.sinks.emplace_back(std::make_unique<RotatingFileLogger>(filename, max_file_size, max_files));
      return *this;
    }

    LoggerConfigBuilder &add_async_sink(std::unique_ptr<LogSink> inner_sink, size_t max_queue_size = 1024, AsyncLogSink::DropPolicy policy = AsyncLogSink::DropPolicy::DROP_NEWEST) {
      cfg_.sinks.emplace_back(std::make_unique<AsyncLogSink>(std::move(inner_sink), max_queue_size, policy));
      return *this;
    }

    LoggerConfigBuilder &add_async_stream_sink(std::ostream &out, size_t max_queue_size = 1024, AsyncLogSink::DropPolicy policy = AsyncLogSink::DropPolicy::DROP_NEWEST) {
      cfg_.sinks.emplace_back(std::make_unique<AsyncLogSink>(std::make_unique<StreamSink>(out), max_queue_size, policy));
      return *this;
    }

    LoggerConfigBuilder &add_async_file_sink(const std::string &filename, size_t max_queue_size = 1024, AsyncLogSink::DropPolicy policy = AsyncLogSink::DropPolicy::DROP_NEWEST, size_t max_file_size = 10485760, int max_files = 5) {
      cfg_.sinks.emplace_back(std::make_unique<AsyncLogSink>(std::make_unique<RotatingFileLogger>(filename, max_file_size, max_files), max_queue_size, policy));
      return *this;
    }

    LoggerConfigBuilder &set_formatter(std::unique_ptr<LogFormatter> formatter) {
      cfg_.formatter = std::move(formatter);
      return *this;
    }

    LoggerConfig build() { return std::move(cfg_); }
  };

  static void configure(LoggerConfig cfg) {
    std::lock_guard<std::mutex> lock(log_mutex);
    current_level = cfg.level;
    sinks = std::move(cfg.sinks);
    formatter = std::move(cfg.formatter);
    
  }

  class LoggerRegistry {
  public:
    static void set_config(const std::string &name, LoggerConfig cfg) {
      std::lock_guard<std::mutex> lock(registry_mutex());
      registry()[name] = std::move(cfg);
    }

    static bool has_config(const std::string &name) {
      std::lock_guard<std::mutex> lock(registry_mutex());
      return registry().find(name) != registry().end();
    }

    static LoggerConfig *get_config(const std::string &name) {
      std::lock_guard<std::mutex> lock(registry_mutex());
      auto it = registry().find(name);
      if (it == registry().end()) return nullptr;
      return &it->second;
    }

  private:
    static std::unordered_map<std::string, LoggerConfig> &registry() {
      static std::unordered_map<std::string, LoggerConfig> instance;
      return instance;
    }

    static std::mutex &registry_mutex() {
      static std::mutex m;
      return m;
    }
  };

  class CategoryLogger {
  private:
    std::string name_;
  public:
    explicit CategoryLogger(const std::string &name) : name_(name) {}

    void log(LogLevel level, const std::string &message) {
      
      LoggerConfig *cfg = LoggerRegistry::get_config(name_);
      LogLevel effective_level = current_level;
      if (cfg) effective_level = cfg->level;
      if (level < effective_level) return;

      
      const auto now = std::chrono::system_clock::now();
      const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
      std::tm local_tm; localtime_threadsafe(&now_time_t, &local_tm);

      std::string formatted_message;
      if (cfg && cfg->formatter) {
        formatted_message = cfg->formatter->format(level, message, local_tm);
      } else if (formatter) {
        formatted_message = formatter->format(level, message, local_tm);
      } else {
        std::ostringstream oss;
        oss << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] [" << name_ << "] [" << log_level_to_string(level) << "] " << message;
        formatted_message = oss.str();
      }

      
      std::vector<LogSink*> write_sinks;
      if (cfg && !cfg->sinks.empty()) {
        for (auto &s : cfg->sinks) write_sinks.push_back(s.get());
      } else if (!sinks.empty()) {
        for (auto &s : sinks) write_sinks.push_back(s.get());
      }

      
      if (write_sinks.empty()) {
        std::lock_guard<std::mutex> lock(log_mutex);
        try {
          std::ostream &out_stream = (level == LOG_ERROR || level == LOG_FATAL) ? *error_stream : *output_stream;
          out_stream << formatted_message << std::endl;
          if (!out_stream.good()) out_stream.clear();
        } catch (...) {
          std::cerr << "Unknown logging error occurred" << std::endl;
          std::cerr << formatted_message << std::endl;
        }
        return;
      }

      
      for (auto *sink : write_sinks) {
        try {
          sink->write(formatted_message);
        } catch (const std::exception &e) {
          std::cerr << "Logging error: " << e.what() << std::endl;
          std::cerr << formatted_message << std::endl;
        } catch (...) {
          std::cerr << "Unknown logging error occurred" << std::endl;
          std::cerr << formatted_message << std::endl;
        }
      }
    }

    
    void debug(const std::string &message) { log(LOG_DEBUG, message); }
    void info(const std::string &message) { log(LOG_INFO, message); }
    void warning(const std::string &message) { log(LOG_WARNING, message); }
    void error(const std::string &message) { log(LOG_ERROR, message); }
    void trace(const std::string &message) { log(LOG_TRACE, message); }
    void fatal(const std::string &message) { log(LOG_FATAL, message); }
  };

  static CategoryLogger get(const std::string &name) { return CategoryLogger(name); }

  static void log(LogLevel level, const std::string &message) {
    
    {
      std::lock_guard<std::mutex> lock(log_mutex);
      if (level < current_level) {
        return;
      }
    }

    
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    
    std::tm local_tm;
    localtime_threadsafe(&now_time_t, &local_tm);

    std::string formatted_message;

    
    if (formatter) {
      formatted_message = formatter->format(level, message, local_tm);
    } else {
      std::ostringstream oss;
      oss << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ["
          << log_level_to_string(level) << "] " << message;
      formatted_message = oss.str();

      
      {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (!sinks.empty()) {
          for (auto &sink : sinks) {
            try {
              sink->write(formatted_message);
            } catch (const std::exception &e) {
              if (auto *ss = dynamic_cast<StreamSink*>(sink.get())) {
                ss->clear_stream();
              }
              std::cerr << "Logging error: " << e.what() << std::endl;
              std::cerr << formatted_message << std::endl;
              std::cerr.flush();
            } catch (...) {
              if (auto *ss = dynamic_cast<StreamSink*>(sink.get())) {
                ss->clear_stream();
              }
              std::cerr << "Unknown logging error occurred" << std::endl;
              std::cerr << formatted_message << std::endl;
              std::cerr.flush();
            }
          }
          return;
        }
      }
    }

    
    std::lock_guard<std::mutex> lock(log_mutex);
    if (file_logger) {
      file_logger->write(formatted_message);
    } else {
      try {
        
        std::ostream &out_stream =
            (level == LOG_ERROR) ? *error_stream : *output_stream;
        out_stream << formatted_message << std::endl;

        
        if (!out_stream.good()) {
          
          out_stream.clear();
        }
      } catch (const std::exception &e) {
        
        std::cerr << "Logging error: " << e.what() << std::endl;
        std::cerr << formatted_message << std::endl;
      } catch (...) {
        
        std::cerr << "Unknown logging error occurred" << std::endl;
        std::cerr << formatted_message << std::endl;
      }
    }
  }

  static void log(LogLevel level, const std::string &message, const char *file,
                  int line) {
    
    {
      std::lock_guard<std::mutex> lock(log_mutex);
      if (level < current_level) {
        return;
      }
    }

    
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    
    std::tm local_tm;
    localtime_threadsafe(&now_time_t, &local_tm);

    std::string formatted_message;

    
    if (formatter) {
      formatted_message =
          formatter->format(level, message, local_tm, file, line);
    } else {
      
      const char *filename = file;
      for (const char *p = file; *p; ++p) {
        if (*p == '/' || *p == '\\') {
          filename = p + 1;
        }
      }

      
      std::ostringstream oss;
      oss << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ["
          << log_level_to_string(level) << "] " << message << " (" << filename
          << ":" << line << ")";
      formatted_message = oss.str();

      
      {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (!sinks.empty()) {
          for (auto &sink : sinks) {
            try {
              sink->write(formatted_message);
            } catch (const std::exception &e) {
              if (auto *ss = dynamic_cast<StreamSink*>(sink.get())) {
                ss->clear_stream();
              }
              std::cerr << "Logging error: " << e.what() << std::endl;
              std::cerr << formatted_message << std::endl;
              std::cerr.flush();
            } catch (...) {
              if (auto *ss = dynamic_cast<StreamSink*>(sink.get())) {
                ss->clear_stream();
              }
              std::cerr << "Unknown logging error occurred" << std::endl;
              std::cerr << formatted_message << std::endl;
              std::cerr.flush();
            }
          }
          return;
        }
      }
    }

    
    std::lock_guard<std::mutex> lock(log_mutex);
    if (file_logger) {
      file_logger->write(formatted_message);
    } else {
      try {
        
        std::ostream &out_stream =
            (level == LOG_ERROR) ? *error_stream : *output_stream;
        out_stream << formatted_message << std::endl;

        
        if (!out_stream.good()) {
          
          out_stream.clear();
        }
      } catch (const std::exception &e) {
        
        std::cerr << "Logging error: " << e.what() << std::endl;
        std::cerr << formatted_message << std::endl;
      } catch (...) {
        
        std::cerr << "Unknown logging error occurred" << std::endl;
        std::cerr << formatted_message << std::endl;
      }
    }
  }

#if INTERLACED_COMPILED_LOG_LEVEL <= INTERLACED_LOG_LEVEL_DEBUG
  static void debug(const std::string &message) { log(LOG_DEBUG, message); }

  static void debug(const std::string &message, const char *file, int line) {
    log(LOG_DEBUG, message, file, line);
  }

  template <typename... Args>
  static void debug(const char *format, Args &&...args) {
    format_and_log_with_format_string<Args...>(LOG_DEBUG, format, std::forward<Args>(args)...);
  }
#else
  
  static void debug(const std::string &) { }
  static void debug(const std::string &, const char *, int) { }
  template <typename... Args>
  static void debug(const char *, Args &&...) { }
#endif

  static void info(const std::string &message) { log(LOG_INFO, message); }

  static void info(const std::string &message, const char *file, int line) {
    log(LOG_INFO, message, file, line);
  }

  static void warning(const std::string &message) { log(LOG_WARNING, message); }

  static void warning(const std::string &message, const char *file, int line) {
    log(LOG_WARNING, message, file, line);
  }

  static void error(const std::string &message) { log(LOG_ERROR, message); }

  static void error(const std::string &message, const char *file, int line) {
    log(LOG_ERROR, message, file, line);
  }

#if INTERLACED_COMPILED_LOG_LEVEL <= INTERLACED_LOG_LEVEL_TRACE
  static void trace(const std::string &message) { log(LOG_TRACE, message); }
  static void trace(const std::string &message, const char *file, int line) {
    log(LOG_TRACE, message, file, line);
  }

  template <typename... Args>
  static void trace(const char *format, Args &&...args) {
    format_and_log_with_format_string<Args...>(LOG_TRACE, format, std::forward<Args>(args)...);
  }
#else
  
  static void trace(const std::string &) { }
  static void trace(const std::string &, const char *, int) { }
  template <typename... Args>
  static void trace(const char *, Args &&...) { }
#endif

  static void fatal(const std::string &message) { log(LOG_FATAL, message); }
  static void fatal(const std::string &message, const char *file, int line) {
    log(LOG_FATAL, message, file, line);
  }

  template <typename... Args>
  static void fatal(const char *format, Args &&...args) {
    format_and_log_with_format_string<Args...>(LOG_FATAL, format, std::forward<Args>(args)...);
  }

  
  template <typename... Args>
  static void log(LogLevel level, const std::string &message, Args &&...args) {
    
    {
      std::lock_guard<std::mutex> lock(log_mutex);
      if (level < current_level) {
        return;
      }
    }

    
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    
    std::tm local_tm;
    localtime_threadsafe(&now_time_t, &local_tm);

    std::string formatted_message;

    
    if (formatter) {
      formatted_message = formatter->format(level, message, local_tm);
    } else {
      
      std::ostringstream oss;
      oss << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ["
          << log_level_to_string(level) << "] " << message;

      
      format_key_value_pairs(oss, std::forward<Args>(args)...);
      formatted_message = oss.str();

      
      {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (!sinks.empty()) {
          for (auto &sink : sinks) {
            try {
              sink->write(formatted_message);
            } catch (const std::exception &e) {
              if (auto *ss = dynamic_cast<StreamSink*>(sink.get())) {
                ss->clear_stream();
              }
              std::cerr << "Logging error: " << e.what() << std::endl;
              std::cerr << formatted_message << std::endl;
              std::cerr.flush();
            } catch (...) {
              if (auto *ss = dynamic_cast<StreamSink*>(sink.get())) {
                ss->clear_stream();
              }
              std::cerr << "Unknown logging error occurred" << std::endl;
              std::cerr << formatted_message << std::endl;
              std::cerr.flush();
            }
          }
          return;
        }
      }
    }

    
    std::lock_guard<std::mutex> lock(log_mutex);
    if (file_logger) {
      file_logger->write(formatted_message);
    } else {
      try {
        
        std::ostream &out_stream =
            (level == LOG_ERROR) ? *error_stream : *output_stream;
        out_stream << formatted_message << std::endl;

        
        if (!out_stream.good()) {
          
          out_stream.clear();
        }
      } catch (const std::exception &e) {
        
        std::cerr << "Logging error: " << e.what() << std::endl;
        std::cerr << formatted_message << std::endl;
      } catch (...) {
        
        std::cerr << "Unknown logging error occurred" << std::endl;
        std::cerr << formatted_message << std::endl;
      }
    }
  }

  template <typename... Args>
  static void debug(const std::string &message, Args &&...args) {
    log(LOG_DEBUG, message, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void info(const std::string &message, Args &&...args) {
    log(LOG_INFO, message, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void warning(const std::string &message, Args &&...args) {
    log(LOG_WARNING, message, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void error(const std::string &message, Args &&...args) {
    log(LOG_ERROR, message, std::forward<Args>(args)...);
  }
  
  template <typename... Args>
  static void format_and_log_with_format_string(LogLevel level, const char *format, Args &&...args) {
    
    if (std::strstr(format, "{}") != nullptr) {
      std::ostringstream oss;
      format_message(oss, format, std::forward<Args>(args)...);
      log(level, oss.str());
      return;
    }

    
    if constexpr (sizeof...(Args) == 2) {
      using First = std::tuple_element_t<0, std::tuple<Args...>>;
      using Second = std::tuple_element_t<1, std::tuple<Args...>>;
      if constexpr (std::is_convertible_v<std::decay_t<First>, const char *> &&
                    std::is_integral_v<std::decay_t<Second>>) {
        log(level, std::string(format), std::forward<Args>(args)...);
        return;
      }
    }

    if constexpr (sizeof...(Args) > 0) {
      if constexpr (sizeof...(Args) % 2 == 0) {
        
        log(level, std::string(format), std::forward<Args>(args)...);
        return;
      } else {
        
        log(level, std::string(format));
        return;
      }
    } else {
      
      log(level, std::string(format));
      return;
    }
  }

  
  

  template <typename... Args>
  static void info(const char *format, Args &&...args) {
    format_and_log_with_format_string<Args...>(LOG_INFO, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void warning(const char *format, Args &&...args) {
    format_and_log_with_format_string<Args...>(LOG_WARNING, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void error(const char *format, Args &&...args) {
    format_and_log_with_format_string<Args...>(LOG_ERROR, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void format_message(std::ostringstream &oss, const char *format,
                             Args &&...args) {
    format_helper(oss, format, std::forward<Args>(args)...);
  }

private:
  template <typename Key, typename Value, typename... Args>
  static void format_key_value_pairs(std::ostringstream &oss, Key &&key,
                                     Value &&value, Args &&...args) {
    oss << " " << key << "=" << value;
    format_key_value_pairs(oss, std::forward<Args>(args)...);
  }

  static void format_key_value_pairs(std::ostringstream &oss) {
    
    (void)oss;
  }

  template <typename T, typename... Args>
  static void format_helper(std::ostringstream &oss, const char *format,
                            T &&value, Args &&...args) {
    while (*format) {
      if (*format == '{' && *(format + 1) == '}') {
        oss << value;
        format += 2;
        format_helper(oss, format, std::forward<Args>(args)...);
        return;
      } else {
        oss << *format;
        ++format;
      }
    }
  }

  static void format_helper(std::ostringstream &oss, const char *format) {
    while (*format) {
      oss << *format;
      ++format;
    }
  }
};


inline LogLevel Logger::current_level = LOG_INFO;
inline std::mutex Logger::log_mutex;
inline std::ostream *Logger::output_stream = &std::cout;
inline std::ostream *Logger::error_stream = &std::cerr;
inline std::vector<std::unique_ptr<LogSink>> Logger::sinks;
inline std::unique_ptr<LogFormatter> Logger::formatter = nullptr;
inline std::unique_ptr<RotatingFileLogger> Logger::file_logger = nullptr;


inline size_t Logger::get_async_dropped_count() {
  std::lock_guard<std::mutex> lock(Logger::log_mutex);
  size_t tot = 0;
  for (auto &s : Logger::sinks) {
    if (auto *a = dynamic_cast<AsyncLogSink *>(s.get())) {
      tot += a->dropped_count();
    }
  }
  return tot;
}

inline size_t Logger::get_async_queue_size() {
  std::lock_guard<std::mutex> lock(Logger::log_mutex);
  size_t tot = 0;
  for (auto &s : Logger::sinks) {
    if (auto *a = dynamic_cast<AsyncLogSink *>(s.get())) {
      tot += a->queue_size();
    }
  }
  return tot;
}

inline void Logger::async_flush() {
  std::lock_guard<std::mutex> lock(Logger::log_mutex);
  for (auto &s : Logger::sinks) {
    if (auto *a = dynamic_cast<AsyncLogSink *>(s.get())) {
      a->flush();
    }
  }
}

inline void Logger::async_shutdown() {
  std::lock_guard<std::mutex> lock(Logger::log_mutex);
  for (auto &s : Logger::sinks) {
    if (auto *a = dynamic_cast<AsyncLogSink *>(s.get())) {
      a->shutdown();
    }
  }
}
#if INTERLACED_COMPILED_LOG_LEVEL <= INTERLACED_LOG_LEVEL_TRACE
#ifndef INTERLACED_CORE_DISABLE_TRACE_LOGS
#define LOG_TRACE(msg)                                                         \
  interlaced::core::logging::Logger::trace(msg, __FILE__, __LINE__)
#else
#define LOG_TRACE(msg) (void)0
#endif
#else
#define LOG_TRACE(msg) (void)0
#endif

#if INTERLACED_COMPILED_LOG_LEVEL <= INTERLACED_LOG_LEVEL_DEBUG
#ifndef INTERLACED_CORE_DISABLE_DEBUG_LOGS
#define LOG_DEBUG(msg)                                                         \
  interlaced::core::logging::Logger::debug(msg, __FILE__, __LINE__)
#else
#define LOG_DEBUG(msg) (void)0
#endif
#else
#define LOG_DEBUG(msg) (void)0
#endif
#define LOG_INFO(msg)                                                          \
  interlaced::core::logging::Logger::info(msg, __FILE__, __LINE__)
#define LOG_FATAL(msg)                                                         \
  interlaced::core::logging::Logger::fatal(msg, __FILE__, __LINE__)
#define LOG_WARNING(msg)                                                       \
  interlaced::core::logging::Logger::warning(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg)                                                         \
  interlaced::core::logging::Logger::error(msg, __FILE__, __LINE__)



inline void test_force_clear_stream(std::ostream &s) {
  if (!s.good()) {
    s.clear();
  }
}

inline void test_force_logging_error_messages(const std::string &msg) {
  std::cerr << "File logging error: " << msg << std::endl;
  std::cerr << msg << std::endl;
  std::cerr << "Unknown logging error occurred" << std::endl;
  std::cerr << msg << std::endl;
}

} 

} 

} 

#endif 
