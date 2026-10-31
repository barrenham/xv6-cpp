module xv.fs:snapshot_text;
import :buffer;

namespace xv::fs
{
// Shared, allocation-aware formatter for synthetic filesystem snapshots.
class SnapshotText final
{
  PageBuffer &buffer_;bool failed_=false;
public:
  explicit SnapshotText(PageBuffer &buffer):buffer_(buffer) {}
  void raw(const void *data,unsigned count){if(!buffer_.append(data,count))failed_=true;}
  void append(const char *text)
  {unsigned size=0;while(text[size])++size;raw(text,size);}
  void number(unsigned long value,unsigned radix=10)
  {
    char digits[65];unsigned count=0;
    do{digits[count++]="0123456789abcdef"[value%radix];value/=radix;}while(value);
    while(count){char digit=digits[--count];raw(&digit,1);}
  }
  void field(const char *name,unsigned long value,const char *unit="\n")
  {append(name);number(value);append(unit);}
  void hexadecimal(const char *name,unsigned long value)
  {append(name);append("0x");number(value,16);append("\n");}
  bool good() const {return !failed_;}
};
}
