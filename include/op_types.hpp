#pragma once

enum class BinOp {
    ADD,    //+
    SUB,    //-
    MUL,    //*
    DIV,    // /
    MOD,    //%
    AND,    //&&
    OR,     //||
    ADD_ASSIGN, //+=
    SUB_ASSIGN, //-=
    MUL_ASSIGN, //*=
    DIV_ASSIGN, // /=
    MOD_ASSIGN, //%=
    EQ_ASSIGN,  //==
    ASSIGN,     //=
    NEQ_ASSIGN, //!=
    LOW,        //<
    HIGH,       //>
    LOW_EQ,     //<=
    HIGH_EQ     //>=
};

enum class UnaryOp {
    NOT
};
