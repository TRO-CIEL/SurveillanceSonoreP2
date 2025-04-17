/**
 * @file CCom.cpp
 * @brief Communication class implementation
 */

#include "CCom.h"

CCom::CCom() {
    // Initialize attributes
    data = "";
}

bool CCom::Setup() {
    // Initialize WiFi (to be implemented based on requirements)
    return true;
}

bool CCom::SendData(String data) {
    // Send data via HTTP (to be implemented based on requirements)
    return true;
}