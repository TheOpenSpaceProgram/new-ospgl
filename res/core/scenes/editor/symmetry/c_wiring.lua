-- Auto-wiring for symmetric groups
-- Default behaviour, appropiate for most symmetry modes

local wiring = {}

-- Happens when a machine outside any symmetry group is wired to one
-- in a symmetry group, or a machine in a symmetry group is wired to one outside
wiring.outside_symmetry = {
	disabled = 0, -- No mirroring is done
	mirror = 1 -- All machines in the symmetry group get wired to target
}

-- Happens when a machine within the symmetry group is wired to one in the same group
wiring.same_symmetry = {
	disabled = 0, -- No miroring is done
	cyclic = 1, -- Machines are wired in the order of the symmetry array
	exhaustive = 2 -- Machines are wired in all possible combinations exhaustively
}

return wiring
