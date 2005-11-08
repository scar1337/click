/*
 * settxrate.{cc,hh} -- sets wifi txrate annotation on a packet
 * John Bicket
 *
 * Copyright (c) 2003 Massachusetts Institute of Technology
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/glue.hh>
#include <click/packet_anno.hh>
#include "settxrate.hh"
#include <clicknet/ether.h>
#include <click/etheraddress.hh>
#include <clicknet/wifi.h>
CLICK_DECLS

SetTXRate::SetTXRate()
{
}

SetTXRate::~SetTXRate()
{
}

int
SetTXRate::configure(Vector<String> &conf, ErrorHandler *errh)
{
  _rate = 0;
  _et = 0;
  _offset = 0;
  if (cp_va_parse(conf, this, errh,
		  cpOptional,
		  cpUnsigned, "rate", &_rate, 
		  cpKeywords, 
		  "RATE", cpUnsigned, "rate", &_rate, 
		  "ETHTYPE", cpUnsignedShort, "Ethernet encapsulation type", &_et,
		  "OFFSET", cpUnsigned, "offset", &_offset,
		  cpEnd) < 0) {
    return -1;
  }

  if (_rate < 0) {
    return errh->error("RATE must be >= 0");
  }

  
  
  
  return 0;
}

Packet *
SetTXRate::simple_action(Packet *p_in)
{
  uint8_t *dst_ptr = (uint8_t *) p_in->data() + _offset;
  click_ether *eh = (click_ether *) dst_ptr;

  if (_et && eh->ether_type != htons(_et)) {
    return p_in;
  }

  struct click_wifi_extra *ceh = (struct click_wifi_extra *) p_in->all_user_anno();
  ceh->magic = WIFI_EXTRA_MAGIC;
  ceh->rate = _rate ? _rate : 2;
  ceh->max_tries = WIFI_MAX_RETRIES+1;

  return p_in;
}
enum {H_RATE};

String
SetTXRate::read_handler(Element *e, void *thunk)
{
  SetTXRate *foo = (SetTXRate *)e;
  switch((uintptr_t) thunk) {
  case H_RATE: return String(foo->_rate) + "\n";
  default:   return "\n";
  }
  
}

int
SetTXRate::write_handler(const String &arg, Element *e,
			 void *vparam, ErrorHandler *errh) 
{
  SetTXRate *f = (SetTXRate *) e;
  String s = cp_uncomment(arg);
  switch((int)vparam) {
  case H_RATE: {
    unsigned m;
    if (!cp_unsigned(s, &m)) 
      return errh->error("rate parameter must be unsigned");
    f->_rate = m;
    break;
  }
  }
  return 0;
}

void
SetTXRate::add_handlers()
{
  add_read_handler("rate", read_handler, (void *) H_RATE);
  add_write_handler("rate", write_handler, (void *) H_RATE);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(SetTXRate)

