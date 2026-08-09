/**
 * echo.cpp — Shared C++ CLI Echo Tool for zenoh_ros
 * =================================================
 * Subscribes to any Zenoh ROS 2 topic (e.g., 'robot/hello_string',
 * 'robot/sim_counter', 'robot/mpu6050', 'ws2812b', 'test/pose')
 * and pretty-prints live payloads with color formatting and header decoding.
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <csignal>
#include <glob.h>
#include <zenoh.h>

static bool g_running = true;

void echo_signal_handler(int signum) {
    (void)signum;
    g_running = false;
}

// ANSI Color Constants
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_RED     "\033[31m"
#define COLOR_GRAY    "\033[90m"
#define COLOR_BLUE    "\033[34m"

// ─── Standalone MessagePack → Clean Formatted String / JSON ────────────────
struct MsgpackReader {
    const uint8_t* buf;
    size_t         len;
    size_t         pos;

    bool eof() const { return pos >= len; }

    uint8_t read_u8() {
        if (pos >= len) throw std::runtime_error("msgpack: truncated u8");
        return buf[pos++];
    }
    uint16_t read_u16() {
        if (pos + 2 > len) throw std::runtime_error("msgpack: truncated u16");
        uint16_t v = (uint16_t(buf[pos]) << 8) | buf[pos + 1];
        pos += 2; return v;
    }
    uint32_t read_u32() {
        if (pos + 4 > len) throw std::runtime_error("msgpack: truncated u32");
        uint32_t v = (uint32_t(buf[pos]) << 24) | (uint32_t(buf[pos+1]) << 16)
                   | (uint32_t(buf[pos+2]) << 8)  |  uint32_t(buf[pos+3]);
        pos += 4; return v;
    }
    uint64_t read_u64() {
        if (pos + 8 > len) throw std::runtime_error("msgpack: truncated u64");
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i) v = (v << 8) | buf[pos++];
        return v;
    }
    float read_f32() {
        uint32_t bits = read_u32();
        float v; std::memcpy(&v, &bits, 4); return v;
    }
    double read_f64() {
        uint64_t bits = read_u64();
        double v; std::memcpy(&v, &bits, 8); return v;
    }
    std::string read_str(size_t n) {
        if (pos + n > len) throw std::runtime_error("msgpack: truncated str");
        std::string s(reinterpret_cast<const char*>(buf + pos), n);
        pos += n; return s;
    }
    void skip(size_t n) {
        if (pos + n > len) throw std::runtime_error("msgpack: truncated skip");
        pos += n;
    }
};

static std::string msgpack_to_pretty_json(MsgpackReader& r, int indent, int depth);

static std::string json_indent(int indent, int depth) {
    if (indent < 0) return "";
    return std::string(size_t(indent * depth), ' ');
}

static std::string json_string(const std::string& s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else if (c < 0x20) {
            char buf[8];
            snprintf(buf, sizeof(buf), "\\u%04x", c);
            out += buf;
        } else {
            out += c;
        }
    }
    return out + "\"";
}

static std::string json_double(double v) {
    if (std::isnan(v))  return "null";
    if (std::isinf(v))  return v > 0 ? "1e308" : "-1e308";
    std::ostringstream ss;
    ss << std::setprecision(6) << v;
    std::string s = ss.str();
    if (s.find('.') == std::string::npos && s.find('e') == std::string::npos)
        s += ".0";
    return s;
}

static std::string parse_array(MsgpackReader& r, uint32_t n, int indent, int depth) {
    std::string nl    = (indent >= 0) ? "\n" : "";
    std::string inner = json_indent(indent, depth + 1);
    std::string outer = json_indent(indent, depth);

    if (n == 0) return "[]";

    std::string out = "[" + nl;
    for (uint32_t i = 0; i < n; ++i) {
        out += inner + msgpack_to_pretty_json(r, indent, depth + 1);
        if (i + 1 < n) out += ",";
        out += nl;
    }
    return out + outer + "]";
}

static std::string parse_map(MsgpackReader& r, uint32_t n, int indent, int depth) {
    std::string nl    = (indent >= 0) ? "\n" : "";
    std::string inner = json_indent(indent, depth + 1);
    std::string outer = json_indent(indent, depth);
    std::string colon = (indent >= 0) ? ": " : ":";

    if (n == 0) return "{}";

    std::string out = "{" + nl;
    for (uint32_t i = 0; i < n; ++i) {
        std::string key;
        uint8_t kb = r.buf[r.pos];
        if ((kb & 0xe0) == 0xa0) {
            r.pos++;
            key = json_string(r.read_str(kb & 0x1f));
        } else if (kb == 0xd9) {
            r.pos++;
            key = json_string(r.read_str(r.read_u8()));
        } else if (kb == 0xda) {
            r.pos++;
            key = json_string(r.read_str(r.read_u16()));
        } else if (kb == 0xdb) {
            r.pos++;
            key = json_string(r.read_str(r.read_u32()));
        } else {
            std::string v = msgpack_to_pretty_json(r, -1, 0);
            key = "\"" + v + "\"";
        }

        std::string val = msgpack_to_pretty_json(r, indent, depth + 1);

        out += inner + COLOR_CYAN + key + COLOR_RESET + colon + val;
        if (i + 1 < n) out += ",";
        out += nl;
    }
    return out + outer + "}";
}

static std::string msgpack_to_pretty_json(MsgpackReader& r, int indent, int depth) {
    if (r.eof()) throw std::runtime_error("msgpack: unexpected end of data");

    uint8_t b = r.read_u8();

    if (b <= 0x7f)          return COLOR_YELLOW + std::to_string(b) + COLOR_RESET;
    if ((b & 0xf0) == 0x80) return parse_map(r, b & 0x0f, indent, depth);
    if ((b & 0xf0) == 0x90) return parse_array(r, b & 0x0f, indent, depth);
    if ((b & 0xe0) == 0xa0) return COLOR_GREEN + json_string(r.read_str(b & 0x1f)) + COLOR_RESET;
    if (b >= 0xe0)          return COLOR_YELLOW + std::to_string(static_cast<int8_t>(b)) + COLOR_RESET;

    switch (b) {
        case 0xc0: return COLOR_MAGENTA "null" COLOR_RESET;
        case 0xc2: return COLOR_MAGENTA "false" COLOR_RESET;
        case 0xc3: return COLOR_MAGENTA "true" COLOR_RESET;

        case 0xc4: { uint8_t  n = r.read_u8();  r.skip(n); return COLOR_GRAY "<bin>" COLOR_RESET; }
        case 0xc5: { uint16_t n = r.read_u16(); r.skip(n); return COLOR_GRAY "<bin>" COLOR_RESET; }
        case 0xc6: { uint32_t n = r.read_u32(); r.skip(n); return COLOR_GRAY "<bin>" COLOR_RESET; }

        case 0xca: return COLOR_YELLOW + json_double(r.read_f32()) + COLOR_RESET;
        case 0xcb: return COLOR_YELLOW + json_double(r.read_f64()) + COLOR_RESET;

        case 0xcc: return COLOR_YELLOW + std::to_string(r.read_u8()) + COLOR_RESET;
        case 0xcd: return COLOR_YELLOW + std::to_string(r.read_u16()) + COLOR_RESET;
        case 0xce: return COLOR_YELLOW + std::to_string(r.read_u32()) + COLOR_RESET;
        case 0xcf: return COLOR_YELLOW + std::to_string(r.read_u64()) + COLOR_RESET;

        case 0xd0: return COLOR_YELLOW + std::to_string(static_cast<int8_t>(r.read_u8())) + COLOR_RESET;
        case 0xd1: return COLOR_YELLOW + std::to_string(static_cast<int16_t>(r.read_u16())) + COLOR_RESET;
        case 0xd2: return COLOR_YELLOW + std::to_string(static_cast<int32_t>(r.read_u32())) + COLOR_RESET;
        case 0xd3: return COLOR_YELLOW + std::to_string(static_cast<int64_t>(r.read_u64())) + COLOR_RESET;

        case 0xd4: { r.skip(2);  return COLOR_GRAY "<ext1>" COLOR_RESET; }
        case 0xd5: { r.skip(3);  return COLOR_GRAY "<ext2>" COLOR_RESET; }
        case 0xd6: { r.skip(5);  return COLOR_GRAY "<ext4>" COLOR_RESET; }
        case 0xd7: { r.skip(9);  return COLOR_GRAY "<ext8>" COLOR_RESET; }
        case 0xd8: { r.skip(17); return COLOR_GRAY "<ext16>" COLOR_RESET; }

        case 0xd9: return COLOR_GREEN + json_string(r.read_str(r.read_u8())) + COLOR_RESET;
        case 0xda: return COLOR_GREEN + json_string(r.read_str(r.read_u16())) + COLOR_RESET;
        case 0xdb: return COLOR_GREEN + json_string(r.read_str(r.read_u32())) + COLOR_RESET;

        case 0xdc: return parse_array(r, r.read_u16(), indent, depth);
        case 0xdd: return parse_array(r, r.read_u32(), indent, depth);

        case 0xde: return parse_map(r, r.read_u16(), indent, depth);
        case 0xdf: return parse_map(r, r.read_u32(), indent, depth);

        default:
            throw std::runtime_error("msgpack: unknown type byte");
    }
}

static bool extract_clean_msgpack_string(const uint8_t* data, size_t len, std::string& out_str) {
    if (len == 0) return false;

    // Check if data starts with Msgpack String Header (0xa0..0xbf or 0xd9, 0xda, 0xdb)
    uint8_t b = data[0];
    size_t str_len = 0;
    size_t header_len = 0;

    if ((b & 0xe0) == 0xa0) {
        str_len = b & 0x1f;
        header_len = 1;
    } else if (b == 0xd9 && len >= 2) {
        str_len = data[1];
        header_len = 2;
    } else if (b == 0xda && len >= 3) {
        str_len = (uint16_t(data[1]) << 8) | data[2];
        header_len = 3;
    } else if (b == 0xdb && len >= 5) {
        str_len = (uint32_t(data[1]) << 24) | (uint32_t(data[2]) << 16) | (uint32_t(data[3]) << 8) | data[4];
        header_len = 5;
    }

    if (header_len > 0 && (header_len + str_len == len || header_len + str_len == len - 1)) {
        // Verify printable ASCII string
        bool is_ascii = true;
        for (size_t i = 0; i < str_len; ++i) {
            unsigned char c = data[header_len + i];
            if (c < 32 && c != '\n' && c != '\r' && c != '\t') {
                is_ascii = false;
                break;
            }
        }
        if (is_ascii) {
            out_str = std::string(reinterpret_cast<const char*>(data + header_len), str_len);
            return true;
        }
    }
    return false;
}

static std::string try_msgpack_decode(const uint8_t* data, size_t len, size_t offset, int indent) {
    if (offset >= len) return "";
    MsgpackReader r{data, len, offset};
    try {
        std::string json = msgpack_to_pretty_json(r, indent, 0);
        if (r.pos == len || (r.pos + 1 == len && r.buf[r.pos] == 0)) return json;
    } catch (...) {}
    return "";
}

void echo_sample_callback(z_loaned_sample_t* sample, void* arg) {
    (void)arg;
    try {
        const z_loaned_bytes_t* payload = z_sample_payload(sample);
        if (!payload) return;

        z_owned_slice_t slice;
        z_bytes_to_slice(payload, &slice);
        size_t len = z_slice_len(z_slice_loan(&slice));
        const uint8_t* data = reinterpret_cast<const uint8_t*>(z_slice_data(z_slice_loan(&slice)));

        if (len == 0) {
            std::cout << COLOR_GRAY << "<empty payload>" << COLOR_RESET << "\n";
            z_slice_drop(z_slice_move(&slice));
            return;
        }

        // 1. Check for Msgpack String header (e.g. z_String "HelloWorld_X")
        std::string clean_str;
        if (extract_clean_msgpack_string(data, len, clean_str)) {
            std::cout << COLOR_GREEN << clean_str << COLOR_RESET << "\n";
            z_slice_drop(z_slice_move(&slice));
            return;
        }

        // 2. Check for Plain-text ROS 2 Log Output (e.g. [INFO] [node_name]: ...)
        std::string text(reinterpret_cast<const char*>(data), len);
        if (text.size() > 2 && text[0] == '[') {
            const char* color = COLOR_RESET;
            if      (text.rfind("[DEBUG]", 0) == 0) color = COLOR_CYAN;
            else if (text.rfind("[INFO]",  0) == 0) color = COLOR_RESET;
            else if (text.rfind("[WARN]",  0) == 0) color = COLOR_YELLOW;
            else if (text.rfind("[ERROR]", 0) == 0) color = COLOR_RED;
            else if (text.rfind("[FATAL]", 0) == 0) color = COLOR_RED COLOR_BOLD;
            std::cout << color << text << COLOR_RESET << "\n";
            z_slice_drop(z_slice_move(&slice));
            return;
        }

        // 3. Try MessagePack Object / Array / Map decoding
        std::string json = try_msgpack_decode(data, len, 0, 2);
        if (!json.empty()) {
            std::cout << json << "\n";
            z_slice_drop(z_slice_move(&slice));
            return;
        }

        for (size_t off = 1; off <= 8 && off < len; ++off) {
            json = try_msgpack_decode(data, len, off, 2);
            if (!json.empty()) {
                std::cout << json << "\n";
                z_slice_drop(z_slice_move(&slice));
                return;
            }
        }

        // 4. Printable ASCII fallback (Strict ASCII range 32-126)
        bool is_printable = true;
        for (size_t i = 0; i < len; ++i) {
            unsigned char c = data[i];
            if (c != '\n' && c != '\r' && c != '\t' && (c < 32 || c > 126)) {
                is_printable = false;
                break;
            }
        }
        if (is_printable) {
            std::cout << COLOR_GREEN << text << COLOR_RESET << "\n";
            z_slice_drop(z_slice_move(&slice));
            return;
        }

        // 5. Unknown Binary Payload
        std::cout << COLOR_YELLOW << "[binary payload: " << len << " bytes]" << COLOR_RESET << "\n";
        z_slice_drop(z_slice_move(&slice));
    }
    catch (const std::exception& e) {
        std::cerr << COLOR_RED << "[echo error] " << e.what() << COLOR_RESET << "\n";
    }
}

static std::string auto_detect_serial_port() {
    glob_t glob_result;
    std::vector<std::string> patterns = {"/dev/ttyACM*", "/dev/ttyUSB*"};
    for (const auto& pattern : patterns) {
        if (glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result) == 0) {
            if (glob_result.gl_pathc > 0) {
                std::string port = glob_result.gl_pathv[0];
                globfree(&glob_result);
                return port;
            }
            globfree(&glob_result);
        }
    }
    return "/dev/ttyACM0";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << COLOR_BOLD << "Usage: " << COLOR_RESET << "echo <topic_name> [host_ip_or_endpoint] [port]\n";
        std::cout << COLOR_GRAY << "Examples:\n";
        std::cout << "  echo robot/hello_string 192.168.4.1\n";
        std::cout << "  echo robot/hello_string 10.42.0.50 2547\n";
        std::cout << "  echo serial/counter /dev/ttyACM0" << COLOR_RESET << "\n";
        return 1;
    }

    std::string topic = argv[1];
    std::string endpoint_arg = (argc > 2) ? argv[2] : "auto";
    std::string custom_port  = (argc > 3) ? argv[3] : "7447";

    signal(SIGINT,  echo_signal_handler);
    signal(SIGTERM, echo_signal_handler);

    std::cout << COLOR_CYAN COLOR_BOLD << "==========================================" << COLOR_RESET << "\n";
    std::cout << COLOR_CYAN COLOR_BOLD << "  zenoh_ros C++ Echo — Topic: '" << topic << "'" << COLOR_RESET << "\n";
    std::cout << COLOR_CYAN COLOR_BOLD << "==========================================" << COLOR_RESET << "\n";

    z_owned_config_t config;
    z_config_default(&config);
    zc_config_insert_json5(z_config_loan_mut(&config), Z_CONFIG_MODE_KEY, "\"peer\"");

    std::string connect_url;

    if (endpoint_arg == "auto") {
        // Default to Wi-Fi SoftAP 192.168.4.1:7447
        connect_url = "[\"tcp/192.168.4.1:" + custom_port + "\"]";
        std::cout << COLOR_GRAY << "[echo] Connecting to endpoint: 192.168.4.1:" << custom_port << "..." << COLOR_RESET << "\n";
    } else if (endpoint_arg.find('/') != std::string::npos && endpoint_arg.find("dev") != std::string::npos) {
        // Serial Port (e.g. /dev/ttyACM0 or /dev/ttyUSB0)
        connect_url = "[\"serial/" + endpoint_arg + "\"]";
        std::cout << COLOR_GRAY << "[echo] Connecting to serial port: " << endpoint_arg << "..." << COLOR_RESET << "\n";
    } else if (endpoint_arg.find(':') != std::string::npos) {
        // Locator or IP with port (e.g. 10.42.0.50:2547)
        connect_url = "[\"tcp/" + endpoint_arg + "\"]";
        std::cout << COLOR_GRAY << "[echo] Connecting to endpoint: " << endpoint_arg << "..." << COLOR_RESET << "\n";
    } else {
        // IP Address with custom or default port
        connect_url = "[\"tcp/" + endpoint_arg + ":" + custom_port + "\"]";
        std::cout << COLOR_GRAY << "[echo] Connecting to endpoint: " << endpoint_arg << ":" << custom_port << "..." << COLOR_RESET << "\n";
    }

    zc_config_insert_json5(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, connect_url.c_str());

    z_owned_session_t session;
    if (z_open(&session, z_config_move(&config), NULL) < 0) {
        std::cerr << COLOR_RED << "[echo] ERROR: Failed to open Zenoh session!" << COLOR_RESET << "\n";
        return 1;
    }

    std::cout << COLOR_CYAN << "[echo] Subscribed live on '" << topic << "'. Press Ctrl+C to exit." << COLOR_RESET << "\n\n";

    z_owned_closure_sample_t closure;
    z_closure_sample(&closure, echo_sample_callback, NULL, NULL);

    z_owned_subscriber_t sub;
    z_view_keyexpr_t keyexpr;
    z_view_keyexpr_from_str(&keyexpr, topic.c_str());

    if (z_declare_subscriber(z_session_loan(&session), &sub,
                             z_view_keyexpr_loan(&keyexpr),
                             z_closure_sample_move(&closure), NULL) < 0) {
        std::cerr << COLOR_RED << "[echo] ERROR: Failed to declare subscriber on '"
                  << topic << "'!" << COLOR_RESET << "\n";
        z_close(z_session_loan_mut(&session), NULL);
        return 1;
    }

    while (g_running) {
        z_sleep_ms(100);
    }

    std::cout << "\n" << COLOR_GRAY << "[echo] Unsubscribing and shutting down..." << COLOR_RESET << "\n";
    z_undeclare_subscriber(z_subscriber_move(&sub));
    z_close(z_session_loan_mut(&session), NULL);
    std::cout << COLOR_GRAY << "[echo] Session closed cleanly." << COLOR_RESET << "\n";

    return 0;
}
