#ifndef ZENOH_ROS_PC_H
#define ZENOH_ROS_PC_H

#include <zenoh.h>
#include <cstdint>
#include <functional>
#include <thread>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "z_logger.h"


// Global flag to handle clean shutdown via Ctrl+C
inline volatile sig_atomic_t shutdown_requested = 0;

inline void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n[Signal] Shutdown requested via SIGINT.\n";
        shutdown_requested = 1;
    }
}

// QoS classes
enum class Reliability : std::uint8_t {
    RELIABLE,
    BEST_EFFORT
};

enum class Durability : std::uint8_t {
    VOLATILE,
    TRANSIENT_LOCAL
};

struct QoS {
    Reliability reliability = Reliability::RELIABLE;
    Durability durability = Durability::VOLATILE;
    uint32_t depth = 10;
};

struct SensorDataQoS : public QoS {
    SensorDataQoS() {
        reliability = Reliability::BEST_EFFORT;
        durability = Durability::VOLATILE;
        depth = 5;
    }
};

struct SystemDefaultsQoS : public QoS {
    SystemDefaultsQoS() {
        reliability = Reliability::RELIABLE;
        durability = Durability::VOLATILE;
        depth = 10;
    }
};


template <typename T>
std::vector<uint8_t> serialize_msg(const T& msg);

template <typename T>
void deserialize_msg(const std::vector<uint8_t>& buffer, T& msg);

// --- ROS 2 Time / Clock Abstraction for PC ---
struct ZenohTime {
    int32_t sec = 0;
    uint32_t nanosec = 0;
};

class ZenohClock {
public:
    ZenohTime now() const {
        ZenohTime t;
        auto now_p = std::chrono::system_clock::now();
        auto duration = now_p.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
        t.sec = static_cast<int32_t>(seconds.count());
        t.nanosec = static_cast<uint32_t>(nanoseconds.count());
        return t;
    }
};

// Callback types
typedef std::function<void()> TimerCallback;

template <typename MsgType>
using SubscriptionCallback = std::function<void(const MsgType& msg)>;

// --- Thread-Safety Mutex Helper for PC C++ ---
struct ZenohSessionMutexPC {
    static std::mutex mutex;

    static inline void lock() {
        mutex.lock();
    }

    static inline void unlock() {
        mutex.unlock();
    }
};

// Templated Zenoh Publisher Class for PC
template <typename MsgType>
class ZenohPublisher {
private:
    z_owned_publisher_t pub;
    const char* topic;
    bool declared;
    QoS qos_profile;

public:
    ZenohPublisher(const char* topic_name, const QoS& qos = QoS())
        : topic(topic_name), declared(false), qos_profile(qos) {}

    ~ZenohPublisher() {
        if (declared) {
            z_undeclare_publisher(z_publisher_move(&pub));
        }
    }

    bool declare(const z_loaned_session_t* z_session) {
        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, topic);

        z_publisher_options_t options;
        z_publisher_options_default(&options);

        // Apply reliability QoS mapping
        if (qos_profile.reliability == Reliability::BEST_EFFORT) {
            options.congestion_control = Z_CONGESTION_CONTROL_DROP;
        } else {
            options.congestion_control = Z_CONGESTION_CONTROL_BLOCK;
        }

        if (z_declare_publisher(z_session, &pub, z_view_keyexpr_loan(&keyexpr), &options) < 0) {
            std::cerr << "[Zenoh PC] ERROR: Unable to declare publisher on '" << topic << "'\n";
            return false;
        }

        std::cout << "[Zenoh PC] Publisher ready on '" << topic 
                  << "' with QoS Reliability: " 
                  << (qos_profile.reliability == Reliability::BEST_EFFORT ? "BEST_EFFORT" : "RELIABLE") 
                  << '\n';
        declared = true;
        return true;
    }

    bool publish(const MsgType& msg) {
        std::vector<uint8_t> buffer = serialize_msg<MsgType>(msg);

        z_publisher_put_options_t options;
        z_publisher_put_options_default(&options);

        z_owned_bytes_t bytes;
        z_bytes_copy_from_buf(&bytes, buffer.data(), buffer.size());

        ZenohSessionMutexPC::lock();
        int res = z_publisher_put(z_publisher_loan(&pub), z_bytes_move(&bytes), &options);
        ZenohSessionMutexPC::unlock();

        return res == 0;
    }
};

