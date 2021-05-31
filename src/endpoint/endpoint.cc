#include "endpoint.h"


Endpoint::Endpoint(Event* event): event_(event) { }

Endpoint::~Endpoint() { }

const Event* Endpoint::inner_event() const
{
    return event_;
}