/**
 * RXbyak - Xbyak for Ruby
 * 
 * @author Nakatani Shuyo.
 */

#include <map>
#include <xbyak/xbyak.h>
#include <ruby.h>
#define RB_FUNC(f) reinterpret_cast<VALUE (*)(...)>(f)



/**
 * @brief RXbyak implementation
 */
class RXbyakGenerator : public Xbyak::CodeGenerator {
private:
    typedef std::map<ID, const Xbyak::Reg*> Regmap;
    Regmap regmap;

    const Xbyak::Reg& id2reg(const VALUE& x) {
        Check_Type(x, T_SYMBOL);
        ID id = rb_to_id(x);
        Regmap::const_iterator it = regmap.find(id);
        if (it==regmap.end()) rb_raise(rb_eNameError, "no register name");
        return *(it->second);
    }

    const Xbyak::Reg8& id2reg8(const VALUE& x) {
        const Xbyak::Reg& reg = id2reg(x);
        if (!reg.isREG(8)) rb_raise(rb_eTypeError, "not suitable register");
        return (const Xbyak::Reg8&)reg;
    }
    const Xbyak::Reg16& id2reg16(const VALUE& x) {
        const Xbyak::Reg& reg = id2reg(x);
        if (!reg.isREG(16)) rb_raise(rb_eTypeError, "not suitable register");
        return (const Xbyak::Reg16&)reg;
    }
    const Xbyak::Reg32& id2reg32(const VALUE& x) {
        const Xbyak::Reg& reg = id2reg(x);
        if (!reg.isREG(32)) rb_raise(rb_eTypeError, "not suitable register");
        return (const Xbyak::Reg32&)reg;
    }
    const Xbyak::Mmx& id2mmx(const VALUE& x) {
        const Xbyak::Reg& reg = id2reg(x);
        if (!reg.isMMX() && !reg.isXMM())
            rb_raise(rb_eTypeError, "not suitable register");
        return (const Xbyak::Mmx&)reg;
    }
    const Xbyak::Xmm& id2xmmx(const VALUE& x) {
        const Xbyak::Reg& reg = id2reg(x);
        if (!reg.isXMM()) rb_raise(rb_eTypeError, "not suitable register");
        return (const Xbyak::Xmm&)reg;
    }

    Xbyak::Address ary2address(const VALUE& address) {
        size_t len = RARRAY_LEN(address);
        const VALUE* ary = RARRAY_PTR(address);
        if (len<=0 || len>3) rb_raise(rb_eTypeError, "number of address parameters must be between 1 and 3");

        const Xbyak::Reg32& pointer = id2reg32(ary[0]);
        if (len==1) return ptr[pointer];

        int offset = FIX2INT(ary[1]);
        if (len==2) return ptr[pointer + offset];

        const Xbyak::Reg32 offset_r = id2reg32(ary[2]);
        return ptr[pointer + offset_r * offset];
    }

public:
    RXbyakGenerator() {
        regmap[rb_intern("al")] = &al;
        regmap[rb_intern("bl")] = &bl;
        regmap[rb_intern("cl")] = &cl;
        regmap[rb_intern("dl")] = &dl;
        regmap[rb_intern("ah")] = &ah;
        regmap[rb_intern("bh")] = &bh;
        regmap[rb_intern("ch")] = &ch;
        regmap[rb_intern("dh")] = &dh;

        regmap[rb_intern("ax")] = &ax;
        regmap[rb_intern("bx")] = &bx;
        regmap[rb_intern("cx")] = &cx;
        regmap[rb_intern("dx")] = &dx;
        regmap[rb_intern("sp")] = &sp;
        regmap[rb_intern("bp")] = &bp;
        regmap[rb_intern("si")] = &si;
        regmap[rb_intern("di")] = &di;

        regmap[rb_intern("eax")] = &eax;
        regmap[rb_intern("ebx")] = &ebx;
        regmap[rb_intern("ecx")] = &ecx;
        regmap[rb_intern("edx")] = &edx;
        regmap[rb_intern("esp")] = &esp;
        regmap[rb_intern("ebp")] = &ebp;
        regmap[rb_intern("esi")] = &esi;
        regmap[rb_intern("edi")] = &edi;

        regmap[rb_intern("mm0")] = &mm0;
        regmap[rb_intern("mm1")] = &mm1;
        regmap[rb_intern("mm2")] = &mm2;
        regmap[rb_intern("mm3")] = &mm3;
        regmap[rb_intern("mm4")] = &mm4;
        regmap[rb_intern("mm5")] = &mm5;
        regmap[rb_intern("mm6")] = &mm6;
        regmap[rb_intern("mm7")] = &mm7;

        regmap[rb_intern("xmm0")] = &xmm0;
        regmap[rb_intern("xmm1")] = &xmm1;
        regmap[rb_intern("xmm2")] = &xmm2;
        regmap[rb_intern("xmm3")] = &xmm3;
        regmap[rb_intern("xmm4")] = &xmm4;
        regmap[rb_intern("xmm5")] = &xmm5;
        regmap[rb_intern("xmm6")] = &xmm6;
        regmap[rb_intern("xmm7")] = &xmm7;
    }

    void _mov_reg_long(const VALUE& dist, long x) {
        const Xbyak::Operand& reg1 = id2reg(dist);
        mov(reg1, x);
    }

    void _mov_reg_address(const VALUE& dist, const VALUE& src) {
        const Xbyak::Operand& reg = id2reg(dist);
        const Xbyak::Address addr = ary2address(src);
        mov(reg, addr);
    }