// Templated Zenoh Subscription Class for PC
template <typename MsgType>
class ZenohSubscription {
private:
    z_owned_subscriber_t sub;
    bool declared;
    SubscriptionCallback<MsgType> callback;
    QoS qos_profile;

public:
    ZenohSubscription(const QoS& qos = QoS()) : declared(false), qos_profile(qos) {}

    ~ZenohSubscription() {
        if (declared) {
            z_undeclare_subscriber(z_subscriber_move(&sub));
        }
    }

    bool declare(const z_loaned_session_t* z_session, const char* topic, SubscriptionCallback<MsgType> cb) {
        callback = std::move(cb);

        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, topic);

        z_owned_closure_sample_t closure;
        z_closure_sample(
            &closure,
            [](z_loaned_sample_t* sample, void* context) {
                ZenohSubscription* self = (ZenohSubscription*)context;
                if (self && self->callback) {
                    const z_loaned_bytes_t* payload = z_sample_payload(sample);
                    z_owned_slice_t slice;
                    z_bytes_to_slice(payload, &slice);

                    const uint8_t* data = z_slice_data(z_slice_loan(&slice));
                    size_t len = z_slice_len(z_slice_loan(&slice));

                    std::vector<uint8_t> buffer(data, data + len);

                    MsgType msg;
                    deserialize_msg<MsgType>(buffer, msg);
                    self->callback(msg);

                    // slice cleanup handled by zenoh-c sample lifecycle
                }
            },
            NULL,
            this
        );

        if (z_declare_subscriber(z_session, &sub, z_view_keyexpr_loan(&keyexpr), z_closure_sample_move(&closure), NULL) < 0) {
            std::cerr << "[Zenoh PC] ERROR: Unable to declare subscriber on '" << topic << "'\n";
            return false;
        }

        std::cout << "[Zenoh PC] Subscription ready on '" << topic 
                  << "' with QoS Reliability: " 
                  << (qos_profile.reliability == Reliability::BEST_EFFORT ? "BEST_EFFORT" : "RELIABLE") 
                  << '\n';
        declared = true;
        return true;
    }
};

// Templated Zenoh Service Server Class for PC
template <typename SrvType>
class ZenohService {
public:
    using ServiceCallback = std::function<void(const typename SrvType::Request& req, typename SrvType::Response& res)>;

private:
    z_owned_queryable_t queryable;
    bool declared;
    ServiceCallback callback;

public:
    ZenohService() : declared(false) {}

    ~ZenohService() {
        if (declared) {
            z_undeclare_queryable(z_queryable_move(&queryable));
        }
    }

    bool declare(const z_loaned_session_t* z_session, const char* service_name, ServiceCallback cb) {
        callback = std::move(cb);

        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, service_name);

        z_owned_closure_query_t closure;
        z_closure_query(
            &closure,
            [](z_loaned_query_t* query, void* context) {
                ZenohService* self = (ZenohService*)context;
                if (self && self->callback) {
                    const z_loaned_bytes_t* payload = z_query_payload(query);
                    std::vector<uint8_t> buffer;
                    if (payload) {
                        z_owned_slice_t slice;
                        z_bytes_to_slice(payload, &slice);
                        const uint8_t* data = z_slice_data(z_slice_loan(&slice));
                        size_t len = z_slice_len(z_slice_loan(&slice));
                        buffer.assign(data, data + len);
                        // slice cleanup handled by zenoh-c sample lifecycle
                    }

                    typename SrvType::Request req;
                    if (!buffer.empty()) {
                        deserialize_msg<typename SrvType::Request>(buffer, req);
                    }

                    typename SrvType::Response res;
                    self->callback(req, res);

                    std::vector<uint8_t> reply_buf = serialize_msg<typename SrvType::Response>(res);

                    z_query_reply_options_t options;
                    z_query_reply_options_default(&options);

                    z_owned_bytes_t reply_bytes;
                    z_bytes_copy_from_buf(&reply_bytes, reply_buf.data(), reply_buf.size());

                    const z_loaned_keyexpr_t* q_key = z_query_keyexpr(query);

                    ZenohSessionMutexPC::lock();
                    z_query_reply(query, q_key, z_bytes_move(&reply_bytes), &options);
                    ZenohSessionMutexPC::unlock();
                }
            },
            NULL,
            this
        );

