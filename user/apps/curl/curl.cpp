#include "kernel/support/types.h"
#include "kernel/fs/fcntl.h"
#include "user/runtime/user.h"
#include "user/runtime/http/http.h"
#include "user/runtime/http/codec.h"
#include "user/runtime/parse.h"
namespace
{
class FileSink final : public xv::http::Sink
{
  int fd_;
  char *buffer_;
  unsigned used_=0;
  unsigned long writes_=0;
  bool failed_=false, buffered_;
  bool write_all(const char *data,unsigned size)
  {
    if(failed_)return false;
    while(size)
    {
      ++writes_;int n=write(fd_,data,int(size));
      if(n<=0) { failed_=true;return false; }
      data+=n;size-=unsigned(n);
    }
    return true;
  }
  bool flush()
  { unsigned count=used_;used_=0;return !count || write_all(buffer_,count); }
  bool append(const char *data,unsigned size)
  {
    if(failed_)return false;
    if(!buffered_)return write_all(data,size);
    while(size)
    {
      if(!used_ && size>=Capacity)return write_all(data,size);
      unsigned take=Capacity-used_;if(take>size)take=size;
      memmove(buffer_+used_,data,int(take));used_+=take;data+=take;size-=take;
      if(used_==Capacity && !flush())return false;
    }
    return true;
  }
public:
  static constexpr unsigned Capacity=8192;
  explicit FileSink(int fd,bool buffered) : fd_(fd),buffer_(buffered ? static_cast<char *>(malloc(Capacity)) : nullptr),buffered_(buffered) {}
  bool ready() const { return !buffered_ || buffer_; }
  bool headers(const char *data,unsigned size) override { return append(data,size); }
  bool body(const char *data,unsigned size) override { return append(data,size); }
  bool finish() { bool ok=flush() && !failed_;if(buffer_)free(buffer_);buffer_=nullptr;return ok; }
  unsigned long writes() const { return writes_; }
};
void usage()
{ fprintf(2,"usage: curl [-iILfNsS] [-o FILE] [-m SECONDS] [--connect-timeout SECONDS] [-w FORMAT] HTTP-URL\n"); }
bool field(const char *begin,unsigned size,const char *name)
{ return strlen(name)==size && !memcmp(begin,name,size); }
bool known(const char *name,unsigned size)
{
  static constexpr const char *fields[] = {"http_code","size_download","time_total","time_namelookup","time_connect","time_starttransfer","speed_download","num_connects","output_writes"};
  for(const char *option : fields)
    if(field(name,size,option))return true;
  return false;
}
bool valid_format(const char *s)
{
  while(*s)
  {
    if(*s++!='%')continue;
    if(*s=='%') { ++s;continue; }
    if(*s++!='{')return false;
    const char *begin=s;while(*s && *s!='}')++s;
    if(!*s || !known(begin,unsigned(s-begin)))return false;
    ++s;
  }
  return true;
}
void time_value(unsigned long long us)
{
  char digits[7];digits[6]=0;
  for(unsigned i=0;i<6;++i) { digits[5-i]=char('0'+us%10);us/=10; }
  printf("%llu.%s",us,digits);
}
void write_out(const char *s,const xv::http::Response &r,unsigned long writes)
{
  while(*s)
  {
    if(*s=='\\' && s[1])
    { ++s;char c=*s++;if(c=='n')c='\n';else if(c=='t')c='\t';else if(c=='r')c='\r';printf("%c",c);continue; }
    if(*s!='%') { printf("%c",*s++);continue; }
    ++s;if(*s=='%') { ++s;printf("%%");continue; }
    ++s;const char *begin=s;while(*s!='}')++s;unsigned n=unsigned(s-begin);++s;
    if(field(begin,n,"http_code"))printf("%c%c%c",char('0'+r.status/100%10),char('0'+r.status/10%10),char('0'+r.status%10));
    else if(field(begin,n,"size_download"))printf("%llu",r.body_bytes);
    else if(field(begin,n,"time_total"))time_value(r.total_us);
    else if(field(begin,n,"time_namelookup"))time_value(r.namelookup_us);
    else if(field(begin,n,"time_connect"))time_value(r.connect_us);
    else if(field(begin,n,"time_starttransfer"))time_value(r.first_byte_us);
    else if(field(begin,n,"speed_download"))printf("%llu",r.total_us ? (r.body_bytes/r.total_us)*1000000+(r.body_bytes%r.total_us)*1000000/r.total_us : 0);
    else if(field(begin,n,"num_connects"))printf("%u",r.connects);
    else if(field(begin,n,"output_writes"))printf("%lu",writes);
  }
}
}
extern "C" int main(int argc,char **argv)
{
  xv::http::Options options;const char *url=nullptr,*output=nullptr,*format=nullptr;
  bool positional=false,buffered=true,silent=false,show_error=false;
  for(int i=1;i<argc;++i)
  {
    const char *arg=argv[i];
    if(!positional && !strcmp(arg,"--")) { positional=true;continue; }
    if(!positional && (!strcmp(arg,"--help") || !strcmp(arg,"-h"))) { usage();exit(0); }
    if(!positional && (!strcmp(arg,"-o") || !strcmp(arg,"--output")))
    { if(++i==argc || output) { usage();exit(2); }output=argv[i];continue; }
    if(!positional && (!strcmp(arg,"-w") || !strcmp(arg,"--write-out")))
    { if(++i==argc || !valid_format(argv[i])) { usage();exit(2); }format=argv[i];continue; }
    if(!positional && (!strcmp(arg,"-m") || !strcmp(arg,"--max-time") || !strcmp(arg,"--connect-timeout")))
    {
      unsigned value;
      if(++i==argc || !xv::cli::duration_ms(argv[i],value)) { usage();exit(2); }
      if(!strcmp(arg,"--connect-timeout"))options.connect_timeout_ms=value;else options.timeout_ms=value;
      continue;
    }
    if(!positional && arg[0]=='-')
    {
      if(!strcmp(arg,"--location")) { options.follow=true;continue; }
      if(!strcmp(arg,"--head")) { options.head=true;continue; }
      if(!strcmp(arg,"--include")) { options.include_headers=true;continue; }
      if(!strcmp(arg,"--fail")) { options.fail_http=true;continue; }
      if(!strcmp(arg,"--no-buffer")) { buffered=false;continue; }
      if(!strcmp(arg,"--silent")) { silent=true;continue; }
      if(!strcmp(arg,"--show-error")) { show_error=true;continue; }
      if(!arg[1]) { usage();exit(2); }
      for(unsigned j=1;arg[j];++j)
      {
        switch(arg[j])
        {
          case 'i':options.include_headers=true;break;
          case 'I':options.head=true;break;
          case 'L':options.follow=true;break;
          case 'f':options.fail_http=true;break;
          case 'N':buffered=false;break;
          case 's':silent=true;break;
          case 'S':show_error=true;break;
          default:usage();exit(2);
        }
      }
    }
    else { if(url) { usage();exit(2); }url=arg; }
  }
  if(!url) { usage();exit(2); }
  // Validate before opening/truncating output, without placing Url on the small user stack.
  auto *parsed=static_cast<xv::http::Url *>(malloc(sizeof(xv::http::Url)));
  if(!parsed)exit(xv::http::Memory);
  int status=xv::http::parse_url(url,*parsed);free(parsed);
  if(status) { if(!silent || show_error)fprintf(2,"curl: (%d) %s\n",status,xv::http::error_string(status));exit(status); }
  int fd=1;
  if(output && strcmp(output,"-"))
  { fd=open(output,O_WRONLY|O_CREATE|O_TRUNC);if(fd<0) { if(!silent || show_error)fprintf(2,"curl: cannot open %s\n",output);exit(23); } }
  FileSink sink(fd,buffered);xv::http::Client client;xv::http::Response response;
  if(!sink.ready()) { if(fd!=1)close(fd);if(!silent || show_error)fprintf(2,"curl: out of memory\n");exit(27); }
  auto started=clockticks();
  status=client.get(url,options,sink,response);
  if(!sink.finish())status=xv::http::Output;
  if(fd!=1 && close(fd) && !status)status=xv::http::Output;
  response.total_us=(clockticks()-started)/10;
  if(format)write_out(format,response,sink.writes());
  if(status && (!silent || show_error))fprintf(2,"curl: (%d) %s (HTTP %u)\n",status,xv::http::error_string(status),response.status);
  exit(status);
}
