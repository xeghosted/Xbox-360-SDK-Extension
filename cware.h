#pragma once

#include <xtl.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <xkelib.h>

#define array_size(x) (sizeof(x) / sizeof((x)[0]))

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif
#ifndef FILE_ATTRIBUTE_REPARSE_POINT
#define FILE_ATTRIBUTE_REPARSE_POINT 0x0400
#endif

#ifndef FILE_ATTRIBUTE_READONLY
#define FILE_ATTRIBUTE_READONLY 0x0001
#endif

#ifndef FILE_ATTRIBUTE_HIDDEN
#define FILE_ATTRIBUTE_HIDDEN 0x0002
#endif

#ifndef FILE_ATTRIBUTE_SYSTEM
#define FILE_ATTRIBUTE_SYSTEM 0x0004
#endif

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x0010
#endif

#ifndef FILE_ATTRIBUTE_ARCHIVE
#define FILE_ATTRIBUTE_ARCHIVE 0x0020
#endif

namespace cwarejson {
    namespace json {
        class json;
    }
}

namespace cwarejson {
    namespace json {
        class json {
        public:
            enum value_t {
                null,
                object,
                array,
                string,
                boolean,
                number_integer,
                number_float
            };

        private:
            value_t m_type;
            union {
                bool boolean;
                long long number_integer;
                double number_float;
            } m_value;

            std::string* m_string;
            std::map<std::string, json>* m_object;
            std::vector<json>* m_array;

        public:
            // Constructors
            json() : m_type(null), m_string(nullptr), m_object(nullptr), m_array(nullptr) {
                m_value.number_integer = 0;
            }

            json(void* null_ptr) : m_type(null), m_string(nullptr), m_object(nullptr), m_array(nullptr) {
                (void)null_ptr; // suppress unused parameter warning
                m_value.number_integer = 0;
            }

            json(bool val) : m_type(boolean), m_string(nullptr), m_object(nullptr), m_array(nullptr) {
                m_value.boolean = val;
            }

            json(int val) : m_type(number_integer), m_string(nullptr), m_object(nullptr), m_array(nullptr) {
                m_value.number_integer = static_cast<long long>(val);
            }

            json(long long val) : m_type(number_integer), m_string(nullptr), m_object(nullptr), m_array(nullptr) {
                m_value.number_integer = val;
            }

            json(double val) : m_type(number_float), m_string(nullptr), m_object(nullptr), m_array(nullptr) {
                m_value.number_float = val;
            }

            json(const std::string& val) : m_type(string), m_object(nullptr), m_array(nullptr) {
                m_string = new std::string(val);
                m_value.number_integer = 0;
            }

            json(const char* val) : m_type(string), m_object(nullptr), m_array(nullptr) {
                m_string = new std::string(val);
                m_value.number_integer = 0;
            }

            // Copy constructor
            json(const json& other) : m_type(other.m_type), m_string(nullptr), m_object(nullptr), m_array(nullptr) {
                copy_from(other);
            }

            // Destructor
            ~json() {
                clear();
            }

            // Assignment operator
            json& operator=(const json& other) {
                if (this != &other) {
                    clear();
                    m_type = other.m_type;
                    copy_from(other);
                }
                return *this;
            }

            // Type checking
            bool is_null() const { return m_type == null; }
            bool is_boolean() const { return m_type == boolean; }
            bool is_number() const { return m_type == number_integer || m_type == number_float; }
            bool is_string() const { return m_type == string; }
            bool is_array() const { return m_type == array; }
            bool is_object() const { return m_type == object; }

            // Value access
            bool get_bool() const {
                if (m_type != boolean) throw std::runtime_error("not a boolean");
                return m_value.boolean;
            }

            long long get_int() const {
                if (m_type == number_integer) return m_value.number_integer;
                if (m_type == number_float) return static_cast<long long>(m_value.number_float);
                throw std::runtime_error("not a number");
            }

            double get_float() const {
                if (m_type == number_float) return m_value.number_float;
                if (m_type == number_integer) return static_cast<double>(m_value.number_integer);
                throw std::runtime_error("not a number");
            }

            const std::string& get_string() const {
                if (m_type != string) throw std::runtime_error("not a string");
                return *m_string;
            }

            // Object access
            json& operator[](const std::string& key) {
                if (m_type == null) {
                    m_type = object;
                    m_object = new std::map<std::string, json>();
                }
                if (m_type != object) throw std::runtime_error("not an object");
                return (*m_object)[key];
            }

            const json& operator[](const std::string& key) const {
                if (m_type != object) throw std::runtime_error("not an object");
                std::map<std::string, json>::const_iterator it = m_object->find(key);
                if (it == m_object->end()) throw std::runtime_error("key not found");
                return it->second;
            }

            // Array access
            json& operator[](size_t index) {
                if (m_type == null) {
                    m_type = array;
                    m_array = new std::vector<json>();
                }
                if (m_type != array) throw std::runtime_error("not an array");
                if (index >= m_array->size()) m_array->resize(index + 1);
                return (*m_array)[index];
            }

            const json& operator[](size_t index) const {
                if (m_type != array) throw std::runtime_error("not an array");
                if (index >= m_array->size()) throw std::runtime_error("index out of range");
                return (*m_array)[index];
            }

            // Array methods
            void push_back(const json& val) {
                if (m_type == null) {
                    m_type = array;
                    m_array = new std::vector<json>();
                }
                if (m_type != array) throw std::runtime_error("not an array");
                m_array->push_back(val);
            }

            size_t size() const {
                if (m_type == array) return m_array->size();
                if (m_type == object) return m_object->size();
                if (m_type == string) return m_string->size();
                return 0;
            }

            bool empty() const {
                return size() == 0;
            }

            // Serialization
            std::string dump() const {
                std::ostringstream ss;
                serialize(ss);
                return ss.str();
            }

            // Static parsing method
            static json parse(const std::string& str) {
                size_t pos = 0;
                return parse_value(str, pos);
            }

        private:
            void clear() {
                if (m_string) {
                    delete m_string;
                    m_string = nullptr;
                }
                if (m_object) {
                    delete m_object;
                    m_object = nullptr;
                }
                if (m_array) {
                    delete m_array;
                    m_array = nullptr;
                }
            }

            void copy_from(const json& other) {
                m_value = other.m_value;
                if (other.m_string) {
                    m_string = new std::string(*other.m_string);
                }
                if (other.m_object) {
                    m_object = new std::map<std::string, json>(*other.m_object);
                }
                if (other.m_array) {
                    m_array = new std::vector<json>(*other.m_array);
                }
            }

            void serialize(std::ostringstream& ss) const {
                switch (m_type) {
                case null:
                    ss << "null";
                    break;
                case boolean:
                    ss << (m_value.boolean ? "true" : "false");
                    break;
                case number_integer:
                    ss << m_value.number_integer;
                    break;
                case number_float:
                    ss << m_value.number_float;
                    break;
                case string:
                    ss << "\"" << escape_string(*m_string) << "\"";
                    break;
                case object:
                    ss << "{";
                    {
                        bool first = true;
                        for (std::map<std::string, json>::const_iterator it = m_object->begin();
                            it != m_object->end(); ++it) {
                            if (!first) ss << ",";
                            ss << "\"" << escape_string(it->first) << "\":";
                            it->second.serialize(ss);
                            first = false;
                        }
                    }
                    ss << "}";
                    break;
                case array:
                    ss << "[";
                    for (size_t i = 0; i < m_array->size(); ++i) {
                        if (i > 0) ss << ",";
                        (*m_array)[i].serialize(ss);
                    }
                    ss << "]";
                    break;
                }
            }

            static std::string escape_string(const std::string& str) {
                std::string result;
                for (size_t i = 0; i < str.length(); ++i) {
                    char c = str[i];
                    switch (c) {
                    case '\"': result += "\\\""; break;
                    case '\\': result += "\\\\"; break;
                    case '\b': result += "\\b"; break;
                    case '\f': result += "\\f"; break;
                    case '\n': result += "\\n"; break;
                    case '\r': result += "\\r"; break;
                    case '\t': result += "\\t"; break;
                    default: result += c; break;
                    }
                }
                return result;
            }

            static void skip_whitespace(const std::string& str, size_t& pos) {
                while (pos < str.length() && (str[pos] == ' ' || str[pos] == '\t' ||
                    str[pos] == '\n' || str[pos] == '\r')) {
                    ++pos;
                }
            }

            static json parse_value(const std::string& str, size_t& pos) {
                skip_whitespace(str, pos);
                if (pos >= str.length()) throw std::runtime_error("unexpected end of input");

                char c = str[pos];
                if (c == 'n') return parse_null(str, pos);
                if (c == 't' || c == 'f') return parse_boolean(str, pos);
                if (c == '"') return parse_string(str, pos);
                if (c == '[') return parse_array(str, pos);
                if (c == '{') return parse_object(str, pos);
                if (c == '-' || (c >= '0' && c <= '9')) return parse_number(str, pos);

                throw std::runtime_error("unexpected character");
            }

            static json parse_null(const std::string& str, size_t& pos) {
                if (str.substr(pos, 4) == "null") {
                    pos += 4;
                    return json();
                }
                throw std::runtime_error("invalid null value");
            }

            static json parse_boolean(const std::string& str, size_t& pos) {
                if (str.substr(pos, 4) == "true") {
                    pos += 4;
                    return json(true);
                }
                if (str.substr(pos, 5) == "false") {
                    pos += 5;
                    return json(false);
                }
                throw std::runtime_error("invalid boolean value");
            }

            static json parse_string(const std::string& str, size_t& pos) {
                if (str[pos] != '"') throw std::runtime_error("expected '\"'");
                ++pos;

                std::string result;
                while (pos < str.length() && str[pos] != '"') {
                    if (str[pos] == '\\') {
                        ++pos;
                        if (pos >= str.length()) throw std::runtime_error("unexpected end in string");
                        char escaped = str[pos];
                        switch (escaped) {
                        case '"': result += '"'; break;
                        case '\\': result += '\\'; break;
                        case '/': result += '/'; break;
                        case 'b': result += '\b'; break;
                        case 'f': result += '\f'; break;
                        case 'n': result += '\n'; break;
                        case 'r': result += '\r'; break;
                        case 't': result += '\t'; break;
                        default: throw std::runtime_error("invalid escape sequence");
                        }
                    }
                    else {
                        result += str[pos];
                    }
                    ++pos;
                }

                if (pos >= str.length()) throw std::runtime_error("unterminated string");
                ++pos; // skip closing quote
                return json(result);
            }

