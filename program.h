//
// Created by Lenovo on 2021/7/2.
//

#ifndef REMAKE_RISICV_PROGRAM_H
#define REMAKE_RISICV_PROGRAM_H

#include <string>
#include <iostream>
#include "my_tools.h"
using namespace std;
class program{
public:
    unsigned program_count=0;
    unsigned reg[50]={0u};
    unsigned mem[500000]={0u};
    bool is_stop=false;
    program()=default;
    ~program()=default;

    void input()
    {
        string input_token,string_mid;
        unsigned int start_address;
        unsigned int single_introduction;
        while (cin>>input_token)
        {
            if (input_token[0]=='#'||input_token[0]==EOF)break;//仅用于调试代码
            if (input_token[0]=='@')
            {
                start_address=get_address(input_token, true);
            } else//输入指令
            {
                single_introduction=get_address_two(input_token);
                mem[start_address]=single_introduction;
                for (unsigned int i=1;i<=3;i++)
                {
                    cin>>input_token;
                    single_introduction=get_address_two(input_token);
                    mem[start_address+i]=single_introduction;
                }
                start_address+=4u;
            }
        }
    }

    struct buffer{
        bool is_null=true;
        unsigned now_instruction=0u;
        unsigned func=0u;
        unsigned opcode=0u;
        bool is_write_to_reg=false;
        pair<bool,int> is_write_to_mem=make_pair(false,0);
        pair<bool,int> is_read_from_mem=make_pair(false,0);
        bool is_unsign=false;
        unsigned write_to_reg=0u;
        unsigned write_to_mem[4]={0u};
        unsigned rs1=0u;
        unsigned rs2=0u;
        unsigned reg_rs1=0u;
        unsigned reg_rs2=0u;
        unsigned rd=0u;
        unsigned index=0u;
       // unsigned imm=0u;
        unsigned pipeline_pc=0;
        void clear(){
            is_null=true;
            is_write_to_reg=false;
            is_write_to_mem.first=false;
            is_read_from_mem.first=false;
        }
    };

    buffer buffers[5];

    bool check(){
        if (is_stop&&buffers[1].is_null&&buffers[2].is_null&&buffers[3].is_null&&buffers[4].is_null)return true;
        return false;
    }

    void run()
    {
        input();
        buffers[0].is_null=false;
        int fuck=20;
        while (true)
        {
           // reg[0]=0u;
            WB();
           //reg[0]=0u;
            MEM();
            //reg[0]=0u;
            EXE();
            //reg[0]=0u;
            ID();
            IF();
            if (check())break;

        }
        cout<<(reg[10]&255u)<<endl;
    }


    void IF()
    {
        if (is_stop)return;
        buffers[0].is_null=true;
        unsigned now_instruction=get_now_instruction(program_count);
        buffers[1].is_null=false;
        buffers[1].now_instruction=now_instruction;
        buffers[1].pipeline_pc=program_count;
    }

