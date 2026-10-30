#pragma once
namespace xv::cli
{
// Positive decimal seconds with at most three fractional digits, up to one hour.
inline bool duration_ms(const char *text, unsigned &out)
{
  unsigned whole=0, fraction=0, digits=0; bool any=false;
  while (*text>='0' && *text<='9')
  { any=true; if(whole>3600)return false;whole=whole*10+unsigned(*text++-'0'); }
  if(*text=='.')
  {
    ++text;
    while(*text>='0' && *text<='9')
    { any=true;if(++digits>3)return false;fraction=fraction*10+unsigned(*text++-'0'); }
    if(!digits)return false;
  }
  if(*text || !any || whole>3600 || (whole==3600 && fraction))return false;
  while(digits++<3)fraction*=10;
  out=whole*1000+fraction;return out!=0;
}
}