            static json parse_number(const std::string& str, size_t& pos) {
                size_t start = pos;
                bool is_float = false;

                if (str[pos] == '-') ++pos;

                if (pos >= str.length() || str[pos] < '0' || str[pos] > '9') {
                    throw std::runtime_error("invalid number");
                }

                while (pos < str.length() && str[pos] >= '0' && str[pos] <= '9') ++pos;

                if (pos < str.length() && str[pos] == '.') {
                    is_float = true;
                    ++pos;
                    while (pos < str.length() && str[pos] >= '0' && str[pos] <= '9') ++pos;
                }

                if (pos < str.length() && (str[pos] == 'e' || str[pos] == 'E')) {
                    is_float = true;
                    ++pos;
                    if (pos < str.length() && (str[pos] == '+' || str[pos] == '-')) ++pos;
                    while (pos < str.length() && str[pos] >= '0' && str[pos] <= '9') ++pos;
                }

                std::string num_str = str.substr(start, pos - start);
                if (is_float) {
                    return json(atof(num_str.c_str()));
                }
                else {
                    long long result = 0;
                    bool negative = false;
                    size_t i = 0;
                    if (num_str[0] == '-') {
                        negative = true;
                        i = 1;
                    }
                    for (; i < num_str.length(); ++i) {
                        result = result * 10 + (num_str[i] - '0');
                    }
                    return json(negative ? -result : result);
                }
            }

            static json parse_array(const std::string& str, size_t& pos) {
                if (str[pos] != '[') throw std::runtime_error("expected '['");
                ++pos;

                json result;
                result.m_type = array;
                result.m_array = new std::vector<json>();

                skip_whitespace(str, pos);
                if (pos < str.length() && str[pos] == ']') {
                    ++pos;
                    return result;
                }

                while (true) {
                    result.m_array->push_back(parse_value(str, pos));
                    skip_whitespace(str, pos);

                    if (pos >= str.length()) throw std::runtime_error("unterminated array");

                    if (str[pos] == ']') {
                        ++pos;
                        break;
                    }
                    else if (str[pos] == ',') {
                        ++pos;
                        skip_whitespace(str, pos);
                    }
                    else {
                        throw std::runtime_error("expected ',' or ']'");
                    }
                }

                return result;
            }

            static json parse_object(const std::string& str, size_t& pos) {
                if (str[pos] != '{') throw std::runtime_error("expected '{'");
                ++pos;

                json result;
                result.m_type = object;
                result.m_object = new std::map<std::string, json>();

                skip_whitespace(str, pos);
                if (pos < str.length() && str[pos] == '}') {
                    ++pos;
                    return result;
                }

                while (true) {
                    skip_whitespace(str, pos);
                    json key = parse_string(str, pos);
                    skip_whitespace(str, pos);

                    if (pos >= str.length() || str[pos] != ':') {
                        throw std::runtime_error("expected ':'");
                    }
                    ++pos;

                    json value = parse_value(str, pos);
                    (*result.m_object)[key.get_string()] = value;

                    skip_whitespace(str, pos);
                    if (pos >= str.length()) throw std::runtime_error("unterminated object");

                    if (str[pos] == '}') {
                        ++pos;
                        break;
                    }
                    else if (str[pos] == ',') {
                        ++pos;
                    }
                    else {
                        throw std::runtime_error("expected ',' or '}'");
                    }
                }

                return result;
            }
        };
    }

} // namespace cwarejson

namespace std {
    template <typename T>
    inline std::unique_ptr<T> make_unique()
    {
        return std::unique_ptr<T>(new T());
    }

    template <typename T, typename A1>
    inline std::unique_ptr<T> make_unique(const A1& a1)
    {
        return std::unique_ptr<T>(new T(a1));
    }

    template <typename T, typename A1, typename A2>
    inline std::unique_ptr<T> make_unique(const A1& a1, const A2& a2)
    {
        return std::unique_ptr<T>(new T(a1, a2));
    }

    template <typename T, typename A1, typename A2, typename A3>
    inline std::unique_ptr<T> make_unique(const A1& a1, const A2& a2, const A3& a3)
    {
        return std::unique_ptr<T>(new T(a1, a2, a3));
    }

