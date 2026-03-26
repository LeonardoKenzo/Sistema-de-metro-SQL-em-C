#ifndef HEADER_REGISTER_H
    #define HEADER_REGISTER_H
    #include <stdio.h>
    #include <stdlib.h>

    typedef struct headerRegister HEADER_REGISTER;

    HEADER_REGISTER *CreateHeaderRegister();
    void FreeHeaderRegister(HEADER_REGISTER **hRegister);

#endif