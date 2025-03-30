//
// Created by Albin Horlaville on 28/03/2025.
//

#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include <iostream>

class Serializable {
  public:
    virtual void serialize(std::ostream& ostr) const = 0;
    virtual void unserialize(std::istream& istr) const = 0;
    virtual ~Serializable() = default;
};

#endif //SERIALIZABLE_H
