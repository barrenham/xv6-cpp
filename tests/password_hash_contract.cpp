#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "user/runtime/password_hash.h"
static void check(const unsigned char *bytes,const char *expected)
{ char text[65]; password_hash::hex(bytes,32,text); if(std::strcmp(text,expected)) { std::fprintf(stderr,"hash mismatch: %s\n",text); std::exit(1); } }
int main()
{
  unsigned char result[32];
  { password_hash::Sha256 hash; const char *text=""; hash.add(text,0); hash.finish(result); check(result,"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"); }
  { password_hash::Sha256 hash; const char *text="abc"; hash.add(text,3); hash.finish(result); check(result,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"); }
  { password_hash::Sha256 hash; const char *text="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"; hash.add(text,55); hash.finish(result); check(result,"9f4390f8d30c2dd92ec9f095b65e2b9ae9b0a925a5258e241c9f1e910f734318"); }
  { password_hash::Sha256 hash; const char *text="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"; hash.add(text,56); hash.finish(result); check(result,"b35439a4ac6f0948b6d6f9e3c6af0f5f590ce20f1bde7090ef7970686ec6738a"); }
  { password_hash::Sha256 hash; const char *text="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"; hash.add(text,64); hash.finish(result); check(result,"ffe054fe7ae0cb6dc65c3af9b61d5209f439851db43d0ba5997337df154668eb"); }
  { password_hash::Sha256 hash; const char *text="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"; hash.add(text,100); hash.finish(result); check(result,"2816597888e4a0d3a36b82b83316ab32680eb8f00f8cd3b904d681246d285a0e"); }
  { unsigned char key[20]; for(auto &b:key) b=0x0b; password_hash::Hmac hash(key,20); hash.digest("Hi There",8,result); check(result,"b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"); }
  { unsigned char salt[16]={}; password_hash::derive("password",8,salt,1,result); check(result,"1fefe125ab13dd2c06db86711ec448e9490e6c73024d7d8d659126c6f9fadd68"); }
  { unsigned char salt[16]={}; password_hash::derive("password",8,salt,2,result); check(result,"9e41dea5935137cf748669cbf242b3ea049d20e0add3db231dabc9840bc58a6a"); }
  { unsigned char salt[16]={}; password_hash::derive("password",8,salt,100000,result); check(result,"251f8a288adbd397631627dbaf9fc2cf11bf027e4e36cc88ed51e5237b7e4a98"); }
  std::puts("password hash contracts: passed");
}
