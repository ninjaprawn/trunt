//!!map_string_to_enum(armv8_64_mnemonic, "ADR", "ADRP", "ADD", "MOV", "ADDS", "CMN", "SUB", "SUBS", "CMP", "AND", "ORR", "EOR", "ANDS", "TST", "MOVN", "MOVZ", "MOVK", "SBFM", "ASR", "SBFIZ", "SBFX", "SXTB", "SXTH", "SXTW", "BFM", "BFI", "BFXIL", "UBFM", "LSL", "LSR", "UBFIZ", "UBFX", "UXTB", "UXTH", "EXTR", "ROR", "B", "SVC", "HVC", "SMC", "BRK", "HLT", "DCPS1", "DCPS2", "DCPS3")

typedef struct parent {
    int property1;
    char* property2;
} parent;

typedef struct child inherits parent {
    int property3;
} child;
