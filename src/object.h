#ifndef TOG_OBJECT_H
#define TOG_OBJECT_H

#include <vector>

namespace tog {

class TogObject {
public:
    virtual const std::vector<unsigned char>& serialize() = 0;
    virtual ~TogObject() = default;
};

}  // namespace tog

#endif  // TOG_OBJECT_H