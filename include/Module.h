#ifndef MODULE_H
#define MODULE_H

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <string>


class Module {

public:
    virtual ~Module() {}    //Destructeur virtuel pour assurer une destruction correcte

protected:
    uint8_t mac_address[8];

private:
/***************************************
 * 
 * Créer ici les base des module en uitilisant le polymorphisme
 * 
 * 
 */

};


#endif //MODULE_H