    template <typename T>
    inline std::unique_ptr<T> make_unique_array(std::size_t size)
    {
        return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]());
    }

    template<typename ret, typename val>
    inline ret stcast(val v)
    {
        return static_cast<ret>(v);
    }

    template<typename ret, typename val>
    inline ret dycast(val v)
    {
        return dynamic_cast<ret>(v);
    }

    template<typename ret, typename val>
    inline ret cocast(val v)
    {
        return const_cast<ret>(v);
    }

    template<typename ret, typename val>
    inline ret recast(val v)
    {
        return reinterpret_cast<ret>(v);
    }

    inline bool isdigit(char c) {
        return c >= '0' && c <= '9';
    }

    inline bool isspace(char c) {
        return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
    }

    class mutex;
    class condition_variable;
    template<class Mutex> class unique_lock;
    template<class Mutex> class lock_guard;

    class mutex {
    private:
        CRITICAL_SECTION cs_;

        // Non-copyable
        mutex(const mutex&);
        mutex& operator=(const mutex&);

    public:
        // Default constructor
        mutex() {
            InitializeCriticalSection(&cs_);
        }

        // Destructor
        ~mutex() {
            DeleteCriticalSection(&cs_);
        }

        // Lock the mutex (blocking)
        void lock() {
            EnterCriticalSection(&cs_);
        }

        // Try to lock the mutex (non-blocking)
        // Returns true if lock was acquired, false otherwise
        bool try_lock() {
            return TryEnterCriticalSection(&cs_) != 0;
        }

        // Unlock the mutex
        void unlock() {
            LeaveCriticalSection(&cs_);
        }

        // Get native handle (for advanced usage)
        CRITICAL_SECTION* native_handle() {
            return &cs_;
        }
    };

    template<class Mutex>
    class lock_guard {
    private:
        Mutex& mutex_;

        // Non-copyable
        lock_guard(const lock_guard&);
        lock_guard& operator=(const lock_guard&);

    public:
        explicit lock_guard(Mutex& m) : mutex_(m) {
            mutex_.lock();
        }

        ~lock_guard() {
            mutex_.unlock();
        }
    };

    template<class Mutex>
    class unique_lock {
    private:
        Mutex* mutex_;
        bool owns_lock_;

        // Non-copyable
        unique_lock(const unique_lock&);
        unique_lock& operator=(const unique_lock&);

    public:
        // Constructor - locks the mutex
        explicit unique_lock(Mutex& m) : mutex_(&m), owns_lock_(true) {
            mutex_->lock();
        }

        // Constructor with defer_lock
        struct defer_lock_t {};
        static const defer_lock_t defer_lock;

        unique_lock(Mutex& m, defer_lock_t) : mutex_(&m), owns_lock_(false) {
            // Don't lock immediately
        }

        // Constructor with try_to_lock
        struct try_to_lock_t {};
        static const try_to_lock_t try_to_lock;

        unique_lock(Mutex& m, try_to_lock_t) : mutex_(&m), owns_lock_(false) {
            owns_lock_ = mutex_->try_lock();
        }

        // Destructor
        ~unique_lock() {
            if (owns_lock_) {
                mutex_->unlock();
            }
        }

        // Manual lock
        void lock() {
            if (!mutex_) return;
            if (owns_lock_) return; // Already locked

            mutex_->lock();
            owns_lock_ = true;
        }

        // Manual try_lock
        bool try_lock() {
            if (!mutex_) return false;
            if (owns_lock_) return false; // Already locked

            owns_lock_ = mutex_->try_lock();
            return owns_lock_;
        }

        // Manual unlock
        void unlock() {
            if (!owns_lock_) return;

            mutex_->unlock();
            owns_lock_ = false;
        }

        // Release ownership without unlocking
        Mutex* release() {
            Mutex* ret = mutex_;
            mutex_ = nullptr;
            owns_lock_ = false;
            return ret;
        }

        // Check if we own the lock
        bool owns_lock() const {
            return owns_lock_;
        }

        // Implicit conversion to bool
        operator bool() const {
            return owns_lock_;
        }

        // Get the mutex pointer
        Mutex* mutex() const {
            return mutex_;
        }
    };

    template<class Mutex>
    const typename unique_lock<Mutex>::defer_lock_t unique_lock<Mutex>::defer_lock = {};

    template<class Mutex>
    const typename unique_lock<Mutex>::try_to_lock_t unique_lock<Mutex>::try_to_lock = {};

    class condition_variable {
    private:
        HANDLE event_;
        CRITICAL_SECTION waiters_lock_;
        int waiters_count_;
        bool was_broadcast_;

        // Non-copyable
        condition_variable(const condition_variable&);
        condition_variable& operator=(const condition_variable&);

    public:
        condition_variable() : waiters_count_(0), was_broadcast_(false) {
            event_ = CreateEvent(NULL, TRUE, FALSE, NULL); // Manual reset event
            InitializeCriticalSection(&waiters_lock_);
        }

        ~condition_variable() {
            CloseHandle(event_);
            DeleteCriticalSection(&waiters_lock_);
        }

        // Wait for notification
        template<class Mutex>
        void wait(unique_lock<Mutex>& lock) {
            if (!lock.owns_lock()) {
                return; // Can't wait on unlocked mutex
            }

            // Register as waiter
            EnterCriticalSection(&waiters_lock_);
            waiters_count_++;
            LeaveCriticalSection(&waiters_lock_);

            // Release the mutex and wait
            lock.unlock();
            WaitForSingleObject(event_, INFINITE);

            // Reacquire the mutex
            lock.lock();

            // Unregister as waiter
            EnterCriticalSection(&waiters_lock_);
            waiters_count_--;
            bool last_waiter = was_broadcast_ && (waiters_count_ == 0);
            LeaveCriticalSection(&waiters_lock_);

            // Reset event if we were the last waiter in a broadcast
            if (last_waiter) {
                ResetEvent(event_);
            }
        }

        // Wait with predicate
        template<class Mutex, class Predicate>
        void wait(unique_lock<Mutex>& lock, Predicate pred) {
            while (!pred()) {
                wait(lock);
            }
        }

        // Wait with timeout (milliseconds)
        template<class Mutex>
        bool wait_for(unique_lock<Mutex>& lock, DWORD timeout_ms) {
            if (!lock.owns_lock()) {
                return false;
            }

            // Register as waiter
            EnterCriticalSection(&waiters_lock_);
            waiters_count_++;
            LeaveCriticalSection(&waiters_lock_);

            // Release the mutex and wait with timeout
            lock.unlock();
            DWORD result = WaitForSingleObject(event_, timeout_ms);

            // Reacquire the mutex
            lock.lock();

            // Unregister as waiter
            EnterCriticalSection(&waiters_lock_);
            waiters_count_--;
            bool last_waiter = was_broadcast_ && (waiters_count_ == 0);
            LeaveCriticalSection(&waiters_lock_);

            // Reset event if we were the last waiter in a broadcast
            if (last_waiter) {
                ResetEvent(event_);
            }

            return (result == WAIT_OBJECT_0);
        }

        // Wait with timeout and predicate
        template<class Mutex, class Predicate>
        bool wait_for(unique_lock<Mutex>& lock, DWORD timeout_ms, Predicate pred) {
            DWORD start_time = GetTickCount();
            while (!pred()) {
                DWORD elapsed = GetTickCount() - start_time;
                if (elapsed >= timeout_ms) {
                    return pred(); // Final check
                }

                DWORD remaining = timeout_ms - elapsed;
                if (!wait_for(lock, remaining)) {
                    return pred(); // Timeout occurred, final check
                }
            }
            return true;
        }

        // Notify one waiting thread
        void notify_one() {
            EnterCriticalSection(&waiters_lock_);
            bool have_waiters = (waiters_count_ > 0);
            LeaveCriticalSection(&waiters_lock_);

            if (have_waiters) {
                PulseEvent(event_);
            }
        }

        // Notify all waiting threads
        void notify_all() {
            EnterCriticalSection(&waiters_lock_);
            bool have_waiters = (waiters_count_ > 0);
            if (have_waiters) {
                was_broadcast_ = true;
            }
            LeaveCriticalSection(&waiters_lock_);

            if (have_waiters) {
                SetEvent(event_);
            }
        }

        // Get native handle
        HANDLE native_handle() {
            return event_;
        }
    };

    enum cv_status {
        no_timeout,
        timeout
    };

    enum launch_policy {
        launch_deferred = 1,
        launch_async = 2,
        launch_async_or_deferred = launch_async | launch_deferred
    };

    class future_error {
    public:
        const char* what() const { return "future_error"; }
    };

    enum future_status {
        future_ready,
        future_timeout,
        future_deferred
    };

    template<typename T>
    class future_state {
    public:
        mutex mtx;
        condition_variable cv;
        bool ready;
        bool has_exception;
        T result;
        const char* exception_msg;

        future_state() : ready(false), has_exception(false), exception_msg(nullptr) {}
    };

    template<typename T>
    class future {
    private:
        future_state<T>* state_;

    public:
        future() : state_(nullptr) {}

        explicit future(future_state<T>* state) : state_(state) {}

        future(future&& other) : state_(other.state_) {
            other.state_ = nullptr;
        }

        future& operator=(future&& other) {
            if (this != &other) {
                state_ = other.state_;
                other.state_ = nullptr;
            }
            return *this;
        }

        ~future() {

        }

        T get() {
            if (!state_) {
                throw future_error();
            }

            unique_lock<mutex> lock(state_->mtx);
            state_->cv.wait(lock, [this]() { return state_->ready; });

            if (state_->has_exception) {
                throw future_error();
            }

            return state_->result;
        }

        // Check if result is ready
        bool valid() const {
            return state_ != nullptr;
        }

        // Wait for result
        void wait() const {
            if (!state_) return;

            unique_lock<mutex> lock(state_->mtx);
            state_->cv.wait(lock, [this]() { return state_->ready; });
        }

        // Wait with timeout
        future_status wait_for(DWORD timeout_ms) const {
            if (!state_) return future_ready;

            unique_lock<mutex> lock(state_->mtx);
            if (state_->cv.wait_for(lock, timeout_ms, [this]() { return state_->ready; })) {
                return future_ready;
            }
            return future_timeout;
        }

    private:
        // Non-copyable
        future(const future&);
        future& operator=(const future&);
    };

    template<typename T>
    class promise {
    private:
        future_state<T>* state_;

    public:
        promise() : state_(new future_state<T>()) {}

        ~promise() {
            // Simplified cleanup
        }

        // Get the associated future
        future<T> get_future() {
            if (!state_) {
                throw future_error();
            }
            return future<T>(state_);
        }

        // Set the value
        void set_value(const T& value) {
            if (!state_) return;

            lock_guard<mutex> lock(state_->mtx);
            state_->result = value;
            state_->ready = true;
            state_->cv.notify_all();
        }

        // Set exception (simplified)
        void set_exception(const char* msg) {
            if (!state_) return;

            lock_guard<mutex> lock(state_->mtx);
            state_->exception_msg = msg;
            state_->has_exception = true;
            state_->ready = true;
            state_->cv.notify_all();
        }

    private:
        // Non-copyable
        promise(const promise&);
        promise& operator=(const promise&);
    };

    class thread_pool {
    private:
        struct task_base {
            virtual ~task_base() {}
            virtual void execute() = 0;
        };

        template<typename F>
        struct task : public task_base {
            F func;
            task(F&& f) : func(f) {}
            virtual void execute() { func(); }
        };

        static const int POOL_SIZE = 4; // Adjust for Xbox 360
        HANDLE threads_[POOL_SIZE];
        HANDLE shutdown_event_;
        mutex queue_mutex_;
        condition_variable queue_cv_;
        std::queue<task_base*> task_queue_;
        bool shutdown_;

        static void worker_thread(thread_pool* pool) {
            pool->worker_loop();
        }

        void worker_loop() {
            while (true) {
                task_base* task = nullptr;

                {
                    unique_lock<mutex> lock(queue_mutex_);
                    queue_cv_.wait(lock, [this]() { return !task_queue_.empty() || shutdown_; });

                    if (shutdown_ && task_queue_.empty()) {
                        break;
                    }

                    if (!task_queue_.empty()) {
                        task = task_queue_.front();
                        task_queue_.pop();
                    }
                }

                if (task) {
                    task->execute();
                    delete task;
                }
            }
        }

    public:
        thread_pool() : shutdown_(false) {
            shutdown_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);

            for (int i = 0; i < POOL_SIZE; ++i) {
                threads_[i] = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)worker_thread, this, 0, 0);
            }
        }

        ~thread_pool() {
            {
                lock_guard<mutex> lock(queue_mutex_);
                shutdown_ = true;
            }
            queue_cv_.notify_all();

            WaitForMultipleObjects(POOL_SIZE, threads_, TRUE, INFINITE);

            for (int i = 0; i < POOL_SIZE; ++i) {
                CloseHandle(threads_[i]);
            }
            CloseHandle(shutdown_event_);

            // Clean up remaining tasks
            while (!task_queue_.empty()) {
                delete task_queue_.front();
                task_queue_.pop();
            }
        }

        template<typename F>
        void enqueue(F&& func) {
            {
                lock_guard<mutex> lock(queue_mutex_);
                if (shutdown_) return;
                task_queue_.push(new task<F>(func));
            }
            queue_cv_.notify_one();
        }

        static thread_pool& instance() {
            static thread_pool pool;
            return pool;
        }
    };

    template<typename T>
    class Atomic {
    private:
        volatile T value_;

        static_assert(sizeof(T) <= sizeof(long), "Type too large for atomic operations");

    public:
        // Constructor
        Atomic() : value_(T()) {}
        explicit Atomic(T initial) : value_(initial) {}

        // Copy constructor and assignment (not atomic)
        Atomic(const Atomic& other) : value_(other.load()) {}
        Atomic& operator=(const Atomic& other) {
            store(other.load());
            return *this;
        }

        // Load operation
        T load() const {
            // Memory barrier to ensure proper ordering
            __lwsync();
            T result = value_;
            __lwsync();
            return result;
        }

        // Store operation
        void store(T new_value) {
            __lwsync();
            value_ = new_value;
            __lwsync();
        }

        // Exchange operation
        T exchange(T new_value) {
            return static_cast<T>(_InterlockedExchange(
                reinterpret_cast<volatile long*>(&value_),
                static_cast<long>(new_value)
            ));
        }

        // Compare and swap
        bool compare_exchange_weak(T& expected, T desired) {
            T old_val = static_cast<T>(_InterlockedCompareExchange(
                reinterpret_cast<volatile long*>(&value_),
                static_cast<long>(desired),
                static_cast<long>(expected)
            ));

            if (old_val == expected) {
                return true;
            }
            else {
                expected = old_val;
                return false;
            }
        }

        bool compare_exchange_strong(T& expected, T desired) {
            return compare_exchange_weak(expected, desired);
        }

        // Fetch and add (for integral types only)
        T fetch_add(T increment) {
            return static_cast<T>(_InterlockedExchangeAdd(
                reinterpret_cast<volatile long*>(&value_),
                static_cast<long>(increment)
            ));
        }

        // Fetch and subtract
        T fetch_sub(T decrement) {
            return fetch_add(-decrement);
        }

        // Pre-increment
        T operator++() {
            return fetch_add(1) + 1;
        }

        // Post-increment
        T operator++(int) {
            return fetch_add(1);
        }

        // Pre-decrement
        T operator--() {
            return fetch_sub(1) - 1;
        }

        // Post-decrement
        T operator--(int) {
            return fetch_sub(1);
        }

        // Addition assignment
        T operator+=(T increment) {
            return fetch_add(increment) + increment;
        }

        // Subtraction assignment
        T operator-=(T decrement) {
            return fetch_sub(decrement) - decrement;
        }

        // Implicit conversion operator
        operator T() const {
            return load();
        }

        // Assignment operator
        T operator=(T new_value) {
            store(new_value);
            return new_value;
        }
    };

    enum memory_order {
        memory_order_relaxed,
        memory_order_acquire,
        memory_order_release,
        memory_order_acq_rel,
        memory_order_seq_cst
    };

    inline void atomic_thread_fence(memory_order order) {
        switch (order) {
        case memory_order_acquire:
        case memory_order_release:
        case memory_order_acq_rel:
        case memory_order_seq_cst:
            __lwsync();
            break;
        case memory_order_relaxed:
        default:
            // No barrier needed for relaxed ordering
            break;
        }
    }

    typedef Atomic<int> AtomicInt;
    typedef Atomic<long> AtomicLong;
    typedef Atomic<bool> AtomicBool;
    typedef Atomic<unsigned int> AtomicUInt;
    typedef Atomic<unsigned long> AtomicULong;

    namespace chrono {

        // Simple ratio implementation (since std::ratio doesn't exist in Xbox 360)
        template<int64_t Num, int64_t Den = 1>
        struct ratio {
            static const int64_t num = Num;
            static const int64_t den = Den;
        };

        // Common ratios
        typedef ratio<1, 1000000000> nano;
        typedef ratio<1, 1000000> micro;
        typedef ratio<1, 1000> milli;
        typedef ratio<1> ratio_1;
        typedef ratio<60> ratio_60;
        typedef ratio<3600> ratio_3600;

        // Simple type traits (since we don't have full C++11 support)
        template<typename T, typename U>
        struct is_same {
            static const bool value = false;
        };

        template<typename T>
        struct is_same<T, T> {
            static const bool value = true;
        };

        template<typename T, typename U>
        struct common_type {
            typedef T type;
        };

        // Duration class - simplified for Xbox 360
        template<typename Rep, typename Period>
        class duration {
        public:
            typedef Rep rep;
            typedef Period period;

        private:
            rep count_;

        public:
            // Constructors
            duration() : count_(0) {}

            explicit duration(const rep& r) : count_(r) {}

            // Copy constructor for same type
            duration(const duration& other) : count_(other.count_) {}

            // Assignment
            duration& operator=(const duration& other) {
                count_ = other.count_;
                return *this;
            }

            // Accessors
            rep count() const { return count_; }

            // Arithmetic operators
            duration operator+() const { return *this; }
            duration operator-() const { return duration(-count_); }

            duration& operator++() { ++count_; return *this; }
            duration operator++(int) { duration temp(*this); ++count_; return temp; }
            duration& operator--() { --count_; return *this; }
            duration operator--(int) { duration temp(*this); --count_; return temp; }

            duration& operator+=(const duration& d) { count_ += d.count(); return *this; }
            duration& operator-=(const duration& d) { count_ -= d.count(); return *this; }
            duration& operator*=(const rep& r) { count_ *= r; return *this; }
            duration& operator/=(const rep& r) { count_ /= r; return *this; }

            // Comparison operators
            bool operator==(const duration& rhs) const { return count_ == rhs.count_; }
            bool operator!=(const duration& rhs) const { return count_ != rhs.count_; }
            bool operator<(const duration& rhs) const { return count_ < rhs.count_; }
            bool operator<=(const duration& rhs) const { return count_ <= rhs.count_; }
            bool operator>(const duration& rhs) const { return count_ > rhs.count_; }
            bool operator>=(const duration& rhs) const { return count_ >= rhs.count_; }

            // Static zero function
            static duration zero() { return duration(0); }
        };

        // Duration arithmetic (simplified - same types only)
        template<typename Rep, typename Period>
        duration<Rep, Period> operator+(const duration<Rep, Period>& lhs, const duration<Rep, Period>& rhs) {
            return duration<Rep, Period>(lhs.count() + rhs.count());
        }

        template<typename Rep, typename Period>
        duration<Rep, Period> operator-(const duration<Rep, Period>& lhs, const duration<Rep, Period>& rhs) {
            return duration<Rep, Period>(lhs.count() - rhs.count());
        }

        template<typename Rep, typename Period>
        duration<Rep, Period> operator*(const duration<Rep, Period>& d, const Rep& s) {
            return duration<Rep, Period>(d.count() * s);
        }

        template<typename Rep, typename Period>
        duration<Rep, Period> operator*(const Rep& s, const duration<Rep, Period>& d) {
            return duration<Rep, Period>(s * d.count());
        }

        template<typename Rep, typename Period>
        duration<Rep, Period> operator/(const duration<Rep, Period>& d, const Rep& s) {
            return duration<Rep, Period>(d.count() / s);
        }

        // Common duration types
        typedef duration<int64_t, nano> nanoseconds;
        typedef duration<int64_t, micro> microseconds;
        typedef duration<int64_t, milli> milliseconds;
        typedef duration<int64_t, ratio_1> seconds;
        typedef duration<int, ratio_60> minutes;
        typedef duration<int, ratio_3600> hours;

        // Duration cast function (simplified)
        template<typename ToDuration, typename Rep, typename Period>
        ToDuration duration_cast(const duration<Rep, Period>& d) {
            // Simple conversion - assumes same base units for now
            // In a full implementation, this would handle ratio conversions
            typedef typename ToDuration::rep to_rep;

            // Convert based on period ratios
            int64_t from_factor = Period::den / Period::num;
            int64_t to_factor = ToDuration::period::den / ToDuration::period::num;

            int64_t converted = (static_cast<int64_t>(d.count()) * from_factor) / to_factor;
            return ToDuration(static_cast<to_rep>(converted));
        }

        // Specializations for common conversions
        template<>
        inline milliseconds duration_cast<milliseconds, int64_t, nano>(const nanoseconds& d) {
            return milliseconds(d.count() / 1000000);
        }

        template<>
        inline microseconds duration_cast<microseconds, int64_t, nano>(const nanoseconds& d) {
            return microseconds(d.count() / 1000);
        }

        template<>
        inline seconds duration_cast<seconds, int64_t, milli>(const milliseconds& d) {
            return seconds(d.count() / 1000);
        }

        template<>
        inline milliseconds duration_cast<milliseconds, int64_t, ratio_1>(const seconds& d) {
            return milliseconds(d.count() * 1000);
        }

        // Time point class
        template<typename Clock, typename Duration = typename Clock::duration>
        class time_point {
        public:
            typedef Clock clock;
            typedef Duration duration;
            typedef typename duration::rep rep;
            typedef typename duration::period period;

        private:
            duration d_;

        public:
            // Constructors
            time_point() : d_(duration(0)) {}
            explicit time_point(const duration& d) : d_(d) {}
            time_point(const time_point& other) : d_(other.d_) {}

            // Assignment
            time_point& operator=(const time_point& other) {
                if (this != &other) {
                    d_ = other.d_;
                }
                return *this;
            }

            // Accessors
            duration time_since_epoch() const { return d_; }

            // Arithmetic
            time_point& operator+=(const duration& d) { d_ += d; return *this; }
            time_point& operator-=(const duration& d) { d_ -= d; return *this; }

            // Comparison
            bool operator==(const time_point& rhs) const { return d_ == rhs.d_; }
            bool operator!=(const time_point& rhs) const { return d_ != rhs.d_; }
            bool operator<(const time_point& rhs) const { return d_ < rhs.d_; }
            bool operator<=(const time_point& rhs) const { return d_ <= rhs.d_; }
            bool operator>(const time_point& rhs) const { return d_ > rhs.d_; }
            bool operator>=(const time_point& rhs) const { return d_ >= rhs.d_; }
        };

        // Time point arithmetic
        template<typename Clock, typename Duration>
        time_point<Clock, Duration> operator+(const time_point<Clock, Duration>& pt, const Duration& d) {
            return time_point<Clock, Duration>(pt.time_since_epoch() + d);
        }

        template<typename Clock, typename Duration>
        time_point<Clock, Duration> operator+(const Duration& d, const time_point<Clock, Duration>& pt) {
            return time_point<Clock, Duration>(d + pt.time_since_epoch());
        }

        template<typename Clock, typename Duration>
        time_point<Clock, Duration> operator-(const time_point<Clock, Duration>& pt, const Duration& d) {
            return time_point<Clock, Duration>(pt.time_since_epoch() - d);
        }

        template<typename Clock, typename Duration>
        Duration operator-(const time_point<Clock, Duration>& lhs, const time_point<Clock, Duration>& rhs) {
            return lhs.time_since_epoch() - rhs.time_since_epoch();
        }

        // Clock implementations
        class system_clock {
        public:
            typedef milliseconds duration;
            typedef duration::rep rep;
            typedef duration::period period;
            typedef time_point<system_clock, duration> time_point;

            static const bool is_steady = false;

            static time_point now() {
#ifdef _XBOX
                // Xbox 360 specific implementation
                LARGE_INTEGER freq, counter;
                QueryPerformanceFrequency(&freq);
                QueryPerformanceCounter(&counter);

                // Convert to milliseconds
                int64_t ms = (counter.QuadPart * 1000) / freq.QuadPart;
                return time_point(duration(ms));
#else
                // Windows fallback
                FILETIME ft;
                GetSystemTimeAsFileTime(&ft);

                ULARGE_INTEGER uli;
                uli.LowPart = ft.dwLowDateTime;
                uli.HighPart = ft.dwHighDateTime;

                // Convert from 100ns intervals to milliseconds
                int64_t ms = uli.QuadPart / 10000;
                return time_point(duration(ms));
#endif
            }
        };

        class steady_clock {
        public:
            typedef nanoseconds duration;
            typedef duration::rep rep;
            typedef duration::period period;
            typedef time_point<steady_clock, duration> time_point;

            static const bool is_steady = true;

            static time_point now() {
#ifdef _XBOX
                // Xbox 360 high resolution timer
                LARGE_INTEGER freq, counter;
                QueryPerformanceFrequency(&freq);
                QueryPerformanceCounter(&counter);

                // Convert to nanoseconds
                int64_t ns = (counter.QuadPart * 1000000000LL) / freq.QuadPart;
                return time_point(duration(ns));
#else
                // Windows fallback
                LARGE_INTEGER freq, counter;
                QueryPerformanceFrequency(&freq);
                QueryPerformanceCounter(&counter);

                int64_t ns = (counter.QuadPart * 1000000000LL) / freq.QuadPart;
                return time_point(duration(ns));
#endif
            }
        };

        typedef steady_clock high_resolution_clock;

        // Helper functions for creating durations
        inline milliseconds make_milliseconds(int64_t ms) {
            return milliseconds(ms);
        }

        inline seconds make_seconds(int64_t s) {
            return seconds(s);
        }

        inline minutes make_minutes(int min) {
            return minutes(min);
        }

        inline hours make_hours(int h) {
            return hours(h);
        }

        // Sleep function
        inline void sleep_for(const milliseconds& ms) {
#ifdef _XBOX
            Sleep(static_cast<DWORD>(ms.count()));
#else
            Sleep(static_cast<DWORD>(ms.count()));
#endif
        }

        // Helper class for measuring elapsed time
        class stopwatch {
        private:
            steady_clock::time_point start_time_;
            bool running_;

        public:
            stopwatch() : running_(false) {
                // Initialize start_time_ with a default constructed time_point
                start_time_ = steady_clock::time_point();
            }

            void start() {
                start_time_ = steady_clock::now();
                running_ = true;
            }

            void stop() {
                running_ = false;
            }

            void reset() {
                start_time_ = steady_clock::now();
                running_ = false;
            }

            int64_t elapsed_milliseconds() const {
                if (!running_) return 0;
                steady_clock::time_point now = steady_clock::now();
                steady_clock::duration elapsed_ns = now.time_since_epoch() - start_time_.time_since_epoch();
                return duration_cast<milliseconds>(elapsed_ns).count();
            }

            int64_t elapsed_seconds() const {
                if (!running_) return 0;
                steady_clock::time_point now = steady_clock::now();
                steady_clock::duration elapsed_ns = now.time_since_epoch() - start_time_.time_since_epoch();
                return duration_cast<seconds>(elapsed_ns).count();
            }

            bool is_running() const { return running_; }
        };

        // Game timing helper class
        class game_timer {
        private:
            steady_clock::time_point last_frame_;
            milliseconds target_frame_time_;

        public:
            game_timer(int target_fps)
                : target_frame_time_(1000 / target_fps) {
                last_frame_ = steady_clock::now();
            }

            float get_delta_time() {
                steady_clock::time_point now = steady_clock::now();
                steady_clock::duration delta = now.time_since_epoch() - last_frame_.time_since_epoch();
                last_frame_ = now;

                milliseconds delta_ms = duration_cast<milliseconds>(delta);
                return static_cast<float>(delta_ms.count()) / 1000.0f; // Return as seconds
            }

            void limit_framerate() {
                steady_clock::time_point now = steady_clock::now();
                steady_clock::duration frame_time = now.time_since_epoch() - last_frame_.time_since_epoch();
                milliseconds frame_ms = duration_cast<milliseconds>(frame_time);

                if (frame_ms.count() < target_frame_time_.count()) {
                    milliseconds sleep_time(target_frame_time_.count() - frame_ms.count());
                    sleep_for(sleep_time);
                }
            }
        };

    } // namespace chrono

    namespace filesystem {
        class path {
        public:
            typedef std::string string_type;
            typedef std::vector<string_type> string_list;

            path() {}
            path(const char* s) : m_path(s) {}
            path(const std::string& s) : m_path(s) {}

            // Conversion
            const std::string& string() const { return m_path; }
            const char* c_str() const { return m_path.c_str(); }

            // Concatenation with /
            path& operator/=(const path& other) {
                if (!m_path.empty() && m_path[m_path.size() - 1] != '/' && m_path[m_path.size() - 1] != '\\') {
#ifdef _WIN32
                    m_path += '\\';
#else
                    m_path += '/';
#endif
                }
                m_path += other.m_path;
                return *this;
            }

            friend path operator/(const path& lhs, const path& rhs) {
                path tmp(lhs);
                tmp /= rhs;
                return tmp;
            }

            // Filename extraction
            std::string filename() const {
                size_t pos = m_path.find_last_of("/\\");
                if (pos == std::string::npos)
                    return m_path;
                return m_path.substr(pos + 1);
            }

            // Parent path extraction
            path parent_path() const {
                size_t pos = m_path.find_last_of("/\\");
                if (pos == std::string::npos)
                    return path();
                return path(m_path.substr(0, pos));
            }

            // Extension (including dot)
            std::string extension() const {
                std::string fname = filename();
                size_t pos = fname.find_last_of('.');
                if (pos == std::string::npos) return "";
                return fname.substr(pos);
            }

            // Stem (filename without extension)
            std::string stem() const {
                std::string fname = filename();
                size_t pos = fname.find_last_of('.');
                if (pos == std::string::npos) return fname;
                return fname.substr(0, pos);
            }

            bool empty() const { return m_path.empty(); }

        private:
            std::string m_path;
        };

        // Checks if path exists
        inline bool exists(const path& p) {
            DWORD attrs = GetFileAttributesA(p.c_str());
            return (attrs != INVALID_FILE_ATTRIBUTES);
        }

        // Checks if path is a directory
        inline bool is_directory(const path& p) {
            DWORD attrs = GetFileAttributesA(p.c_str());
            return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
        }

        // Create directories recursively
        inline bool create_directories(const path& p) {
            if (p.empty())
                return false;

            if (CreateDirectoryA(p.c_str(), NULL))
                return true;

            DWORD error = GetLastError();
            if (error == ERROR_ALREADY_EXISTS && is_directory(p))
                return true;

            if (error == ERROR_PATH_NOT_FOUND) {
                path parent = p.parent_path();
                if (!parent.empty() && create_directories(parent)) {
                    return CreateDirectoryA(p.c_str(), NULL) != 0;
                }
            }

            return false;
        }

        // Copy file (basic)
        inline bool copy_file(const path& from, const path& to, bool overwrite = false) {
            return CopyFileA(from.c_str(), to.c_str(), overwrite ? FALSE : TRUE) != 0;
        }

        // Copy function (basic version of std::filesystem::copy)
        inline bool copy(const path& from, const path& to, bool recursive = false) {
            if (is_directory(from)) {
                if (!create_directories(to))
                    return false;

                WIN32_FIND_DATAA ffd;
                std::string search = from.string() + "\\*";
                HANDLE hFind = FindFirstFileA(search.c_str(), &ffd);

                if (hFind == INVALID_HANDLE_VALUE)
                    return false;

                do {
                    std::string name = ffd.cFileName;
                    if (name == "." || name == "..")
                        continue;

                    path src = from / name;
                    path dst = to / name;

                    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        if (recursive) {
                            if (!copy(src, dst, true)) {
                                FindClose(hFind);
                                return false;
                            }
                        }
                    }
                    else {
                        if (!copy_file(src, dst, true)) {
                            FindClose(hFind);
                            return false;
                        }
                    }
                } while (FindNextFileA(hFind, &ffd));

                FindClose(hFind);
                return true;
            }
            else {
                return copy_file(from, to, true);
            }
        }

        // Move a single file
        inline bool move_file(const path& from, const path& to, bool overwrite = false) {
            if (!exists(from) || is_directory(from))
                return false;

            if (overwrite && exists(to))
                DeleteFileA(to.c_str());

            return MoveFileA(from.c_str(), to.c_str()) != 0;
        }

        // Move a folder recursively
        inline bool move_folder(const path& from, const path& to, bool overwrite = false) {
            if (!is_directory(from))
                return false;

            if (exists(to) && overwrite) {
                return false;
            }

            // Attempt simple MoveFile first (works if on same drive)
            if (MoveFileA(from.c_str(), to.c_str()) != 0)
                return true;

            // Otherwise, fallback to copy + delete
            if (!copy(from, to, true))
                return false;

            // Delete source recursively
            std::function<void(const path&)> delete_recursive = [&](const path& p) {
                WIN32_FIND_DATAA ffd;
                std::string search = p.string() + "\\*";
                HANDLE hFind = FindFirstFileA(search.c_str(), &ffd);
                if (hFind == INVALID_HANDLE_VALUE) return;

                do {
                    std::string name = ffd.cFileName;
                    if (name == "." || name == "..") continue;

                    path child = p / name;
                    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        delete_recursive(child);
                        RemoveDirectoryA(child.c_str());
                    }
                    else {
                        DeleteFileA(child.c_str());
                    }
                } while (FindNextFileA(hFind, &ffd));
                FindClose(hFind);
                };

            delete_recursive(from);
            return RemoveDirectoryA(from.c_str()) != 0;
        }

        inline bool remove(const path& p) {
            if (is_directory(p)) {
                return RemoveDirectoryA(p.c_str()) != 0;
            }
            return DeleteFileA(p.c_str()) != 0;
        }

        inline bool remove_all(const path& p) {
            if (is_directory(p)) {
                WIN32_FIND_DATAA ffd;
                std::string search = p.string() + "\\*";
                HANDLE hFind = FindFirstFileA(search.c_str(), &ffd);
                if (hFind != INVALID_HANDLE_VALUE) {
                    do {
                        std::string name = ffd.cFileName;
                        if (name == "." || name == "..") continue;
                        path child = p / name;
                        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                            remove_all(child);
                        }
                        else {
                            remove(child);
                        }
                    } while (FindNextFileA(hFind, &ffd));
                    FindClose(hFind);
                }
                return RemoveDirectoryA(p.c_str()) != 0;
            }
            else {
                return DeleteFileA(p.c_str()) != 0;
            }
        }

        inline bool rename(const path& from, const path& to) {
            return MoveFileA(from.c_str(), to.c_str()) != 0;
        }

        inline uint64_t file_size(const path& p) {
            WIN32_FILE_ATTRIBUTE_DATA info;
            if (GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &info)) {
                LARGE_INTEGER size;
                size.HighPart = info.nFileSizeHigh;
                size.LowPart = info.nFileSizeLow;
                return static_cast<uint64_t>(size.QuadPart);
            }
            return 0;
        }

        inline bool is_regular_file(const path& p) {
            DWORD attrs = GetFileAttributesA(p.c_str());
            return attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
        }

        inline bool is_symlink(const path& p) {
            DWORD attrs = GetFileAttributesA(p.c_str());
            return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_REPARSE_POINT);
        }

        inline std::vector<path> directory_iterator(const path& p) {
            std::vector<path> result;
            WIN32_FIND_DATAA ffd;
            HANDLE hFind = FindFirstFileA((p / "*").c_str(), &ffd);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    std::string name = ffd.cFileName;
                    if (name != "." && name != "..") {
                        result.push_back(p / name);
                    }
                } while (FindNextFileA(hFind, &ffd));
                FindClose(hFind);
            }
            return result;
        }
    }
}

