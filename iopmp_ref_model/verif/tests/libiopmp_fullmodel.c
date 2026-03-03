#include "iopmp.h"
#include "config.h"
#include "test_utils.h"

#include "libiopmp.h"

// Declarations
iopmp_trans_req_t iopmp_trans_req;
iopmp_trans_rsp_t iopmp_trans_rsp;
err_info_t err_info_temp;

// Create IOPMP instance
iopmp_dev_t iopmp_dev = {0};
iopmp_cfg_t cfg = {0};
uint8_t intrpt;

/* Override libiopmp IO functions */
uint32_t io_read32(uintptr_t addr)
{
    return read_register(&iopmp_dev, addr, 4);
}

void io_write32(uintptr_t addr, uint32_t val)
{
    return write_register(&iopmp_dev, addr, val, 4);
}

int main(void)
{
    IOPMP_t iopmp = {0};
    enum iopmp_error ret;
    uint32_t val_u32;
    uint64_t val_u64;
    struct iopmp_entry entries[8] = {0};

    FAIL_IF(create_memory(1) < 0)

    // Configure and reset IOPMP device
    cfg.vendor = 1;
    cfg.specver = 1;
    cfg.impid = 0;
    cfg.no_err_rec = false;
    cfg.md_num = 63;
    cfg.addrh_en = true;
    cfg.tor_en = true;
    cfg.rrid_num = 64;
    cfg.entry_num = 512;
    cfg.prio_entry = 16;
    cfg.prio_ent_prog = false;
    cfg.non_prio_en = true;
    cfg.msi_en = true;
    cfg.peis = true;
    cfg.pees = true;
    cfg.sps_en= true;
    cfg.stall_en = true;
    cfg.mfr_en = true;
    cfg.mdcfg_fmt = 0;
    cfg.srcmd_fmt = 0;
    cfg.md_entry_num = 0;
    cfg.xinr = false;
    cfg.no_x = false;
    cfg.no_w = false;
    cfg.rrid_transl_en = true;
    cfg.rrid_transl_prog = false;
    cfg.rrid_transl = 48;
    cfg.entryoffset = 0x2000;
    cfg.granularity = MIN_GRANULARITY;
    cfg.imp_mdlck = true;
    cfg.imp_err_reqid_eid = true;
    cfg.imp_rridscp = true;
    cfg.imp_stall_buffer = true;
    cfg.stall_buffer_size = 32;

    /* Start unit tests */

#if (SRC_ENFORCEMENT_EN == 0)
    bool val_bool;
    IOPMP_ERR_REPORT_t err_report = {0};

    START_TEST("Test OFF - Read Access permissions");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4,
                             IOPMP_ENTRY_FORCE_OFF | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single OFF entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_OFF | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 364, 0, 0, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test OFF - Write Access permissions");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4,
                             IOPMP_ENTRY_FORCE_OFF | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single OFF entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_OFF | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 364, 0, 0, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test OFF - Instruction Fetch permissions");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4,
                             IOPMP_ENTRY_FORCE_OFF | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single OFF entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_OFF | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 364, 0, 0, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test OFF - UNKNOWN RRID ERROR");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4,
                             IOPMP_ENTRY_FORCE_OFF | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single OFF entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_OFF | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(70, 364, 0, 0, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, UNKNOWN_RRID);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST_IF(iopmp_dev.reg_file.hwcfg0.tor_en,
                  "Test TOR - Partial hit on a priority rule error",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 2, 0, 368,
                             IOPMP_ENTRY_FORCE_TOR | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 2);  // Two TOR entries
    FAIL_IF(entries[0].addr != 0);
    FAIL_IF(entries[0].cfg != IOPMP_ENTRY_R);
    FAIL_IF(entries[1].addr != (368 >> 2));
    FAIL_IF(entries[1].cfg != (IOPMP_ENTRY_A_TOR | IOPMP_ENTRY_R));
    ret = iopmp_set_entries(&iopmp, entries, 0, 2);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 364, 0, 3, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, PARTIAL_HIT_ON_PRIORITY);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg0.tor_en, "Test TOR - 4Byte Read Access",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 2, 0, 368,
                             IOPMP_ENTRY_FORCE_TOR | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 2);  // Two TOR entries
    FAIL_IF(entries[0].addr != 0);
    FAIL_IF(entries[0].cfg != IOPMP_ENTRY_R);
    FAIL_IF(entries[1].addr != (368 >> 2));
    FAIL_IF(entries[1].cfg != (IOPMP_ENTRY_A_TOR | IOPMP_ENTRY_R));
    ret = iopmp_set_entries(&iopmp, entries, 0, 2);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 364, 0, 2, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg0.tor_en && iopmp_dev.reg_file.hwcfg2.sps_en,
                  "Test TOR - 4Byte Read Access with SRCMD_R not set",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 2, 0, 368,
                             IOPMP_ENTRY_FORCE_TOR | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 2);  // Two TOR entries
    FAIL_IF(entries[0].addr != 0);
    FAIL_IF(entries[0].cfg != IOPMP_ENTRY_R);
    FAIL_IF(entries[1].addr != (368 >> 2));
    FAIL_IF(entries[1].cfg != (IOPMP_ENTRY_A_TOR | IOPMP_ENTRY_R));
    ret = iopmp_set_entries(&iopmp, entries, 0, 2);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 364, 0, 2, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg0.tor_en && !iopmp_dev.reg_file.hwcfg2.sps_en,
                  "Test TOR - 4Byte Read Access, SRCMD_R not set, SPS disabled",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 2, 0, 368,
                             IOPMP_ENTRY_FORCE_TOR | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 2);  // Two TOR entries
    FAIL_IF(entries[0].addr != 0);
    FAIL_IF(entries[0].cfg != IOPMP_ENTRY_R);
    FAIL_IF(entries[1].addr != (368 >> 2));
    FAIL_IF(entries[1].cfg != (IOPMP_ENTRY_A_TOR | IOPMP_ENTRY_R));
    ret = iopmp_set_entries(&iopmp, entries, 0, 2);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 364, 0, 2, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg0.tor_en, "Test TOR - 4Byte AMO Write Access",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 2, 0, 368,
                             IOPMP_ENTRY_FORCE_TOR | IOPMP_ENTRY_W | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 2);  // Two TOR entries
    FAIL_IF(entries[0].addr != 0);
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_W | IOPMP_ENTRY_R));
    FAIL_IF(entries[1].addr != (368 >> 2));
    FAIL_IF(entries[1].cfg != (IOPMP_ENTRY_A_TOR | IOPMP_ENTRY_W | IOPMP_ENTRY_R));
    ret = iopmp_set_entries(&iopmp, entries, 0, 2);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 364, 0, 2, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg0.tor_en,
                  "Test TOR - 4Byte Non-AMO Write Access",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 2, 0, 368,
                             IOPMP_ENTRY_FORCE_TOR | IOPMP_ENTRY_W, 0);
    FAIL_IF(ret != 2);  // Two TOR entries
    FAIL_IF(entries[0].addr != 0);
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_W));
    FAIL_IF(entries[1].addr != (368 >> 2));
    FAIL_IF(entries[1].cfg != (IOPMP_ENTRY_A_TOR | IOPMP_ENTRY_W));
    ret = iopmp_set_entries(&iopmp, entries, 0, 2);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 364, 0, 2, WRITE_ACCESS, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg0.tor_en, "Test TOR - 4Byte Write Access",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 2, 0, 368,
                             IOPMP_ENTRY_FORCE_TOR | IOPMP_ENTRY_W, 0);
    FAIL_IF(ret != 2);  // Two TOR entries
    FAIL_IF(entries[0].addr != 0);
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_W));
    FAIL_IF(entries[1].addr != (368 >> 2));
    FAIL_IF(entries[1].cfg != (IOPMP_ENTRY_A_TOR | IOPMP_ENTRY_W));
    ret = iopmp_set_entries(&iopmp, entries, 0, 2);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 364, 0, 2, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST("Test NA4 - 4Byte Read Access");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4, IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NA4 | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 364, 0, 2, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Read Access error");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4, 0, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != IOPMP_ENTRY_A_NA4);
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 364, 0, 2, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.sps_en,
                  "Test NA4 - 4Byte No SPS Read Access error",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 32, 0x0, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x0);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4, IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NA4 | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 364, 0, 2, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST("Test NA4 - 4Byte Write Access");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4, IOPMP_ENTRY_W | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NA4 | IOPMP_ENTRY_W | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 364, 0, 2, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte Non-AMO Write Access");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4, IOPMP_ENTRY_W, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NA4 | IOPMP_ENTRY_W));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 364, 0, 2, WRITE_ACCESS, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Write Access error");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4, IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NA4 | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 364, 0, 2, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.sps_en,
                  "Test NA4 - 4Byte No SPS Write Access error",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 32, 0x0, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x0);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4, IOPMP_ENTRY_W | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NA4 | IOPMP_ENTRY_W | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 364, 0, 2, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST("Test NA4 - 4Byte Execute Access");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4,
                             IOPMP_ENTRY_X | IOPMP_ENTRY_W | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NA4 | IOPMP_ENTRY_X | IOPMP_ENTRY_W | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 364, 0, 2, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - 4Byte No Execute Access");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4,
                             IOPMP_ENTRY_W | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NA4 | IOPMP_ENTRY_W | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 364, 0, 2, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.sps_en,
                  "Test NA4 - 4Byte No SPS.X, Execute Access",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 32, 0x0, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x0);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4,
                             IOPMP_ENTRY_X | IOPMP_ENTRY_W | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NA4 | IOPMP_ENTRY_X | IOPMP_ENTRY_W | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 364, 0, 2, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST("Test NA4 - 8Byte Access error");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4, IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NA4 | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 364, 0, 3, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, PARTIAL_HIT_ON_PRIORITY);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NA4 - For exact 4 Byte error");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 364, 4, IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NA4 entry
    FAIL_IF(entries[0].addr != (364 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NA4 | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 368, 0, 0, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte read access");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte read access error");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, 0, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != IOPMP_ENTRY_A_NAPOT);
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, READ_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte write access error");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, 0, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != IOPMP_ENTRY_A_NAPOT);
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_WRITE_ACCESS);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte write access");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8,
                             IOPMP_ENTRY_W | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_W | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Non-AMO write access");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_W, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_W));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, WRITE_ACCESS, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Instruction access error");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, 0, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != IOPMP_ENTRY_A_NAPOT);
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test NAPOT - 8 Byte Instruction access");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.non_prio_en,
                  "Test NAPOT - 8 Byte Instruction access for non-priority Entry",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 31, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 31, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 17;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 17);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 296, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (296 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x10, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x10);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 32, 0x10, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x10);
    val_u32 = 25;
    ret = iopmp_set_md_entry_association(&iopmp, 4, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 25);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, 0, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != IOPMP_ENTRY_A_NAPOT);
    ret = iopmp_set_entry(&iopmp, entries, 18);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 20);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST("Test NAPOT - 8 Byte Instruction access when xinr=1");
    cfg.xinr = true;
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, 0, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != IOPMP_ENTRY_A_NAPOT);
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    iopmp_trans_req.perm = READ_ACCESS; // Since xinr=1, we expect ttype="Read access" in CHECK_IOPMP_TRANS()
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_READ_ACCESS);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    cfg.xinr = false;
    END_TEST();

    START_TEST_IF(iopmp_dev.imp_mdlck, "Test MDLCK, updating locked srcmd_en field",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_u64 = 0x8;
    ret = iopmp_lock_md(&iopmp, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.imp_mdlck, "Test MDLCK, updating unlocked srcmd_en field",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_u64 = 0x4;
    ret = iopmp_lock_md(&iopmp, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x4);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST("Test MDCFG_LCK, updating locked MDCFG field");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_u32 = 4;
    ret = iopmp_lock_mdcfg(&iopmp, &val_u32, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 4);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test MDCFG_LCK, updating unlocked MDCFG field");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_u32 = 2;
    ret = iopmp_lock_mdcfg(&iopmp, &val_u32, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK, updating locked ENTRY field");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_u32 = 4;
    ret = iopmp_lock_entries(&iopmp, &val_u32, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 4);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK, updating unlocked ENTRY field");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_u32 = 4;
    ret = iopmp_lock_entries(&iopmp, &val_u32, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 4);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 5;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 5);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 4);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test SRCMD_EN lock bit, updating locked SRCMD Table");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0, 0, &val_u64, true);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test SRCMD_EN lock bit, updating unlocked SRCMD Table");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 1, 0, 0, &val_u64, true);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 5;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 5);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 4);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST_IF(iopmp_dev.imp_mdlck, "Test MDLCK register lock bit",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_u64 = 0x4;
    ret = iopmp_lock_md(&iopmp, &val_u64, false);   // MD[2] is locked
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x4);
    ret = iopmp_lock_md(&iopmp, &val_u64, true);    // Locking MDLCK register
    FAIL_IF(ret != IOPMP_OK);
    val_u64 = 0x8;
    ret = iopmp_lock_md(&iopmp, &val_u64, false);   // Trying to lock MD[3] but it shouldn't be locked
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST("Test MDCFG_LCK register lock bit");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_u32 = 4;
    ret = iopmp_lock_mdcfg(&iopmp, &val_u32, false);    // MD[0]-MD[3] are locked
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 4);
    ret = iopmp_lock_mdcfg(&iopmp, &val_u32, true);     // MDCFGLCK is locked
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 4);
    val_u32 = 8;
    ret = iopmp_lock_mdcfg(&iopmp, &val_u32, false);    // Updating locked MD's MD[0]-MD[1] are locked
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST("Test Entry_LCK register lock bit");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_u32 = 4;
    ret = iopmp_lock_entries(&iopmp, &val_u32, false);  // ENTRY[0]-ENTRY[3] are locked
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 4);
    ret = iopmp_lock_entries(&iopmp, &val_u32, true);   // ENTRYLCK is locked
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 4);
    val_u32 = 8;
    ret = iopmp_lock_entries(&iopmp, &val_u32, false);  // Updating locked entries but ENTRYLCK was locked
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    END_TEST();

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.mfr_en, "Test MFR Extension",
    // Following the previous test
    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    uint16_t svi = 0;
    uint16_t svw = 0;
    ret = iopmp_mfr_get_sv_window(&iopmp, &svi, &svw);
    FAIL_IF(ret != IOPMP_ERR_NOT_EXIST);
    FAIL_IF(svi != 0);
    FAIL_IF(svw != 0);
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    ret = iopmp_mfr_get_sv_window(&iopmp, &svi, &svw);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(svi != 0);
    FAIL_IF(svw != 4);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.imp_mdlck, "Test MDLCK, updating locked srcmd_enh field",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_u64 = 0x80000000;
    ret = iopmp_lock_md(&iopmp, &val_u64, false);   // MD[31] is locked
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x80000000, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x80000000, 0, &val_u64);
    FAIL_IF(ret != IOPMP_ERR_REG_IS_LOCKED);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 31, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, NOT_HIT_ANY_RULE);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.imp_mdlck, "Test MDLCK, updating unlocked srcmd_enh field",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_u64 = 0x100000000;
    ret = iopmp_lock_md(&iopmp, &val_u64, false);   // MD[32] is locked
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x100000000);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x80000000, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x80000000, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 31, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.peis, "Test Interrupt Suppression is Enabled",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_global_intr(&iopmp, true);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x80000000, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x80000000, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 31, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8,
                             IOPMP_ENTRY_SIXE | IOPMP_ENTRY_R, 0);  // Address Mode is NAPOT, with read permission and ixe suppression
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_SIXE | IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((intrpt == 1)); // Interrupt is suppressed
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST("Test Interrupt Suppression is disabled");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_global_intr(&iopmp, true);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x80000000, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x80000000, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 31, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((intrpt == 0)); // Interrupt is not suppressed
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.pees, "Test Error Suppression is Enabled",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_bool = true;
    ret = iopmp_set_global_err_resp(&iopmp, &val_bool);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_bool != true);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x80000000, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x80000000, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 31, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8,
                             IOPMP_ENTRY_SEXE | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_SEXE | IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_R));    // Address Mode is NAPOT, with read permission and exe suppression
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_SUCCESS));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != USER));
    error_record_chk(&iopmp_dev, NO_ERROR, INSTR_FETCH, 360, 0);
    ret = iopmp_capture_error(&iopmp, &err_report, false);
    FAIL_IF(ret != IOPMP_ERR_NOT_EXIST);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.pees,
                  "Test Error Suppression is Enabled but rs is zero",
    // Receiver Port Signals
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x80000000, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x80000000, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 31, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8,
                             IOPMP_ENTRY_SEXE | IOPMP_ENTRY_R, 0);  // Address Mode is NAPOT, with read permission and exe suppression
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_SEXE | IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_SUCCESS));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != USER));
    error_record_chk(&iopmp_dev, NO_ERROR, INSTR_FETCH, 360, 0);
    ret = iopmp_capture_error(&iopmp, &err_report, false);
    FAIL_IF(ret != IOPMP_ERR_NOT_EXIST);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST("Test Error Suppression is disabled");
    // Receiver Port Signals
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x80000000, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x80000000, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 31, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_ERROR));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != 0));
    error_record_chk(&iopmp_dev, ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360, 1);
    ret = iopmp_capture_error(&iopmp, &err_report, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF((err_report.etype != IOPMP_ERRINFO_ETYPE_INST_FETCH));
    FAIL_IF((err_report.ttype != IOPMP_ERRINFO_TTYPE_INST_FETCH));
    FAIL_IF((err_report.addr != (360 >> 2)));
    FAIL_IF((err_report.rrid != 2));
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.peis && iopmp_dev.reg_file.hwcfg2.pees,
                  "Test Interrupt and Error Suppression is Enabled",
    // Receiver Port Signals
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_global_intr(&iopmp, true);
    FAIL_IF(ret != IOPMP_OK);
    val_bool = true;
    ret = iopmp_set_global_err_resp(&iopmp, &val_bool);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_bool != true);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x80000000, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x80000000, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 31, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8,
                             IOPMP_ENTRY_SEXE | IOPMP_ENTRY_SIXE | IOPMP_ENTRY_R, 0);  // Address Mode is NAPOT, with read permission and ixe/exe suppression
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_SEXE | IOPMP_ENTRY_SIXE | IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((intrpt == 1));
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_SUCCESS));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    FAIL_IF((iopmp_trans_rsp.user != USER));
    error_record_chk(&iopmp_dev, ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360, 0);
    ret = iopmp_capture_error(&iopmp, &err_report, false);
    FAIL_IF(ret != IOPMP_ERR_NOT_EXIST);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST("Test Interrupt and Error Suppression is disabled");
    // Receiver Port Signals
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_global_intr(&iopmp, true);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x80000000, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x80000000, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 31, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((intrpt != 1));
    FAIL_IF((iopmp_trans_rsp.status != IOPMP_ERROR));
    FAIL_IF((iopmp_trans_rsp.rrid != 2));
    error_record_chk(&iopmp_dev, ILLEGAL_INSTR_FETCH, INSTR_FETCH, 360, 1);
    ret = iopmp_capture_error(&iopmp, &err_report, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF((err_report.etype != IOPMP_ERRINFO_ETYPE_INST_FETCH));
    FAIL_IF((err_report.ttype != IOPMP_ERRINFO_TTYPE_INST_FETCH));
    FAIL_IF((err_report.addr != (360 >> 2)));
    FAIL_IF((err_report.rrid != 2));
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.stall_en && iopmp_dev.imp_rridscp, "Stall MD Feature",
    cfg.imp_stall_buffer = true;
    cfg.stall_buffer_size = 32;
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 5, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 5, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);
    val_u64 = 0x8;
    ret = iopmp_stall_transactions_by_mds(&iopmp, &val_u64, false, true);
    FAIL_IF(ret != IOPMP_OK);
    enum iopmp_rridscp_stat stat = {0};
    val_u32 = 5;
    ret = iopmp_stall_cherry_pick_rrid(&iopmp, &val_u32, true, &stat);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(stat != IOPMP_RRIDSCP_STAT_STALLED);

    receiver_port(5, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.rrid_stalled != 1));
    val_u32 = 5;
    ret = iopmp_query_stall_stat_by_rrid(&iopmp, &val_u32, &stat);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(stat != IOPMP_RRIDSCP_STAT_STALLED);
    FAIL_IF((iopmp_trans_rsp.rrid != 5));
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.stall_en && iopmp_dev.imp_rridscp,
                  "Faulting Stalled Transactions Feature",
    cfg.imp_stall_buffer = false;
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    val_bool = true;
    ret = iopmp_set_stall_violation_en(&iopmp, &val_bool);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_bool != true);
    ret = iopmp_set_rrid_md_association(&iopmp, 5, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 5, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_X, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_X));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);
    val_u64 = 0x8;
    ret = iopmp_stall_transactions_by_mds(&iopmp, &val_u64, false, true);
    FAIL_IF(ret != IOPMP_OK);
    enum iopmp_rridscp_stat stat = {0};
    val_u32 = 5;
    ret = iopmp_stall_cherry_pick_rrid(&iopmp, &val_u32, true, &stat);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(stat != IOPMP_RRIDSCP_STAT_STALLED);

    receiver_port(5, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.rrid_stalled == 1));
    val_u32 = 5;
    ret = iopmp_query_stall_stat_by_rrid(&iopmp, &val_u32, &stat);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(stat != IOPMP_RRIDSCP_STAT_STALLED);
    FAIL_IF((iopmp_trans_rsp.rrid != 5));
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, STALLED_TRANSACTION);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    // Reset configuration
    cfg.imp_stall_buffer = true;
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg3.rrid_transl_en, "Test Cascading IOPMP Feature",
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 32, 0x8, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 32, 0x8, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x8);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 3, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8,
                             IOPMP_ENTRY_W | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_W | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    FAIL_IF((iopmp_trans_rsp.rrid_transl != iopmp_dev.reg_file.hwcfg3.rrid_transl));
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.msi_en, "Test MSI Write error",
    uint64_t read_data;
    uint64_t msiaddr64;
    uint16_t msidata;
    reset_iopmp(&iopmp_dev, &cfg);
    bus_error = 0x8000;

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_global_intr(&iopmp, true);
    FAIL_IF(ret != IOPMP_OK);
    val_bool = true;
    ret = iopmp_set_msi_sel(&iopmp, &val_bool);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_bool != true);
    msiaddr64 = 0x8000;
    msidata = 0x8F;
    ret = iopmp_set_msi_info(&iopmp, &msiaddr64, &msidata);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(msiaddr64 != 0x8000);
    FAIL_IF(msidata != 0x8F);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x80000000, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x80000000, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 31, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    bus_error = 0;
    read_memory(0x8000, 4, &read_data);
    FAIL_IF(intrpt == 1);
    FAIL_IF(read_data == 0x8F); // Interrupt is not suppressed
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)

    START_TEST_IF(iopmp_dev.reg_file.hwcfg2.msi_en, "Test MSI",
    uint64_t read_data;
    uint64_t msiaddr64;
    uint16_t msidata;
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_global_intr(&iopmp, true);
    FAIL_IF(ret != IOPMP_OK);
    val_bool = true;
    ret = iopmp_set_msi_sel(&iopmp, &val_bool);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_bool != true);
    msiaddr64 = 0x8000;
    msidata = 0x8F;
    ret = iopmp_set_msi_info(&iopmp, &msiaddr64, &msidata);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(msiaddr64 != 0x8000);
    FAIL_IF(msidata != 0x8F);
    ret = iopmp_set_rrid_md_association(&iopmp, 2, 0x80000000, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    ret = iopmp_sps_set_rrid_md_insn_fetch(&iopmp, 2, 0x80000000, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x80000000);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 31, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8, IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 1);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(2, 360, 0, 3, INSTR_FETCH, 0, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    read_memory(0x8000, 4, &read_data);
    FAIL_IF(intrpt == 1);
    FAIL_IF(read_data != 0x8F); // Interrupt is not suppressed
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_ERROR, ILLEGAL_INSTR_FETCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();)
#endif

    free(memory);

