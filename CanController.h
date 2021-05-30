//
// Created by user on 06.12.2020.
//

#ifndef FOCUSIPCCTRL_CONTROLLERELX_H
#define FOCUSIPCCTRL_CONTROLLERELX_H

#include "CanController.h"
#include "stn1170-tools/stnlib/CanDevice.h"
#include <mutex>
#include <atomic>

using namespace stnlib;

constexpr int CAN_frame_sz  = 8;

class CanController {

public:

    CanController(const std::string& port_name, uint32_t baudrate, bool maximize);
    CanController(CanController const &) = delete;

    int set_protocol(CAN_PROTO protocol);
    int transaction(unsigned ecu_address, std::vector<uint8_t> &data);

private:

    int send_data(std::vector<uint8_t> &data);
    int set_ecu_address(unsigned ecu_address);
    std::atomic<int> m_ecu_addr = 0;

    CanDevice comPort;
    std::mutex mutex;

    std::atomic<bool> is_logger_set = false;

    inline static void hex2ascii(const uint8_t* bin, unsigned int binsz, char* result)
    {
        unsigned char     hex_str[]= "0123456789ABCDEF";

        for (auto i = 0; i < binsz; ++i)
        {
            result[i * 2 + 0] = (char)hex_str[(bin[i] >> 4u) & 0x0Fu];
            result[i * 2 + 1] = (char)hex_str[(bin[i]      ) & 0x0Fu];
        }
    };
};


#endif //FOCUSIPCCTRL_CONTROLLERELX_H