        if (z_declare_queryable(z_session, &queryable, z_view_keyexpr_loan(&keyexpr), z_closure_query_move(&closure), NULL) < 0) {
            std::cerr << "[Zenoh PC] ERROR: Unable to declare service queryable on '" << service_name << "'\n";
            return false;
        }

        std::cout << "[Zenoh PC] Service ready on '" << service_name << "'\n";
        declared = true;
        return true;
    }
};

// Templated Zenoh Service Client Class for PC
template <typename SrvType>
class ZenohClient {
private:
    const z_loaned_session_t* session;
    const char* service_name;
    bool ready;

public:
    ZenohClient(const char* name) : session(nullptr), service_name(name), ready(false) {}

    void set_session(const z_loaned_session_t* z_session) {
        session = z_session;
        ready = (session != nullptr);
    }

    bool wait_for_service(uint32_t timeout_ms = 5000) {
        if (!session) return false;
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() < timeout_ms) {
            if (shutdown_requested) return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return true;
        }
        return false;
    }

    bool call(const typename SrvType::Request& req, typename SrvType::Response& res, uint32_t timeout_ms = 5000) {
        if (shutdown_requested || !session) return false;

        std::vector<uint8_t> req_buf = serialize_msg<typename SrvType::Request>(req);

        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, service_name);

        z_get_options_t options;
        z_get_options_default(&options);
        options.timeout_ms = timeout_ms;

        z_owned_bytes_t req_bytes;
        z_bytes_copy_from_buf(&req_bytes, req_buf.data(), req_buf.size());
        options.payload = z_bytes_move(&req_bytes);

        struct ReplyContext {
            std::mutex mtx;
            std::condition_variable cv;
            std::vector<uint8_t> reply_data;
            bool received = false;
        };
        auto ctx = std::make_shared<ReplyContext>();

        z_owned_closure_reply_t closure;
        z_closure_reply(
            &closure,
            [](z_loaned_reply_t* reply, void* context) {
                auto c = static_cast<std::shared_ptr<ReplyContext>*>(context);
                if (c && *c && z_reply_is_ok(reply)) {
                    const z_loaned_sample_t* sample = z_reply_ok(reply);
                    const z_loaned_bytes_t* payload = z_sample_payload(sample);
                    if (payload) {
                        z_owned_slice_t slice;
                        z_bytes_to_slice(payload, &slice);
                        const uint8_t* data = z_slice_data(z_slice_loan(&slice));
                        size_t len = z_slice_len(z_slice_loan(&slice));
                        std::lock_guard<std::mutex> lock((*c)->mtx);
                        (*c)->reply_data.assign(data, data + len);
                        (*c)->received = true;
                        (*c)->cv.notify_one();
                    }
                }
            },
            [](void* context) {
                delete static_cast<std::shared_ptr<ReplyContext>*>(context);
            },
            new std::shared_ptr<ReplyContext>(ctx)
        );

        ZenohSessionMutexPC::lock();
        int get_res = z_get(session, z_view_keyexpr_loan(&keyexpr), "", z_closure_reply_move(&closure), &options);
        ZenohSessionMutexPC::unlock();

        if (get_res < 0) {
            if (!shutdown_requested) {
                std::cerr << "[ZenohClient PC] ERROR: Failed to send service call to '" << service_name << "'\n";
            }
            return false;
        }

        std::unique_lock<std::mutex> lock(ctx->mtx);
        if (ctx->cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [&]() { return ctx->received || shutdown_requested; })) {
            if (shutdown_requested) return false;
            if (!ctx->reply_data.empty()) {
                deserialize_msg<typename SrvType::Response>(ctx->reply_data, res);
                return true;
            }
        }

        if (!shutdown_requested) {
            std::cerr << "[ZenohClient PC] ERROR: Service call to '" << service_name << "' timed out or returned empty!\n";
        }
        return false;
    }
};

