/*
base class for PDUs

all PDUs start with the first ten bytes as specified:
 01 PDU type
 02 reserved
 3-6 PDU Length (unsigned)
 7-10 variable

 on some, 7-10 are split (7-8 as protocol version in Associate-RQ, for instance, 
 while associate-rj splits those four bytes differently).

 Also common to all the PDUs is their ability to read and write to a stream.

 So, let's just get them all bunched together into one (abstract) class, shall we?

  Why?
  1) so that the ULEvent can have the PDU stored in it, since the event takes PDUs and not
  other class structures (other class structures get converted into PDUs)
  2) to make reading PDUs in the event loop cleaner

  but! leave Mathieu's original classes untouched at this point (except for inheriting from this class)
  because those work, at least with c-echo.
*/



#ifndef BASEPDU_H
#define BASEPDU_H
#include <iostream>

namespace gdcm {
  namespace network {
    class BasePDU{
    public:

      virtual std::istream &Read(std::istream &is) = 0;
      virtual const std::ostream &Write(std::ostream &os) const = 0;
      
      virtual size_t Size() const = 0;
      virtual void Print(std::ostream &os) const = 0;
    };
  }
}

#endif //BASEPDU_H