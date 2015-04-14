/* coap_parse.c -- implementation for packet decoding.
 * Author: Luis Colorado <luis.colorado@ericsson.com>
 * Date: Fri Apr 10 09:54:34 EEST 2015
 * Disclaimer: (C) LUIS COLORADO.  All rights reserved.
 */

#include <assert.h>
#include "coapP.h"

static uint32_t
parse_unsigned(const uint8_t *buff, size_t bufsz)
{
    uint32_t res = 0;
    DEB(D("BEGIN\n"));
    while (bufsz--) {
        res <<= 8;
        res |= *buff++;
    } /* while */
    DEB(D("RETURN ==> 0x%04x/%d\n"), res, res);
    return res;
} /* parse_unsigned */
   
/* The next macro is to process the Options of a CoAP message.
 * It acts on the values taken from the OptDelta and OptLength
 * fields of the first byte from the option field. */
#define SWITCH(var)  do{\
        switch(var) { \
        case 15: \
            DEB(D("RETURN COAP_INVALID_"#var"\n")); \
            return COAP_INVALID_##var; \
        case 14: \
            /* two bytes of data */ \
            CHK(2); \
            var = 256 + 13 + parse_unsigned(buff, 2); \
            DEB(D(#var " <== 0x%x(%d)\n"), var, var); \
            ACT(2); \
            break; \
        case 13: /* one byte of delta */ \
            CHK(1); \
            var = 13 + parse_unsigned(buff, 1); \
            DEB(D(#var " <== 0x%x(%d)\n"), var, var); \
            ACT(1); \
            break; \
        } /* switch */\
    } while(0)

coap_err
coap_parse(
        const uint8_t  *buff,
        size_t          bufsz,
        coap_msg       *tgt)
{
    uint32_t opt_name = 0;

    DEB(D("BEGIN\n"));
    PRB(bufsz, buff, D("PACKET (size = %d bytes)"), bufsz);

#define VALUEOF(T) ((buff[COAP_##T##_OFFS] & COAP_##T##_MASK) \
        >> COAP_##T##_SHFT)

    /* check we have enough buffer to parse */
    CHK(COAP_HDR_LEN);

    coap_msg_init(tgt); /* initialize structure */
    tgt->c_pktdat = buff; /* pointer to packet data */

    /* check the packet version */
    if (VALUEOF(VERS) != COAP_VERS_VALUE) {
        DEB(D("==> COAP_INVALID_VERSION\n"));
        return COAP_INVALID_VERSION;
    } /* if */

    /* packet type */
    tgt->c_typ  = VALUEOF(TYPE);
    DEB(D("TYPE = %s\n"),
            coap_typ2str(tgt->c_typ));

    /* TKL */
    if ((tgt->c_toklen = VALUEOF(TKL)) > COAP_TKL_MAX) {
        DEB(D("==> COAP_INVALID_TKL(%lu)\n"), tgt->c_toklen);
        return COAP_INVALID_TKL;
    } /* if */

    /* Code */
    tgt->c_code = VALUEOF(CODE);
    DEB(D("CODE = COAP_CODE(%d, %d)\n"),
            COAP_CODE_MAJOR(tgt->c_code),
            COAP_CODE_MINOR(tgt->c_code));

    /* Message ID */
    tgt->c_msgid = parse_unsigned(buff + COAP_MSGID_OFFS, COAP_MSGID_SZ);
    DEB(D("MSGID = 0x%04x\n"),
            tgt->c_msgid);

    ACT(COAP_HDR_LEN);
    if (tgt->c_toklen) CHK(tgt->c_toklen); /* check for space for the token */

    /* token */
    tgt->c_tokdat = buff;
    PRB(tgt->c_toklen, tgt->c_tokdat,
            D("TOKEN: %d bytes"),
            tgt->c_toklen);
    if (tgt->c_toklen) ACT(tgt->c_toklen);

    while(bufsz) { /* process options */
        size_t OptDLT, OptLEN;
        coap_opt *opt;

        CHK(1); /* no need to CHK(1); as we got into the loop */

        if (buff[0] == COAP_END_OF_OPTIONS) {
            DEB(D("END_OF_OPTIONS\n"));
            ACT(1); /* move the pointer */
            /* it's invalid to have payload data of length 0, so... */
            CHK(1); /* check at least one byte of payload */
            tgt->c_plddat = buff;
            tgt->c_pldlen = bufsz;
            break;
        } /* if */

        /* buff[0] != COAP_END_OF_OPTIONS */
        TR(OptDLT = VALUEOF(OptDLT));
        TR(OptLEN = VALUEOF(OptLEN));
        ACT(1); /* advance */

        /* see macro defined before this function */
        SWITCH(OptDLT); /* process Delta */
        SWITCH(OptLEN); /* process Length */

        if (OptLEN) CHK(OptLEN);

        /* space for the option struct */
        assert(opt = malloc(sizeof(coap_opt)));

        opt_name += OptDLT;
        opt->o_typ = opt_name;
        opt->o_len = OptLEN;
        opt->o_val = buff;

        PRB(opt->o_len, opt->o_val,
                D("OPT %d: len=%d\n"),
                opt->o_typ, opt->o_len);

        coap_msg_addopt(tgt, opt);

        if (OptLEN) ACT(OptLEN);
    } /* while */

    DEB(D("RETURN ==> COAP_OK\n"));

    return COAP_OK;
} /* coap_parse */