// Zenoh Timer Class for PC
class ZenohTimer {
private:
    TimerCallback callback;
    uint32_t period_ms;
    std::thread timer_thread;
    bool running;

    void run() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(period_ms));
            if (running) {
                try {
                    callback();
                } catch (const std::exception& e) {
                    std::cerr << "[ZenohTimer PC] Exception in callback: " << e.what() << '\n';
                }
            }
        }
    }

public:
    ZenohTimer(uint32_t period, TimerCallback cb)
        : callback(std::move(cb)), period_ms(period), running(true) {
        timer_thread = std::thread(&ZenohTimer::run, this);
    }

    ~ZenohTimer() {
        running = false;
        if (timer_thread.joinable()) {
            timer_thread.join();
        }
    }
};

// Configuration structure for the Zenoh node on PC
struct ZenohConfig {
    enum class CommunicationMode {
        ZENOH_COMM_UART_DEFAULT = 0,
        ZENOH_COMM_UART_USB_CDC = 1,
        ZENOH_COMM_UART_HW      = 2,
        ZENOH_COMM_WIFI         = 3
    };

    enum class BaudRate : uint32_t {
        UART_STANDARD   = 115200,
        UART_HIGH_SPEED = 921600,
        USB_STANDARD    = 3000000,
        USB_HIGH_SPEED  = 12000000
    };

    static constexpr CommunicationMode ZENOH_COMM_WIFI         = CommunicationMode::ZENOH_COMM_WIFI;
    static constexpr CommunicationMode ZENOH_COMM_UART_DEFAULT = CommunicationMode::ZENOH_COMM_UART_DEFAULT;
    static constexpr CommunicationMode ZENOH_COMM_UART_USB_CDC = CommunicationMode::ZENOH_COMM_UART_USB_CDC;
    static constexpr CommunicationMode ZENOH_COMM_UART_HW      = CommunicationMode::ZENOH_COMM_UART_HW;

    static constexpr const char* MODE_WIFI   = "wifi";
    static constexpr const char* MODE_SERIAL = "serial";

    static constexpr uint32_t UART_STANDARD   = 115200;
    static constexpr uint32_t UART_HIGH_SPEED = 921600;
    static constexpr uint32_t USB_STANDARD    = 3000000;
    static constexpr uint32_t USB_HIGH_SPEED  = 12000000;

    CommunicationMode communication_mode = CommunicationMode::ZENOH_COMM_WIFI;
    std::string mode = "wifi";
    std::string transport = "wifi";
    std::string uart_port = "auto";
    uint32_t    baudrate  = UART_STANDARD;

    // Wi-Fi / TCP Parameters
    std::string host = "192.168.4.1";
    uint16_t    port = 7447;
    std::string connect_endpoint = "";
};

// Main Zenoh Node Class for PC
class ZenohNode {
private:
    static z_owned_session_t session;
    static z_owned_liveliness_token_t liveliness_token;
    static bool session_opened;
    const char* node_name;
    std::vector<std::function<void()>> cleanup_callbacks;
    ZenohClock node_clock;
    std::vector<std::pair<std::string, std::string>> parameters;
    std::shared_ptr<ZLoggerPC> logger_;

public:
    ZenohNode(const char* name) : node_name(name), logger_(z_get_logger(name)) {}

    std::shared_ptr<ZLoggerPC> get_logger() {
        return logger_;
    }

