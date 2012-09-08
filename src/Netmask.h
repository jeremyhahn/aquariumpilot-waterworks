/**
 * All Rights Reserved
 *
 * @author Jeremy Hahn
 * @copyright (c) 2012 Make A byte, inc
 */
#ifndef NETMASK_H_
#define NETMASK_H_

#include "Arduino.h"
#include "EEPROM.h"

class Netmask {

    private:
       byte _netmask_octet1;
       byte _netmask_octet2;
       byte _netmask_octet3;
       byte _netmask_octet4;
       byte _netmask[4];
       String _netmask_string;

    public:
        Netmask();
        ~Netmask();
        void set(byte octet1, byte octet2, byte octet3, byte octet4);
        byte* getBytes();
        void erase();
        String toString();
};

#endif
