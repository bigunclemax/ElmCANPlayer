#ifndef ELMCANPLAYER_UTILS_H
#define ELMCANPLAYER_UTILS_H

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace ElmPlayerUtils {

    struct Packet {
        uint16_t ID;
        union {
            uint64_t data64;
            uint8_t data[8];
        };

        bool operator<(const Packet &rhs) const {
            return rhs.ID < ID || rhs.ID == ID && (rhs.data64 < data64);
        };

        std::string str() const {
            constexpr int can_data_sz = 8;
            constexpr int can_id_sz = 3;
            std::string str;
            str.resize(can_id_sz+can_data_sz+1);
            unsigned char hex_str[] = "0123456789ABCDEF";

            str[0] = (char) hex_str[(ID >> 8u) & 0x0Fu];
            str[1] = (char) hex_str[(ID >> 4u) & 0x0Fu];
            str[2] = (char) hex_str[(ID >> 0u) & 0x0Fu];
            str[3] = ':';

            for (auto i = 0; i < can_data_sz; ++i) {
                str[i * 2 + 0 + can_id_sz + 1] = (char) hex_str[(data[i] >> 4u) & 0x0Fu];
                str[i * 2 + 1 + can_id_sz + 1] = (char) hex_str[(data[i]) & 0x0Fu];
            }
            return str;
        }
    };

    inline std::vector<Packet> convert(const fs::path &file) {

        std::set<uint16_t> uniq_ids;
        std::vector<Packet> CAP;

        std::ifstream infile(file);

        if (!infile.good()) {
            throw std::runtime_error("Can't open dump file");
        }

        std::string line;
        while (getline(infile, line).good()) {

            std::stringstream ss(line);

            Packet packet{};
            ss >> std::hex >> packet.ID;
            uniq_ids.insert(packet.ID);
            for (unsigned char &i : packet.data) {
                int b;
                ss >> std::hex >> b;
                i = b;
            }
            CAP.push_back(packet);
        }

        return CAP;
    }
}

#endif //ELMCANPLAYER_UTILS_H