    static bool init_session(const char* connect_endpoint) {
        if (session_opened) return true;

        // Register signal handler for Ctrl+C
        std::signal(SIGINT, signal_handler);

        // Register automatic session shutdown upon exit
        std::atexit(ZenohNode::shutdown);

        z_owned_config_t z_config;
        z_config_default(&z_config);

        if (connect_endpoint && std::strlen(connect_endpoint) > 0) {
            std::string endpoint_json = "[\"" + std::string(connect_endpoint) + "\"]";
            zc_config_insert_json5(z_config_loan_mut(&z_config), Z_CONFIG_CONNECT_KEY, endpoint_json.c_str());
            std::cout << "[Zenoh PC] Connecting to endpoint: " << connect_endpoint << "\n";
        } else {
            std::cout << "[Zenoh PC] Scouting for peers...\n";
        }

        if (z_open(&session, z_config_move(&z_config), NULL) < 0) {
            std::cerr << "[Zenoh PC] ERROR: Unable to open Zenoh session!\n";
            return false;
        }

        std::cout << "[Zenoh PC] Session opened successfully!\n";
        session_opened = true;

        z_view_keyexpr_t live_key;
        z_view_keyexpr_from_str(&live_key, "@ros2/pc_node/liveliness");
        z_liveliness_declare_token(z_session_loan(&session), &liveliness_token, z_view_keyexpr_loan(&live_key), NULL);

        return true;
    }



    ~ZenohNode() {
        for (auto& cleanup : cleanup_callbacks) {
            cleanup();
        }
    }

