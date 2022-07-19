#include "domain.h"

bool transport::Stop::operator==(const Stop &other) const {
  return name == other.name;
}

bool transport::Bus::operator==(const Bus &other) const {
  return name == other.name;
}