    void ID()
    {
        //先写个faker，再写个真的
        if (buffers[1].is_null)return;

        //cout<<"ID"<<endl;

        buffers[1].is_null=true;
        unsigned now_instruction=buffers[1].now_instruction;
        if (now_instruction==267388179u){is_stop=true;return;}
        buffers[2].is_null=false;
        unsigned opcode=get_opcode(now_instruction);
        unsigned func=get_func(now_instruction);
        unsigned rs1=get_rs1(now_instruction);
        unsigned rs2=get_rs2(now_instruction);
        unsigned reg_rs1=reg[rs1];
        unsigned reg_rs2=reg[rs2];
        unsigned rd=get_rd(now_instruction);
        buffers[2].opcode=opcode;
        buffers[2].func=func;
        buffers[2].rs1=rs1;
        buffers[2].rs2=rs2;
        buffers[2].reg_rs1=reg_rs1;
        buffers[2].reg_rs2=reg_rs2;
        buffers[2].rd=rd;
        buffers[2].pipeline_pc=buffers[1].pipeline_pc;
        buffers[2].now_instruction=buffers[1].now_instruction;


        if (buffers[4].is_write_to_reg){
            if (buffers[4].rd==buffers[2].rs1){
                reg_rs1=buffers[2].reg_rs1=buffers[4].write_to_reg;
            }
            if (buffers[4].rd==buffers[2].rs2){
                reg_rs2=buffers[2].reg_rs2=buffers[4].write_to_reg;
            }
        }

        if (buffers[3].is_write_to_reg){
            if (buffers[3].rd==buffers[2].rs1){
                if (!buffers[3].is_read_from_mem.first) {
                    reg_rs1 = buffers[2].reg_rs1 = buffers[3].write_to_reg;
                } else {
                    reg_rs1=buffers[2].reg_rs1 =little_read();
                }
            }
            if (buffers[3].rd==buffers[2].rs2){
                if (!buffers[3].is_read_from_mem.first) {
                    reg_rs2 = buffers[2].reg_rs2 = buffers[3].write_to_reg;
                } else {
                    reg_rs2=buffers[2].reg_rs2=little_read();
                }
            }
        }


        //然后跳转
        if (opcode==111u){//jal
            //imm用的时候再存储吧
            //cout<<"jal"<<endl;
            unsigned imm=get_J_imm(now_instruction);
            program_count+=imm;
            return;
        }

        if (opcode==103u)//jalr
        {
            unsigned imm=get_I_imm(now_instruction);
            program_count=(imm+reg_rs1)&(~(1u));
            return;
        }

        if (opcode==99u)
        {
           // cout<<"B"<<endl;
            unsigned imm=get_B_imm(now_instruction);
            program_count+=imm;
            return;
        }

        //如果不是跳转指令
        program_count+=4u;
    }