namespace cware {
    namespace network {
        void Initialize() {
            XNetStartupParams xnsp;
            xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
            xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;

            NetDll_XNetStartup(XNCALLER_SYSAPP, &xnsp);

            WSADATA wsaData;
            NetDll_WSAStartupEx(XNCALLER_SYSAPP, MAKEWORD(0x2, 0x2), &wsaData, 0x2);
        }

        bool ip_from_string(const char* ip_str, in_addr& out_addr) {
            unsigned long ip = NetDll_inet_addr(ip_str);
            if (ip == INADDR_NONE) {
                return false; // invalid string
            }
            out_addr.S_un.S_addr = ip;
            return true;
        }

        class AsyncTcpClient {
        private:
            struct TcpConnection {
                SOCKET socket;
                sockaddr_in server_addr;
                bool connected;
                bool connecting;
            };

            struct AsyncOperation {
                enum Type {
                    CONNECT,
                    SEND,
                    RECEIVE,
                    CLOSE
                };

                Type type;
                std::function<void(bool, const std::string&)> callback;
                std::vector<uint8_t> send_data;
                size_t receive_size;
                std::string host;
                uint16_t port;
            };

            TcpConnection connection_;
            std::queue<AsyncOperation> operation_queue_;
            bool operation_in_progress_;
            HANDLE network_thread_;
            HANDLE shutdown_event_;

