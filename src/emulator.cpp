#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <set>
#include <vector>
#include <bitset>
#include "../inc/emulator.hpp"

using namespace std;


reg toLittleEndian(char lowerByte, char higherByte){
  return (short)((higherByte) << 8) | (lowerByte & 0xff);
}

void print_hex_representation(const std::vector<char>& v) {
  for (char c : v) {
    printf("%02x ", (c & 0xff));
  }
  std::puts("");
}

void readState(cpu_registers registers){
  
  bitset<16> binaryNumber(registers.psw);

  cout<< "------------------------------------------------" <<endl;
  cout<<"Emulated processor executed halt instruction"<<endl;
  cout<<"Emulated processor state: psw=0b";
  cout << binaryNumber << endl;
  cout<<"r0=0x";
  printf("%04x ", (static_cast<short>(registers.regs[0])) & 0xffff);
  cout<<"r1=0x";
  printf("%04x ", (static_cast<short>(registers.regs[1])) & 0xffff);
  cout<<"r2=0x";
  printf("%04x ", (static_cast<short>(registers.regs[2])) & 0xffff);
  cout<<"r3=0x";
  printf("%04x ", (static_cast<short>(registers.regs[3])) & 0xffff);
  cout<<endl;
  cout<<"r4=0x";
  printf("%04x ", (static_cast<short>(registers.regs[4])) & 0xffff);
  cout<<"r5=0x";
  printf("%04x ", (static_cast<short>(registers.regs[5])) & 0xffff);
  cout<<"r6=0x";
  printf("%04x ", (static_cast<short>(registers.regs[6])) & 0xffff);
  cout<<"r7=0x";
  printf("%04x ", (static_cast<short>(registers.regs[7])) & 0xffff);
  cout<<endl;
}