    void EXE()
    {
        if (buffers[2].is_null)return;
        buffers[2].is_null=true;
//        unsigned forward_rd=buffers[3].rd;
//        unsigned forward_write_to_rd=buffers[3].write_to_reg;

//        if (buffers[0].is_write_to_reg)
//        {
//            if (buffers[0].rd==buffers[2].rs1){
//                buffers[2].reg_rs1=buffers[0].write_to_reg;
//            }
//            if (buffers[0].rd==buffers[2].rs2){
//                buffers[2].reg_rs2=buffers[0].write_to_reg;
//            }
//        }
//
//        //不肯定是对的
//        if (buffers[4].is_write_to_reg){
//            if (buffers[4].rd==buffers[2].rs1){
//                buffers[2].reg_rs1=buffers[4].write_to_reg;
//            }
//            if (buffers[4].rd==buffers[2].rs2){
//                buffers[2].reg_rs2=buffers[4].write_to_reg;
//            }
//        }



        buffers[3]=buffers[2];
        buffers[3].is_null= false;
        buffers[3].is_write_to_reg=false;
        buffers[3].is_write_to_mem.first=false;
        buffers[3].is_read_from_mem.first=false;
        unsigned opcode=buffers[2].opcode;
        unsigned now_introduction=buffers[2].now_instruction;
        unsigned func=buffers[2].func;
        unsigned rs1=buffers[2].rs1;
        unsigned rs2=buffers[2].rs2;
        unsigned reg_rs1=buffers[2].reg_rs1;
        unsigned reg_rs2=buffers[2].reg_rs2;
        unsigned imm;
        unsigned rd=buffers[2].rd;
//        if (rd==1){
//            cout<<"opcode=="<<buffers[2].opcode<<endl;
//            cout<<"func=="<<buffers[2].func<<endl;
//        }


//        cout<<"func_=="<<buffers[2].func<<endl;
//        cout<<"func=="<<get_func(now_introduction)<<endl;

        if (opcode==55u){//lui
            imm=get_U_imm(now_introduction);
            buffers[3].is_write_to_reg=true;
            buffers[3].write_to_reg=imm;
            if (buffers[3].rd==0){buffers[3].is_write_to_reg=false;}
            return;
        }
        if (opcode==23u){//auipc
            imm=get_U_imm(now_introduction);
            buffers[3].is_write_to_reg=true;
            buffers[3].write_to_reg=buffers[3].pipeline_pc+imm;
            if (buffers[3].rd==0)buffers[3].is_write_to_reg=false;
            return;
        }
        if (opcode==111u||opcode==103u){//jal
           buffers[3].is_write_to_reg=true;
           buffers[3].write_to_reg=buffers[3].pipeline_pc+4u;
            if (buffers[3].rd==0)buffers[3].is_write_to_reg=false;
            return;
        }
        if (opcode==99u){//B
            imm=get_B_imm(now_introduction);
           if (func==0u){//beq
               if (reg_rs1!=reg_rs2){
                   program_count=buffers[3].pipeline_pc+4u;
                   buffers[1].clear();
                   return;
               }
           }

           if (func==1u){//bne
               if (reg_rs1==reg_rs2){
                   program_count=buffers[3].pipeline_pc+4u;
                   buffers[1].clear();
                   return;
               }
           }

            if (func==4u){//blt
                if (int(reg_rs1)>=int(reg_rs2)){
                    program_count=buffers[3].pipeline_pc+4u;
                    buffers[1].clear();
                    return;
                }
            }

            if (func==5u){//bge
                if (int(reg_rs1)<int(reg_rs2)){
                    program_count=buffers[3].pipeline_pc+4u;
                    buffers[1].clear();
                    return;
                }
            }

            if (func==6u){//bltu
                if ((reg_rs1)>=(reg_rs2)){
                    program_count=buffers[3].pipeline_pc+4u;
                    buffers[1].clear();
                    return;
                }
            }

            if (func==7u){//bgeu
                if ((reg_rs1)<(reg_rs2)){
                    program_count=buffers[3].pipeline_pc+4u;
                    buffers[1].clear();
                    return;
                }
            }

        }
        if (opcode==3u){//L
            buffers[3].is_write_to_reg=true;
            buffers[3].is_read_from_mem.first=true;
            imm=get_I_imm(now_introduction);
            if (buffers[3].rd==0){buffers[3].is_write_to_reg=false;buffers[3].is_read_from_mem.first=false;return;}
            if (func==0u){//lb
                buffers[3].index=imm+reg_rs1;
                buffers[3].is_read_from_mem.second=1;
                buffers[3].is_unsign=false;
                return;
            }
            if (func==1u){//lh
                buffers[3].index=imm+reg_rs1;
                buffers[3].is_read_from_mem.second=2;
                buffers[3].is_unsign=false;
                return;
            }
            if (func==2u){//lw
                buffers[3].index=imm+reg_rs1;



                buffers[3].is_read_from_mem.second=4;
                buffers[3].is_unsign=false;
                return;
            }
            if (func==4u){//lbu
                buffers[3].index=imm+reg_rs1;
                buffers[3].is_read_from_mem.second=1;
                buffers[3].is_unsign=true;
                return;
            }
            if (func==5u){//lhu
                buffers[3].index=imm+reg_rs1;
                buffers[3].is_read_from_mem.second=2;
                buffers[3].is_unsign=true;
                return;
            }
        }
        if (opcode==35u){//s
            imm=get_S_imm(now_introduction);
            buffers[3].is_write_to_mem.first=true;
            buffers[3].index=reg_rs1+imm;
            if (func==0u){//sb
                buffers[3].is_write_to_mem.second=1;
                unsigned value=reg_rs2;
                value=value&255u;
                buffers[3].write_to_mem[0]=value;
                return;
            }
            if (func==1u){//sh
                buffers[3].is_write_to_mem.second=2;
                unsigned value1=reg_rs2&65535u;
                unsigned value2=value1&255u;
                value1=value1>>8u;
                buffers[3].write_to_mem[0]=value2;
                buffers[3].write_to_mem[1]=value1;
                return;
            }
            if (func==2u){//sw
                buffers[3].is_write_to_mem.second=2;
                unsigned value1=reg_rs2;
                unsigned value4=value1&255u;
                value1=value1>>8u;
                unsigned value3=value1&255u;
                value1=value1>>8u;
                unsigned value2=value1&255u;
                value1=value1>>8u;
                buffers[3].write_to_mem[0]=value4;
                buffers[3].write_to_mem[1]=value3;
                buffers[3].write_to_mem[2]=value2;
                buffers[3].write_to_mem[3]=value1;
                return;
            }
        }
        if (opcode==19u){
            if (buffers[3].rd==0){buffers[3].is_write_to_reg=false;return;}
            if (func==0u||func==2U||func==3u||func==4U||func==6u||func==7u)
            {
                imm=get_I_imm(now_introduction);
                if (func==0u){//addi
                    buffers[3].is_write_to_reg=true;
                    buffers[3].write_to_reg=reg_rs1+imm;
                    return;
                }
                if (func==2u){//slti
                    buffers[3].is_write_to_reg=true;
                    if (int(reg_rs1)<int(imm))buffers[3].write_to_reg=1u;
                    else buffers[3].write_to_reg=0u;
                    return;
                }
                if (func==3u){//sltiu
                    buffers[3].is_write_to_reg=true;
                    if ((reg_rs1)<(imm))buffers[3].write_to_reg=1u;
                    else buffers[3].write_to_reg=0u;
                    return;
                }
                if (func==4u){//xori
                    buffers[3].is_write_to_reg=true;
                    buffers[3].write_to_reg=reg_rs1^imm;
                    return;
                }
                if (func==6u){//ori
                    buffers[3].is_write_to_reg=true;
                    buffers[3].write_to_reg=reg_rs1|imm;
                    return;
                }
                if (func==7u){//andi
                    buffers[3].is_write_to_reg=true;
                    buffers[3].write_to_reg=reg_rs1&imm;
                    return;
                }
            }
            if (func==1u){//slli
                buffers[3].is_write_to_reg=true;
                buffers[3].write_to_reg=reg_rs1<<rs2;
                return;
            }
            if (func==5u){
                //srli
                if (get_30_bit(now_introduction)==0u) {
                    buffers[3].is_write_to_reg = true;
                    buffers[3].write_to_reg = reg_rs1 >> rs2;
                    return;
                }
                //srai
                unsigned head,value;
                if ((reg_rs1>>31u)==0u)head=0u;
                else head=1u;
                head=head<<31u;
                for (unsigned int i=1;i<=rs2;i++)
                {
                    value=(reg_rs1>>1u)|head;
                }
                buffers[3].write_to_reg=value;
                buffers[3].is_write_to_reg=true;
                return;
            }
        }
        if (opcode==51u){
            buffers[3].is_write_to_reg=true;//应该是
            if (buffers[3].rd==0){buffers[3].is_write_to_reg=false;return;}
            if (func==0u){
                if (get_30_bit(now_introduction)==0u){//add
                    buffers[3].write_to_reg=reg_rs1+reg_rs2;
                    return;
                }
                //sub
                buffers[3].write_to_reg=reg_rs1-reg_rs2;
                return;
            }
            if (func==1u){//sll
                buffers[3].write_to_reg=reg_rs1<<reg_rs2;
                return;
            }
            if (func==2u){//slt
                if (int(reg_rs1)<int(reg_rs2))buffers[3].write_to_reg=1u;
                else buffers[3].write_to_reg=0u;
                return;
            }
            if (func==3u){//sltu
                if ((reg_rs1)<(reg_rs2))buffers[3].write_to_reg=1u;
                else buffers[3].write_to_reg=0u;
                return;
            }
            if (func==4u){//xor
                buffers[3].write_to_reg=reg_rs1^reg_rs2;
                return;
            }
            if (func==5u){
                if (get_30_bit(now_introduction)==0u){//srl
                    buffers[3].write_to_reg=reg_rs1>>(reg_rs2&(31u));
                    return;
                }
                //sra
                unsigned value;
                unsigned head;
                if ((reg_rs1>>31u)==0u)head=0u;
                else head=1u;
                head=head<<31u;
                unsigned index=reg_rs2&(31u);
                for (unsigned int i=1;i<=index;i++)
                {
                    value=(reg_rs1>>1u)|head;
                }
                buffers[3].write_to_reg=value;
                return;
            }
            if (func==6u){//or
                buffers[3].write_to_reg=reg_rs1|reg_rs2;
                return;
            }
            if (func==7u){//and
                buffers[3].write_to_reg=reg_rs1&reg_rs2;
            }
        }
    }

