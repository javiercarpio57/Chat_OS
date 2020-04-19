
// string::copy
#include <iostream>
#include <stdio.h>
#include <string.h>

void sendBySocket (std::string msg, int sock) {
  char buffer[8096];
  size_t lengthStr = msg.length();
  std::cout << "original size: " << lengthStr << '\n';
  strcpy(buffer, msg.c_str());
  //std::size_t length = msg.copy(buffer,lengthStr, 0);
  /*
  for (int i = 0 ; i < lengthStr; i++){
    buffer[i] = msg[i];
  }*/
  //buffer[lengthStr]='\0';
  std::cout << "buffer contains: " << buffer << '\n';
  std::cout << "buffer size: " << (unsigned)strlen(buffer) << '\n';
}

int main ()
{
    std::string str ("Test string.alksjdnfioansdin   asodfnosndlnasff  asodf..");
    sendBySocket(str, 0);
}
/*
#include <stdio.h>
#include <string.h>

int main ()
{
  char szInput[256];
  printf ("Enter a sentence: ");
  //gets (szInput);
  printf ("The sentence entered is %u characters long.\n",(unsigned)strlen(szInput));
  return 0;
}*/