#if (SRC_ENFORCEMENT_EN)
    START_TEST("Test SourceEnforcement Enable Feature");
    reset_iopmp(&iopmp_dev, &cfg);

    ret = iopmp_init(&iopmp, 0, IOPMP_SRCMD_FMT_0, IOPMP_MDCFG_FMT_0,
                     IOPMP_IMPID_NOT_SPECIFIED);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_rrid_md_association(&iopmp, 0, 0x1, 0, &val_u64, false);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x1);
    ret = iopmp_sps_set_rrid_md_read(&iopmp, 0, 0x1, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x1);
    ret = iopmp_sps_set_rrid_md_write(&iopmp, 0, 0x1, 0, &val_u64);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u64 != 0x1);
    val_u32 = 2;
    ret = iopmp_set_md_entry_association(&iopmp, 0, &val_u32);
    FAIL_IF(ret != IOPMP_OK);
    FAIL_IF(val_u32 != 2);
    ret = iopmp_encode_entry(&iopmp, entries, 1, 360, 8,
                             IOPMP_ENTRY_W | IOPMP_ENTRY_R, 0);
    FAIL_IF(ret != 1);  // Single NAPOT entry
    FAIL_IF(entries[0].addr != (360 >> 2));
    FAIL_IF(entries[0].cfg != (IOPMP_ENTRY_A_NAPOT | IOPMP_ENTRY_W | IOPMP_ENTRY_R));
    ret = iopmp_set_entry(&iopmp, entries, 0);
    FAIL_IF(ret != IOPMP_OK);
    ret = iopmp_set_enable(&iopmp);
    FAIL_IF(ret != IOPMP_OK);

    receiver_port(32, 360, 0, 3, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    receiver_port(12, 360, 0, 3, WRITE_ACCESS, 1, &iopmp_trans_req);
    // requestor Port Signals
    iopmp_validate_access(&iopmp_dev, &iopmp_trans_req, &iopmp_trans_rsp, &intrpt);
    CHECK_IOPMP_TRANS(&iopmp_dev, IOPMP_SUCCESS, ENTRY_MATCH);
    write_register(&iopmp_dev, ERR_INFO_OFFSET, 0, 4);
    END_TEST();
#endif

    return 0;
}