            static DWORD WINAPI network_thread_proc(LPVOID param) {
                AsyncTcpClient* client = static_cast<AsyncTcpClient*>(param);
                client->network_thread();
                return 0;
            }

            void network_thread() {
                while (WaitForSingleObject(shutdown_event_, 0) != WAIT_OBJECT_0) {
                    if (!operation_in_progress_ && !operation_queue_.empty()) {
                        AsyncOperation op = operation_queue_.front();
                        operation_queue_.pop();
                        operation_in_progress_ = true;

                        switch (op.type) {
                        case AsyncOperation::CONNECT:
                            handle_connect(op);
                            break;
                        case AsyncOperation::SEND:
                            handle_send(op);
                            break;
                        case AsyncOperation::RECEIVE:
                            handle_receive(op);
                            break;
                        case AsyncOperation::CLOSE:
                            handle_close(op);
                            break;
                        }
                    }
                    Sleep(10);
                }
            }

            void handle_connect(const AsyncOperation& op) {
                bool success = false;
                std::string error_msg;

                // Initialize socket
                connection_.socket = NetDll_socket(XNCALLER_SYSAPP, AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (connection_.socket == INVALID_SOCKET) {
                    error_msg = "Failed to create socket";
                    if (op.callback) op.callback(false, error_msg);
                    operation_in_progress_ = false;
                    return;
                }

                // Set socket options
                BOOL sockOpt = TRUE;
                NetDll_setsockopt(XNCALLER_SYSAPP, connection_.socket, SOL_SOCKET, 0x5801,
                    (const char*)&sockOpt, sizeof(sockOpt));

                DWORD sendRecvSize = 0x100000;
                NetDll_setsockopt(XNCALLER_SYSAPP, connection_.socket, SOL_SOCKET, SO_SNDBUF,
                    (const char*)&sendRecvSize, sizeof(sendRecvSize));
                NetDll_setsockopt(XNCALLER_SYSAPP, connection_.socket, SOL_SOCKET, SO_RCVBUF,
                    (const char*)&sendRecvSize, sizeof(sendRecvSize));

                DWORD timeout = 10000; // 10 seconds
                NetDll_setsockopt(XNCALLER_SYSAPP, connection_.socket, SOL_SOCKET, SO_RCVTIMEO,
                    (char*)&timeout, sizeof(timeout));


                // Setup server address
                connection_.server_addr.sin_family = AF_INET;

                ip_from_string(op.host.c_str(), connection_.server_addr.sin_addr);

                connection_.server_addr.sin_port = htons(op.port);

                // Connect with retries
                for (int i = 0; i < 5; i++) {
                    int result = NetDll_connect(XNCALLER_SYSAPP, connection_.socket,
                        (struct sockaddr*)&connection_.server_addr,
                        sizeof(sockaddr_in));

                    if (result != SOCKET_ERROR) {
                        success = true;
                        connection_.connected = true;
                        break;
                    }

                    if (i < 4) {
                        Sleep(1000); // Wait 1 second before retry
                    }
                }

                if (!success) {
                    error_msg = "Failed to connect after 5 attempts";
                    NetDll_closesocket(XNCALLER_SYSAPP, connection_.socket);
                    connection_.socket = INVALID_SOCKET;
                }

                if (op.callback) {
                    op.callback(success, success ? "Connected successfully" : error_msg);
                }

                operation_in_progress_ = false;
            }

            void handle_send(const AsyncOperation& op) {
                if (!connection_.connected || connection_.socket == INVALID_SOCKET) {
                    if (op.callback) {
                        op.callback(false, "Not connected");
                    }
                    operation_in_progress_ = false;
                    return;
                }

                int bytes_sent = NetDll_send(XNCALLER_SYSAPP, connection_.socket,
                    (const char*)op.send_data.data(),
                    op.send_data.size(), 0);

                bool success = (bytes_sent != SOCKET_ERROR);
                std::string msg = success ? "Send successful" : "Send failed";

                if (op.callback) {
                    op.callback(success, msg);
                }

                operation_in_progress_ = false;
            }

            void handle_receive(const AsyncOperation& op) {
                if (!connection_.connected || connection_.socket == INVALID_SOCKET) {
                    if (op.callback) {
                        op.callback(false, "Not connected");
                    }
                    operation_in_progress_ = false;
                    return;
                }

                std::vector<uint8_t> buffer(op.receive_size);
                int bytes_received = NetDll_recv(XNCALLER_SYSAPP, connection_.socket,
                    (char*)buffer.data(),
                    op.receive_size, 0);

                bool success = (bytes_received != SOCKET_ERROR && bytes_received > 0);

                if (success) {
                    buffer.resize(bytes_received);
                    if (op.callback) {
                        op.callback(true, "Receive successful");
                    }
                }
                else {
                    if (op.callback) {
                        op.callback(false, "Receive failed or connection closed");
                    }
                    connection_.connected = false;
                }

                operation_in_progress_ = false;
            }

            void handle_close(const AsyncOperation& op) {
                if (connection_.socket != INVALID_SOCKET) {
                    NetDll_closesocket(XNCALLER_SYSAPP, connection_.socket);
                    connection_.socket = INVALID_SOCKET;
                    connection_.connected = false;
                }

                if (op.callback) {
                    op.callback(true, "Connection closed");
                }

                operation_in_progress_ = false;
            }

        public:
            AsyncTcpClient() :
                operation_in_progress_(false),
                network_thread_(NULL),
                shutdown_event_(NULL) {

                connection_.socket = INVALID_SOCKET;
                connection_.connected = false;
                connection_.connecting = false;

                shutdown_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
                network_thread_ = CreateThread(NULL, 0, network_thread_proc, this, 0, NULL);
            }

            ~AsyncTcpClient() {
                // Signal shutdown
                SetEvent(shutdown_event_);

                // Wait for thread to exit
                if (network_thread_) {
                    WaitForSingleObject(network_thread_, INFINITE);
                    CloseHandle(network_thread_);
                }

                // Close connection if still open
                if (connection_.socket != INVALID_SOCKET) {
                    NetDll_closesocket(XNCALLER_SYSAPP, connection_.socket);
                }

                CloseHandle(shutdown_event_);
            }

            void connect_async(const std::string& host, uint16_t port,
                std::function<void(bool, const std::string&)> callback) {
                AsyncOperation op;
                op.type = AsyncOperation::CONNECT;
                op.callback = callback;
                op.host = host;
                op.port = port;

                operation_queue_.push(op);
            }

            void send_async(const std::vector<uint8_t>& data,
                std::function<void(bool, const std::string&)> callback) {
                AsyncOperation op;
                op.type = AsyncOperation::SEND;
                op.callback = callback;
                op.send_data = data;

                operation_queue_.push(op);
            }

            void receive_async(size_t size,
                std::function<void(bool, const std::string&)> callback) {
                AsyncOperation op;
                op.type = AsyncOperation::RECEIVE;
                op.callback = callback;
                op.receive_size = size;

                operation_queue_.push(op);
            }

            void close_async(std::function<void(bool, const std::string&)> callback = nullptr) {
                AsyncOperation op;
                op.type = AsyncOperation::CLOSE;
                op.callback = callback;

                operation_queue_.push(op);
            }

            bool is_connected() const {
                return connection_.connected;
            }

            SOCKET get_socket() const {
                return connection_.socket;
            }

            // Blocking versions for convenience
            bool connect(const std::string& host, uint16_t port, std::string* error_msg = nullptr) {
                bool success = false;
                std::string error;

                connect_async(host, port, [&](bool result, const std::string& msg) {
                    success = result;
                    error = msg;
                    });

                // Wait for completion
                while (!operation_queue_.empty() || operation_in_progress_) {
                    Sleep(10);
                }

                if (error_msg) *error_msg = error;
                return success;
            }

            bool send(const std::vector<uint8_t>& data, std::string* error_msg = nullptr) {
                bool success = false;
                std::string error;

                send_async(data, [&](bool result, const std::string& msg) {
                    success = result;
                    error = msg;
                    });

                // Wait for completion
                while (!operation_queue_.empty() || operation_in_progress_) {
                    Sleep(10);
                }

                if (error_msg) *error_msg = error;
                return success;
            }

            std::vector<uint8_t> receive(size_t size, bool* success = nullptr, std::string* error_msg = nullptr) {
                std::vector<uint8_t> result;
                bool op_success = false;
                std::string error;

                // This is a simplified version - you'd need to modify the callback to return actual data
                receive_async(size, [&](bool result, const std::string& msg) {
                    op_success = result;
                    error = msg;
                    });

                // Wait for completion
                while (!operation_queue_.empty() || operation_in_progress_) {
                    Sleep(10);
                }

                if (success) *success = op_success;
                if (error_msg) *error_msg = error;

                return result;
            }
        };
    }