int main(int argc, char *argv[]){

  vector<unsigned char> mem;
  mem.reserve(65536);

  ifstream infile(argv[1], std::ios::binary);
  if (infile.is_open()){

    while(!infile.eof()){
      char inputChar;
      infile.read(reinterpret_cast<char*>(&inputChar), sizeof(char));
      mem.push_back(inputChar);
    }
  }
  else{
    cerr<< "Nije moguce uspesno ucitati memorijsku sliku"<<endl;
  }
  infile.close();


  // for(int i = 0; i < 10; i++){
  //   printf("%02x ", static_cast<unsigned int>(mem[i] & 0xff));
  // }


  cpu_registers registers;

  //inicijalizacija na 0
  for(int i = 0; i < 8; i++){
    registers.regs[i] = 0;
  }


  registers.psw = 0;
  registers.regs[pc] = toLittleEndian(mem[0], mem[1]);


  bool running = true;
  uint8_t instrDescr, regsDescr, addrMode, dataHigh, dataLow;

  while(running){

    instrDescr = mem[registers.regs[pc]];
    registers.regs[pc] += 1;

    switch(instrDescr){
      case HALT: {
        running = false;
        break;
      }
      case IRET:{

        registers.psw = toLittleEndian(mem[registers.regs[sp]], mem[registers.regs[sp] + 1]);       
        registers.regs[sp] += 2;

        registers.regs[pc] = ((toLittleEndian(mem[registers.regs[sp]], mem[registers.regs[sp] + 1])));
        registers.regs[sp] += 2;

        
        break;       
      }
      case RET:{

        registers.regs[pc] = ((toLittleEndian(mem[registers.regs[sp]], mem[registers.regs[sp] + 1])) & 0xffff);       
        registers.regs[sp] += 2;
        break;
      }
      case INT:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg destination_reg = ((regsDescr & DESTINATION_REG_MASK) >> 4);
        if(destination_reg > 7 || destination_reg < 0){
          //greska nije dobar registar
        }

        if((regsDescr & SOURCE_REG_MASK) != 0xf){
          //greska nije dobar format instrukcije
        }

         //TODO da li treba da pusuje PC?
         int pcValue = registers.regs[pc];
        mem[--registers.regs[sp]] = (((pcValue & HIGHER_BYTE) >>8) & 0xff);
        mem[--registers.regs[sp]] = pcValue & LOWER_BYTE;
        
       
        int pswValue = registers.psw;
        mem[--registers.regs[sp]] = (((pswValue & HIGHER_BYTE) >>8) & 0xff); 
        mem[--registers.regs[sp]] = pswValue & LOWER_BYTE;
        

        registers.regs[pc] = toLittleEndian(mem[(registers.regs[destination_reg] % 8)*2], mem[(registers.regs[destination_reg] % 8)*2 + 1]);
        break;
      }
      case PUSH_OR_STR:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;
        int vall = regsDescr;
        reg destination_reg = (vall & DESTINATION_REG_MASK) >> 4;
        if(destination_reg > 7 || destination_reg < 0){
          //greska nije dobar registar
        }

        addrMode = mem[registers.regs[pc]];
        registers.regs[pc] += 1;


        if(addrMode == 0x12){
 
          //PUSH OPERACIJA
          mem[--registers.regs[sp]] = (((registers.regs[destination_reg] & HIGHER_BYTE) >> 8) & 0xff); 
          mem[--registers.regs[sp]] = (registers.regs[destination_reg] & LOWER_BYTE);        
        
        
        
        
        }
        else{
          //STR instruction

            if(addrMode == PC_RELATIVE_OR_REG_IND_DISP){
              
              if((regsDescr & 0xf) == 0x7){
                //PC RELATIVNO
                reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
                registers.regs[pc] += 2;

                mem[registers.regs[pc] + payload] = registers.regs[destination_reg] & LOWER_BYTE;
                mem[registers.regs[pc] + payload + 1] = (registers.regs[destination_reg] & HIGHER_BYTE) >> 8;

                //na lokaciji memorije na adresi pc+pomeraj smestam Dreg
              }
              else{
                //REG_IND_DISPLACEMENT
                reg source_register_num = regsDescr & SOURCE_REG_MASK;
                reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
                registers.regs[pc] += 2;


                int helper = registers.regs[destination_reg];
                mem[registers.regs[source_register_num] + payload] = helper & LOWER_BYTE;
                mem[registers.regs[source_register_num] + payload + 1] = ((helper & HIGHER_BYTE) >> 8);
              }
            }
            else if(addrMode == MEM_DIR){
              reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
              registers.regs[pc] += 2;
              
              int helper = registers.regs[destination_reg];

              mem[payload] = helper & LOWER_BYTE;
              mem[payload + 1] = ((helper & HIGHER_BYTE) >> 8);
              
            }
            else if(addrMode == REG_DIR){
              reg source_register_num = regsDescr & SOURCE_REG_MASK;
              registers.regs[source_register_num] = registers.regs[destination_reg];
            }
            else if(addrMode == REG_IND){
              reg source_register_num = regsDescr & SOURCE_REG_MASK;
              //source je operand
              //smestamo D na operand
              mem[registers.regs[source_register_num]] = registers.regs[destination_reg] & LOWER_BYTE;
              mem[registers.regs[source_register_num] + 1] = (registers.regs[destination_reg] & HIGHER_BYTE) >> 8;
            }
            else{
              //greska
            }
        }
        break;
      }
      case POP_OR_LDR:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;
        reg destination_reg = (regsDescr & DESTINATION_REG_MASK) >> 4;
        if(destination_reg > 7 || destination_reg < 0){
          //greska nije dobar registar
        }

        addrMode = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        if(addrMode == 0x42){

          //POP OPERACIJA
          registers.regs[destination_reg] = toLittleEndian(mem[registers.regs[sp]], mem[registers.regs[sp] + 1]);       
          registers.regs[sp] += 2;       
        }
        else{
          //LDR
          
            if(addrMode == IMMEDIATE) {
              registers.regs[destination_reg] = (mem[registers.regs[pc] + 1] << 8) |  mem[registers.regs[pc]];
              registers.regs[pc] += 2;
            }
            else if(addrMode == PC_RELATIVE_OR_REG_IND_DISP){
              
              if((regsDescr & 0xf) == 0x7){
                //PC RELATIVNO
                registers.regs[destination_reg] = (mem[registers.regs[pc] + 1] << 8) |  mem[registers.regs[pc]];
                registers.regs[pc] += 2;
                registers.regs[destination_reg] += registers.regs[pc];
              }
              else{
                //REG_IND_DISPLACEMENT
                reg source_register_num = regsDescr & SOURCE_REG_MASK;
                reg payload = (mem[registers.regs[pc] + 1] << 8) |  mem[registers.regs[pc]];
                registers.regs[pc] += 2;
                registers.regs[destination_reg] = toLittleEndian(mem[registers.regs[source_register_num] + payload], mem[registers.regs[source_register_num] + payload + 1]);
              }
            }
            else if(addrMode == MEM_DIR){
              reg payload = (mem[registers.regs[pc] + 1] << 8) |  mem[registers.regs[pc]];;
              registers.regs[pc] += 2;
              registers.regs[destination_reg] = toLittleEndian(mem[payload], mem[payload + 1]); 
            }
            else if(addrMode == REG_DIR){
              reg source_register_num = regsDescr & SOURCE_REG_MASK;
              registers.regs[destination_reg] = registers.regs[source_register_num];
            }
            else if(addrMode == REG_IND){
              //ove varijante kod regind nemaju u asembleru nikako
              reg source_register_num = regsDescr & SOURCE_REG_MASK;
              registers.regs[destination_reg] = toLittleEndian(mem[registers.regs[source_register_num]], mem[registers.regs[source_register_num] + 1]);
            }
            else{
              //greska
            }

        }
        break;
      }
      case NOT:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg destination_reg = (regsDescr & DESTINATION_REG_MASK) >> 4;
        if(destination_reg > 7 || destination_reg < 0){
          //greska nije dobar registar
        }

        if((regsDescr & SOURCE_REG_MASK) != 0){
          //greska nije dobar format instrukcije
        }

        registers.regs[destination_reg] = ~registers.regs[destination_reg];
        break;
      }
      case XCHG:{

        //TODO treba vrv maskirati prekide, da je ne bi prekinuo posto je atomicna
        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;


        reg source_reg = regsDescr & SOURCE_REG_MASK;
        reg destination_reg = (regsDescr & DESTINATION_REG_MASK) >> 4;
        

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }

        reg temp = registers.regs[destination_reg];
        registers.regs[destination_reg] = registers.regs[source_reg];
        registers.regs[source_reg] = temp; 
        break;
      }
      case ADD:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg source_reg = regsDescr & SOURCE_REG_MASK;
        reg destination_reg = (regsDescr & DESTINATION_REG_MASK) >> 4;
        

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }

        registers.regs[destination_reg] += registers.regs[source_reg];
        break;
      }
      case SUB:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg destination_reg = (regsDescr & DESTINATION_REG_MASK) >> 4;
        reg source_reg = regsDescr & SOURCE_REG_MASK;

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }
                
        registers.regs[destination_reg] -= registers.regs[source_reg];

        break;
      }
      case MUL:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg source_reg = regsDescr & SOURCE_REG_MASK;
        reg destination_reg = ((regsDescr & DESTINATION_REG_MASK) >> 4);
        

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }
                
        registers.regs[destination_reg] *= registers.regs[source_reg];

        break;
      }
      case DIV:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg source_reg = regsDescr & SOURCE_REG_MASK;
        reg destination_reg = ((regsDescr & DESTINATION_REG_MASK) >> 4);
        

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }
                
        registers.regs[destination_reg] /= registers.regs[source_reg];
        break;
      }
      case CMP:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg source_reg = regsDescr & SOURCE_REG_MASK;
        reg destination_reg = (regsDescr & DESTINATION_REG_MASK) >> 4;
        

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }
                
        reg temp = registers.regs[destination_reg] - registers.regs[source_reg];
        
        if(temp == 0){
          registers.psw = registers.psw  | Z_MASK;
          registers.psw = registers.psw & ~N_MASK;
        }
        else if(temp < 0){
          registers.psw = registers.psw & ~Z_MASK;
          registers.psw = registers.psw | N_MASK;
        }
        else{
          registers.psw = registers.psw & ~Z_MASK;
          registers.psw = registers.psw & ~N_MASK;
        }


        if (registers.regs[source_reg] > registers.regs[destination_reg]) {
          registers.psw = registers.psw | C_MASK;
        }
        else{
          registers.psw = registers.psw & ~C_MASK;
        }

        if(((registers.regs[destination_reg] > 0) && (registers.regs[source_reg]) < 0 && (registers.regs[destination_reg] - registers.regs[source_reg] < 0))
        || ((registers.regs[destination_reg] < 0) && (registers.regs[source_reg]) > 0 && (registers.regs[destination_reg] - registers.regs[source_reg] > 0))){
          registers.psw = registers.psw | O_MASK;
        }
        else{
          registers.psw = registers.psw & ~O_MASK;
        }

        break;
      }
      case AND:{


        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg source_reg = regsDescr & SOURCE_REG_MASK;
        reg destination_reg = (regsDescr & DESTINATION_REG_MASK) >> 4;
        

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }
                
        registers.regs[destination_reg] = registers.regs[destination_reg] & registers.regs[source_reg];
        break;
      }
      case OR:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg source_reg = regsDescr & SOURCE_REG_MASK;
        reg destination_reg = ((regsDescr & DESTINATION_REG_MASK) >> 4);
        

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }
                
        registers.regs[destination_reg] = registers.regs[destination_reg] | registers.regs[source_reg];

        break;
      }
      case XOR:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg source_reg = regsDescr & SOURCE_REG_MASK;
        reg destination_reg = ((regsDescr & DESTINATION_REG_MASK) >> 4);
        

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }
                
        registers.regs[destination_reg] = registers.regs[destination_reg] ^ registers.regs[source_reg];

        break;
      }
      case TEST:{

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg source_reg = regsDescr & SOURCE_REG_MASK;
        reg destination_reg = (regsDescr & DESTINATION_REG_MASK) >> 4;
        

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }
                
        reg temp = registers.regs[destination_reg] & registers.regs[source_reg];
        
        if(temp == 0){
          registers.psw = registers.psw  | Z_MASK;
          registers.psw = registers.psw & ~N_MASK;
        }
        else if(temp < 0){
          registers.psw = registers.psw & ~Z_MASK;
          registers.psw = registers.psw | N_MASK;
        }
        else{
          registers.psw = registers.psw & ~Z_MASK;
          registers.psw = registers.psw & ~N_MASK;
        }

        break;
      }
      case SHL:{


        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg source_reg = regsDescr & SOURCE_REG_MASK;
        reg destination_reg = (regsDescr & DESTINATION_REG_MASK) >> 4;
        

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }
                
        reg temp = (reg)(registers.regs[destination_reg] << registers.regs[source_reg]);
        
        //Z i N
        if(temp == 0){
          registers.psw = registers.psw  | Z_MASK;
          registers.psw = registers.psw & ~N_MASK;
        }
        else if(temp < 0){
          registers.psw = registers.psw & ~Z_MASK;
          registers.psw = registers.psw | N_MASK;
        }
        else{
          registers.psw = registers.psw & ~Z_MASK;
          registers.psw = registers.psw & ~N_MASK;
        }

        //C
        if(temp & (1 << 16)){
          registers.psw = registers.psw | C_MASK;
        }
        else{
          registers.psw = registers.psw & ~C_MASK;
        }

        registers.regs[destination_reg] = (reg)temp;

        break;
      }
      case SHR:{


        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;

        reg source_reg = regsDescr & SOURCE_REG_MASK;
        reg destination_reg = (regsDescr & DESTINATION_REG_MASK) >> 4;
        

        if(destination_reg > 7 || destination_reg < 0 || source_reg > 7 || source_reg < 0){
          //greska nije dobar registar
        }
                
        reg temp = registers.regs[destination_reg] >> registers.regs[source_reg];
        
        //Z i N
        if(temp == 0){
          registers.psw = registers.psw  | Z_MASK;
          registers.psw = registers.psw & ~N_MASK;
        }
        else if(temp < 0){
          registers.psw = registers.psw & ~Z_MASK;
          registers.psw = registers.psw | N_MASK;
        }
        else{
          registers.psw = registers.psw & ~Z_MASK;
          registers.psw = registers.psw & ~N_MASK;
        }

        //C
        if(registers.regs[destination_reg] & (0x1 << (registers.regs[source_reg] - 1))){
          registers.psw = registers.psw | C_MASK;
        }
        else{
          registers.psw = registers.psw & ~C_MASK;
        }

        registers.regs[destination_reg] = (reg)temp;

        break;
      }
      case CALL:{



        //pusuje PC
        //PC <= operand

        reg callingValue = registers.regs[pc];
           

        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;


        reg source_register_num = (regsDescr & SOURCE_REG_MASK);
        
        if(source_register_num > 7 || source_register_num < 0){
          //greska nije dobar registar
        }

        addrMode = mem[registers.regs[pc]];
        registers.regs[pc] += 1;



            //pusujem tacan pc, tj onaj nakon instrukcije
           if(addrMode == IMMEDIATE) {
              callingValue +=4;
            }
            else if(addrMode == PC_RELATIVE_OR_REG_IND_DISP){ // REGISTARSKO INDIREKTNO SAMO
              callingValue +=4;
            }
            else if(addrMode == PC_REL_ADDR){
              callingValue +=4;
            }
            else if(addrMode == MEM_DIR){
              callingValue +=4;
            }
            else if(addrMode == REG_DIR){
              callingValue +=2;
            }
            else if(addrMode == REG_IND){
              callingValue +=2;
            }            
            
            mem[--registers.regs[sp]] = (((uint16_t)(callingValue & HIGHER_BYTE) >> 8) & 0xff);
            mem[--registers.regs[sp]] = callingValue & LOWER_BYTE;
             
        
          if(addrMode == IMMEDIATE) {
            registers.regs[pc] = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
          }

          else if(addrMode == PC_RELATIVE_OR_REG_IND_DISP){ // REGISTARSKO INDIREKTNO SAMO

              //REG_IND_DISPLACEMENT
              reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
              registers.regs[pc] += 2;
              registers.regs[pc] = toLittleEndian(mem[registers.regs[source_register_num] + payload], mem[registers.regs[source_register_num] + payload + 1]);
          }

          else if(addrMode == PC_REL_ADDR){
            //PC RELATIVNO
              reg temp = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
              registers.regs[pc] += 2;
              registers.regs[pc] += temp;
          }

          else if(addrMode == MEM_DIR){
            reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
            registers.regs[pc] += 2;
            registers.regs[pc] = toLittleEndian(mem[payload], mem[payload + 1]);
          }
          else if(addrMode == REG_DIR){
            registers.regs[pc] = registers.regs[source_register_num];
          }
          else if(addrMode == REG_IND){
            //ove varijante kod regind nemaju u asembleru nikako
            registers.regs[pc] = toLittleEndian(mem[registers.regs[source_register_num]], mem[registers.regs[source_register_num] + 1]);
          }
          else {
            //greska
          }
        
          
        break;
      }
      case JMP:{


        regsDescr = mem[registers.regs[pc]];
        registers.regs[pc] += 1;


        reg source_register_num = (regsDescr & SOURCE_REG_MASK);
        
        if(source_register_num > 7 || source_register_num < 0){
          //greska nije dobar registar
        }

        addrMode = mem[registers.regs[pc]];
        registers.regs[pc] += 1;


       

          
          if(addrMode == IMMEDIATE) {
            registers.regs[pc] = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
          }

          else if(addrMode == PC_RELATIVE_OR_REG_IND_DISP){ // REGISTARSKO INDIREKTNO SAMO

              //REG_IND_DISPLACEMENT
              reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
              registers.regs[pc] += 2;
              registers.regs[pc] = toLittleEndian(mem[registers.regs[source_register_num] + payload], mem[registers.regs[source_register_num] + payload + 1]);
          }

          else if(addrMode == PC_REL_ADDR){
            //PC RELATIVNO
              reg temp = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
              registers.regs[pc] += 2;
              registers.regs[pc] += temp;
          }

          else if(addrMode == MEM_DIR){
            reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
            registers.regs[pc] += 2;
            registers.regs[pc] = toLittleEndian(mem[payload], mem[payload + 1]); 
          }
          else if(addrMode == REG_DIR){
            registers.regs[pc] = registers.regs[source_register_num];
          }
          else if(addrMode == REG_IND){
            //ove varijante kod regind nemaju u asembleru nikako
            registers.regs[pc] = toLittleEndian(mem[registers.regs[source_register_num]], mem[registers.regs[source_register_num] + 1]);
          }

        break;
      }
      case JEQ:{

        
        if(registers.psw & Z_MASK == 1){
          regsDescr = mem[registers.regs[pc]];
          registers.regs[pc] += 1;


          reg source_register_num = regsDescr & SOURCE_REG_MASK;
          
          if(source_register_num > 7 || source_register_num < 0){
            //greska nije dobar registar
          }

          addrMode = mem[registers.regs[pc]];
          registers.regs[pc] += 1;


            if(addrMode == IMMEDIATE) {
              registers.regs[pc] = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
            }

            else if(addrMode == PC_RELATIVE_OR_REG_IND_DISP){ // REGISTARSKO INDIREKTNO SAMO

                //REG_IND_DISPLACEMENT
                reg payload = (mem[registers.regs[pc] + 1] << 8) |  mem[registers.regs[pc]];
                registers.regs[pc] += 2;
                registers.regs[pc] = toLittleEndian(mem[registers.regs[source_register_num] + payload], mem[registers.regs[source_register_num] + payload + 1]);
            }

            else if(addrMode == PC_REL_ADDR){
              //PC RELATIVNO
                reg temp = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
                registers.regs[pc] += 2;
                registers.regs[pc] += temp;
            }

            else if(addrMode == MEM_DIR){
              reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
              registers.regs[pc] += 2;
              registers.regs[pc] = toLittleEndian(mem[payload], mem[payload + 1]); 
            }
            else if(addrMode == REG_DIR){
              registers.regs[pc] = registers.regs[source_register_num];
            }
            else if(addrMode == REG_IND){
              //ove varijante kod regind nemaju u asembleru nikako
              registers.regs[pc] = toLittleEndian(mem[registers.regs[source_register_num]], mem[registers.regs[source_register_num] + 1]);
            }
            else{
              //greska
            }
        }
        else{ //Ako se ne desi skok samo se azurira PC
          if(addrMode == IMMEDIATE) {
            registers.regs[pc] +=4;
          }
          else if(addrMode == PC_RELATIVE_OR_REG_IND_DISP){ // REGISTARSKO INDIREKTNO SAMO
            registers.regs[pc] +=4;
          }
          else if(addrMode == PC_REL_ADDR){
            registers.regs[pc] +=4;
          }
          else if(addrMode == MEM_DIR){
            registers.regs[pc] +=4;
          }
          else if(addrMode == REG_DIR){
            registers.regs[pc] +=2;
          }
          else if(addrMode == REG_IND){
            registers.regs[pc] +=2;
          }
        }
        
        break;
      }
      case JNE:{


        if(!(registers.psw & Z_MASK == 1)){
          regsDescr = mem[registers.regs[pc]];
          registers.regs[pc] += 1;


          reg source_register_num = regsDescr & SOURCE_REG_MASK;
          
          if(source_register_num > 7 || source_register_num < 0){
            //greska nije dobar registar
          }

          addrMode = mem[registers.regs[pc]];
          registers.regs[pc] += 1;


          
            if(addrMode == IMMEDIATE) {
              registers.regs[pc] = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
            }

            else if(addrMode == PC_RELATIVE_OR_REG_IND_DISP){ // REGISTARSKO INDIREKTNO SAMO

                //REG_IND_DISPLACEMENT
                reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
                registers.regs[pc] += 2;
                registers.regs[pc] = toLittleEndian(mem[registers.regs[source_register_num] + payload], mem[registers.regs[source_register_num] + payload + 1]);
            }

            else if(addrMode == PC_REL_ADDR){
              //PC RELATIVNO
                reg temp = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
                registers.regs[pc] += 2;
                registers.regs[pc] += temp;
            }

            else if(addrMode ==  MEM_DIR){
              reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
              registers.regs[pc] += 2;
              registers.regs[pc] = toLittleEndian(mem[payload], mem[payload + 1]); 
            }
            else if(addrMode == REG_DIR){
              registers.regs[pc] = registers.regs[source_register_num];
            }
            else if(addrMode == REG_IND){
              //ove varijante kod regind nemaju u asembleru nikako
              registers.regs[pc] = toLittleEndian(mem[registers.regs[source_register_num]], mem[registers.regs[source_register_num] + 1]);
            }
            else{
              //greska
            }
          

        }
        else{ //Ako se ne desi skok samo se azurira PC
            if(addrMode == IMMEDIATE) {
              registers.regs[pc] +=4;
            }
            else if(addrMode == PC_RELATIVE_OR_REG_IND_DISP){ // REGISTARSKO INDIREKTNO SAMO
              registers.regs[pc] +=4;
            }
            else if(addrMode == PC_REL_ADDR){
              registers.regs[pc] +=4;
            }
            else if(addrMode == MEM_DIR){
              registers.regs[pc] +=4;
            }
            else if(addrMode == REG_DIR){
              registers.regs[pc] +=2;
            }
            else if(addrMode == REG_IND){
              registers.regs[pc] +=2;
            }
        }

        break;
      }
      case JGT:{

        //N i Z
        //Uslov za vece od
        if(((registers.psw & N_MASK) == 0) && ((registers.psw & Z_MASK) == 0)){

          regsDescr = mem[registers.regs[pc]];
          registers.regs[pc] += 1;


          reg source_register_num = regsDescr & SOURCE_REG_MASK;
          
          if(source_register_num > 7 || source_register_num < 0){
            //greska nije dobar registar
          }

          addrMode = mem[registers.regs[pc]];
          registers.regs[pc] += 1;


          
            if(addrMode == IMMEDIATE) {
              registers.regs[pc] = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
            }

            else if(addrMode == PC_RELATIVE_OR_REG_IND_DISP){ // REGISTARSKO INDIREKTNO SAMO

                //REG_IND_DISPLACEMENT
                reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
                registers.regs[pc] += 2;
                registers.regs[pc] = toLittleEndian(mem[registers.regs[source_register_num] + payload], mem[registers.regs[source_register_num] + payload + 1]);
            }

            else if(addrMode == PC_REL_ADDR){
              //PC RELATIVNO
                reg temp = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
                registers.regs[pc] += 2;
                registers.regs[pc] += temp;
            }

            else if(addrMode == MEM_DIR){
              reg payload = toLittleEndian(mem[registers.regs[pc]], mem[registers.regs[pc] + 1]);
              registers.regs[pc] += 2;
              registers.regs[pc] = toLittleEndian(mem[payload], mem[payload + 1]); 
            }
            else if(addrMode == REG_DIR){
              registers.regs[pc] = registers.regs[source_register_num];
            }
            else if(addrMode == REG_IND){
              //ove varijante kod regind nemaju u asembleru nikako
              registers.regs[pc] = toLittleEndian(mem[registers.regs[source_register_num]], mem[registers.regs[source_register_num] + 1]);
            }
            else{
              //greska
            }
          
          
        }
        else{ //Ako se ne desi skok samo se azurira PC
            if(addrMode == IMMEDIATE) {
              registers.regs[pc] +=4;
            }
            else if(addrMode == PC_RELATIVE_OR_REG_IND_DISP){ // REGISTARSKO INDIREKTNO SAMO
              registers.regs[pc] +=4;
            }
            else if(addrMode == PC_REL_ADDR){
              registers.regs[pc] +=4;
            }
            else if(addrMode == MEM_DIR){
              registers.regs[pc] +=4;
            }
            else if(addrMode == REG_DIR){
              registers.regs[pc] +=2;
            }
            else if(addrMode == REG_IND){
              registers.regs[pc] +=2;
            }
        }
        break;        
      }

    }
  }

  readState(registers);
  return 0;

}