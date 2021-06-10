//
// Created by user on 06.12.2020.
//

#include <iostream>
#include <iomanip>
#include <utility>
#include "CanController.h"

#ifndef VIRTUAL_CAN_DEVICE

CanController::CanController(const std::string &port_name, uint32_t baudrate, bool maximize)
        : comPort(std::make_unique<CanDevice>(port_name, baudrate, true))
{
    if(maximize) {
        comPort->maximize_baudrate();
    }

    comPort->serial_transaction("ATE1\r");   //echo on
    comPort->serial_transaction("ATL0\r");   //linefeeds off
    comPort->serial_transaction("ATS0\r");   //spaces off
    comPort->serial_transaction("STPO\r");   //ATBI Open current protocol.
    comPort->serial_transaction("ATAL\r");   //allow long messages
    comPort->serial_transaction("ATAT0\r");  //disable adaptive timing
    comPort->serial_transaction("ATCAF0\r"); //CAN auto formatting off
    comPort->serial_transaction("ATST01\r"); //Set timeout to hh(13) x 4 ms
    comPort->serial_transaction("ATR0\r");   //Responses on
}

int CanController::set_protocol(CAN_PROTO protocol) {

    const std::lock_guard<std::mutex> lock(mutex);
    return comPort->set_protocol(protocol);
}

int CanController::transaction(unsigned int ecu_address, vector<uint8_t> &data) {

    const std::lock_guard<std::mutex> lock(mutex); //TODO: remove me if you want mess :D
    if (m_ecu_addr != ecu_address) {
        if (set_ecu_address(ecu_address)) {
            std::cerr << "Set ECU address 0x" << std::hex << ecu_address << " error" << std::endl;
            return -1;
        }
        m_ecu_addr = (int)ecu_address;
    }

    if(send_data(data)) {
        std::cerr << "Send data to ECU 0x" << std::hex << ecu_address << " error" << std::endl;
        return -1;
    }

    return 0;
}

int CanController::send_data(std::vector<uint8_t> &data) {
    auto tx_size = data.size() > CAN_frame_sz ? 8 : data.size();
    std::string io_buff;
    io_buff.resize(tx_size * 2 + 1);
    io_buff[tx_size * 2] = '\r';
    hex2ascii(data.data(), tx_size, io_buff.data());
    return comPort->serial_transaction(io_buff).first;
}

int CanController::set_ecu_address(unsigned int ecu_address) {
    std::stringstream ss;
    ss << "ATSH" << std::hex << std::setfill('0') << std::setw(3) <<  ecu_address << '\r';
    return comPort->serial_transaction(ss.str()).first;
}

#else

CanController::CanController(const std::string& port_name, uint32_t baudrate, bool maximize) {}

int CanController::set_protocol(CAN_PROTO protocol)
{
    std::cout << "Set protocol: " << protocol << std::endl;
    return 0;
}

int CanController::transaction(unsigned int ecu_address, vector<uint8_t> &data) {

    auto tx_size = data.size() > CAN_frame_sz ? 8 : data.size();
    std::string io_buff;
    io_buff.resize(tx_size * 2);
    hex2ascii(data.data(), tx_size, io_buff.data());
    std::cout << "Send: " << std::hex << std::setfill('0') << std::setw(3) << ecu_address << "@" << io_buff
              << std::endl;

    return 0;
}

#endif