    namespace string {
        class formatter {
        public:
            formatter() { }

            ~formatter() {  }

            const char* va(const char* format, ...) {
                std::lock_guard<std::mutex> lock(mutex_);

                va_list args;
                va_start(args, format);
                int result = vsnprintf(buffer, sizeof(buffer), format, args);
                va_end(args);

                if (result < 0 || result >= sizeof(buffer)) {
                    buffer[sizeof(buffer) - 1] = '\0';  // Ensure null-termination
                }

                return buffer;
            }

            const wchar_t* vaw(const char* format, ...) {
                std::lock_guard<std::mutex> lock(mutex_);

                va_list args;
                va_start(args, format);
                int result = vsnprintf(wide_buffer, sizeof(wide_buffer), format, args);
                va_end(args);

                if (result < 0 || result >= sizeof(wide_buffer)) {
                    wide_buffer[sizeof(wide_buffer) - 1] = '\0';  // Ensure null-termination
                }

                size_t converted = 0;
                mbstowcs_s(&converted, wbuffer, sizeof(wbuffer) / sizeof(wchar_t), wide_buffer, _TRUNCATE);

                return wbuffer;
            }

        private:
            char buffer[300];
            char wide_buffer[300];
            wchar_t wbuffer[300];
            std::mutex mutex_;

            // Private copy constructor and assignment operator
            formatter(const formatter&);
            formatter& operator=(const formatter&);
        };

        inline std::int32_t stoi(const std::string& string, std::size_t* index, std::int32_t base)
        {
            try
            {
                return std::stoi(string, index, base);
            }

            catch (...)
            {
                return 0;
            }
        }

        inline std::int32_t stol(const std::string& string, std::size_t* index, std::int32_t base)
        {
            try
            {
                return std::stol(string, index, base);
            }

            catch (...)
            {
                return 0;
            }
        }

        inline std::uint32_t stoul(const std::string& string, std::size_t* index, std::int32_t base)
        {
            try
            {
                return std::stoul(string, index, base);
            }

            catch (...)
            {
                return 0u;
            }
        }

        inline std::int64_t stoll(const std::string& string, std::size_t* index, std::int32_t base)
        {
            try
            {
                return std::stoll(string, index, base);
            }

            catch (...)
            {
                return 0i64;
            }
        }

        inline std::uint64_t stoull(const std::string& string, std::size_t* index, std::int32_t base)
        {
            try
            {
                return std::stoull(string, index, base);
            }

            catch (...)
            {
                return 0ui64;
            }
        }

        inline float stof(const std::string& string, std::size_t* index)
        {
            try
            {
                return std::stof(string, index);
            }

            catch (...)
            {
                return 0.0f;
            }
        }

        inline double stod(const std::string& string, std::size_t* index)
        {
            try
            {
                return std::stod(string, index);
            }

            catch (...)
            {
                return 0.0;
            }
        }

        inline double stold(const std::string& string, std::size_t* index)
        {
            try
            {
                return std::stold(string, index);
            }

            catch (...)
            {
                return 0.0;
            }
        }

        inline char* copy(std::uintptr_t dst, const std::string& src, std::size_t len)
        {
            if (len == std::string::npos)
            {
                return std::strcpy(std::recast<char*>(dst), src.c_str());
            }

            else
            {
                return std::strncpy(std::recast<char*>(dst), src.c_str(), len);
            }
        }

        inline char* copy(void* dst, const std::string& src, std::size_t len)
        {
            if (len == std::string::npos)
            {
                return std::strcpy(std::stcast<char*>(dst), src.c_str());
            }

            else
            {
                return std::strncpy(std::stcast<char*>(dst), src.c_str(), len);
            }
        }

        inline char* copy(char* dst, const std::string& src, std::size_t len)
        {
            if (len == std::string::npos)
            {
                return std::strcpy(dst, src.c_str());
            }

            else
            {
                return std::strncpy(dst, src.c_str(), len);
            }
        }

        inline std::string to_lower(const std::string& input)
        {
            std::string output;
            output.reserve(input.size());

            for (size_t i = 0; i < input.size(); ++i)
            {
                output.push_back(static_cast<char>(::tolower(static_cast<unsigned char>(input[i]))));
            }

            return output;
        }

        inline std::string to_upper(const std::string& input)
        {
            std::string output;
            output.reserve(input.size());

            for (size_t i = 0; i < input.size(); ++i)
            {
                output.push_back(static_cast<char>(::toupper(
                    static_cast<unsigned char>(input[i])
                )));
            }

            return output;
        }

