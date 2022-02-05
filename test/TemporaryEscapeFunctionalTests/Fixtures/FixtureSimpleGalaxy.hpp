#include "FixtureClientServer.hpp"

class FixtureSimpleGalaxy: public FixtureClientServer {
public:
    FixtureSimpleGalaxy() = default;

    void generateGalaxy();
};
