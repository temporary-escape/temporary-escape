#include "FixtureSimpleGalaxy.hpp"
#include <TemporaryEscape/Utils/Random.hpp>

void FixtureSimpleGalaxy::generateGalaxy() {
    auto& services = server->getServices();

    GalaxyData galaxy{};
    galaxy.id = uuid();
    galaxy.name = "Test Galaxy";
    galaxy.seed = 123456789ULL;

    services.galaxies.createGalaxy(galaxy);

    RegionData region{};
    region.name = "Test Region";
    region.seed = 123456789ULL;
    region.id = uuid();
    region.galaxyId = galaxy.id;

    services.regions.createRegion(region);

    SystemData system{};
    system.galaxyId = galaxy.id;
    system.regionId = region.id;
    system.id = uuid();
    system.name = "Test System";
    system.seed = 123456789ULL;

    services.systems.createSystem(system);

    SectorData sector{};
    sector.galaxyId = galaxy.id;
    sector.systemId = system.id;
    sector.seed = 123456789ULL;
    sector.name = "Test Sector";
    sector.generated = true;
    sector.id = uuid();

    services.sectors.createSector(sector);
}