        inline std::wstring to_wide(const std::string& input)
        {
            std::wstring output;
            output.reserve(input.size());

            for (size_t i = 0; i < input.size(); ++i)
            {
                wchar_t wc = ::btowc(static_cast<unsigned char>(input[i]));
                if (wc != WEOF)
                    output.push_back(wc);
            }

            return output;
        }

        inline std::string to_multi(const std::wstring& input)
        {
            std::string output;
            output.reserve(input.size());

            for (size_t i = 0; i < input.size(); ++i)
            {
                int c = ::wctob(input[i]);
                if (c != EOF)
                    output.push_back(static_cast<char>(c));
            }

            return output;
        }

        inline bool is_equal(const std::string& lhs, const std::string& rhs, const bool case_sensitive)
        {
            if (case_sensitive)
            {
                return lhs == rhs;
            }

            else
            {
                return to_lower(lhs) == to_lower(rhs);
            }
        }

        inline bool is_equal(const std::string& lhs, const std::string& rhs, const std::size_t len, const bool case_sensitive)
        {
            auto left = lhs;
            auto right = rhs;

            if (lhs.size() > len)
                left.erase(len);

            if (rhs.size() > len)
                right.erase(len);

            return is_equal(left, right, case_sensitive);
        }

        inline bool is_equal(const std::string& lhs, const std::vector<std::string>& rhs, const bool case_sensitive)
        {
            for each (auto & s in rhs)
            {
                if (is_equal(lhs, s, case_sensitive))
                {
                    return true;
                }
            }

            return false;
        }

        inline bool is_not_equal(const std::string& lhs, const std::string& rhs, const bool case_sensitive)
        {
            if (case_sensitive)
            {
                return lhs != rhs;
            }

            else
            {
                return to_lower(lhs) != to_lower(rhs);
            }
        }

        inline bool is_not_equal(const std::string& lhs, const std::string& rhs, const std::size_t len, const bool case_sensitive)
        {
            auto left = lhs;
            auto right = rhs;

            if (lhs.size() > len)
                left.erase(len);

            if (rhs.size() > len)
                right.erase(len);

            return is_not_equal(left, right, case_sensitive);
        }

        inline bool is_not_equal(const std::string& lhs, const std::vector<std::string>& rhs, const bool case_sensitive)
        {
            for each (auto & s in rhs)
            {
                if (is_not_equal(lhs, s, case_sensitive))
                {
                    return true;
                }
            }

            return false;
        }

        inline bool does_contain(const std::string& lhs, const std::string& rhs, const bool case_sensitive)
        {
            if (case_sensitive)
            {
                return lhs.find(rhs) != std::string::npos;
            }

            else
            {
                return to_lower(lhs).find(to_lower(rhs)) != std::string::npos;
            }
        }

        inline bool does_contain(const std::string& lhs, const std::vector<std::string>& rhs, const bool case_sensitive)
        {
            for each (auto & s in rhs)
            {
                if (does_contain(lhs, s, case_sensitive))
                {
                    return true;
                }
            }

            return false;
        }

        inline  bool does_not_contain(const std::string& lhs, const std::string& rhs, const bool case_sensitive)
        {
            if (case_sensitive)
            {
                return lhs.find(rhs) == std::string::npos;
            }

            else
            {
                return to_lower(lhs).find(to_lower(rhs)) == std::string::npos;
            }
        }

        inline bool does_not_contain(const std::string& lhs, const std::vector<std::string>& rhs, const bool case_sensitive)
        {
            for each (auto & s in rhs)
            {
                if (does_not_contain(lhs, s, case_sensitive))
                {
                    return true;
                }
            }

            return false;
        }

        inline std::string to_alternating_lower_first(std::string text)
        {
            auto lower = true;

            for (size_t i = 0; i < text.size(); i++)
            {
                if (::isalpha(text[i]))
                {
                    if (lower)
                    {
                        text[i] = ::tolower(text[i]);
                    }

                    else
                    {
                        text[i] = ::toupper(text[i]);
                    }

                    lower = !lower;
                }
            }

            return text;
        }

        inline std::string to_alternating_upper_first(std::string text)
        {
            auto upper = true;

            for (size_t i = 0; i < text.size(); i++)
            {
                if (::isalpha(text[i]))
                {
                    if (upper)
                    {
                        text[i] = ::toupper(text[i]);
                    }

                    else
                    {
                        text[i] = ::tolower(text[i]);
                    }

                    upper = !upper;
                }
            }

            return text;
        }

        inline std::string random(std::size_t size)
        {
            static char character_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";

            static std::random_device random_device;
            static std::uniform_int_distribution<> random_alpha_numeric_char(0, array_size(character_set) - 1);
            static std::uniform_int_distribution<> random_length(5, 15);

            std::string buf;

            if (size == 0u)
                size = random_length(random_device);

            for (int i = 0; i < size; i++)
                buf += character_set[random_alpha_numeric_char(random_device)];

            return buf;
        }

        inline std::string find_and_replace(std::string text, const std::string& find, const std::string& replace)
        {
            std::size_t pos;

            while ((pos = to_lower(text).find(to_lower(find))) != std::string::npos)
            {
                text.replace(pos, find.size(), replace);
                pos += replace.size();
            }

            return text;
        }

        inline std::string find_and_erase(std::string text, const std::string& find)
        {
            std::size_t pos;

            while ((pos = to_lower(text).find(to_lower(find))) != std::string::npos)
            {
                text.erase(pos, find.size());
            }

            return text;
        }

        inline std::string ensure_alpha_only(const std::string& text)
        {
            std::string buf;



            for (auto i = text.begin(); ::isalpha(*i) && i != text.end(); i++)
            {
                buf += *i;
            }

            return buf;
        }

        inline std::string ensure_numeric_only(const std::string& text)
        {
            std::string buf;

            for (auto i = text.begin(); ::isdigit(*i) && i != text.end(); i++)
            {
                buf += *i;
            }

            return buf;
        }

        inline std::string ensure_alpha_numeric_only(const std::string& text)
        {
            std::string buf;

            for (auto i = text.begin(); ::isalnum(*i) && i != text.end(); i++)
            {
                buf += *i;
            }

            return buf;
        }

        inline std::string ensure_non_spaces_only(const std::string& text)
        {
            std::string buf;

            for (auto i = text.begin(); !::isspace(*i) && i != text.end(); i++)
            {
                buf += *i;
            }

            return buf;
        }

        inline std::vector<std::string> tokenize(const std::string& text, char delimiter)
        {
            std::vector<std::string> buf;
            std::stringstream ss(text);
            std::string item;

            while (std::getline(ss, item, delimiter))
            {
                buf.push_back(item);
            }

            return buf;
        }

        inline std::string concatenate(const std::vector<std::string>& strings, char delimiter, std::size_t index)
        {
            std::string buf;

            for (auto i = index; i < strings.size(); i++)
            {
                buf += strings[i];
                buf += delimiter;
            }

            return buf;
        }

        inline const char** vector_to_pointer_array(const std::vector<std::string>& strings)
        {
            std::vector<const char*> buf;
            buf.clear();

            for each (auto & str in strings)
            {
                buf.push_back(str.c_str());
            }

            return buf.data();
        }

        inline std::vector<std::string> pointer_array_to_vector(char** const& strings, std::size_t count)
        {
            std::vector<std::string> buf;
            buf.clear();

            for (auto i = 0u; i < count; i++)
            {
                if (strings[i])
                {
                    buf.push_back(strings[i]);
                }
            }

            return buf;
        }

        void split(const std::string& s, char c, std::vector<std::string>& v) {
            std::string::size_type i = 0;
            std::string::size_type j = s.find(c);

            while (j != std::string::npos) {
                v.push_back(s.substr(i, j - i));
                i = ++j;
                j = s.find(c, j);

                if (j == std::string::npos)
                    v.push_back(s.substr(i, s.length()));
            }
        }
    }

    namespace memory {
        template<typename type>
        void write(std::uintptr_t address, type value)
        {
            *std::recast<type*>(address) = value;
        }

        template<typename type>
        void write(void* address, type value)
        {
            *std::stcast<type*>(address) = value;
        }

        template <typename type>
        inline void set(std::uintptr_t place, type value)
        {
            return write<type>(std::recast<void*>(place), value);
        }
    }

    namespace call {
        template<typename T>
        static T invoke(uint32_t address) { return ((T(*)())address)(); }

        template<typename T, typename P1>
        static T invoke(uint32_t address, P1 p1) { return ((T(*)(P1))address)(p1); }

        template<typename T, typename P1, typename P2>
        static T invoke(uint32_t address, P1 p1, P2 p2) { return ((T(*)(P1, P2))address)(p1, p2); }

        template<typename T, typename P1, typename P2, typename P3>
        static T invoke(uint32_t address, P1 p1, P2 p2, P3 p3) { return ((T(*)(P1, P2, P3))address)(p1, p2, p3); }

        template<typename T, typename P1, typename P2, typename P3, typename P4>
        static T invoke(uint32_t address, P1 p1, P2 p2, P3 p3, P4 p4) { return ((T(*)(P1, P2, P3, P4))address)(p1, p2, p3, p4); }

        template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
        static T invoke(uint32_t address, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) { return ((T(*)(P1, P2, P3, P4, P5))address)(p1, p2, p3, p4, p5); }

        template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
        static T invoke(uint32_t address, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) { return ((T(*)(P1, P2, P3, P4, P5, P6))address)(p1, p2, p3, p4, p5, p6); }

        template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
        static T invoke(uint32_t address, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) { return ((T(*)(P1, P2, P3, P4, P5, P6, P7))address)(p1, p2, p3, p4, p5, p6, p7); }

        template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
        static T invoke(uint32_t address, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) { return ((T(*)(P1, P2, P3, P4, P5, P6, P7, P8))address)(p1, p2, p3, p4, p5, p6, p7, p8); }

        template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
        static T invoke(uint32_t address, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9) { return ((T(*)(P1, P2, P3, P4, P5, P6, P7, P8, P9))address)(p1, p2, p3, p4, p5, p6, p7, p8, p9); }