    void z_delay(uint32_t ms) const {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void z_sleep_ms(uint32_t ms) const {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    const ZenohClock* get_clock() const {
        return &node_clock;
    }

    ZenohTime now() const {
        return node_clock.now();
    }

    void z_declare_parameter(const std::string& name, int default_val) {
        parameters.push_back({name, std::to_string(default_val)});
    }

    void z_declare_parameter(const std::string& name, float default_val) {
        parameters.push_back({name, std::to_string(default_val)});
    }

    void z_declare_parameter(const std::string& name, const std::string& default_val) {
        parameters.push_back({name, default_val});
    }

    int z_get_parameter(const std::string& name, int default_val) const {
        for (const auto& p : parameters) {
            if (p.first == name) return std::stoi(p.second);
        }
        return default_val;
    }

    float z_get_parameter(const std::string& name, float default_val) const {
        for (const auto& p : parameters) {
            if (p.first == name) return std::stof(p.second);
        }
        return default_val;
    }

    std::string z_get_parameter(const std::string& name, const std::string& default_val) const {
        for (const auto& p : parameters) {
            if (p.first == name) return p.second;
        }
        return default_val;
    }

    static bool init(const ZenohConfig& config = ZenohConfig()) {
        std::string endpoint;
        if (!config.connect_endpoint.empty()) {
            endpoint = config.connect_endpoint;
        } else if (config.communication_mode == ZenohConfig::CommunicationMode::ZENOH_COMM_WIFI || config.mode == "wifi") {
            if (!config.host.empty()) {
                endpoint = "tcp/" + config.host + ":" + std::to_string(config.port);
            }
        } else {
            // Serial modes: rely on zenoh-bridge-serial or a bridge; connect via TCP bridge
            if (!config.host.empty()) {
                endpoint = "tcp/" + config.host + ":" + std::to_string(config.port);
            }
        }
        return init_session(endpoint.empty() ? nullptr : endpoint.c_str());
    }

    static bool init(uint16_t port) {
        ZenohConfig config;
        config.port = port;
        return init(config);
    }

    static bool init(const char* connect_endpoint) {
        ZenohConfig config;
        config.connect_endpoint = connect_endpoint ? connect_endpoint : "";
        return init(config);
    }

    static void shutdown() {
        if (session_opened) {
            z_close(z_session_loan_mut(&session), nullptr);
            z_session_drop(z_session_move(&session));
            session_opened = false;
            std::cout << "[Zenoh PC] Global session closed.\n";
        }
    }

    static const z_loaned_session_t* get_session() {
        return session_opened ? z_session_loan(&session) : nullptr;
    }

    const char* z_get_name() const {
        return node_name;
    }

    template <typename MsgType>
    ZenohPublisher<MsgType>* z_create_publisher(const char* topic_name, const QoS& qos = QoS()) {
        ZenohPublisher<MsgType>* pub = new ZenohPublisher<MsgType>(topic_name, qos);
        if (session_opened) {
            pub->declare(z_session_loan(&session));
        } else {
            std::cerr << "[Zenoh PC] WARNING: Create publisher before ZenohNode::init.\n";
        }
        cleanup_callbacks.push_back([pub]() { delete pub; });
        return pub;
    }

    template <typename MsgType>
    ZenohPublisher<MsgType>* z_create_publisher(const char* topic_name, uint32_t queue_size) {
        QoS qos;
        qos.depth = queue_size;
        return z_create_publisher<MsgType>(topic_name, qos);
    }

    template <typename MsgType>
    ZenohSubscription<MsgType>* z_create_subscription(const char* topic_name, SubscriptionCallback<MsgType> cb, const QoS& qos = QoS()) {
        ZenohSubscription<MsgType>* sub = new ZenohSubscription<MsgType>(qos);
        if (session_opened) {
            sub->declare(z_session_loan(&session), topic_name, std::move(cb));
        } else {
            std::cerr << "[Zenoh PC] WARNING: Create subscription before ZenohNode::init.\n";
        }
        cleanup_callbacks.push_back([sub]() { delete sub; });
        return sub;
    }

    template <typename MsgType>
    ZenohSubscription<MsgType>* z_create_subscription(const char* topic_name, SubscriptionCallback<MsgType> cb, uint32_t queue_size) {
        QoS qos;
        qos.depth = queue_size;
        return z_create_subscription<MsgType>(topic_name, std::move(cb), qos);
    }

    template <typename SrvType>
    ZenohService<SrvType>* z_create_service(const char* service_name, typename ZenohService<SrvType>::ServiceCallback cb) {
        ZenohService<SrvType>* srv = new ZenohService<SrvType>();
        if (session_opened) {
            srv->declare(z_session_loan(&session), service_name, std::move(cb));
        } else {
            std::cerr << "[Zenoh PC] WARNING: Create service before ZenohNode::init.\n";
        }
        cleanup_callbacks.push_back([srv]() { delete srv; });
        return srv;
    }

    template <typename SrvType>
    ZenohClient<SrvType>* z_create_client(const char* service_name) {
        ZenohClient<SrvType>* client = new ZenohClient<SrvType>(service_name);
        if (session_opened) {
            client->set_session(z_session_loan(&session));
        } else {
            std::cerr << "[Zenoh PC] WARNING: Create client before ZenohNode::init.\n";
        }
        cleanup_callbacks.push_back([client]() { delete client; });
        return client;
    }

    ZenohTimer* z_create_timer(uint32_t period_ms, TimerCallback cb) {
        ZenohTimer* timer = new ZenohTimer(period_ms, std::move(cb));
        cleanup_callbacks.push_back([timer]() { delete timer; });
        return timer;
    }

    void z_publish_raw(const char* topic_name, const uint8_t* payload, size_t len) {
        if (!session_opened || payload == nullptr || len == 0) return;

        z_view_keyexpr_t keyexpr;
        z_view_keyexpr_from_str(&keyexpr, topic_name);

        z_put_options_t options;
        z_put_options_default(&options);

        z_owned_bytes_t bytes;
        z_bytes_copy_from_buf(&bytes, payload, len);

        ZenohSessionMutexPC::lock();
        z_put(z_session_loan(&session), z_view_keyexpr_loan(&keyexpr), z_bytes_move(&bytes), &options);
        ZenohSessionMutexPC::unlock();
    }

    void z_spin() {
        while (session_opened && !shutdown_requested) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
};

// Static definitions
z_owned_session_t ZenohNode::session;
z_owned_liveliness_token_t ZenohNode::liveliness_token;
bool ZenohNode::session_opened = false;
std::mutex ZenohSessionMutexPC::mutex;

// Global non-blocking thread sleep helper
inline void z_delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#endif // ZENOH_ROS_PC_H
