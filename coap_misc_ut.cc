/* coap_misc_ut.cc -- Unit Tests for coap_misc.c
 * Author: Luis Colorado <luis.colorado@ericsson.com>
 * Date: mar abr 21 21:28:20 EEST 2015
 * Disclaimer: (C) 2015 LUIS COLORADO.  All rights reserved.
 */

#include <gtest/gtest.h>
#include "coap.h"

TEST(CoapTyp2Str, coap_typ_CON) {
    EXPECT_STREQ("CON", coap_typ2str(coap_typ_CON));
}

TEST(CoapTyp2Str, coap_typ_NON) {
    EXPECT_STREQ("NON", coap_typ2str(coap_typ_NON));
}

TEST(CoapTyp2Str, coap_typ_ACK) {
    EXPECT_STREQ("ACK", coap_typ2str(coap_typ_ACK));
}

TEST(CoapTyp2Str, coap_typ_RST) {
    EXPECT_STREQ("RST", coap_typ2str(coap_typ_RST));
}

/* Common for tests of coap_misc module */
struct CoapMisc: ::testing::Test {
    coap_msg            msg;
    CoapMisc() {
        memset(&msg, 0, sizeof msg);
    } /* Misc() */
}; /* Misc */

TEST_F(CoapMisc, coapMsgSettypeCON) {
    const coap_typ ct = coap_typ_CON;

    EXPECT_EQ(COAP_OK, coap_msg_settype(&msg, ct));
    EXPECT_EQ(ct, msg.c_typ);
}

TEST_F(CoapMisc, coapMsgSettypeNON) {
    const coap_typ ct = coap_typ_NON;

    EXPECT_EQ(COAP_OK, coap_msg_settype(&msg, ct));
    EXPECT_EQ(ct, msg.c_typ);
}

TEST_F(CoapMisc, coapMsgSettypeACK) {
    const coap_typ ct = coap_typ_ACK;

    EXPECT_EQ(COAP_OK, coap_msg_settype(&msg, ct));
    EXPECT_EQ(ct, msg.c_typ);
}

TEST_F(CoapMisc, coapMsgSettypeRST) {
    const coap_typ ct = coap_typ_RST;

    EXPECT_EQ(COAP_OK, coap_msg_settype(&msg, ct));
    EXPECT_EQ(ct, msg.c_typ);
}

TEST_F(CoapMisc, coapMsgSetCode) {
    for (uint8_t code = COAP_CODE(0, 0);
            code < COAP_CODE(7, 31);
            ++code)
    {
        EXPECT_EQ(COAP_OK, coap_msg_setcode(&msg, code));
        EXPECT_EQ(code, msg.c_code);
    } /* for */
}

TEST_F(CoapMisc, coapMsgSetPayload) {
    const uint8_t payload[] =
        "THIS IS A TEST PAYLOAD TO CHECK IF "
        "PAYLOAD ASSIGNMENT IS BEING DONE OK";
    for (size_t l = 0; l < sizeof payload; ++l) {
        EXPECT_EQ(COAP_OK,
                coap_msg_setpayload(
                    &msg,
                    payload, l));
        EXPECT_EQ(msg.c_pldlen, l);
        EXPECT_EQ(msg.c_plddat, payload);
        EXPECT_EQ(COAP_OK,
                coap_msg_setpayload(
                    &msg,
                    payload + l,
                    sizeof payload - l));
        EXPECT_EQ(sizeof payload - l, msg.c_pldlen);
        EXPECT_EQ(payload + l, msg.c_plddat);
    } /* for */
}

TEST_F(CoapMisc, coapMsgSetTokenNormalSize) {
    uint8_t token[] = { 1, 1, 2, 3, };

    EXPECT_EQ(COAP_OK,
            coap_msg_settoken(
                &msg,
                token,
                sizeof token));
}

TEST_F(CoapMisc, coapMsgSetTokenMaxSize) {
    uint8_t token[] = { 1, 1, 2, 3, 5, 8, 13, 21, };

    EXPECT_EQ(COAP_OK,
            coap_msg_settoken(
                &msg,
                token,
                sizeof token));
}

TEST_F(CoapMisc, coapMsgSetTokenMaxSizeOV) {
    uint8_t token[] = { 1, 1, 2, 3, 5, 8, 13, 21, 34, };

    EXPECT_EQ(COAP_INVALID_PARAMETER,
            coap_msg_settoken(
                &msg,
                token,
                sizeof token));
}

TEST_F(CoapMisc, coapMsgSetMsgid) {
    const uint16_t msgid = 0x1234;
    EXPECT_EQ(COAP_OK,
            coap_msg_setmsgid(
                &msg,
                msgid));
    EXPECT_EQ(msgid, msg.c_msgid);
}