        template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
        static T invoke(uint32_t address, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10) { return ((T(*)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10))address)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); }

    }

    namespace math {
        template<typename T>
        struct vector3_;

        template<typename T>
        struct vector2 {
            T m_X;
            T m_Y;

            vector2<T>() {}

            vector2<T>(T in_x, T in_y)
                : m_X(in_x), m_Y(in_y) {
            }

            vector2<T> operator*(float value) {
                return vector2<T>(m_X * value, m_Y * value);
            }

            vector2<T> operator*=(float value) {
                return vector2<T>(m_X * value, m_Y * value);
            }

            vector2<T> operator*(vector2<T>& value) {
                return vector2<T>(m_X * value.m_X, m_Y * value.m_Y);
            }

            vector2<T> operator+=(vector2<T>& value) {
                m_X += value.m_X;
                m_Y += value.m_Y;
                return vector2<T>(m_X, m_Y);
            }

            vector2<T> operator-(float value) {
                return vector2<T>(m_X - value, m_Y - value);
            }

            vector2<T> operator-=(float value) {
                return vector2<T>(m_X - value, m_Y - value);
            }

            vector2<T> operator-(vector2<T>& value) {
                return vector2<T>(m_X - value.m_X, m_Y - value.m_Y);
            }

            vector2<T> operator+(float value) {
                return vector2<T>(m_X + value, m_Y + value);
            }

            vector2<T> operator+=(float value) {
                return vector2<T>(m_X + value, m_Y + value);
            }

            vector2<T> operator+(vector2<T>& value) {
                return vector2<T>(m_X + value.m_X, m_Y + value.m_Y);
            }

            vector2<T> operator/(float value) {
                if (value == 0.f || m_X == 0 || m_Y == 0) return vector2<T>(0, 0);
                return vector2<T>(m_X / value, m_Y / value);
            }

            vector2<T> operator/=(float value) {
                if (value == 0.f || m_X == 0 || m_Y == 0) return vector2<T>(0, 0);
                return vector2<T>(m_X / value, m_Y / value);
            }

            vector2<T> operator/(vector2<T>& value) {
                return vector2<T>(m_X / value.m_X, m_Y / value.m_Y);
            }

            bool operator==(vector2<T>& value) {
                return m_X == value.m_X && m_Y == value.m_Y;
            }

            float GetMathmaticalDistance(vector2<T>& value) {
                /*float a = value.m_X - m_X;
                float b = value.m_Y - m_Y;
                return (float)sqrt((a * a) + (b * b));*/

                return ((float)sqrt(pow(value.m_X - m_X, 2) + pow(value.m_Y - m_Y, 2) * 1.0));
            }

            float GetLength() {
                return (float)sqrt((m_X * m_X) + (m_Y * m_Y));
            }

            void Normalize() {
                float Length = GetLength();
                m_X /= Length;
                m_Y /= Length;
            }
        };

        template<typename T>
        struct vector3 {
            T m_X;
            DWORD AAAAA;
            T m_Y;
            DWORD BBBBB;
            T m_Z;
            DWORD CCCCC;

            vector3<T>() {}

            vector3<T>(T in_x, T in_y, T in_z)
                : m_X(in_x), m_Y(in_y), m_Z(in_z) {
            }

            vector3<T> operator*(float value) {
                return vector3<T>(m_X * value, m_Y * value, m_Z * value);
            }

            vector3<T> operator*=(float value) {
                return vector3<T>(m_X * value, m_Y * value, m_Z * value);
            }

            vector3<T> operator*(vector3<T>& value) {
                return vector3<T>(m_X * value.m_X, m_Y * value.m_Y, m_Z * value.m_Z);
            }

            vector3<T> operator+=(vector3<T>& value) {
                return vector3<T>(m_X + value.m_X, m_Y + value.m_Y, m_Z + value.m_Z);
            }

            vector3<T> operator-(float value) {
                return vector3<T>(m_X - value, m_Y - value, m_Z - value);
            }

            vector3<T> operator-=(float value) {
                return vector3<T>(m_X - value, m_Y - value, m_Z - value);
            }

            vector3<T> operator-(vector3<T>& value) {
                return vector3<T>(m_X - value.m_X, m_Y - value.m_Y, m_Z - value.m_Z);
            }

            vector3<T> operator+(float value) {
                return vector3<T>(m_X + value, m_Y + value, m_Z + value);
            }

            vector3<T> operator+=(float value) {
                return vector3<T>(m_X + value, m_Y + value, m_Z + value);
            }

            vector3<T> operator-=(vector3<T>& value) {
                return vector3<T>(m_X - value.m_X, m_Y - value.m_Y, m_Z - value.m_Z);
            }

            vector3<T> operator+(vector3<T>& value) {
                return vector3<T>(m_X + value.m_X, m_Y + value.m_Y, m_Z + value.m_Z);
            }

            vector3<T> operator/(float value) {
                if (value == 0.f || m_X == 0 || m_Y == 0 || m_Z == 0) return vector3<T>(0, 0);
                return vector3<T>(m_X / value, m_Y / value, m_Z / value);
            }

            vector3<T> operator/=(float value) {
                if (value == 0.f || m_X == 0 || m_Y == 0) return vector3<T>(0, 0);
                return vector3<T>(m_X / value, m_Y / value, m_Z / value);
            }

            vector3<T> operator/(vector3<T>& value) {
                return vector3<T>(m_X / value.m_X, m_Y / value.m_Y, m_Z / value.m_Z);
            }

            bool operator==(vector3<T>& value) {
                return m_X == value.m_X && m_Y == value.m_Y && m_Z == value.m_Z;
            }

            float GetMathmaticalDistance(vector3<T>& value) {
                /*float a = value.m_X - m_X;
                float b = value.m_Y - m_Y;
                float c = value.m_Z - m_Z;
                return (float)sqrt((a * a) + (b * b) + (c * c));*/

                return ((float)sqrt(pow(value.m_X - m_X, 2) + pow(value.m_Y - m_Y, 2) * 1.0));
            }

            bool IsZero() {
                return (m_X == fabs(0.0f) && m_Y == fabs(0.0f) && m_Z == fabs(0.0f));
            }

            float Get2DLength() {
                return (float)sqrt((m_X * m_X) + (m_Z * m_Z));
            }

            float GetLength() {
                return (float)sqrt((m_X * m_X) + (m_Y * m_Y) + (m_Z * m_Z));
            }

            void Normalize() {
                float Length = GetLength();
                m_X /= Length;
                m_Y /= Length;
                m_Z /= Length;
            }

            static vector3_<T> ToSerialized(vector3<T> value) {
                return vector3_<T>(value.m_X, value.m_Y, value.m_Z);
            }

            float Dot(vector3<T> value) {
                return (m_X * m_X + m_Y * m_Y + m_Z * m_Z);
            }
        };

        template<typename T>
        struct vector3_ {
            T m_X;
            T m_Y;
            T m_Z;

            vector3_<T>() {}

            vector3_<T>(T in_x, T in_y, T in_z)
                : m_X(in_x), m_Y(in_y), m_Z(in_z) {
            }

            vector3_<T> operator*(float value) {
                return vector3_<T>(m_X * value, m_Y * value, m_Z * value);
            }

            vector3_<T> operator*=(float value) {
                return vector3_<T>(m_X * value, m_Y * value, m_Z * value);
            }

            vector3_<T> operator*(vector3_<T>& value) {
                return vector3_<T>(m_X * value.m_X, m_Y * value.m_Y, m_Z * value.m_Z);
            }

            vector3_<T> operator-(float value) {
                return vector3_<T>(m_X - value, m_Y - value, m_Z - value);
            }

            vector3_<T> operator-(vector3_<T> value) {
                return vector3_<T>(m_X - value.m_X, m_Y - value.m_Y, m_Z - value.m_Z);
            }

            vector3_<T> operator-=(float value) {
                return vector3_<T>(m_X - value, m_Y - value, m_Z - value);
            }

            vector3_<T> operator-(vector3_<T>& value) {
                return vector3_<T>(m_X - value.m_X, m_Y - value.m_Y, m_Z - value.m_Z);
            }

            vector3_<T> operator+(float value) {
                return vector3_<T>(m_X + value, m_Y + value, m_Z + value);
            }

            vector3_<T> operator+=(float value) {
                return vector3_<T>(m_X + value, m_Y + value, m_Z + value);
            }

            vector3_<T> operator+=(vector3_<T> value) {
                return vector3_<T>(m_X + value.m_X, m_Y + value.m_Y, m_Z + value.m_Z);
            }

            vector3_<T> operator+(vector3_<T>& value) {
                return vector3_<T>(m_X + value.m_X, m_Y + value.m_Y, m_Z + value.m_Z);
            }

            vector3_<T> operator/(float value) {
                if (value == 0.f || m_X == 0 || m_Y == 0 || m_Z == 0) return vector3_<T>(0, 0);
                return vector3_<T>(m_X / value, m_Y / value, m_Z / value);
            }

            vector3_<T> operator/=(float value) {
                if (value == 0.f || m_X == 0 || m_Y == 0) return vector3_<T>(0, 0);
                return vector3_<T>(m_X / value, m_Y / value, m_Z / value);
            }

            vector3_<T> operator/(vector3_<T>& value) {
                return vector3_<T>(m_X / value.m_X, m_Y / value.m_Y, m_Z / value.m_Z);
            }

            bool operator==(vector3_<T>& value) {
                return m_X == value.m_X && m_Y == value.m_Y && m_Z == value.m_Z;
            }

            float GetMathmaticalDistance(vector3_<T>& value) {
                return ((float)sqrt(pow(value.m_X - m_X, 2) + pow(value.m_Y - m_Y, 2) * 1.0));
            }

            static vector3<T> ToPadded(vector3_<T> value) {
                return vector3<T>(value.m_X, value.m_Y, value.m_Z);
            }

            float GetLength() {
                return (float)sqrt((m_X * m_X) + (m_Y * m_Y) + (m_Z * m_Z));
            }

            void Normalize() {
                float Length = GetLength();
                m_X /= Length;
                m_Y /= Length;
                m_Z /= Length;
            }
        };

        template<typename T>
        struct vector4 {
            T m_X;
            T m_Y;
            T m_Z;
            T m_W;
        };

        struct Matrix {
            union {
                struct {
                    vector4<float> m_Left;
                    vector4<float> m_Up;
                    vector4<float> m_Forward;
                    vector4<float> m_Translation;
                };

                float m_Elements[4][4];
            };

            Matrix() {}

            float& operator () (int Row, int Col) {
                return m_Elements[Row][Col];
            }
        };

        template<typename T>
        bool within(T val, T min, T max) {
            return val <= max && val >= min;
        }

        template<typename T>
        T clamp(T val, T min, T max) {
            return val < min ? min : val > max ? max : val;
        }

        inline float lerp(float toEase, float easeFrom, float percent) {
            return (toEase + percent * (easeFrom - toEase));
        }

        inline void ease(float& toEase, float& easeFrom, float multiplier) {
            toEase += toEase < easeFrom ? abs(toEase - easeFrom) / multiplier : -abs(toEase - easeFrom) / multiplier;
        }

        inline float repeat(float t, float length) {
            return clamp(t - floor(t / length) * length, 0.f, length);
        }

        inline float pingpong(float t, float length) {
            t = repeat(t, length * 2.f);
            return length - fabs(t - length);
        }
    }

    namespace util {
        void RunTimedFunction(int* Timer, int Ms, std::function<void()> Callback) {
            if (*Timer < GetTickCount()) {
                *Timer = GetTickCount() + Ms;
                Callback();
            }
        }
    }

    namespace xam {
        DWORD ResolveFunction(const char* ModuleName, DWORD dwOrdinal) {
            HANDLE hProc = 0;
            HANDLE hModule = 0;

            if (!NT_SUCCESS(XexGetModuleHandle((char*)ModuleName, &hModule))) {
                return 0;
            }

            if (!NT_SUCCESS(XexGetProcedureAddress(hModule, dwOrdinal, &hProc))) {
                return 0;
            }

            return reinterpret_cast<DWORD>(hProc);
        }
    }
}
