#ifndef HEADER_REGISTER_H
    #define HEADER_REGISTER_H
    #include <stdio.h>
    #include <stdlib.h>

    typedef struct headerRegister Header;

    Header *create_header_register();
    void free_header_register(Header **hRegister);

    // Setters 
    void header_set_status(Header *h, char status);
    void header_set_topo(Header *h, int topo);
    void header_set_proxRRN(Header *h, int proxRRN);
    void header_set_nroEstacoes(Header *h, int nroEstacoes);
    void header_set_nroParesEst(Header *h, int nroParesEst);

    // I/O - Escrita campo a campo
    void header_write_to_file(FILE *fp, Header *h);
    void header_read_from_file(FILE *fp, Header *h);

#endif