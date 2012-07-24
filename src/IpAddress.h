/**
 * All Rights Reserved
 *
 * @author Jeremy Hahn
 * @copyright (c) 2012 Make A byte, inc
 */
#ifndef IPADDRESS_H
#define IPADDRESS_H

#include "Arduino.h"
#include "EEPROM.h"

class IpAddress {

    private:
       byte _octet1;
       byte _octet2;
       byte _octet3;
       byte _octet4;
       byte _ip[4];

    public:
        IpAddress();
        ~IpAddress();
        void set(byte octet1, byte octet2, byte octet3, byte octet4);
        byte* getBytes();
        void erase();
        String toString();
};

#endif
