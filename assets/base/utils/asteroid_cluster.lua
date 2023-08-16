local engine = require("engine")

local AsteroidCluster = {}

function AsteroidCluster.new (seed, origin, radius)
    local inst = {}
    setmetatable(inst, { __index = AsteroidCluster })
    inst.rng = engine.MT19937.new(seed)
    inst.origin = origin
    inst.radius = radius
    inst.pos = nil
    inst.size = nil
    inst.orientation = nil
    inst.tester = engine.VolumeOccupancyTester.new()
    return inst
end

function AsteroidCluster.next (self, size)
    local tries = 25
    while tries > 0 do
        tries = tries - 1

        local x = self.rng:rand_real(0.0, 1.0) - 0.5
        local y = self.rng:rand_real(0.0, 1.0) - 0.5
        local z = self.rng:rand_real(0.0, 1.0) - 0.5

        local mag = math.sqrt(x * x + y * y + z * z)
        x = x / mag
        y = y / mag
        z = z / mag

        local d = self.rng:rand_real(0.0, self.radius)
        x = x * d
        y = y * d
        z = z * d

        self.pos = engine.Vector3.new(x, y, z)
        self.size = size
        self.orientation = self.rng:rand_quaternion()

        if self.tester:contact_test(self.pos, size + 1.0) == false then
            self.tester:add(self.pos, size + 1.0)
            return true
        end
    end

    return false
end

return AsteroidCluster
