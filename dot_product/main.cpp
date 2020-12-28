#include <iostream>
#include "xbyak/xbyak.h"
#include "xbyak/xbyak_util.h"


struct Code : Xbyak::CodeGenerator {

   Code()
   {

     ret();

   }


};



int main(int argc, char **argv) {



     Code c;
     c.getCode();




  return 0;
}
