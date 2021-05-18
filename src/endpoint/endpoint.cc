#include "endpoint.h"


Endpoint::Endpoint(const SharedEvent event): event_(event) { }

Endpoint::~Endpoint() { }

SharedEvent Endpoint::inner_event() const
{
    return event_;
}