    void _movq_reg_address(const VALUE& dist, const VALUE& src) {
        const Xbyak::Mmx& reg = id2mmx(dist);
        const Xbyak::Address addr = ary2address(src);
        movq(reg, addr);
    }

    void _movq_address_reg(const VALUE& dist, const VALUE& src) {
        const Xbyak::Address addr = ary2address(dist);
        const Xbyak::Mmx& reg = id2mmx(src);
        movq(addr, reg);
    }

    void _mulsd(const VALUE& xmm_dest, const VALUE& xmm_src) {
        const Xbyak::Xmm& dest = id2xmmx(xmm_dest);
        const Xbyak::Xmm& src  = id2xmmx(xmm_src);
        mulsd(dest, src);
    }

    void _ret() {
        ret();
    }
};




//// Ruby extention interface ////

// IA32 operand

extern "C" 
VALUE RXbyak_mov(VALUE self, VALUE a1, VALUE a2) {
    RXbyakGenerator* rx;
    Data_Get_Struct(self, RXbyakGenerator, rx);

    switch (TYPE(a1)) {
    case T_ARRAY:
        switch (TYPE(a2)) {
        case T_ARRAY:
        case T_SYMBOL:
        case T_FIXNUM:
            rb_raise(rb_eStandardError, "not yet support");
            break;
        default:
            rb_raise(rb_eArgError, "hoge");
            break;
        }
        break;
    case T_SYMBOL:
        switch (TYPE(a2)) {
        case T_ARRAY:
            rx->_mov_reg_address(a1, a2);
            break;
        case T_SYMBOL:
            rb_raise(rb_eStandardError, "not yet support");
            break;
        case T_FIXNUM:
            rx->_mov_reg_long(a1, NUM2LONG(a2));
            break;
        default:
            rb_raise(rb_eArgError, "hoge");
            break;
        }
        break;
    default:
        rb_raise(rb_eArgError, "hoge");
        break;
    }
    return Qnil;
}

extern "C" 
VALUE RXbyak_movq(VALUE self, const VALUE a1, const VALUE a2) {
    RXbyakGenerator* rx;
    Data_Get_Struct(self, RXbyakGenerator, rx);

    switch (TYPE(a1)) {
    case T_ARRAY:
        switch (TYPE(a2)) {
        case T_SYMBOL:
            rx->_movq_address_reg(a1, a2);
            break;
        case T_ARRAY:
        case T_FLOAT:
            rb_raise(rb_eStandardError, "not yet support");
            break;
        default:
            rb_raise(rb_eArgError, "hoge");
            break;
        }
        break;
    case T_SYMBOL:
        switch (TYPE(a2)) {
        case T_ARRAY:
            rx->_movq_reg_address(a1, a2);
            break;
        case T_SYMBOL:
        case T_FLOAT:
            rb_raise(rb_eStandardError, "not yet support");
            break;
        default:
            rb_raise(rb_eArgError, "hoge");
            break;
        }
        break;
    default:
        rb_raise(rb_eArgError, "hoge");
        break;
    }
    return Qnil;
}

extern "C" 
VALUE RXbyak_mulsd(VALUE self, VALUE a1, VALUE a2) {
    RXbyakGenerator* rx;
    Data_Get_Struct(self, RXbyakGenerator, rx);
    rx->_mulsd(a1, a2);
    return Qnil;
}

extern "C" 
VALUE RXbyak_ret(VALUE self) {
    RXbyakGenerator* rx;
    Data_Get_Struct(self, RXbyakGenerator, rx);
    rx->_ret();
    return Qnil;
}


// call the generated code

extern "C" 
VALUE RXbyak_call(int argc, const VALUE* argv, VALUE self) {
    RXbyakGenerator* rx;
    Data_Get_Struct(self, RXbyakGenerator, rx);

    //int (*proc)() = (int (*)())rx->getCode();
    //VALUE r = INT2FIX(proc());

    if (argc<2) rb_raise(rb_eTypeError, "too few parameters");
    Check_Type(argv[0], T_FLOAT);
    Check_Type(argv[1], T_FLOAT);

    double result=0;

    void (*proc)(double*, const double*, const double*) = (void (*)(double*, const double*, const double*))rx->getCode();
    proc(&result, &RFLOAT(argv[0])->value, &RFLOAT(argv[1])->value);
    VALUE r = rb_float_new(result);

    return r;
}


// allocator

extern "C" 
void RXbyak_free(void* rx) {
    delete (RXbyakGenerator*)rx;
}

extern "C" 
VALUE RXbyak_alloc(VALUE klass) {
    RXbyakGenerator* rx = new RXbyakGenerator();
    return Data_Wrap_Struct(klass, 0, RXbyak_free, rx);
}


// initialization

extern "C" 
void Init_RXbyak(void) {
    VALUE rb_cRXbyak;
    rb_cRXbyak = rb_define_class("RXbyak", rb_cObject);
    rb_define_alloc_func(rb_cRXbyak, RXbyak_alloc);

    rb_define_method(rb_cRXbyak, "mov", RB_FUNC(RXbyak_mov), 2);
    rb_define_method(rb_cRXbyak, "movq", RB_FUNC(RXbyak_movq), 2);
    rb_define_method(rb_cRXbyak, "mulsd", RB_FUNC(RXbyak_mulsd), 2);
    rb_define_method(rb_cRXbyak, "ret", RB_FUNC(RXbyak_ret), 0);

    rb_define_method(rb_cRXbyak, "call", RB_FUNC(RXbyak_call), -1);
}