    void MEM()
    {
        if (buffers[3].is_null)return;
        buffers[3].is_null=true;
        buffers[4]=buffers[3];
        buffers[4].is_null=false;

        if (buffers[3].is_write_to_mem.first)
        {
            if (buffers[3].is_write_to_mem.second==1) {
                unsigned index = buffers[3].index;
                mem[index] = buffers[3].write_to_mem[0];
                return;
            }
            if (buffers[3].is_write_to_mem.second==2)
            {
                unsigned index=buffers[3].index;
                mem[index] = buffers[3].write_to_mem[0];
                mem[index+1] = buffers[3].write_to_mem[1];
                return;
            }
            if (buffers[3].is_write_to_mem.second==4)
            {
                unsigned index=buffers[3].index;
                mem[index] = buffers[3].write_to_mem[0];
                mem[index+1] = buffers[3].write_to_mem[1];
                mem[index+2] = buffers[3].write_to_mem[2];
                mem[index+3] = buffers[3].write_to_mem[3];
                return;
            }
        }

        if (buffers[3].is_read_from_mem.first)
        {
            unsigned index=buffers[3].index;
            unsigned value;
            if (buffers[3].is_read_from_mem.second==1){
                if (!buffers[3].is_unsign){
                     value=mem[index];
                    if (get_7_bit(value)==1){
                        value=((16777215u)<<8u)|value;
                    }
                    buffers[4].write_to_reg=value;
                } else
                {
                    value=mem[index];
                    buffers[4].write_to_reg=value;
                }
                return;
            }
            if (buffers[3].is_read_from_mem.second==2){
                if (!buffers[3].is_unsign){
                    value=mem[index+1u];
                    value=value*(256u)+mem[index];
                    if (get_15_bit(value)==1){
                        value=((65535u)<<16u)|value;
                    }
                    buffers[4].write_to_reg=value;
                } else
                {
                    value=mem[index+1u];
                    value=value*(256u)+mem[index];
                    buffers[4].write_to_reg=value;
                }
                return;
            }
            if (buffers[3].is_read_from_mem.second==4){
                value=mem[index+3u];
                for ( int i=2;i>=0;i--)
                {
                    value=value*256u+mem[index+i];
                }
                buffers[4].write_to_reg=value;
            }
        }

    }

