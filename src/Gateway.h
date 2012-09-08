/**
 * All Rights Reserved
 *
 * @author Jeremy Hahn
 * @copyright (c) 2012 Make A byte, inc
 */
#ifndef GATEWAY_H_
#define GATEWAY_H_

#include "Arduino.h"
#include "EEPROM.h"

class Gateway {

    private:
       byte _gateway_octet1;
       byte _gateway_octet2;
       byte _gateway_octet3;
       byte _gateway_octet4;
       byte _gateway[4];
       String _gateway_string;

    public:
        Gateway();
        ~Gateway();
        void set(byte octet1, byte octet2, byte octet3, byte octet4);
        byte* getBytes();
        void erase();
        String toString();
};

#endif
