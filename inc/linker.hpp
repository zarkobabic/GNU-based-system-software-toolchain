#ifndef _LINKER__STRUCTS__
#define _LINKER__STRUCTS__

#include <iostream>
#include <string>
using namespace std;
#include <vector>




struct LinkerSimbolTableEntry{
  int index;
  static int indexID;
  short value;
  int type; //-1 -sekcija 
  int secSize;
  int newSection = -1;
  vector<char>* bytes; //sadrzaj sekcije

  //treba sam napraviti za sekcije value
  LinkerSimbolTableEntry(short valuee, int typee, int size){
    index = ++indexID;
    value = valuee;
    type = typee;
    secSize = size;
    bytes = nullptr;
  }

  LinkerSimbolTableEntry(short valuee, int typee, int size, int newSectionn){
    index = ++indexID;
    value = valuee;
    type = typee;
    secSize = size;
    bytes = nullptr;
    newSection = newSectionn;
  }
  
  // void print(){
  //   cout << index << " " << whatSection << " " << value << " " << type << " " << defined << " "<< endl;
  // }
};

struct FilesSectionsEntry{
  string name;
  int indexOld;
  int fromFile;
  int secSize;
  int start_addr;

  FilesSectionsEntry(string namee, int indexOldd, int fromFilee, int sizee, int start_addrr){
    name = namee;
    indexOld = indexOldd;
    fromFile = fromFilee;
    secSize = sizee;
    start_addr = start_addrr;
  }
};






#endif