    void WB()
    {
        if (buffers[4].is_null)return;
        buffers[4].is_null=true;
        buffers[0]=buffers[4];
        buffers[0].is_null=false;

        if (buffers[4].is_write_to_reg)
        {
            reg[buffers[4].rd]=buffers[4].write_to_reg;
        }
    }

    unsigned get_now_instruction(unsigned pc)
    {
        unsigned int now_introduction=0u;
        for (unsigned int i=0;i<=3;i++)
        {
            now_introduction=now_introduction*256+mem[program_count+3-i];
        }
        return now_introduction;
    }

    unsigned little_read(){
        unsigned index=buffers[3].index;
        unsigned value;
        if (buffers[3].is_read_from_mem.second==1){
            if (!buffers[3].is_unsign){
                value=mem[index];
                if (get_7_bit(value)==1){
                    value=((16777215u)<<8u)|value;
                }
                return value;
            } else
            {
                value=mem[index];
                return value;
            }

        }
        if (buffers[3].is_read_from_mem.second==2){
            if (!buffers[3].is_unsign){
                value=mem[index+1u];
                value=value*(256u)+mem[index];
                if (get_15_bit(value)==1){
                    value=((65535u)<<16u)|value;
                }
                return value;
            } else
            {
                value=mem[index+1u];
                value=value*(256u)+mem[index];
                   return value;
            }

        }
        if (buffers[3].is_read_from_mem.second==4){
            value=mem[index+3u];
            for ( int i=2;i>=0;i--)
            {
                value=value*256u+mem[index+i];
            }
            return value;
        }
    }

};




#endif //REMAKE_RISICV_PROGRAM_H
