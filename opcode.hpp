#ifndef OPCODE_H
#define OPCODE_H

enum { r = 0, c };
enum { w = 0, b };
enum { pr = 0, po};

// unions for opcodes

struct arith_bf {
	unsigned dst : 3;
	unsigned src : 3;
	unsigned wb : 1;
	unsigned rc : 1;
	unsigned baseOp : 8;
};

union arith_op {
	unsigned short opcode;
	struct arith_bf bf;
};

struct reginit_bf {
	unsigned dst : 3;
	unsigned byte : 8;
	unsigned baseOp : 5;
};

union reginit_op {
	unsigned short opcode;
	struct reginit_bf bf;
};

struct singr_bf {
	unsigned dst : 3;
	unsigned wb : 1;
	unsigned baseOp : 9;
};

union singr_op {
	unsigned short opcode;
	struct singr_bf bf;
};

struct mem_bf {
	unsigned dst : 3;
	unsigned src : 3;
	unsigned wb : 1;
	unsigned inc : 1;
	unsigned dec : 1;
	unsigned prpo : 1;
	unsigned baseOp : 6;
};

union mem_op {
	unsigned short opcode;
	struct mem_bf bf;
};

struct bra10_bf {
	unsigned off : 10;
	unsigned baseOp : 6;
};

union bra10_op {
	unsigned short opcode;
	struct bra10_bf bf;
};

struct bra13_bf {
	unsigned off : 13;
	unsigned baseOp : 3;
};

union bra13_op {
	unsigned short opcode;
	struct bra10_bf bf;
};

struct memr_bf {
	unsigned dst : 3;
	unsigned src : 3;
	unsigned wb : 1;
	unsigned off : 7;
	unsigned baseOp : 2;
};

union memr_op {
	unsigned short opcode;
	struct memr_bf bf;
};

struct svc_bf {
	unsigned sa : 4;
	unsigned baseOp : 12;
};

union svc_op {
	unsigned short opcode;
	struct svc_bf bf;
};

struct cex_bf {
	unsigned f : 3;
	unsigned t : 3;
	unsigned c : 4;
	unsigned baseOp : 6;
};

union cex_op {
	unsigned short opcode;
	struct cex_bf bf;
};

#endif
